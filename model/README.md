# FIND model

This directory contains a Python implementation of the analytical FIND model presented in Section 2 of our NSDI 2021 paper.
As illustrated below, the model allows to compute the network discovery latency as a function of several key parameters (e.g., delay distribution, charging times of the nodes).
It is also possible to obtain more detailed insights by computing the probability for a node to become active in a slot and the probability for a link to be discovered in a slot.

## Installation

Install the python package together with the requirements for the examples using either

```
pip install neslab.find[examples]
```

or

```
pipenv install neslab.find[examples]
```

## Usage

Instantiate the model with a uniform distribution with scale parameter 10 and three nodes with charging times of 100, 125 and 200 slots that become active for the first time without an offset:

```python
import matplotlib.pyplot as plt
from neslab.find import Model

m = Model(10, "Uniform", [100, 125, 200], offset=0)
```

Compute for every node the probability of being active in a slot and plot the results:

```python
act = m.activity()
plt.plot(act[:2000])
plt.show()
```

Compute the cumulative distribution function (cdf) of the slot in which each of the three links between the three nodes is discovered and plot the results:

```python
cdfs = m.cdf()
plt.plot(cdfs)
plt.show()
```

Compute the fraction of discovered links and plot the result:

```python
dff = m.disco_frac()
plt.plot(dff)
plt.show()
```

Compute the discovery latency:

```python
lat = m.disco_latency()
print(f"Discovery latency: {lat} slots")
```

## Examples

We provide more involved example scripts in the [examples](./examples) directory:

To compare the probability of activity and the cdf of the corresponding discovery latency of two nodes with a charging time of 30 and 60 slots, using a uniform delay distribution, call

```
python examples/plot_example.py
```

To compare the discovery latency of two nodes when using different distributions across a range of their scale parameters, run

```
python examples/compare_dists.py
```

To plot the discovery latency of a clique of nodes over the node density, run

```
python examples/plot_density.py
```

To optimize the scale parameter of the geometric distribution to a range of charging times, run

```
python examples/optimize_scale.py
```


## Reproduce key results

We used the code provided in this directory to gain the insights presented in Section 2 of [our NSDI 2021 paper](https://nes-lab.org/pubs/2021-Geissdoerfer-Find.pdf).
We invite you to reproduce our results with the scripts provided in the [reproducibility](./reproducibility) directory.
Some of the calculations take significant computing resources and require a high performance computing system with a large number of CPUs to complete in a reasonable amount of time.
