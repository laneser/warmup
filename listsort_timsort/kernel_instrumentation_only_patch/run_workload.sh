#!/usr/bin/env bash
# run_workload.sh — Generate list_sort() calls via filesystem workloads
#
# Creates XFS and btrfs loop devices and runs I/O workloads that trigger
# list_sort() in various kernel paths (transaction commit, buffer
# writeback, etc.)
#
# Prerequisites:
#   - Patched kernel with CONFIG_LIST_SORT_STATS=y booted
#   - xfsprogs and btrfs-progs installed
#   - Root privileges
#
# Usage:
#   sudo ./run_workload.sh [duration_seconds]
#   (default: 30 seconds per workload phase)

set -euo pipefail

DURATION=${1:-30}
STATS="/sys/kernel/debug/list_sort_stats/stats"
RESET="/sys/kernel/debug/list_sort_stats/reset"

if [ ! -f "${STATS}" ]; then
    echo "ERROR: ${STATS} not found."
    echo "Is CONFIG_LIST_SORT_STATS=y enabled and debugfs mounted?"
    exit 1
fi

collect_stats() {
    local label="$1"
    echo ""
    echo "=== Stats after: ${label} ==="
    cat "${STATS}"
    echo "---"
}

reset_stats() {
    echo 1 > "${RESET}"
}

# ---------- Helper: run workload phases on a mounted filesystem ----------
run_fs_workload() {
    local fs_type="$1"
    local mnt="$2"

    echo ""
    echo "========================================"
    echo "  Filesystem: ${fs_type}"
    echo "========================================"

    # Phase A: mass file creation
    reset_stats
    echo ""
    echo "--- ${fs_type} Phase A: mass file creation (${DURATION}s) ---"
    local end=$((SECONDS + DURATION))
    local i=0
    while [ $SECONDS -lt $end ]; do
        local dir="${mnt}/dir_$(printf '%04d' $((i / 1000)))"
        mkdir -p "${dir}" 2>/dev/null || true
        dd if=/dev/urandom of="${dir}/f_${i}" bs=4K count=1 status=none 2>/dev/null || true
        i=$((i + 1))
    done
    sync
    echo "  Created ${i} files"
    collect_stats "${fs_type} mass file creation"

    # Phase B: random write workload
    reset_stats
    echo ""
    echo "--- ${fs_type} Phase B: random writes (${DURATION}s) ---"
    dd if=/dev/urandom of="${mnt}/bigfile" bs=1M count=128 status=none 2>/dev/null || true
    sync
    end=$((SECONDS + DURATION))
    i=0
    while [ $SECONDS -lt $end ]; do
        local offset=$((RANDOM % 128))
        dd if=/dev/urandom of="${mnt}/bigfile" bs=4K count=1 \
           seek=$((offset * 256 + RANDOM % 256)) conv=notrunc status=none 2>/dev/null || true
        i=$((i + 1))
    done
    sync
    echo "  Did ${i} random writes"
    collect_stats "${fs_type} random writes"

    # Phase C: metadata scan
    reset_stats
    echo ""
    echo "--- ${fs_type} Phase C: metadata scan (${DURATION}s) ---"
    end=$((SECONDS + DURATION))
    i=0
    while [ $SECONDS -lt $end ]; do
        find "${mnt}" -type f -exec stat {} + > /dev/null 2>&1 || true
        i=$((i + 1))
    done
    echo "  Did ${i} full tree scans"
    collect_stats "${fs_type} metadata scan"

    # Phase D: file deletion storm
    reset_stats
    echo ""
    echo "--- ${fs_type} Phase D: mass file deletion ---"
    rm -rf "${mnt}"/dir_* "${mnt}"/bigfile 2>/dev/null || true
    sync
    collect_stats "${fs_type} mass deletion"
}

# ---------- Setup and run XFS ----------
XFS_IMG="/tmp/xfs_bench.img"
XFS_MNT="/tmp/xfs_bench_mnt"
XFS_LOOP=""

cleanup_xfs() {
    umount "${XFS_MNT}" 2>/dev/null || true
    [ -n "${XFS_LOOP}" ] && losetup -d "${XFS_LOOP}" 2>/dev/null || true
    rm -rf "${XFS_MNT}" "${XFS_IMG}"
}

echo "=== Setup: create 512MB XFS loop device ==="
dd if=/dev/zero of="${XFS_IMG}" bs=1M count=512 status=none
XFS_LOOP=$(losetup --show -f "${XFS_IMG}")
mkfs.xfs -f "${XFS_LOOP}" > /dev/null 2>&1
mkdir -p "${XFS_MNT}"
mount "${XFS_LOOP}" "${XFS_MNT}"
echo "  XFS on ${XFS_LOOP} mounted at ${XFS_MNT}"

run_fs_workload "XFS" "${XFS_MNT}"
cleanup_xfs

# ---------- Setup and run btrfs ----------
BTRFS_IMG="/tmp/btrfs_bench.img"
BTRFS_MNT="/tmp/btrfs_bench_mnt"
BTRFS_LOOP=""

cleanup_btrfs() {
    umount "${BTRFS_MNT}" 2>/dev/null || true
    [ -n "${BTRFS_LOOP}" ] && losetup -d "${BTRFS_LOOP}" 2>/dev/null || true
    rm -rf "${BTRFS_MNT}" "${BTRFS_IMG}"
}

echo ""
echo "=== Setup: create 512MB btrfs loop device ==="
dd if=/dev/zero of="${BTRFS_IMG}" bs=1M count=512 status=none
BTRFS_LOOP=$(losetup --show -f "${BTRFS_IMG}")
mkfs.btrfs -f "${BTRFS_LOOP}" > /dev/null 2>&1
mkdir -p "${BTRFS_MNT}"
mount "${BTRFS_LOOP}" "${BTRFS_MNT}"
echo "  btrfs on ${BTRFS_LOOP} mounted at ${BTRFS_MNT}"

run_fs_workload "btrfs" "${BTRFS_MNT}"
cleanup_btrfs

# ---------- System activity on native filesystem ----------
reset_stats
echo ""
echo "========================================"
echo "  Native filesystem system activity"
echo "========================================"
echo "  Normal system activity on root fs for ${DURATION}s..."
END=$((SECONDS + DURATION))
i=0
while [ $SECONDS -lt $END ]; do
    ls -laR /usr/lib/ > /dev/null 2>&1 || true
    find /proc -maxdepth 2 -type f -readable -exec cat {} + > /dev/null 2>&1 || true
    i=$((i + 1))
    sleep 1
done
echo "  Did ${i} system scan iterations"
collect_stats "system activity (native fs)"

echo ""
echo "=== All phases complete ==="
echo "Key metric: avg_runs / avg_elements ratio."
echo "  Close to 1 run  = highly ordered (run detection very beneficial)"
echo "  Close to n/2 runs = random (run detection adds overhead)"
