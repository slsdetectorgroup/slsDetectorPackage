import setuptools
import numpy as np

c_ext = setuptools.Extension(
    "_decoder",
    sources = [
        "src/decoder.c", 
        "src/pm_decode.c"
        ],
    include_dirs=[
            np.get_include(),
            "src/",
            ],
    extra_compile_args=[
        '-std=c99', 
        '-Wall', 
        '-Wextra'
        ] 
    )
                    

c_ext.language = 'c'
setuptools.setup(
    name= 'pyctbgui',
    version = '2023.7.19',
    description = 'Experimental GUI for the chip test board',
    ext_modules=[c_ext],
)
