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


@ray.remote
def job(dist_scale, dist_name, t_chr):
    m = Model(dist_scale, dist_name, t_chr, n_slots=250000, n_jobs=1)
    lat = m.disco_latency()
    log_entry = {
        "dist_scale": dist_scale,
        "dist_name": dist_name,
        "t_chr": t_chr,
        "disco_latency": lat,
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
    default="results_dists.csv",
)
@click.option("--charging-time", "-t", type=int, default=100)
@click.option("--n-points", "-n", type=int, default=100)
@click.option("-v", "--verbose", count=True, default=1)
def main(
    redis_password: str,
    head_address: str,
    outfile: click.Path,
    charging_time,
    n_points,
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

    futures = list()
    for dist_name in ["Uniform", "Poisson", "Geometric"]:
        for scale in getattr(dists, dist_name).get_scale_range(charging_time, n_points):
            futures.append(job.remote(scale, dist_name, charging_time))

    logger.info(f"Running {len(futures)} jobs")
    results = ray.get(futures)

    df = pd.DataFrame(results)
    df.to_csv(outfile, index=False)


if __name__ == "__main__":
    main()
