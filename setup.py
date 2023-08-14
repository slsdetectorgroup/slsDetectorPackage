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
    version = '2023.8.11',
    description = 'Experimental GUI for the chip test board',
    packages=setuptools.find_packages(
        exclude=['tests',]  
    ),
    include_package_data=True,
    ext_modules=[c_ext],
    scripts=['CtbGui',],
     install_requires=[
          'numpy', #TODO! write proper requires block (can't depend on slsdet at the moment)
          'pyzmq',
          'pillow',
          'PyQt5', 
          'pyqtgraph',
          'matplotlib'
      ],
)
