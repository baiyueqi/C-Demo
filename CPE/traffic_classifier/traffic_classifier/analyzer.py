"""High-level capture analysis pipeline."""

from __future__ import annotations

from pathlib import Path

from .classifier import classify_rows
from .features import extract_feature_rows
from .flow import build_flows
from .models import FeatureRow, PacketRecord
from .parser import parse_packet
from .pcap_reader import read_capture


def analyze_capture(path: str | Path) -> tuple[list[PacketRecord], list[FeatureRow]]:
    """Parse a capture file and return parsed packets plus classified feature rows."""

    raw_packets = read_capture(path)
    return analyze_packets(raw_packets)


def analyze_packets(raw_packets: list) -> tuple[list[PacketRecord], list[FeatureRow]]:
    """Parse raw packets and return parsed packets plus classified feature rows."""

    packets = [packet for raw in raw_packets if (packet := parse_packet(raw)) is not None]
    flows = build_flows(packets)
    rows = extract_feature_rows(flows)
    return packets, classify_rows(rows)
