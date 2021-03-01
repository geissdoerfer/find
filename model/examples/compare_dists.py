import numpy as np
import matplotlib.pyplot as plt

from neslab.find import Model
from neslab.find import distributions

# Charging time of 100 slots
t_chr = 100
# Evaluate distributions at 10 different scales
n_scales = 15

xx = np.arange(n_scales)
for dist_name in ["Geometric", "Uniform", "Poisson"]:
    dist_cls = getattr(distributions, dist_name)
    nd_lat = np.empty((n_scales,))
    for i, scale in enumerate(dist_cls.get_scale_range(t_chr, n_scales)):
        m = Model(scale, dist_name, t_chr, n_slots=250000)
        nd_lat[i] = m.disco_latency()
    plt.plot(xx, nd_lat, label=dist_name)
plt.xticks([])
plt.xlabel("Scale")
plt.ylabel("Discovery Latency [slots]")
plt.legend()
plt.show()
