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
def job(scale, t_chr, n_nodes, tag):
    m = Model(scale, "Geometric", t_chr, n_nodes, n_jobs=1)
    lat = m.disco_latency()

    log_entry = {"t_chr": t_chr, "n_nodes": n_nodes, "disco_latency": lat, "tag": tag}
    return log_entry


@click.command()
@click.option("--redis-password", "-p", type=str, default="pass")
@click.option("--head-address", "-a", type=str, default="auto")
@click.option(
    "--infile",
    "-i",
    type=click.Path(exists=True),
    help="File with fitted scale parameters",
    default="results_scale.csv",
)
@click.option(
    "--outfile",
    "-o",
    type=click.Path(dir_okay=False),
    help="Output file",
    default="results_density.csv",
)
@click.option("-v", "--verbose", count=True, default=1)
def main(
    redis_password: str,
    head_address: str,
    infile: click.Path,
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

    df = pd.read_csv(infile)

    t_chr = 25
    df = df[df["t_chr"] == t_chr]

    # scale parameter optimized for two nodes
    scale_2nodes = df[df["n_nodes"] == 2]["scale"].iat[0]
    # scale parameter optimized for density rho=1
    scale_rho1 = df[df["n_nodes"] == t_chr]["scale"].iat[0]

    futures = list()
    for n_nodes in df["n_nodes"].unique():
        # scale parameter optimized for real density
        scale_clairvoyant = df[df["n_nodes"] == n_nodes]["scale"].iat[0]

        futures.append(job.remote(scale_2nodes, t_chr, n_nodes, "2nodes"))
        futures.append(job.remote(scale_rho1, t_chr, n_nodes, "rho1"))
        futures.append(job.remote(scale_clairvoyant, t_chr, n_nodes, "clairvoyant"))

    logger.info(f"Running {len(futures)} jobs")
    results = ray.get(futures)

    df = pd.DataFrame(results)
    df.to_csv(outfile, index=False)


if __name__ == "__main__":
    main()
