#!/usr/bin/env python3
# /// script
# requires-python = ">=3.12"
# dependencies = ["pandas", "matplotlib"]
# ///
"""
analyze.py — Parse and visualize list_sort benchmark results.

Usage:
    # Single max_run_len:
    uv run python analyze.py results.csv [--output charts/]

    # Sweep across max_run_len values:
    uv run python analyze.py results_sweep_clean.csv --sweep [--output charts/]

Produces comparison count and time bar charts (SVG), plus sweep charts
showing how reduction varies with max_run_len.
"""

import argparse
import math
from pathlib import Path

import matplotlib
matplotlib.use("Agg")
import matplotlib.pyplot as plt
import numpy as np
import pandas as pd


def load_csv(path: Path) -> pd.DataFrame:
    return pd.read_csv(path)


def compute_summary(df: pd.DataFrame, extra_keys=None) -> pd.DataFrame:
    keys = ["variant", "pattern", "n"]
    if extra_keys:
        keys += extra_keys
    grouped = df.groupby(keys)[["comparisons", "time_ns"]].agg(["mean", "std"])
    grouped.columns = ["cmp_mean", "cmp_std", "time_mean", "time_std"]
    return grouped.reset_index()


def plot_metric(summary: pd.DataFrame, metric: str, ylabel: str,
                title_prefix: str, output_dir: Path, suffix: str,
                scale: float = 1.0) -> None:
    """Bar chart: vanilla vs runs, grouped by pattern, one chart per n."""
    patterns = summary["pattern"].unique()
    n_values = sorted(summary["n"].unique())

    for n_val in n_values:
        fig, ax = plt.subplots(figsize=(10, 6))
        sub = summary[summary["n"] == n_val]

        x = np.arange(len(patterns))
        width = 0.35

        for i, variant in enumerate(["vanilla", "runs"]):
            vs = sub[sub["variant"] == variant].set_index("pattern")
            vals = [vs.loc[p, f"{metric}_mean"] * scale
                    if p in vs.index else 0 for p in patterns]
            errs = [vs.loc[p, f"{metric}_std"] * scale
                    if p in vs.index else 0 for p in patterns]
            offset = (i - 0.5) * width
            ax.bar(x + offset, vals, width, yerr=errs,
                   label=variant, capsize=3)

        if metric == "cmp":
            nlogn = n_val * math.log2(n_val)
            ax.axhline(y=nlogn, color="gray", linestyle="--", alpha=0.5,
                       label=f"n·log₂(n) = {nlogn:.0f}")

        ax.set_xlabel("Pattern")
        ax.set_ylabel(ylabel)
        ax.set_title(f"{title_prefix} (n={n_val})")
        ax.set_xticks(x)
        ax.set_xticklabels(patterns, rotation=30, ha="right")
        ax.legend()
        ax.grid(axis="y", alpha=0.3)

        fig.tight_layout()
        fname = f"{suffix}_n{n_val}.svg"
        fig.savefig(output_dir / fname)
        plt.close(fig)
        print(f"  Saved {fname}")


def plot_combined(summary: pd.DataFrame, output_dir: Path) -> None:
    """Single chart with all sizes, showing % reduction in comparisons."""
    patterns = summary["pattern"].unique()
    n_values = sorted(summary["n"].unique())

    fig, ax = plt.subplots(figsize=(12, 6))
    x = np.arange(len(patterns))
    width = 0.8 / len(n_values)

    for i, n_val in enumerate(n_values):
        reductions = []
        for p in patterns:
            van = summary[(summary["variant"] == "vanilla") &
                          (summary["pattern"] == p) &
                          (summary["n"] == n_val)]
            run = summary[(summary["variant"] == "runs") &
                          (summary["pattern"] == p) &
                          (summary["n"] == n_val)]
            if van.empty or run.empty:
                reductions.append(0)
            else:
                v = van.iloc[0]["cmp_mean"]
                r = run.iloc[0]["cmp_mean"]
                reductions.append((1 - r / v) * 100)

        offset = (i - len(n_values) / 2 + 0.5) * width
        ax.bar(x + offset, reductions, width, label=f"n={n_val}")

    ax.axhline(y=0, color="black", linewidth=0.5)
    ax.set_xlabel("Pattern")
    ax.set_ylabel("Comparison reduction (%)")
    ax.set_title("Natural Run Detection: comparison reduction vs vanilla list_sort")
    ax.set_xticks(x)
    ax.set_xticklabels(patterns, rotation=30, ha="right")
    ax.legend(title="List size")
    ax.grid(axis="y", alpha=0.3)

    fig.tight_layout()
    fig.savefig(output_dir / "reduction_summary.svg")
    plt.close(fig)
    print("  Saved reduction_summary.svg")


# ---- Sweep analysis: how reduction varies with max_run_len ----

