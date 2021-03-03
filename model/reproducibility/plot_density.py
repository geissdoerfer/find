import numpy as np
import matplotlib.pyplot as plt
import pandas as pd
import click

from neslab.find import Model
from neslab.find import distributions


@click.command()
@click.option(
    "--input-file",
    "-i",
    type=click.Path(exists=True),
    help="csv file with results",
    required=True,
)
def cli(input_file):
    df = pd.read_csv(input_file)
    t_chr = df["t_chr"].iat[0]

    for tag in df["tag"].unique():
        df_slice = df[df["tag"] == tag]
        df_slice = df_slice.sort_values("n_nodes")
        plt.plot(df_slice["n_nodes"] / t_chr, df_slice["disco_latency"], label=tag)

    plt.xlabel("Network density")
    plt.ylabel("Discovery Latency [slots]")
    plt.legend()
    plt.show()


if __name__ == "__main__":
    cli()