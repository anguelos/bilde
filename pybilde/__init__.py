#from .npbilde import __version__, lbp_transform, lbp_features, enhance_grayscale, label_connected_components, get_connected_components_and_features, lof_binarize, textline_segment
#from .ptbilde import lbp_features as lbp_features_pt, label_connected_components as connected_components_pt
from . import npbilde
__version__ = npbilde.__get_version__()
from .bilde_wrapper import lbp_transform, lbp_features, label_connected_components, label_connected_components_equal, get_connected_components_and_features, lof_binarize, textline_segment, enhance_grayscale


__all__ = [
    "npbilde",
    "__version__",
    "lbp_transform",
    "lbp_features",
    "label_connected_components",
    "get_connected_components_and_features",
    "lof_binarize",
    "textline_segment",
    "enhance_grayscale",
]


try:
    import sklearn
except ImportError:
    print("scikit-learn not found, some functionality may be limited. Please install scikit-learn to use all features of pybilde.")
else:
    from .util.srs import SRS, load_srs, unsupervised_srs
    __all__.extend(["SRS", "load_srs", "unsupervised_srs"])

try:
    from . import ptbilde
except ImportError:
    have_torch = False
else:
    have_torch = True
    __all__.append("ptbilde")

