import numpy as np
import matplotlib.pyplot as plt
from scipy.optimize import minimize_scalar

from neslab.find import distributions
from neslab.find import Model


def objective(scale, t_chr):
    m = Model(scale, "Geometric", t_chr, n_slots=t_chr * 20000)
    return m.disco_latency()


def optimize_scale(t_chr):
    scale_range = distributions.Geometric.get_scale_range(t_chr)
    res = minimize_scalar(
        objective,
        bounds=scale_range,
        method="bounded",
        args=(t_chr),
    )

    return res.x, res.fun


if __name__ == "__main__":
    t_chrs = np.arange(10, 25).astype(int)
    scales = np.empty((len(t_chrs),))
    nd_lat = np.empty_like(scales)

    for i, t_chr in enumerate(t_chrs):
        scales[i], nd_lat[i] = optimize_scale(t_chr)

    plt.plot(t_chrs, nd_lat)
    plt.xlabel("Charging time [slots]")
    plt.ylabel("Discovery Latency [slots]")
    plt.show()