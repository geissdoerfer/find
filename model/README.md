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