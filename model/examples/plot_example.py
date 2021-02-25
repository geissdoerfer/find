import numpy as np
import matplotlib.pyplot as plt

from neslab.find import distributions as dists
from neslab.find import Model

prop_cycle = plt.rcParams["axes.prop_cycle"]
colors = prop_cycle.by_key()["color"]


def plot_activity(
    t_chr: int = 100,
    distribution: str = "Uniform",
    offset_rel: float = 0.5,
    n: int = 100000,
):
    for i, shape in enumerate([30, 60]):
        m = Model(shape, distribution, t_chr, n_slots=n)

        activities = m.activity()

        dist_cls = getattr(dists, distribution.capitalize())
        offset = int(dist_cls(shape).expectation() + t_chr / 2)

        act_0 = np.pad(activities[:, 0], 10, mode="constant")[:-offset]
        act_1 = np.pad(activities[:, 0], 10 + offset, mode="constant")

        f_act, ax_act = plt.subplots(figsize=(5, 1.75))
        ax_act.plot(act_0, label=r"Node 1", color=colors[0], linewidth=0.75)
        ax_act.plot(
            act_1, label=r"Node 2", color=colors[1], linewidth=0.75, linestyle="--"
        )
        ax_act.set_xlabel("Slot")
        ax_act.set_ylabel("Probability\nto be active", multialignment="center")
        ax_act.set_ylim([-0.0025, 0.041])
        ax_act.set_xlim((-100, 3990))
        plt.legend(loc="upper right")
        plt.tight_layout()
        plt.show()


def plot_cdf(
    t_chr: int = 100,
    distribution: str = "Uniform",
    offset_rel: float = 0.5,
    n: int = 200000,
):
    f_cdf, ax_cdf = plt.subplots(figsize=(5, 2))
    shape_labels = [30, 60]
    linestyles = ["-", "--"]
    for i, shape in enumerate([15, 90]):
        m = Model(shape, distribution, t_chr, n_slots=n)
        ax_cdf.semilogx(
            m.cdf(),
            label=fr"Random delay $X\sim U[0, {int(shape_labels[i])}]$",
            linestyle=linestyles[i],
        )

    ax_cdf.set_xlabel("Slot")
    ax_cdf.set_ylabel("Cumulative prob.\nof discovery", multialignment="center")
    ax_cdf.set_xlim((9, ax_cdf.get_xlim()[1]))
    ax_cdf.set_ylim((-0.05, 1.05))
    plt.legend(loc="upper left")
    f_cdf.tight_layout()
    plt.show()


if __name__ == "__main__":
    plot_activity()
    plot_cdf()
