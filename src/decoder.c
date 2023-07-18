#define NPY_NO_DEPRECATED_API NPY_1_7_API_VERSION
#define PY_SSIZE_T_CLEAN
#include <Python.h>
#include <numpy/arrayobject.h>

static PyObject *func(PyObject *Py_UNUSED(self), PyObject *Py_UNUSED(args)) {
    Py_INCREF(Py_None);
    return Py_None;
}

// Module docstring, shown as a part of help(creader)
static char module_docstring[] = "C functions decode CTB data";

// Module methods
static PyMethodDef creader_methods[] = {
    {"func", func, METH_VARARGS, "Does nothing and returns None"},
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
