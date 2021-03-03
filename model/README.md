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
Figures 3 and 4 can be reproduced with the script `examples/plot_example.py`.
The calculations for Figures 5 and 6, and for the lookup table for the online implementation take significant computing resources and require a high performance computing system with a large number of CPUs to complete in a reasonable amount of time.
We use [ray](https://ray.io/) to run the calculations on a distributed memory computing cluster.

### Setup your cluster

Install `ray` and `neslab.find` on all nodes in your cluster. The exact installation procedure depends on the specific cluster setup and your access rights. If in doubt, contact the cluster provider.

First, setup the `head` node responsible for managing the workload across the cluster by starting the head process with:

```
ray start --block --head --redis-password=yourpassword
```

Next, start a worker process on each other node in your cluster, providing the IP address of your head node as  `address`:

```
ray start --block --address=[IP of head node] --redis-password=yourpassword
```

### Comparing distributions

To reproduce the results shown in Figure 5 in our paper, you can compute the neighbor discovery performance of different distributions for a higher number of sampling points than with the example from `examples/compare_dists` according to the following instructions.

Login to any of the nodes in the cluster, clone the repository and run

```
python reproducibility/compute_dists.py -a [IP of head node] -p yourpassword -o results_dists.csv
```

Copy the `results_dists.csv` to your local machine and generate the plot with

```
python reproducibility/plot_dists.py -i results_dists.csv
```

### Fit the scale parameter for various scenarios

To reproduce the results shown in Figure 6 in our paper, and to generate the lookup table for the online implementation, first fit the optimized scale parameter of the geometric distribution for different combinations of charging time and n umber of nodes with

```
python reproducibility/fit_scale.py -a [IP of head node] -p yourpassword -o results_scale.csv
```

This will store the results in a csv file under `results_scale.csv`.

### Discovery latency versus network density

To reproduce Figure 6 in our paper, copy the `results_scale.csv` to your local machine and generate the plot with

```
python reproducibility/plot_density.py -i results_scale.csv
```
