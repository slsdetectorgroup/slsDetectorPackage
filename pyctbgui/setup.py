import setuptools
import numpy as np

c_ext = setuptools.Extension("pyctbgui._decoder",
                             sources=["src/decoder.c", "src/pm_decode.c"],
                             include_dirs=[
                                 np.get_include(),
                                 "src/",
                             ],
                             extra_compile_args=['-std=c99', '-Wall', '-Wextra'])

c_ext.language = 'c'
setuptools.setup(
    name='pyctbgui',
    version='2023.8.17',
    description='Experimental GUI for the chip test board',
    packages=setuptools.find_packages(exclude=[
        'tests',
    ]),
    include_package_data=True,
    ext_modules=[c_ext],
    scripts=[
        'CtbGui',
    ],
    python_requires='>=3.10',  # using match statement
    install_requires=[
        'numpy',
        'pyzmq',
        'pillow',
        'PyQt5',
        'pyqtgraph',
        'matplotlib',
        # 'slsdet', not yet available on pypi, maybe v8
    ],
    extras_require={
        'dev': [
            'pytest==7.4.0',
            'ruff==0.0.285',
            'yapf==0.40.1',
            'pytest-ruff==0.1.1',
            'pre-commit',
            'pytest-qt',
        ],
    })
