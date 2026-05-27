"""Predict capture traffic classes using a saved KNN model."""

from __future__ import annotations

import argparse
from pathlib import Path

from .analyzer import analyze_capture
from .csv_io import write_csv
from .ml import apply_model, load_model
from .pcap_reader import PcapError


def main() -> int:
    parser = argparse.ArgumentParser(description="Predict pcap/pcapng flows with a saved KNN model")
    parser.add_argument("capture", help="Path to a .pcap or .pcapng file")
    parser.add_argument("--model", default="models/knn_model.json", help="Model path")
    parser.add_argument("--csv", help="Optional output CSV path")
    parser.add_argument("--limit", type=int, default=20, help="Maximum rows to print")
    args = parser.parse_args()

    try:
        model = load_model(args.model)
        packets, rows = analyze_capture(Path(args.capture))
        rows = apply_model(model, rows)
    except (OSError, PcapError, ValueError) as exc:
        print(f"error: {exc}")
        return 1

    print(f"capture: {args.capture}")
    print(f"model: {args.model}")
    print(f"parsed_packets: {len(packets)}")
    print(f"flows: {len(rows)}")

    if args.csv:
        write_csv(rows, args.csv)
        print(f"csv: {args.csv}")

    print()
    for idx, row in enumerate(rows[: args.limit], 1):
        print(
            f"{idx:>3}. ML={row['ml_label']:<10} "
            f"confidence={row['ml_confidence']:.2f} "
            f"rule={row['label']:<10} "
            f"packets={row['packet_count']:<4} "
            f"{row['flow_id']}"
        )

    if len(rows) > args.limit:
        print(f"... {len(rows) - args.limit} more flows omitted")
    return 0


if __name__ == "__main__":
    raise SystemExit(main())
