from pathlib import Path
import sklearn.decomposition
import numpy as np
import sys
from PIL import Image
from ..bilde import lbp_features
from typing import Tuple, Union, List


class SRS():
    E: float = .00000000000001  # Small value to prevent division by zero
    MAX_AUGMENTATION: int = 5  # Maximum number of times to augment the data for PCA training
    MIN_SAMPLES: int = 10  # Minimum number of samples to calculate means and stds

    def block_normalise(self, values: np.ndarray) -> np.ndarray:
        assert values.shape[1] == 12 * 256
        res_values = values.copy().astype(np.float64)
        for block in range(12):
            block_start = 256 * block
            block_end = 256 * (block + 1)
            for suppres_bin in self.suppres_bins:
                res_values[:, block_start + suppres_bin] = 0
            block_sum = res_values[:, block_start: block_end].sum(axis=1)[:, None] + SRS.E
            res_values[:, block_start:block_end] = res_values[:, block_start:block_end] / block_sum
        return res_values

    def std_normalise(self, values: np.array) -> Tuple[np.array, np.array, np.array]:
        if self.means is None:
            assert values.shape[0] > 10  # Need at least 10 samples to calculate means and stds
            self.means = values.mean(axis=0)[None, :]

        values = values - self.means

        if self.stds is None:
            self.stds = values.std(axis=0)[None, :]
        values = values/(self.stds + SRS.E)
        return values

    def apply_pca(self, values: np.ndarray) -> np.ndarray:
        if self.pca_componet_matrix is None:
            assert values.shape[0] * SRS.MAX_AUGMENTATION < self.n_components
        return values @ self.pca_componet_matrix.T

    @staticmethod
    def hellinger_normalise(values: np.ndarray) -> np.ndarray:
        return np.sign(values)*np.abs(values)**.5

    @staticmethod
    def l2_normalise(values: np.ndarray, e: float = .00000000000001) -> np.ndarray:
        return values/((np.sum(values ** 2, 1)**.5)+e)[:, None]

    def __init__(self, n_components: int = 200, suppres_bins: List[int] = [0], pca_componet_matrix: Union[np.array, None] = None, means: Union[np.array, None] = None, stds: Union[np.array, None] = None, filename: Union[Path, str, None] = None):
        self.n_components = n_components
        self.suppres_bins = suppres_bins
        if pca_componet_matrix is not None:
            assert means is not None and stds is not None
            assert filename is None
            self.pca_componet_matrix = pca_componet_matrix
            self.means = means
            self.stds = stds
        else:
            assert means is None and stds is None
            self.pca_componet_matrix = None
            self.means = None
            self.stds = None
        if filename is not None:
            self.load(filename)

    def train(self, histograms: np.array, n_components: int = 200):
        x = self.block_normalise(histograms)
        x = self.std_normalise(x)
        pca = sklearn.decomposition.PCA(n_components)
        if x.shape[0] >= n_components:
            augmented_x = x
        elif x.shape[0] * SRS.MAX_AUGMENTATION >= n_components:    
            augmented_x = x.copy()
            while augmented_x.shape[0] < n_components:
                augmented_x = np.concatenate([augmented_x, x + np.random.rand(*x.shape) * SRS.E])
        else:
            raise ValueError(f"Need at least 1/{SRS.MAX_AUGMENTATION} samples of the components in order to augment them")
        pca.fit(augmented_x)
        self.pca_componet_matrix = pca.components_
        self.n_components = n_components

    def __call__(self, histograms: np.array) -> np.array:
        x = self.block_normalise(histograms)
        x = self.std_normalise(x)
        x = self.apply_pca(x)
        x = SRS.hellinger_normalise(x)
        x = SRS.l2_normalise(SRS.hellinger_normalise(x))
        return x

    def save(self, filename: Union[Path, str]):
        if isinstance(filename, str):
            filename = Path(filename)
        with open(filename, "wb") as f:
            np.savez(f, component_matrix=self.pca_componet_matrix, means=self.means,
                     stds=self.stds, suppres_bins=self.suppres_bins)

    def load(self, filename: Union[Path, str]):
        if isinstance(filename, str):
            filename = Path(filename)
        with open(filename, "rb") as f:
            data = np.load(f, allow_pickle=True)
            self.pca_componet_matrix = data["component_matrix"]
            self.means = data["means"]
            self.stds = data["stds"]
            self.suppres_bins = data["suppres_bins"]
            self.n_components = self.pca_componet_matrix.shape[0]


def load_srs(filename: Union[Path, str]) -> SRS:
    srs = SRS()
    srs.load(filename)
    return srs


def unsupervised_srs(histograms: np.array, n_components: int = 200) -> Tuple[np.array, SRS]:
    srs = SRS()
    srs.train(histograms, n_components)
    return srs(histograms), srs