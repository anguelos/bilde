#from setuptools import setup, Extension
import pybind11
import os
import re
from setuptools import setup, Extension

from setuptools import setup, Extension
import pybind11


def get_version():
    version_file = os.path.join('include', 'version.hpp')
    with open(version_file, 'r', encoding="utf-8") as f:
        content = f.read()
    # Regular expression to find the version number
    version_match = re.search(r'#define\s+BILDE_VERSION\s+"(.*)"', content)
    if version_match:
        return version_match.group(1)
    raise RuntimeError("Unable to find version string.")


ext_modules = [
    Extension(
        'pybilde.bilde',  # Name of the generated Python module
        sources=['pybilde/bildepybind.cc'],  # Path to your C++ source file
        include_dirs=[pybind11.get_include(), "./include"],  # Include pybind11 headers
        extra_compile_args=[
            #"-O3",               # Maximum optimization level
            #"-march=native",      # Optimize for the host machine’s architecture
            #"-funroll-loops",     # Unroll loops for optimization
            #"-ffast-math",        # Fast floating-point optimizations
            #"-ftree-vectorize",   # Enable vectorization
            #"-mfma",              # Fused Multiply-Add instructions
            #"-mavx",              # Enable AVX (or -mavx2 if you need AVX2)
        ],
        extra_link_args=[],  # You can add linker options here if needed        
        language='c++',
    ),
]



# Setup function
setup(
    name="bilde",  # Name of your package
    version="0.1.0",          # Version of your package
    author="Anguelos Nicolaou",       # Your name
    author_email="anguelos.nicolaou@gmail.com",  # Your email
    description="A python version of the Bilde library",  # Short description
    long_description=open("README.md").read(),  # Long description from the README file
    long_description_content_type="text/markdown",  # Content type for long description
    url="https://github.com/anguelos/bilde",  # URL to your project (if available)
    packages=["pybilde"],  # Packages to include
    classifiers=[
        "Programming Language :: Python :: 3",
        "Programming Language :: C++",
        "License :: OSI Approved :: MIT License",  # Update to your license
        "Operating System :: OS Independent",
    ],
    python_requires='>=3.6',  # Specify required Python version
    install_requires=[
        "numpy",  # List your package dependencies
    ],
    ext_modules=ext_modules,  # Extension modules to build
)
