import numpy as np
from scipy import stats
from typing import Union
from typing import Iterable


class ProbabilityDist(object):
    rv_class = stats.rv_discrete

    def __init__(self, scale: float):
        self._scale = scale

    def pmf(self, k: Union[int, Iterable]):
        return self.rv_class.pmf(k, self._scale)

    def cdf(self, k: Union[int, Iterable]):
        return self.rv_class.cdf(k, self._scale)

    def expectation(self):
        return self.rv_class.mean(self._scale)

    def pmf_nsum(self, n: int):
        pmf_base = self.pmf(np.arange(self.min_support()))
        if n < 1:
            raise ValueError("n must be greater 1")

        yield pmf_base
        n_slots = n * len(pmf_base) - (n - 1)
        pmf_sample = np.empty((n_slots,))
        current_len = len(pmf_base)
        pmf_sample[:current_len] = pmf_base
        for _ in range(1, n):
            new_len = current_len + len(pmf_base) - 1
            pmf_sample[:new_len] = np.convolve(pmf_sample[:current_len], pmf_base)
            yield pmf_sample[:new_len]
            current_len = new_len

    def _icdf(self, k: Union[int, Iterable]):
        return self.rv_class.isf(1 - k, self._scale)

    def itf_sample(self, x: float):
        return self._icdf(x)

    def sample(self, size=None):
        return self.itf_sample(np.random.uniform(size=size)).astype(int)

    def gen_table(self, n_in: int = 1024):
        ys = np.linspace(0.01, 0.99, n_in)
        return self._icdf(ys).astype(np.uint32)

    def min_support(self, thr: float = 1e-6):
        min_support = 1
        while self.pmf(min_support) > thr:
            min_support += 1
        return min_support

    @classmethod
    def get_scale_range(cls, c: int, n_points: int = 2):
        raise NotImplementedError


class Geometric(ProbabilityDist):
    rv_class = stats.geom

    @classmethod
    def get_scale_range(cls, c: int, n_points: int = 2):
        return np.logspace(np.log10(1.0 / c), min(0, np.log10(50 / c)), n_points)

    def pmf(self, k: Union[int, Iterable]):
        return super().pmf(k + 1)

    def cdf(self, k: Union[int, Iterable]):
        return super().cdf(k + 1)

    def pmf_nsum(self, n: int):
        n_slots = n * self.min_support() - (n - 1)
        ks = np.arange(n_slots)
        for i in range(1, n):
            yield stats.nbinom.pmf(ks, i, self._scale)


class Poisson(ProbabilityDist):
    rv_class = stats.poisson

    @classmethod
    def get_scale_range(cls, c: int, n_points: int = 2):
        return np.linspace(1, (c + 1) // 2, n_points).astype(int)

    def min_support(self, thr: float = 1e-6):
        min_support = self._scale
        while self.pmf(min_support) > thr:
            min_support += 1
        return min_support


class Uniform(ProbabilityDist):
    rv_class = stats.randint

    @classmethod
    def get_scale_range(cls, c: int, n_points: int = 2):
        return np.linspace(2, c + 1, n_points).astype(int)

    def pmf(self, k: Union[int, Iterable]):
        return self.rv_class.pmf(k, 0, self._scale)

    def cdf(self, k: Union[int, Iterable]):
        return self.rv_class.cdf(k, 0, self._scale)

    def expectation(self):
        return self.rv_class.mean(0, self._scale)

    def _icdf(self, k: Union[int, Iterable]):
        return self.rv_class.isf(1 - k, 0, self._scale)