def plot_sweep_reduction(summary: pd.DataFrame, output_dir: Path) -> None:
    """Line chart: comparison reduction (%) vs max_run_len, one line per n.

    One subplot per pattern (excluding full_random).
    """
    patterns = [p for p in summary["pattern"].unique()
                if p != "full_random"]
    mrl_values = sorted(summary["max_run_len"].unique())
    n_values = sorted(summary["n"].unique())

    fig, axes = plt.subplots(1, len(patterns), figsize=(6 * len(patterns), 5),
                             sharey=True)
    if len(patterns) == 1:
        axes = [axes]

    for ax, pat in zip(axes, patterns):
        for n_val in n_values:
            reductions = []
            for mrl in mrl_values:
                van = summary[(summary["variant"] == "vanilla") &
                              (summary["pattern"] == pat) &
                              (summary["n"] == n_val) &
                              (summary["max_run_len"] == mrl)]
                run = summary[(summary["variant"] == "runs") &
                              (summary["pattern"] == pat) &
                              (summary["n"] == n_val) &
                              (summary["max_run_len"] == mrl)]
                if van.empty or run.empty:
                    reductions.append(0)
                else:
                    v = van.iloc[0]["cmp_mean"]
                    r = run.iloc[0]["cmp_mean"]
                    reductions.append((1 - r / v) * 100)

            ax.plot(mrl_values, reductions, "o-", label=f"n={n_val}")

        ax.set_title(pat)
        ax.set_xlabel("max_run_len")
        ax.set_xscale("log", base=2)
        ax.set_xticks(mrl_values)
        ax.set_xticklabels([str(v) for v in mrl_values])
        ax.grid(alpha=0.3)

    axes[0].set_ylabel("Comparison reduction (%)")
    axes[-1].legend(title="n", bbox_to_anchor=(1.02, 1), loc="upper left")
    fig.suptitle("Run detection benefit vs max_run_len", fontsize=14, y=1.02)
    fig.tight_layout()
    fig.savefig(output_dir / "sweep_reduction.svg", bbox_inches="tight")
    plt.close(fig)
    print("  Saved sweep_reduction.svg")


def plot_sweep_time(summary: pd.DataFrame, output_dir: Path) -> None:
    """Line chart: time ratio (runs/vanilla) vs max_run_len."""
    patterns = [p for p in summary["pattern"].unique()
                if p != "full_random"]
    mrl_values = sorted(summary["max_run_len"].unique())
    n_values = sorted(summary["n"].unique())

    fig, axes = plt.subplots(1, len(patterns), figsize=(6 * len(patterns), 5),
                             sharey=True)
    if len(patterns) == 1:
        axes = [axes]

    for ax, pat in zip(axes, patterns):
        for n_val in n_values:
            ratios = []
            for mrl in mrl_values:
                van = summary[(summary["variant"] == "vanilla") &
                              (summary["pattern"] == pat) &
                              (summary["n"] == n_val) &
                              (summary["max_run_len"] == mrl)]
                run = summary[(summary["variant"] == "runs") &
                              (summary["pattern"] == pat) &
                              (summary["n"] == n_val) &
                              (summary["max_run_len"] == mrl)]
                if van.empty or run.empty:
                    ratios.append(1.0)
                else:
                    ratios.append(run.iloc[0]["time_mean"] /
                                  van.iloc[0]["time_mean"])

            ax.plot(mrl_values, ratios, "o-", label=f"n={n_val}")

        ax.axhline(y=1.0, color="black", linewidth=0.5, linestyle="--")
        ax.set_title(pat)
        ax.set_xlabel("max_run_len")
        ax.set_xscale("log", base=2)
        ax.set_xticks(mrl_values)
        ax.set_xticklabels([str(v) for v in mrl_values])
        ax.grid(alpha=0.3)

    axes[0].set_ylabel("Time ratio (runs / vanilla)")
    axes[-1].legend(title="n", bbox_to_anchor=(1.02, 1), loc="upper left")
    fig.suptitle("Execution time ratio vs max_run_len (< 1 = faster)",
                 fontsize=14, y=1.02)
    fig.tight_layout()
    fig.savefig(output_dir / "sweep_time_ratio.svg", bbox_inches="tight")
    plt.close(fig)
    print("  Saved sweep_time_ratio.svg")


