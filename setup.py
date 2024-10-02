from setuptools import setup, Extension
import pybind11

ext_modules = [
    Extension(
        'bilde',  # Name of the generated Python module
        sources=['src/bildepybind.cc'],  # Path to your C++ source file
        include_dirs=[pybind11.get_include(), "./include"],  # Include pybind11 headers
        language='c++',
    ),
]

setup(
    name='bilde',
    ext_modules=ext_modules,
)