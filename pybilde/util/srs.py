import sklearn.decomposition
import numpy as np
import sys
from PIL import Image
from ..bilde import lbp_features
from typing import Union, List


def extract_features(img_path: str) -> np.ndarray:
    img = Image.open(img_path).convert("L")
    img = np.array(img)
    features = lbp_features(img)[None, :]
    return features


def block_normalise(values: np.ndarray, block_sizes: Union[None, List[int]] = None, suppress_bins: List[int] = [0]) -> np.ndarray:
    if block_sizes is None:
        block_sizes = [256]*(values.shape[1]//256)
    res_values = values.copy()
    for block in range(len(block_sizes)):
        block_start = sum(block_sizes[:block])
        block_end = sum(block_sizes[:block+1])
        for supress_bin in suppress_bins:
            res_values[:, 256] = 0
            res_values[:, supress_bin + block_start] = 0
        block_sum = res_values[:, block_start: block_end].sum(axis=1)
        res_values[:, block_start:block_end] = res_values[:, block_start:block_end] // block_sum[:, None]
    return res_values


def std_normalise(values: np.array, e=.000000000001) -> np.array:
    means = values.mean(axis=0)[None, :]
    values = values - means
    stds = values.std(axis=0)[None, :]
    values = values/(stds + e)
    return values


def pre_pca_normalise(values: np.array) -> np.array:
    return std_normalise(block_normalise(values))


def hellinger_normalise(values: np.ndarray) -> np.ndarray:
    return np.sign(values)*np.abs(values)**.5


def l2_normalise(values: np.ndarray, e: float = .00000000000001) -> np.ndarray:
    return values/((np.sum(values ** 2, 1)**.5)+e)[:, None]


def post_pca_normalise(values: np.array) -> np.array:
    return l2_normalise(hellinger_normalise(values))


def apply_pca(values: np.ndarray, pca_values: Union[None, np.array] = None, n_components: int = 200) -> np.ndarray:
    if pca_values is None:
        return values @ pca_values.T
    else:
        pca_values = pca_train(values)
        return values @ pca_values.T


def pca_train(values: np.ndarray, n_components: int = 200, max_augmentation: int = 5, noise: float = .000000001) -> np.array:
    pca = sklearn.decomposition.PCA(n_components)
    if values.shape[0] * max_augmentation < n_components:
        raise ValueError(f"Need at least 1/{max_augmentation} samples of the components in order to augment them")
    elif values.shape[0] < n_components:
        aug_values = values.copy()
        while aug_values.shape[0] < n_components:
            aug_values = np.concatenate([aug_values, values + noise * np.random.rand(*values.shape)])
        pca.fit(aug_values)
    else:
        pca.fit(values)
    return pca.components_


def srs(histograms: np.array, pca_componets: np.array) -> np.array:
    values = pre_pca_normalise(histograms)
    values = apply_pca(values, pca_componets)
    values = post_pca_normalise(values)
    return values


def train_srs(histograms: np.array, n_components: int = 200) -> np.array:
    values = pre_pca_normalise(histograms)
    return pca_train(values=values, n_components=n_components)
