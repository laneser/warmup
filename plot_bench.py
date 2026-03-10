# /// script
# requires-python = ">=3.12"
# dependencies = ["matplotlib"]
# ///
"""
Plot linked list vs array insert benchmark results from CSV.

Single machine:
  uv run plot_bench.py <csv> <outdir>

Multi-machine comparison:
  uv run plot_bench.py <outdir> --data label1:csv1 label2:csv2 ...

Multi-element-size comparison (one machine):
  uv run plot_bench.py <outdir> --sizes csv_4:csv_32:csv_128

Multi-machine × multi-element-size (grid layout):
  uv run plot_bench.py <outdir> --multi-sizes "ARM:a4,a32,a128" "x86:x4,x32,x128"

Options:
  --title "Custom title"   Override auto-detected suptitle
"""

import csv
import platform
import subprocess
import sys
from pathlib import Path

import matplotlib
matplotlib.use("Agg")
import matplotlib.pyplot as plt
import matplotlib.ticker as ticker


def detect_cpu():
    """Auto-detect CPU model name from the local machine."""
    system = platform.system()
    try:
        if system == "Linux":
            with open("/proc/cpuinfo") as f:
                for line in f:
                    if line.startswith("model name") or \
                       line.startswith("Model name"):
                        return line.split(":", 1)[1].strip()
            result = subprocess.run(
                ["lscpu"], capture_output=True, text=True, timeout=5
            )
            for line in result.stdout.splitlines():
                if line.startswith("Model name:"):
                    return line.split(":", 1)[1].strip()
        elif system == "Darwin":
            result = subprocess.run(
                ["sysctl", "-n", "machdep.cpu.brand_string"],
                capture_output=True, text=True, timeout=5,
            )
            return result.stdout.strip()
    except (OSError, subprocess.TimeoutExpired):
        pass
    return platform.processor() or "Unknown CPU"


def read_csv(path):
    """Read benchmark CSV into dict keyed by mode."""
    data = {"head": [], "tail": [], "random": []}
    with open(path) as f:
        reader = csv.DictReader(f)
        for row in reader:
            mode = row["mode"]
            entry = {
                "n": int(row["n"]),
                "ll_per_op": float(row["ll_per_op"]),
                "da_per_op": float(row["da_per_op"]),
            }
            if "elem_size" in row:
                entry["elem_size"] = int(row["elem_size"])
            data[mode].append(entry)
    return data


# Color pairs: (linked list, array) for each dataset
PALETTE = [
    ("#e74c3c", "#2980b9"),   # red / blue
    ("#e67e22", "#27ae60"),   # orange / green
    ("#9b59b6", "#1abc9c"),   # purple / teal
    ("#d35400", "#2c3e50"),   # dark orange / dark blue
]

MARKERS_LL = ["o", "^", "D", "v"]
MARKERS_DA = ["s", "P", "X", "h"]

MODE_LABELS = {
    "head": "Head insert (i = 0)",
    "tail": "Tail insert (i = n)",
    "random": "Random insert",
}


def _setup_ax(ax, mode):
    ax.set_xscale("log")
    ax.set_yscale("log")
    ax.set_xlabel("n (elements)", fontsize=11)
    if mode == "head":
        ax.set_ylabel("Cost per insert (ns)", fontsize=11)
    ax.set_title(MODE_LABELS[mode], fontsize=12)
    ax.grid(True, which="both", alpha=0.3)
    ax.xaxis.set_major_formatter(ticker.FuncFormatter(
        lambda x, _: f"{int(x):,}" if x >= 1 else str(x)
    ))


def plot_single(data, outdir, title):
    """Plot one machine: 3 subplots, LL vs Array."""
    fig, axes = plt.subplots(1, 3, figsize=(16, 5))

    for ax, mode in zip(axes, ["head", "tail", "random"]):
        rows = data[mode]
        ns = [r["n"] for r in rows]
        ax.plot(ns, [r["ll_per_op"] for r in rows], "o-",
                label="Linked list", color=PALETTE[0][0],
                markersize=4, linewidth=1.5)
        ax.plot(ns, [r["da_per_op"] for r in rows], "s-",
                label="Array (doubling)", color=PALETTE[0][1],
                markersize=4, linewidth=1.5)
        _setup_ax(ax, mode)
        ax.legend(fontsize=9, loc="upper left")

    fig.suptitle(f"Linked List vs Array Insert — {title}",
                 fontsize=14, y=1.02)
    fig.tight_layout()
    outpath = Path(outdir) / "bench_combined.svg"
    fig.savefig(outpath, format="svg", bbox_inches="tight")
    plt.close(fig)
    print(f"Saved: {outpath}")


