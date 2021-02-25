import numpy as np
import logging
from typing import Union
from typing import Iterable
import multiprocessing

from itertools import combinations
import warnings

from . import distributions as dists

logger = logging.getLogger("model")
logger.setLevel(logging.DEBUG)

np.errstate(all="raise")
warnings.simplefilter("error", RuntimeWarning)


class ThresholdException(Exception):
    pass


def expected_value(pmf: np.array):
    """Calculates expected value from pmf

    Args:
        pmf (np.array): pmf of random variable.

    Returns:
        float: Expected value

    """
    return np.sum(pmf * np.arange(len(pmf)))


def act2rend(activities: np.ndarray):
    """Calculates probability of rendezvous for given probability of acitivities

    Takes the probability of activity of all nodes in a clique and calculates the
    probability of a successful rendezvous for each link at each slot.

    Args:
        activities (np.ndarray): Shape (n, m) array with n slots and m nodes

    Returns:
        np.ndarray: Shape (n, l) array with probability for rendezvous in n slots and l links
    """
    node_ids = range(activities.shape[1])
    links = list(combinations(node_ids, 2))
    p_rendz = np.empty((activities.shape[0], len(links)))
    for i, link in enumerate(links):
        others = list(set(node_ids) - set(link))
        # probability that the two 'link' nodes are active at the same time
        p_sim_on = np.product(activities[:, link], axis=1)
        # probability that none of the other nodes is active
        p_no_coll = np.product(1.0 - activities[:, others], axis=1)
        p_rendz[:, i] = p_sim_on * p_no_coll
    return p_rendz


def p_act(scale: float, dist_name: str, t_chr: int, n_slots: int = 100000):
    """Calculates probability of activity for given distribution and charging time

    Args:
        scale (float): scale parameter for distribution.
        dist_name (str): Name of probability distribution.
        t_chr (int): charging times (int or iterable).
        n_slots (int): Number of slots.
    """
    dist_class = getattr(dists, dist_name.lower().capitalize())
    dist = dist_class(scale)
    tot_support = t_chr + dist.min_support()
    if n_slots < tot_support:
        raise ValueError("Number of slots must be longer than one wakeup period")

    n_wkups = int(n_slots / tot_support) - 1
    logger.debug(f"Calculating {n_wkups} wakeups")

    p_act_arr = np.zeros((n_slots,))
    for i, pmf_wkup in enumerate(dist.pmf_nsum(n_wkups)):
        p_act_arr[i * t_chr : i * t_chr + len(pmf_wkup)] += pmf_wkup
        if i * t_chr > 10 * tot_support:
            ts_end = i * t_chr
            ts_start = int(max(0, ts_end - 10 * tot_support))
            if np.std(p_act_arr[ts_start:ts_end]) < 1e-9:
                logger.debug("Probability converged! fast-forwarding...")
                p_act_arr[ts_start:] = p_act_arr[ts_start]
                return p_act_arr

    return p_act_arr


