import numpy as np
import sys
import logging
import pickle
import matplotlib.pyplot as plt
from pathlib import Path
from scipy.optimize import minimize_scalar
from itertools import product

import ray
import pandas as pd
import click

from neslab.find import distributions as dists
from neslab.find import Model

logger = logging.getLogger("model")


def f_obj(scale, t_chr, n_nodes):
    m = Model(scale, "Geometric", t_chr, n_nodes, n_jobs=1)
    return m.disco_latency()


@ray.remote
def job(t_chr, n_nodes):
    scale_range = dists.Geometric.get_scale_range(t_chr)
    res = minimize_scalar(
        f_obj,
        bounds=scale_range,
        method="bounded",
        args=(t_chr, n_nodes),
    )
    log_entry = {
        "t_chr": t_chr,
        "n_nodes": n_nodes,
        "scale": res.x,
        "disco_latency": res.fun,
    }
    return log_entry


@click.command()
@click.option("--redis-password", "-p", type=str, default="pass")
@click.option("--head-address", "-a", type=str, default="auto")
@click.option(
    "--outfile",
    "-o",
    type=click.Path(dir_okay=False),
    help="Output file",
    default="results_scale.csv",
)
@click.option("-v", "--verbose", count=True, default=1)
def main(
    redis_password: str,
    head_address: str,
    outfile: click.Path,
    verbose,
):

    hnd = logging.StreamHandler()
    logger.addHandler(hnd)

    if verbose == 0:
        logger.setLevel(logging.ERROR)
    elif verbose == 1:
        logger.setLevel(logging.WARNING)
    elif verbose == 2:
        logger.setLevel(logging.INFO)
    elif verbose > 2:
        logger.setLevel(logging.DEBUG)

    ray.init(address=head_address, _redis_password=redis_password)

    # Configs for 2 nodes and different charging times
    args_tchrs = list(product(np.arange(5, 2500, 5), [2]))
    # Configs for charging time 25 and different numbers of nodes
    args_nnodes = list(product([25], np.arange(3, 110, 5)))

    futures = list()
    for arg in args_tchrs + args_nnodes:
        futures.append(job.remote(arg[0], arg[1]))

    logger.info(f"Running {len(futures)} jobs")
    results = ray.get(futures)

    df = pd.DataFrame(results)
    df.to_csv(outfile, index=False)


if __name__ == "__main__":
    main()