def plot_multi(datasets, outdir, title):
    """Plot multiple machines: 3 subplots, each with LL+Array per machine."""
    fig, axes = plt.subplots(1, 3, figsize=(16, 5.5))

    for ax, mode in zip(axes, ["head", "tail", "random"]):
        for i, (label, data) in enumerate(datasets):
            rows = data[mode]
            ns = [r["n"] for r in rows]
            c_ll, c_da = PALETTE[i % len(PALETTE)]
            ax.plot(ns, [r["ll_per_op"] for r in rows],
                    linestyle="-", marker=MARKERS_LL[i % len(MARKERS_LL)],
                    label=f"{label} — LL", color=c_ll,
                    markersize=4, linewidth=1.5)
            ax.plot(ns, [r["da_per_op"] for r in rows],
                    linestyle="--", marker=MARKERS_DA[i % len(MARKERS_DA)],
                    label=f"{label} — Array", color=c_da,
                    markersize=4, linewidth=1.5)
        _setup_ax(ax, mode)
        ax.legend(fontsize=7, loc="upper left", ncol=1)

    fig.suptitle(f"Linked List vs Array Insert — {title}",
                 fontsize=14, y=1.02)
    fig.tight_layout()
    outpath = Path(outdir) / "bench_combined.svg"
    fig.savefig(outpath, format="svg", bbox_inches="tight")
    plt.close(fig)
    print(f"Saved: {outpath}")


def plot_sizes(size_datasets, outdir, title):
    """Plot element size comparison: 3 subplots, LL+Array per size."""
    fig, axes = plt.subplots(1, 3, figsize=(16, 5.5))

    for ax, mode in zip(axes, ["head", "tail", "random"]):
        for i, (size_label, data) in enumerate(size_datasets):
            rows = data[mode]
            ns = [r["n"] for r in rows]
            c_ll, c_da = PALETTE[i % len(PALETTE)]
            ax.plot(ns, [r["ll_per_op"] for r in rows],
                    linestyle="-", marker=MARKERS_LL[i % len(MARKERS_LL)],
                    label=f"{size_label} — LL", color=c_ll,
                    markersize=4, linewidth=1.5)
            ax.plot(ns, [r["da_per_op"] for r in rows],
                    linestyle="--", marker=MARKERS_DA[i % len(MARKERS_DA)],
                    label=f"{size_label} — Array", color=c_da,
                    markersize=4, linewidth=1.5)
        _setup_ax(ax, mode)
        ax.legend(fontsize=7, loc="upper left", ncol=1)

    fig.suptitle(f"Linked List vs Array Insert — {title}",
                 fontsize=14, y=1.02)
    fig.tight_layout()
    outpath = Path(outdir) / "bench_sizes.svg"
    fig.savefig(outpath, format="svg", bbox_inches="tight")
    plt.close(fig)
    print(f"Saved: {outpath}")


def plot_multi_sizes(machine_datasets, outdir, title):
    """Plot element size comparison across multiple machines.

    machine_datasets: list of (machine_label, [(size_label, data), ...])
    Layout: rows = machines, columns = head/tail/random.
    """
    n_machines = len(machine_datasets)
    fig, axes = plt.subplots(n_machines, 3, figsize=(16, 4.5 * n_machines),
                             squeeze=False)

    for row, (machine_label, size_datasets) in enumerate(machine_datasets):
        for col, mode in enumerate(["head", "tail", "random"]):
            ax = axes[row][col]
            for i, (size_label, data) in enumerate(size_datasets):
                rows = data[mode]
                ns = [r["n"] for r in rows]
                c_ll, c_da = PALETTE[i % len(PALETTE)]
                ax.plot(ns, [r["ll_per_op"] for r in rows],
                        linestyle="-", marker=MARKERS_LL[i % len(MARKERS_LL)],
                        label=f"{size_label} — LL", color=c_ll,
                        markersize=4, linewidth=1.5)
                ax.plot(ns, [r["da_per_op"] for r in rows],
                        linestyle="--", marker=MARKERS_DA[i % len(MARKERS_DA)],
                        label=f"{size_label} — Array", color=c_da,
                        markersize=4, linewidth=1.5)
            _setup_ax(ax, mode)
            if col == 0:
                ax.set_ylabel(f"{machine_label}\nCost per insert (ns)",
                              fontsize=11)
            ax.legend(fontsize=7, loc="upper left", ncol=1)

    fig.suptitle(f"Linked List vs Array Insert — {title}",
                 fontsize=14, y=1.01)
    fig.tight_layout()
    outpath = Path(outdir) / "bench_sizes.svg"
    fig.savefig(outpath, format="svg", bbox_inches="tight")
    plt.close(fig)
    print(f"Saved: {outpath}")


def read_qsort_csv(path):
    """Read quicksort per-trial CSV into lists of nanosecond values."""
    data = {}
    with open(path) as f:
        reader = csv.DictReader(f)
        for col in reader.fieldnames:
            if col != "trial":
                data[col] = []
        for row in reader:
            for col in data:
                data[col].append(float(row[col]))
    return data


