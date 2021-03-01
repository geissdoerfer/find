import numpy as np
import matplotlib.pyplot as plt

from neslab.find import distributions as dists
from neslab.find import Model

prop_cycle = plt.rcParams["axes.prop_cycle"]
colors = prop_cycle.by_key()["color"]

# Charging time of 25 slots
t_chr = 100
# Plot for scales 30 and 60
scales = [30, 60]

cdfs = list()
for i, scale in enumerate(scales):

    m = Model(scale, "Uniform", t_chr)

    activities = m.activity()
    cdf = m.cdf()

    plt.plot(activities[:4000, 0], label=r"Node 1")
    plt.plot(activities[:4000, 1], label=r"Node 2")
    plt.xlabel("Slot")
    plt.ylabel("Probability of Activity")
    plt.legend()
    plt.title(fr"Random delay $X\sim U[0, {int(scale)}]$")
    plt.show()

    cdfs.append(cdf)

for i, scale in enumerate(scales):
    plt.semilogx(
        cdfs[i],
        label=fr"Random delay $X\sim U[0, {int(scale)}]$",
    )

plt.xlabel("Slot")
plt.ylabel("Cumulative probability of discovery")
plt.legend(loc="upper left")
plt.show()
