#define NPY_NO_DEPRECATED_API NPY_1_7_API_VERSION
#define PY_SSIZE_T_CLEAN
#include <Python.h>
#include <numpy/arrayobject.h>

#include <pthread.h>
#include <stdbool.h>

#include "pm_decode.h"
#include "thread_utils.h"

/*Decode various types of CTB data using a pixel map. Works on single frames and
on stacks of frames*/
static PyObject *decode(PyObject *Py_UNUSED(self), PyObject *args,
                        PyObject *kwds) {
    // Function arguments to be parsed
    PyObject *raw_data_obj = NULL;
    PyObject *data_obj = NULL;
    PyObject *pm_obj = NULL;
    size_t n_threads = 1;

    static char *kwlist[] = {"raw_data", "pixel_map", "out", "n_threads", NULL};
    if (!PyArg_ParseTupleAndKeywords(args, kwds, "OO|On", kwlist, &raw_data_obj,
                                     &pm_obj, &data_obj, &n_threads)) {
        return NULL;
    }

    // Create a handle to the numpy array from the generic python object
    PyObject *raw_data = PyArray_FROM_OTF(
        raw_data_obj, NPY_UINT16,
        NPY_ARRAY_C_CONTIGUOUS | NPY_ARRAY_ENSUREARRAY | NPY_ARRAY_ALIGNED);
    if (!raw_data) {
        return NULL;
    }

    // Handle to the pixel map
    PyObject *pixel_map = PyArray_FROM_OTF(
        pm_obj, NPY_UINT32, NPY_ARRAY_C_CONTIGUOUS); // Make 64bit?
    if (!pixel_map) {
        return NULL;
    }
    if (PyArray_NDIM((PyArrayObject *)pixel_map) != 2) {
        PyErr_SetString(PyExc_TypeError, "The pixel map needs to be 2D");
        return NULL;
    }
    npy_intp n_rows = PyArray_DIM((PyArrayObject *)pixel_map, 0);
    npy_intp n_cols = PyArray_DIM((PyArrayObject *)pixel_map, 1);

    // If called with an output array get an handle to it, otherwise allocate
    // the output array
    PyObject *data_out = NULL;
    if (data_obj) {
        data_out =
            PyArray_FROM_OTF(data_obj, NPY_UINT16, NPY_ARRAY_C_CONTIGUOUS);
    } else {
        int ndim = PyArray_NDIM((PyArrayObject *)raw_data) + 1;
        npy_intp dims_arr[3] = {PyArray_DIM((PyArrayObject *)raw_data, 0),
                                n_rows, n_cols};
        npy_intp *dims = NULL;
        if (ndim == 2)
            dims = &dims_arr[1];
        else
            dims = &dims_arr[0];
        // Allocate output array
        data_out = PyArray_SimpleNew(ndim, dims, NPY_UINT16);
    }

    // Check that raw_data has one less dimension than frame
    // eg raw_data[n_frames, pixels] frame[nframes, nrows, ncols]
    // raw data is an array of values and data_out is 2/3D
    int rd_dim = PyArray_NDIM((PyArrayObject *)raw_data);
    int f_dim = PyArray_NDIM((PyArrayObject *)data_out);

    if (rd_dim != (f_dim - 1)) {
        PyErr_SetString(
            PyExc_TypeError,
            "Raw data and data needs to have the one less dim"); // eg -1
        return NULL;
    }

    uint16_t *src = (uint16_t *)PyArray_DATA((PyArrayObject *)raw_data);
    uint16_t *dst = (uint16_t *)PyArray_DATA((PyArrayObject *)data_out);
    uint32_t *pm = (uint32_t *)PyArray_DATA((PyArrayObject *)pixel_map);

    // Check sizes
    npy_intp rd_size = PyArray_SIZE((PyArrayObject *)raw_data);
    npy_intp fr_size = PyArray_SIZE((PyArrayObject *)data_out);
    npy_intp pm_size = PyArray_SIZE((PyArrayObject *)pixel_map);

    // TODO! Add exceptions
    if (rd_size != fr_size) {
        PyErr_SetString(PyExc_TypeError,
                        "Raw data size and data size needs to match");
        return NULL;
    }

    int64_t n_frames = 1;
    if (rd_dim == 2)
        n_frames = PyArray_DIM((PyArrayObject *)raw_data, 0);
    // printf("n_frames: %lu\n", n_frames);

    // do the correct size check
    if (rd_size / n_frames != pm_size) {
        PyErr_SetString(PyExc_TypeError,
                        "Pixel map size needs to match with frame size");
        return NULL;
    }

    if (n_threads == 1) {
        pm_decode(src, dst, pm, n_frames, n_rows * n_cols);
    } else {
        // Multithreaded processing
        pthread_t *threads = malloc(sizeof(pthread_t *) * n_threads);
        thread_args *arguments = malloc(sizeof(thread_args) * n_threads);

        size_t frames_per_thread = n_frames / n_threads;
        size_t assigned_frames = 0;
        for (size_t i = 0; i < n_threads; i++) {
            arguments[i].src = src + (i * frames_per_thread * pm_size);
            arguments[i].dst = dst + (i * frames_per_thread * pm_size);
            arguments[i].pm = pm;
            arguments[i].n_frames =
                frames_per_thread; // TODO! not matching frames.
            arguments[i].n_pixels = n_rows * n_cols;
            assigned_frames += frames_per_thread;
        }
        arguments[n_threads - 1].n_frames += n_frames - assigned_frames;

        for (size_t i = 0; i < n_threads; i++) {
            pthread_create(&threads[i], NULL, (void *)thread_pmdecode,
                           &arguments[i]);
        }
        for (size_t i = 0; i < n_threads; i++) {
            pthread_join(threads[i], NULL);
        }
        free(threads);
        free(arguments);
    }

    Py_DECREF(raw_data);
    Py_DECREF(pixel_map);

    return data_out;
}

// Module docstring, shown as a part of help(creader)
static char module_docstring[] = "C functions decode CTB data";

// Module methods
static PyMethodDef creader_methods[] = {
    {"decode", (PyCFunction)(void (*)(void))decode,
     METH_VARARGS | METH_KEYWORDS, "Decode analog data using a pixel map"},
    {NULL, NULL, 0, NULL} /* Sentinel */
};

// Module defenition
static struct PyModuleDef decoder_def = {
    PyModuleDef_HEAD_INIT,
    "_decoder",
    module_docstring,
    -1,
    creader_methods, // m_methods
    NULL,            // m_slots
    NULL,            // m_traverse
    NULL,            // m_clear
    NULL             // m_free
};

// Initialize module and add classes
PyMODINIT_FUNC PyInit__decoder(void) {

    PyObject *m = PyModule_Create(&decoder_def);
    if (m == NULL)
        return NULL;
    import_array(); // Needed for numpy
    return m;
}