QSORT_COLORS = {
    "original_ns": "#2980b9",    # blue
    "tco_ns":      "#e74c3c",    # red
    "method2_ns":  "#27ae60",    # green
    "iterative_ns":"#e67e22",    # orange
}

QSORT_LABELS = {
    "original_ns": "original (recursive)",
    "tco_ns":      "linD026 tco",
    "method2_ns":  "method2 (recurse smaller)",
    "iterative_ns":"iterative (explicit stack)",
}


def _plot_qsort_ax(ax, data, opt_label, n_bins=100):
    """Plot one quicksort subplot: bin-averaged per-trial time."""
    import statistics

    all_vals = []
    for col in data:
        all_vals.extend(data[col])
    all_vals.sort()
    ylim = all_vals[int(len(all_vals) * 0.95)] * 1.15

    n_trials = len(next(iter(data.values())))
    bin_size = max(1, n_trials // n_bins)
    actual_bins = (n_trials + bin_size - 1) // bin_size

    for col in data:
        vals = data[col]
        binned = []
        for b in range(actual_bins):
            chunk = vals[b * bin_size : (b + 1) * bin_size]
            binned.append(statistics.mean(chunk))
        xs = list(range(actual_bins))

        import statistics as _st
        color = QSORT_COLORS.get(col, "#333333")
        label = QSORT_LABELS.get(col, col)
        median = _st.median(vals)
        label_with_median = f"{label} (med {median/1e6:.2f} ms)"
        ax.plot(xs, binned, "-", color=color,
                label=label_with_median, linewidth=1.2, alpha=0.85)

    ax.set_xlabel(f"Bin (each = {bin_size} trials)", fontsize=10)
    ax.set_ylabel("Time (ns)", fontsize=10)
    ax.set_title(f"{opt_label} — {n_trials} trials", fontsize=11)
    ax.set_ylim(0, ylim)
    ax.grid(True, alpha=0.3)
    ax.legend(fontsize=7, loc="upper right")


def plot_qsort(csv_O0, csv_O3, outdir, title):
    """Plot quicksort benchmark: two subplots (-O0 vs -O3), per-trial time."""
    data_O0 = read_qsort_csv(csv_O0)
    data_O3 = read_qsort_csv(csv_O3)

    fig, axes = plt.subplots(1, 2, figsize=(14, 5))
    _plot_qsort_ax(axes[0], data_O0, "-O0 (no optimization)")
    _plot_qsort_ax(axes[1], data_O3, "-O3")

    fig.suptitle(f"Quicksort Variants — {title}", fontsize=14, y=1.02)
    fig.tight_layout()
    outpath = Path(outdir) / "qsort_bench.svg"
    fig.savefig(outpath, format="svg", bbox_inches="tight")
    plt.close(fig)
    print(f"Saved: {outpath}")


def plot_qsort_multi(machines, outdir, title):
    """Plot quicksort benchmark across multiple machines.

    machines: list of (label, csv_O0_path, csv_O3_path)
    Layout: rows = machines, columns = -O0 / -O3.
    """
    n_machines = len(machines)
    fig, axes = plt.subplots(n_machines, 2,
                             figsize=(14, 4.2 * n_machines),
                             squeeze=False)

    for row, (label, csv_O0, csv_O3) in enumerate(machines):
        data_O0 = read_qsort_csv(csv_O0)
        data_O3 = read_qsort_csv(csv_O3)
        _plot_qsort_ax(axes[row][0], data_O0, f"{label} — -O0")
        _plot_qsort_ax(axes[row][1], data_O3, f"{label} — -O3")

    fig.suptitle(f"Quicksort Variants — {title}", fontsize=14, y=1.01)
    fig.tight_layout()
    outpath = Path(outdir) / "qsort_bench.svg"
    fig.savefig(outpath, format="svg", bbox_inches="tight")
    plt.close(fig)
    print(f"Saved: {outpath}")


def parse_args(argv):
    """Parse command-line arguments."""
    title = None
    data_args = []
    sizes_arg = None
    multi_sizes_args = []
    qsort_mode = False
    positional = []
    i = 1
    while i < len(argv):
        if argv[i] == "--title" and i + 1 < len(argv):
            title = argv[i + 1]
            i += 2
        elif argv[i] == "--data":
            i += 1
            while i < len(argv) and not argv[i].startswith("--"):
                data_args.append(argv[i])
                i += 1
        elif argv[i] == "--sizes" and i + 1 < len(argv):
            sizes_arg = argv[i + 1]
            i += 2
        elif argv[i] == "--multi-sizes":
            i += 1
            while i < len(argv) and not argv[i].startswith("--"):
                multi_sizes_args.append(argv[i])
                i += 1
        elif argv[i] == "--qsort":
            qsort_mode = True
            i += 1
        else:
            positional.append(argv[i])
            i += 1
    return positional, data_args, sizes_arg, multi_sizes_args, qsort_mode, title


def main():
    positional, data_args, sizes_arg, multi_sizes_args, qsort_mode, title = parse_args(sys.argv)

    if qsort_mode:
        # Multi-machine: --qsort "label:O0.csv:O3.csv" ... <outdir>
        # Single-machine: --qsort <O0.csv> <O3.csv> <outdir>
        if len(positional) >= 2 and ":" in positional[0]:
            # Multi-machine format
            outdir = positional[-1]
            Path(outdir).mkdir(parents=True, exist_ok=True)
            machines = []
            for arg in positional[:-1]:
                parts = arg.split(":")
                if len(parts) != 3:
                    print(f"Error: expected 'label:O0.csv:O3.csv', got '{arg}'")
                    sys.exit(1)
                machines.append((parts[0], parts[1], parts[2]))
            if title is None:
                title = "n=10000, 1000 trials"
            plot_qsort_multi(machines, outdir, title)
        elif len(positional) >= 3:
            csv_O0, csv_O3, outdir = positional[0], positional[1], positional[2]
            Path(outdir).mkdir(parents=True, exist_ok=True)
            if title is None:
                title = f"n=10000 — {detect_cpu()}"
            plot_qsort(csv_O0, csv_O3, outdir, title)
        else:
            print("Usage:")
            print("  plot_bench.py --qsort <O0.csv> <O3.csv> <outdir>")
            print('  plot_bench.py --qsort "L1:O0:O3" "L2:O0:O3" <outdir>')
            sys.exit(1)
        return

    if multi_sizes_args:
        # Multi-machine × multi-size: --multi-sizes "label:csv1,csv2,..." ...
        if not positional:
            print('Usage: plot_bench.py <outdir> --multi-sizes "L1:c1,c2" "L2:c3,c4"')
            sys.exit(1)
        outdir = positional[0]
        Path(outdir).mkdir(parents=True, exist_ok=True)

        machine_datasets = []
        for arg in multi_sizes_args:
            label, csv_list = arg.split(":", 1)
            csv_paths = csv_list.split(",")
            size_datasets = []
            for csv_path in csv_paths:
                data = read_csv(csv_path)
                first_row = data["head"][0] if data["head"] else {}
                es = first_row.get("elem_size", "?")
                size_datasets.append((f"{es}B", data))
            machine_datasets.append((label, size_datasets))

        if title is None:
            title = "Element Size Comparison"
        plot_multi_sizes(machine_datasets, outdir, title)

    elif sizes_arg:
        # Element size comparison: --sizes csv4:csv32:csv128
        if not positional:
            print("Usage: plot_bench.py <outdir> --sizes csv4:csv32:csv128")
            sys.exit(1)
        outdir = positional[0]
        Path(outdir).mkdir(parents=True, exist_ok=True)

        csv_paths = sizes_arg.split(":")
        size_datasets = []
        for csv_path in csv_paths:
            data = read_csv(csv_path)
            # Extract elem_size from first data row
            first_row = data["head"][0] if data["head"] else {}
            es = first_row.get("elem_size", "?")
            size_datasets.append((f"{es}B", data))

        if title is None:
            title = detect_cpu()
        plot_sizes(size_datasets, outdir, title)

    elif data_args:
        # Multi-machine mode
        if not positional:
            print("Usage: plot_bench.py <outdir> --data label1:csv1 ...")
            sys.exit(1)
        outdir = positional[0]
        Path(outdir).mkdir(parents=True, exist_ok=True)

        datasets = []
        for arg in data_args:
            label, csv_path = arg.split(":", 1)
            datasets.append((label, read_csv(csv_path)))

        if title is None:
            title = "Multi-platform Comparison"
        plot_multi(datasets, outdir, title)

    elif len(positional) >= 2:
        # Single-machine mode
        csv_path = positional[0]
        outdir = positional[1]
        Path(outdir).mkdir(parents=True, exist_ok=True)

        if title is None:
            title = detect_cpu()
        data = read_csv(csv_path)
        plot_single(data, outdir, title)

    else:
        print("Usage:")
        print(f"  {sys.argv[0]} <csv> <outdir> [--title TITLE]")
        print(f"  {sys.argv[0]} <outdir> --data label1:csv1 label2:csv2 ...")
        print(f"  {sys.argv[0]} <outdir> --sizes csv4:csv32:csv128 [--title T]")
        print(f'  {sys.argv[0]} <outdir> --multi-sizes "L1:c1,c2" "L2:c3,c4"')
        sys.exit(1)


if __name__ == "__main__":
    main()
