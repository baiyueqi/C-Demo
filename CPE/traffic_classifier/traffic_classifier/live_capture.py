"""Live packet capture from a selected Linux network interface."""

from __future__ import annotations

import argparse
import socket
import time
from pathlib import Path

from .analyzer import analyze_packets
from .ml import apply_model, load_model
from .pcap_reader import RawPacket


def list_interfaces() -> list[str]:
    sys_class_net = Path("/sys/class/net")
    if not sys_class_net.exists():
        return []
    return sorted(path.name for path in sys_class_net.iterdir() if path.name != "lo")


def capture_packets(interface: str, count: int = 100, timeout: float = 30.0) -> list[RawPacket]:
    """Capture Ethernet frames from a Linux interface."""

    packets: list[RawPacket] = []
    deadline = time.time() + timeout
    with socket.socket(socket.AF_PACKET, socket.SOCK_RAW, socket.ntohs(0x0003)) as sock:
        sock.bind((interface, 0))
        sock.settimeout(1.0)
        while len(packets) < count and time.time() < deadline:
            try:
                data = sock.recv(65535)
            except TimeoutError:
                continue
            packets.append(RawPacket(time.time(), data))
    return packets


def _choose_interface(requested: str | None) -> str:
    interfaces = list_interfaces()
    if requested:
        return requested
    if not interfaces:
        raise ValueError("no network interfaces found")

    print("interfaces:")
    for idx, name in enumerate(interfaces, 1):
        print(f"{idx}. {name}")
    selected = input("select interface number: ").strip()
    if not selected.isdigit() or not 1 <= int(selected) <= len(interfaces):
        raise ValueError("invalid interface selection")
    return interfaces[int(selected) - 1]


def main() -> int:
    parser = argparse.ArgumentParser(description="Capture live packets and classify current traffic")
    parser.add_argument("-i", "--interface", help="Network interface name, for example wlan0 or eth0")
    parser.add_argument("-c", "--count", type=int, default=100, help="Number of packets to capture")
    parser.add_argument("--timeout", type=float, default=30.0, help="Maximum capture seconds")
    parser.add_argument("--model", help="Optional KNN model path for ML prediction")
    parser.add_argument("--limit", type=int, default=20, help="Maximum rows to print")
    args = parser.parse_args()

    try:
        model = load_model(args.model) if args.model else None
        interface = _choose_interface(args.interface)
        print(f"capturing {args.count} packets on {interface} ...")
        raw_packets = capture_packets(interface, count=args.count, timeout=args.timeout)
        packets, rows = analyze_packets(raw_packets)
        if model:
            rows = apply_model(model, rows)
    except (OSError, ValueError) as exc:
        print(f"error: {exc}")
        print("hint: live capture usually needs root or CAP_NET_RAW permission")
        return 1

    print(f"interface: {interface}")
    print(f"captured_packets: {len(raw_packets)}")
    print(f"parsed_packets: {len(packets)}")
    print(f"flows: {len(rows)}")

    if not rows:
        print("no TCP/UDP/ICMP IPv4 flows found")
        return 0

    print()
    for idx, row in enumerate(rows[: args.limit], 1):
        label = row.get("ml_label", row["label"])
        confidence = row.get("ml_confidence", row["confidence"])
        print(
            f"{idx:>3}. {label:<10} "
            f"confidence={confidence:.2f} "
            f"rule={row['label']:<10} "
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
