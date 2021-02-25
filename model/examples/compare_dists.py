import numpy as np
import matplotlib.pyplot as plt

from neslab.find import Model
from neslab.find import distributions

# Charging time of 100 slots
t_chr = 100
# Evaluate distributions at 10 different shapes
n_shapes = 15

xx = np.arange(n_shapes)
for dist_name in ["Geometric", "Uniform", "Poisson"]:
    dist_cls = getattr(distributions, dist_name)
    nd_lat = np.empty((n_shapes,))
    for i, shape in enumerate(dist_cls.get_shape_range(t_chr, n_shapes)):
        m = Model(shape, dist_name, t_chr, n_slots=250000)
        nd_lat[i] = m.disco_latency()
    plt.plot(xx, nd_lat, label=dist_name)
plt.xticks([])
plt.xlabel("Shape")
plt.ylabel("Discovery Latency [slots]")
plt.legend()
plt.show()
