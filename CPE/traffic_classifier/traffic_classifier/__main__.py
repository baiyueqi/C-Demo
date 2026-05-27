"""Command-line entry point for the offline traffic classifier."""

from __future__ import annotations

import argparse
from pathlib import Path

from .analyzer import analyze_capture
from .classifier import is_attack_label
from .csv_io import write_csv
from .pcap_reader import PcapError


def main() -> int:
    parser = argparse.ArgumentParser(description="Offline pcap/pcapng network traffic classifier")
    parser.add_argument("capture", help="Path to a .pcap or .pcapng file")
    parser.add_argument("--csv", help="Optional output CSV path")
    parser.add_argument("--limit", type=int, default=20, help="Maximum rows to print")
    args = parser.parse_args()

    capture = Path(args.capture)
    try:
        packets, rows = analyze_capture(capture)
    except (OSError, PcapError, ValueError) as exc:
        print(f"error: {exc}")
        return 1

    print(f"capture: {capture}")
    print(f"parsed_packets: {len(packets)}")
    print(f"flows: {len(rows)}")

    if args.csv:
        write_csv(rows, args.csv)
        print(f"csv: {args.csv}")

    if not rows:
        print("no TCP/UDP/ICMP IPv4 flows found")
        return 0

    print()
    for idx, row in enumerate(rows[: args.limit], 1):
        alert = "ALERT " if is_attack_label(str(row["label"])) else ""
        print(
            f"{idx:>3}. {alert}{row['label']:<10} "
            f"confidence={row['confidence']:.2f} "
            f"packets={row['packet_count']:<4} "
            f"bytes={row['total_bytes']:<6} "
            f"{row['flow_id']} "
            f"({row['reason']})"
        )

    if len(rows) > args.limit:
        print(f"... {len(rows) - args.limit} more flows omitted")
    return 0


if __name__ == "__main__":
    raise SystemExit(main())