def plot_sweep_random_overhead(summary: pd.DataFrame,
                               output_dir: Path) -> None:
    """Bar chart: full_random overhead (%) for each max_run_len and n."""
    mrl_values = sorted(summary["max_run_len"].unique())
    n_values = sorted(summary["n"].unique())

    fig, ax = plt.subplots(figsize=(10, 5))
    x = np.arange(len(mrl_values))
    width = 0.8 / len(n_values)

    for i, n_val in enumerate(n_values):
        overheads = []
        for mrl in mrl_values:
            van = summary[(summary["variant"] == "vanilla") &
                          (summary["pattern"] == "full_random") &
                          (summary["n"] == n_val) &
                          (summary["max_run_len"] == mrl)]
            run = summary[(summary["variant"] == "runs") &
                          (summary["pattern"] == "full_random") &
                          (summary["n"] == n_val) &
                          (summary["max_run_len"] == mrl)]
            if van.empty or run.empty:
                overheads.append(0)
            else:
                v = van.iloc[0]["cmp_mean"]
                r = run.iloc[0]["cmp_mean"]
                overheads.append((r / v - 1) * 100)

        offset = (i - len(n_values) / 2 + 0.5) * width
        ax.bar(x + offset, overheads, width, label=f"n={n_val}")

    ax.axhline(y=0, color="black", linewidth=0.5)
    ax.set_xlabel("max_run_len")
    ax.set_ylabel("Comparison overhead (%)")
    ax.set_title("full_random: overhead of run detection (higher = worse)")
    ax.set_xticks(x)
    ax.set_xticklabels([str(v) for v in mrl_values])
    ax.legend(title="n")
    ax.grid(axis="y", alpha=0.3)

    fig.tight_layout()
    fig.savefig(output_dir / "sweep_random_overhead.svg")
    plt.close(fig)
    print("  Saved sweep_random_overhead.svg")


def print_summary_table(summary: pd.DataFrame,
                        mrl_col: bool = False) -> None:
    """Print text summary of comparison counts and reductions."""
    patterns = summary["pattern"].unique()
    n_values = sorted(summary["n"].unique())

    if mrl_col:
        mrl_values = sorted(summary["max_run_len"].unique())
    else:
        mrl_values = [None]

    print("\n=== Comparison Count Summary ===")
    hdr = f"{'pattern':<22} {'n':>7}"
    if mrl_col:
        hdr += f" {'mrl':>5}"
    hdr += f" {'vanilla':>10} {'runs':>10} {'reduction':>10} {'n·lg(n)':>10}"
    print(hdr)
    print("-" * len(hdr))

    for p in patterns:
        for n_val in n_values:
            for mrl in mrl_values:
                filt_v = ((summary["variant"] == "vanilla") &
                          (summary["pattern"] == p) &
                          (summary["n"] == n_val))
                filt_r = ((summary["variant"] == "runs") &
                          (summary["pattern"] == p) &
                          (summary["n"] == n_val))
                if mrl is not None:
                    filt_v = filt_v & (summary["max_run_len"] == mrl)
                    filt_r = filt_r & (summary["max_run_len"] == mrl)

                van = summary[filt_v]
                run = summary[filt_r]
                if van.empty or run.empty:
                    continue
                v = van.iloc[0]["cmp_mean"]
                r = run.iloc[0]["cmp_mean"]
                red = (1 - r / v) * 100
                nlogn = n_val * math.log2(n_val)
                line = f"{p:<22} {n_val:>7}"
                if mrl is not None:
                    line += f" {mrl:>5}"
                line += (f" {v:>10.0f} {r:>10.0f} "
                         f"{red:>9.1f}% {nlogn:>10.0f}")
                print(line)


def main():
    parser = argparse.ArgumentParser(
        description="Analyze list_sort benchmark results")
    parser.add_argument("csv", type=Path, help="Path to results CSV")
    parser.add_argument("--output", type=Path, default=None,
                        help="Output directory for charts")
    parser.add_argument("--sweep", action="store_true",
                        help="Enable sweep analysis (multiple max_run_len)")
    args = parser.parse_args()

    if args.output is None:
        args.output = args.csv.parent
    args.output.mkdir(parents=True, exist_ok=True)

    print("Loading data...")
    df = load_csv(args.csv)
    print(f"  {len(df)} rows loaded")

    if args.sweep:
        summary = compute_summary(df, extra_keys=["max_run_len"])

        print("\nGenerating sweep reduction chart...")
        plot_sweep_reduction(summary, args.output)

        print("\nGenerating sweep time ratio chart...")
        plot_sweep_time(summary, args.output)

        print("\nGenerating random overhead chart...")
        plot_sweep_random_overhead(summary, args.output)

        print_summary_table(summary, mrl_col=True)
    else:
        summary = compute_summary(df)

        print("\nGenerating comparison charts...")
        plot_metric(summary, "cmp", "Comparisons",
                    "Comparison Count: vanilla vs runs", args.output, "cmp")

        print("\nGenerating time charts...")
        plot_metric(summary, "time", "Time (µs)",
                    "Wall-clock Time: vanilla vs runs", args.output, "time",
                    scale=1e-3)

        print("\nGenerating reduction summary...")
        plot_combined(summary, args.output)

        print_summary_table(summary)

    print(f"\nAll charts saved to {args.output}/")


if __name__ == "__main__":
    main()
