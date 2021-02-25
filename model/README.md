# FIND model

This directory provides a python implementation of the FIND model and protocol.

Model assumptions:
A node needs to charge for a constant number of M slots until it reaches the minimum energy level required to be active for one slot. Once the node reaches the minimum energy level, it can wait for a random number of slots (drawn from a configurable distribution) before becoming active. During an active slot, a node fully depletes its energy storage. A number of nodes with potentially different charging times reach the minimum energy level for the first time in potentially different slots.

The provided code allows to calculate the probability for each node to become active in any slot, the probability for any individual link to be discovered in any slot as well as the network discovery latency.

## Installation

Depending on your setup, install the python package using

```
pipenv install neslab.find
```

or

```
pip install neslab.find
```

## Usage

Instantiate the model with a uniform distribution with shape 10 and three nodes with charging times of 100, 125 and 200, respectively:

```python
from neslab.find import Model

m = Model(10, "Uniform", [100, 125, 200])
```

Calculate the probability to be active of all three nodes:

```python
act = m.activity()
plt.plot(act[:1000])
plt.show()
```

Calculate the cdf of discovery of the three links:

```python
cdfs = m.cdf()
plt.plot(cdfs)
plt.show()
```

Calculate the fraction of discovered links:

```python
dff = m.disco_frac()
plt.plot(dff)
plt.show()
```

Calculate the discovery latency:


```python
lat = m.disco_lat())
print(f"Discovery latency: {lat} slots")
```

## Reproduce key results

We used the code provided in this directory to gain the insights presented in section 2 of [our paper](https://nes-lab.org/pubs/2021-Geissdoerfer-Find.pdf).
We invite you to reproduce our results with the scripts provided in the [examples directory](./examples).
Although we've reduced the number of sample points, number of nodes and charging times for the provided examples, expect the examples to utilize most of your computer's resource and run for multiple minutes.
If you are interested in reproducing the full results across a larger range and have access to an HPC facility, get in touch with us.

To plot the probability of activity of two nodes with a charging time of 60 slots, using a uniform delay distribution (Figures 3 and 4 in the paper), call

```
python examples/plot_example.py
```

To compare the discovery latency of two nodes when using different distributions across a range of their scale parameters (Figure 5 in the paper), run

```
python examples/compare_dists.py
```

To plot the discovery latency of a clique of nodes over the node density (Figure 6 in the paper), run

```
python examples/plot_density.py
```

Finally, to optimize the scale parameter of the geometric distribution to a range of charging times, run

```
python examples/optimize_scale.py
```