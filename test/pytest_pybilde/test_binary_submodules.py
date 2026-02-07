import pybilde

if pybilde.have_torch:
    assert pybilde.ptbilde.__get_version__() == pybilde.npbilde.__get_version__(), f"PyTorch extension version({pybilde.ptbilde.__get_version__()}) does not match numpy extension version({pybilde.npbilde.__get_version__()})."
    if set(dir(pybilde.ptbilde)) != set(dir(pybilde.npbilde)):
        #print("PyTorch extension API does not match numpy extension API. Please ensure both extensions have the same functions and classes defined.", file=sys.stderr)
        raise RuntimeError("PyTorch extension API does not match numpy extension API. Please ensure both extensions have the same functions and classes defined.")