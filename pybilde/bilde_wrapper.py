from . import npbilde
from typing import Literal, Tuple, Union, List, Optional, TypeVar, Union
import numpy as np

TImg = TypeVar("TImg", bound=np.ndarray)


try:
    from . import ptbilde
    import torch
except ImportError:
    have_torch = False
else:
    have_torch = True
    assert ptbilde.__get_version__() == npbilde.__get_version__(), f"PyTorch extension version({ptbilde.__get_version__()}) does not match numpy extension version({npbilde.__get_version__()})."
    TImg = TypeVar("TImg", bound=Union[np.ndarray, torch.Tensor])


def lbp_transform(image: TImg, nb_samples: int =8, radius: float = 1.,
                 interpolation: Literal["bilienear", "nearCityblock", "nearChessboard", "arc-convolution"] = "bilinear",
                 cmp_operation: Literal["one-tail", "two-tail"] = "one-tail",
                 cmp_threshold: Union[int, Literal["otsu"]] = "otsu") -> TImg:
    cmp_threshold = str(cmp_threshold)
    if have_torch and isinstance(image, torch.Tensor):
        return ptbilde.lbp_transform(image, nb_samples, radius, interpolation, cmp_operation, cmp_threshold)
    elif isinstance(image, np.ndarray):
        return npbilde.lbp_transform(image, nb_samples, radius, interpolation, cmp_operation, cmp_threshold)
    else:
        raise ValueError("Unsupported image type. Expected numpy array or torch tensor.")


def lbp_features(image: TImg, nb_samples: int =8, radii: List[float] = [1., 2., 3.], 
                 interpolation: Literal["bilienear", "nearCityblock", "nearChessboard", "arc-convolution"] = "bilinear",
                 cmp_operation: Literal["one-tail", "two-tail"] = "one-tail",
                 cmp_threshold: Union[int, Literal["otsu"]] = "otsu") -> TImg:
    cmp_threshold = str(cmp_threshold)
    if have_torch and isinstance(image, torch.Tensor):
        return ptbilde.lbp_features(image, nb_samples, radii, interpolation, cmp_operation, cmp_threshold)
    elif isinstance(image, np.ndarray):
        return npbilde.lbp_features(image, nb_samples, radii, interpolation, cmp_operation, cmp_threshold)
    else:
        raise ValueError("Unsupported image type. Expected numpy array or torch tensor.")


def label_connected_components(image: TImg, neighborhood: Literal[8, 4] = 8) -> Tuple[TImg, int]:
    if have_torch and isinstance(image, torch.Tensor):
        return ptbilde.label_connected_components(image, neighborhood)
    elif isinstance(image, np.ndarray):
        return npbilde.label_connected_components(image, neighborhood)
    else:
        raise ValueError("Unsupported image type. Expected numpy array or torch tensor.")

def label_connected_components_equal(image: TImg, neighborhood: Literal[8, 4] = 8) -> Tuple[TImg, int]:
    if have_torch and isinstance(image, torch.Tensor):
        return ptbilde.label_connected_components_equal(image, neighborhood)
    elif isinstance(image, np.ndarray):
        return npbilde.label_connected_components_equal(image, neighborhood)
    else:
        raise ValueError("Unsupported image type. Expected numpy array or torch tensor.")
    
def get_connected_components_and_features(image: TImg, neighborhood: Literal[8, 4] = 8) -> Tuple[TImg, np.ndarray]:
    if have_torch and isinstance(image, torch.Tensor):
        return ptbilde.get_connected_components_and_features(image, neighborhood)
    elif isinstance(image, np.ndarray):
        return npbilde.get_connected_components_and_features(image, neighborhood)
    else:
        raise ValueError("Unsupported image type. Expected numpy array or torch tensor.")

def lof_binarize(image: TImg, bitDepth: Literal[3,4,5,6,7,8] = 8) -> TImg:
    if have_torch and isinstance(image, torch.Tensor):
        return ptbilde.lof_binarize(image, bitDepth)
    elif isinstance(image, np.ndarray):
        return npbilde.lof_binarize(image, bitDepth)
    else:
        raise ValueError(f"Unsupported image type. Expected numpy array or torch tensor got {repr(type(image))}.")


def textline_segment(image: TImg, windowWidth: int = 51, windowHeight: int = 51, tracerDencity: int = 1, compareDistance: int = 50, minimumLetterHeight: int = 10, maximumLetterHeight: int = 500) -> TImg:
    if have_torch and isinstance(image, torch.Tensor):        
        return ptbilde.textline_segment(image, windowWidth, windowHeight, tracerDencity, compareDistance, minimumLetterHeight, maximumLetterHeight)
    elif isinstance(image, np.ndarray):
        return npbilde.textline_segment(image, windowWidth, windowHeight, tracerDencity, compareDistance, minimumLetterHeight, maximumLetterHeight)
    else:
        raise ValueError("Unsupported image type. Expected numpy array or torch tensor.")

def enhance_grayscale(image: TImg, bitDepth: Literal[3,4,5,6,7,8] = 8, mode: Literal["equalise", "clip"] = "equalise", windowWidth: int = 51, windowHeight: int = 51, globalHistogramCoeficient: float = 0, localHistogramCoeficient: float = 1, topQuantile: float = .95, bottomQuantile: float = .05) -> TImg:
    if have_torch and isinstance(image, torch.Tensor):
        return ptbilde.enhance_grayscale(image, bitDepth, mode, windowWidth, windowHeight, globalHistogramCoeficient, localHistogramCoeficient, topQuantile, bottomQuantile)
    elif isinstance(image, np.ndarray):
        return npbilde.enhance_grayscale(image, bitDepth, mode, windowWidth, windowHeight, globalHistogramCoeficient, localHistogramCoeficient, topQuantile, bottomQuantile)
    else:
        raise ValueError("Unsupported image type. Expected numpy array or torch tensor.")

