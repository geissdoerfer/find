import pandas as pd
import matplotlib.pyplot as plt
import numpy as np
import sys
from scipy.interpolate import interp1d
import click
from pathlib import Path

prop_cycle = plt.rcParams["axes.prop_cycle"]
colors = prop_cycle.by_key()["color"]


BASE_PATH = Path(__file__).resolve().parent


@click.command(
    short_help="Generates binary lookup table for optimized scale of geometric distro"
)
@click.option(
    "--input-path",
    "-i",
    type=click.Path(exists=True, dir_okay=False),
    default=str(BASE_PATH / "src" / "opt_scale.csv"),
    help="Path of csv with optimized scale parameters",
)
@click.option(
    "--output-path",
    "-o",
    type=click.Path(dir_okay=False),
    default=str(BASE_PATH / "_build" / "opt_scale.bin"),
    help="Output path for binary lookup table",
)
@click.pass_context
def build(ctx, input_path, output_path):

    df = pd.read_csv(input_path, index_col="t_chr")
    df.sort_index(inplace=True)
    df = df.reindex(np.arange(10, df.index[-1] - 30, 10))
    df.interpolate(inplace=True)

    table = df["x_opt"].values.astype(np.float32)
    table.tofile(Path(output_path))


if __name__ == "__main__":
    build()
