import pytest
from scipy.special import binom
from neslab.find import Model
import numpy as np


@pytest.fixture
def model():
    return Model(0.5, "Geometric", 100, n_nodes=4)


def test_instantiation():
    for distribution, shape in zip(["Uniform", "Geometric", "Poisson"], [20, 0.5, 0.5]):
        Model(shape, distribution, 100)


def test_cdf(model):
    cdfs = model.cdf()
    assert cdfs.shape[1] == binom(model.n_nodes, 2)
    assert (cdfs[-1, :] < 1.0).all()


def test_activity(model):
    act = model.activity()
    assert act.shape[0] < model.n_slots
    assert act.shape[1] == model.n_nodes
    assert (act < 1.0).all()


def test_disco_latency(model):
    lat = model.disco_latency()
    assert lat > 0 and lat < model.n_slots


def test_disco_frac(model):
    dfrac = model.disco_frac()
    assert dfrac[-1] < 1.0
    assert (np.diff(dfrac) >= 0).all()
