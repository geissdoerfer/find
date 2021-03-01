import numpy as np
import matplotlib.pyplot as plt

from neslab.find import Model

# Charging time of 25 slots
t_chr = 25
# Geometric distribution with scale 0.3
scale = 0.3

densities = np.linspace(0.1, 1.5, 15)

nd_lat = np.empty((len(densities),))
for i, density in enumerate(densities):
    n_nodes = np.ceil(density * t_chr).astype(int)
    m = Model(scale, "Geometric", t_chr, n_nodes=n_nodes)

    nd_lat[i] = m.disco_latency()

plt.plot(densities, nd_lat)
plt.xlabel("Network density")
plt.ylabel("Discovery Latency [slots]")
plt.show()
