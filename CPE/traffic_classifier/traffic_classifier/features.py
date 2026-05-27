"""Feature extraction from flows."""

from __future__ import annotations

from statistics import mean, pvariance

from .models import FeatureRow, Flow

TCP_FIN = 0x01
TCP_SYN = 0x02
TCP_RST = 0x04
TCP_PSH = 0x08
TCP_ACK = 0x10


def extract_features(flow: Flow) -> FeatureRow:
    """Build numeric and descriptive features for one flow."""

    packets = sorted(flow.packets, key=lambda packet: packet.timestamp)
    times = [packet.timestamp for packet in packets]
    lengths = [packet.length for packet in packets]
    ttls = [packet.ttl for packet in packets if packet.ttl is not None]
    intervals = [right - left for left, right in zip(times, times[1:]) if right >= left]
    ports = [packet.src_port for packet in packets] + [packet.dst_port for packet in packets]

    first = packets[0]
    duration = (times[-1] - times[0]) if len(times) > 1 else 0.0
    syn_count = sum(1 for packet in packets if packet.tcp_flags & TCP_SYN)
    ack_count = sum(1 for packet in packets if packet.tcp_flags & TCP_ACK)
    fin_count = sum(1 for packet in packets if packet.tcp_flags & TCP_FIN)
    rst_count = sum(1 for packet in packets if packet.tcp_flags & TCP_RST)
    psh_count = sum(1 for packet in packets if packet.tcp_flags & TCP_PSH)

    return {
        "flow_id": flow.key.display(),
        "src_ip": first.src_ip,
        "dst_ip": first.dst_ip,
        "src_port": first.src_port,
        "dst_port": first.dst_port,
        "protocol": first.protocol,
        "packet_count": len(packets),
        "total_bytes": sum(lengths),
        "avg_packet_size": _round(mean(lengths)),
        "max_packet_size": max(lengths),
        "min_packet_size": min(lengths),
        "duration": _round(duration),
        "avg_interval": _round(mean(intervals)) if intervals else 0.0,
        "interval_variance": _round(pvariance(intervals)) if len(intervals) > 1 else 0.0,
        "avg_ttl": _round(mean(ttls)) if ttls else 0.0,
        "syn_count": syn_count,
        "ack_count": ack_count,
        "fin_count": fin_count,
        "rst_count": rst_count,
        "psh_count": psh_count,
        "unique_ports": len(set(ports)),
    }


def extract_feature_rows(flows: list[Flow]) -> list[FeatureRow]:
    return [extract_features(flow) for flow in flows if flow.packets]


def _round(value: float) -> float:
    return round(float(value), 6)