class Model(object):
    def __init__(
        self,
        scale: Union[float, Iterable],
        dist_name: str,
        t_chr: Union[int, Iterable],
        n_nodes: int = None,
        offset: Union[int, Iterable] = None,
        n_slots: int = 100000,
        n_jobs: int = None,
    ):
        if n_nodes is None:
            if isinstance(t_chr, Iterable):
                self.n_nodes = len(t_chr)
            else:
                self.n_nodes = 2
        else:
            self.n_nodes = n_nodes

        self.n_slots = n_slots
        if n_jobs is None:
            self.n_jobs = multiprocessing.cpu_count()
        else:
            self.n_jobs = n_jobs

        self._activities = self._calc_activities(scale, dist_name, t_chr, offset)

    def _calc_activities(
        self,
        scale: Union[float, Iterable],
        dist_name: str,
        t_chr: Union[int, Iterable],
        offset: Union[int, Iterable] = None,
    ):

        activities = np.empty((self.n_slots, self.n_nodes))
        if isinstance(t_chr, Iterable) or isinstance(scale, Iterable):
            if offset is None:
                raise ValueError(
                    "Can't estimate worst-case offset for different scale/t_chr"
                )

            if isinstance(t_chr, Iterable):
                if len(t_chr) != self.n_nodes:
                    raise ValueError("Number of t_chrs must match number of nodes")
            else:
                t_chr = [t_chr for _ in range(self.n_nodes)]

            if isinstance(scale, Iterable):
                if len(scale) != self.n_nodes:
                    raise ValueError("Number of t_chrs must match number of nodes")
            else:
                scale = [scale for _ in range(self.n_nodes)]

            for i in range(self.n_nodes):
                activities[:, i] = p_act(scale[i], dist_name, t_chr[i], self.n_slots)

        else:
            activity = p_act(scale, dist_name, t_chr, self.n_slots)
            for i in range(self.n_nodes):
                activities[:, i] = activity.copy()

        if offset is not None and not isinstance(offset, Iterable):
            if offset == 0:
                return activities
            if self.n_nodes != 2:
                raise ValueError(
                    "Scalar offset does not make sense with more than two nodes"
                )

            offset = [0, offset]

        if isinstance(offset, Iterable):
            if len(offset) != self.n_nodes:
                raise ValueError("Number of offsets must match number of nodes")
        elif offset is None:
            dist_class = getattr(dists, dist_name.lower().capitalize())

            distance = t_chr + 2 * dist_class(scale).expectation()
            offset = np.zeros((self.n_nodes,), dtype=int)
            for i in range(self.n_nodes):
                offset[i] = int(np.round(i * (distance / self.n_nodes)))

        max_offset = max(offset)
        activities_cut = np.empty(
            (activities.shape[0] - max_offset - 1, activities.shape[1])
        )
        for i, os in enumerate(offset):
            activities_cut[:, i] = activities[os : -(max_offset - os + 1), i]

        return activities_cut

    def activity(self):
        return self._activities

    def cdf(self):
        """Calculates cdf of discovery for given probability of acitivities

        Takes the probability of activity of all nodes in a clique and calculates the
        cdf of a successful discovery for each link at each slot. Allows to split
        calculations of rendezvous probability in n_jobs partitions to enable
        concurrent calculations on multiple CPUs.

        Args:
            thr_valid (float): Minimum probability convergence criterion

        Returns:
            np.ndarray: Shape (n, l) array with cdf for rendezvous in n slots and l links
        """
        if self.n_jobs == 1:
            p_rendz = act2rend(self._activities)
        else:
            partition_size = self.n_slots // self.n_jobs
            args = list()
            for i in range(self.n_jobs - 1):
                idx_start = i * partition_size
                idx_end = (i + 1) * partition_size
                args.append((self._activities[idx_start:idx_end],))

            idx_start = (self.n_jobs - 1) * partition_size
            idx_end = self.n_slots
            args.append((self._activities[idx_start:idx_end],))
            with multiprocessing.Pool(self.n_jobs) as p:
                logger.debug(f"Calculating rendezvous with {self.n_jobs} jobs")
                results = p.starmap(act2rend, args)

            p_rendz = np.concatenate(results, axis=0)

        cdfs = 1.0 - np.cumprod(1.0 - p_rendz, axis=0)
        return cdfs

    def links(self):
        node_ids = range(self.n_nodes)
        return list(combinations(node_ids, 2))

    def disco_frac(self, thr_valid: float = 0.975):
        cdfs = self.cdf()
        cdf = np.sum(cdfs, axis=1) / cdfs.shape[1]
        if cdf[-1] < thr_valid:
            logger.warning(f"Not converged: {cdf[-1]:.2f}")
            warnings.warn(f"Not converged: {cdf[-1]:.2f}")

        return cdf

    def disco_quant(self, q: float):
        """Time to first rendezvous with given probability

        Calculates the number of slots until the first successful rendezvous happens with
        a given probability.

        Args:
            q (float): Probability with which rendezvous should happen

        Returns:
            int: Slot at which probability for discovery crosses threshold.
        """
        cdf = self.disco_frac(q)
        return np.argmax(cdf >= q)

    def disco_latency(self):
        """Number of slots until discovery"""
        dfrac = self.disco_frac()
        pmf = np.diff(dfrac)
        return np.sum(pmf * np.arange(len(pmf)))
