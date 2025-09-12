# setup.py
import os, re
import pybind11
from setuptools import setup, Extension


try:
    from torch.utils.cpp_extension import BuildExtension, CppExtension  # <-- add this
    have_torch = True
except ImportError:
    have_torch = False
    print("PyTorch not found, skipping PyTorch extension.")


def get_version():
    with open(os.path.join("include", "version.hpp"), encoding="utf-8") as f:
        m = re.search(r'#define\s+BILDE_VERSION\s+"(.*)"', f.read())
    if not m:
        raise RuntimeError("Unable to find version string.")
    return m.group(1)

# --- numpy binding as before (ok with plain Extension)
np_ext = Extension(
    "pybilde.npbilde",
    sources=["pybilde/bilde_pybind_np.cc"],
    include_dirs=[pybind11.get_include(), "./include"],
    extra_compile_args=["-Wfatal-errors", "-std=c++17"],
    language="c++",
)


# IMPORTANT: you had `ext_modules = [...]` twice; the second overwrote the first.
ext_modules = [np_ext]


if have_torch:
    pt_ext = CppExtension(
        "pybilde.ptbilde",
        sources=["pybilde/bilde_pybind_pt.cc"],
    include_dirs=[pybind11.get_include(), "./include"],
    extra_compile_args=["-Wfatal-errors", "-std=c++17"],)
    ext_modules = ext_modules + [pt_ext]


# Setup function
setup(
    name="bilde",  # Name of your package
    version=get_version(),          # Version of your package
    author="Anguelos Nicolaou",       # Your name
    author_email="anguelos.nicolaou@gmail.com",  # Your email
    description="A python version of the Bilde library",  # Short description
    long_description=open("README.md").read(),  # Long description from the README file
    long_description_content_type="text/markdown",  # Content type for long description
    url="https://github.com/anguelos/bilde",  # URL to your project (if available)
    packages=["pybilde", "pybilde.util"],  # Packages to include
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
