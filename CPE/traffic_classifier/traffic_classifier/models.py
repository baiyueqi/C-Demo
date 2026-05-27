"""Shared data structures for parsed packets, flows, and classifications."""

from __future__ import annotations

from dataclasses import dataclass, field
from typing import Any


@dataclass(frozen=True)
class PacketRecord:
    """A normalized packet view used by the flow and feature modules."""

    timestamp: float
    src_ip: str
    dst_ip: str
    src_port: int
    dst_port: int
    protocol: str
    length: int
    ttl: int | None = None
    tcp_flags: int = 0
    icmp_type: int | None = None
    src_mac: str | None = None
    dst_mac: str | None = None


@dataclass(frozen=True)
class FlowKey:
    """Canonical bidirectional five-tuple key."""

    endpoint_a: tuple[str, int]
    endpoint_b: tuple[str, int]
    protocol: str

    @classmethod
    def from_packet(cls, packet: PacketRecord) -> "FlowKey":
        left = (packet.src_ip, packet.src_port)
        right = (packet.dst_ip, packet.dst_port)
        endpoint_a, endpoint_b = sorted([left, right])
        return cls(endpoint_a=endpoint_a, endpoint_b=endpoint_b, protocol=packet.protocol)

    def display(self) -> str:
        a_ip, a_port = self.endpoint_a
        b_ip, b_port = self.endpoint_b
        return f"{a_ip}:{a_port} <-> {b_ip}:{b_port} {self.protocol}"


@dataclass
class Flow:
    """Packets grouped under one bidirectional five-tuple."""

    key: FlowKey
    packets: list[PacketRecord] = field(default_factory=list)

    def add(self, packet: PacketRecord) -> None:
        self.packets.append(packet)


@dataclass(frozen=True)
class Classification:
    """Human-readable classification result."""

    label: str
    confidence: float
    reason: str


FeatureRow = dict[str, Any]

