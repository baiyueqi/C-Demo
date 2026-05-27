"""Ethernet, IPv4, TCP, UDP, and ICMP parsing helpers."""

from __future__ import annotations

import ipaddress
import struct

from .models import PacketRecord
from .pcap_reader import RawPacket


ETHERTYPE_IPV4 = 0x0800
ETHERTYPE_VLAN = 0x8100
PROTO_ICMP = 1
PROTO_TCP = 6
PROTO_UDP = 17


def parse_packet(raw: RawPacket) -> PacketRecord | None:
    """Parse one Ethernet frame into a normalized packet record."""

    data = raw.data
    if len(data) < 14:
        return None

    dst_mac = _format_mac(data[0:6])
    src_mac = _format_mac(data[6:12])
    ethertype = struct.unpack("!H", data[12:14])[0]
    offset = 14

    if ethertype == ETHERTYPE_VLAN and len(data) >= 18:
        ethertype = struct.unpack("!H", data[16:18])[0]
        offset = 18

    if ethertype != ETHERTYPE_IPV4:
        return None

    return _parse_ipv4(raw.timestamp, data[offset:], src_mac, dst_mac)


def _parse_ipv4(timestamp: float, data: bytes, src_mac: str, dst_mac: str) -> PacketRecord | None:
    if len(data) < 20:
        return None
    version_ihl = data[0]
    version = version_ihl >> 4
    ihl = (version_ihl & 0x0F) * 4
    if version != 4 or ihl < 20 or len(data) < ihl:
        return None

    total_length = struct.unpack("!H", data[2:4])[0]
    protocol_num = data[9]
    ttl = data[8]
    src_ip = str(ipaddress.IPv4Address(data[12:16]))
    dst_ip = str(ipaddress.IPv4Address(data[16:20]))
    payload = data[ihl:total_length] if total_length <= len(data) else data[ihl:]

    if protocol_num == PROTO_TCP:
        return _parse_tcp(timestamp, src_ip, dst_ip, ttl, total_length, payload, src_mac, dst_mac)
    if protocol_num == PROTO_UDP:
        return _parse_udp(timestamp, src_ip, dst_ip, ttl, total_length, payload, src_mac, dst_mac)
    if protocol_num == PROTO_ICMP:
        return _parse_icmp(timestamp, src_ip, dst_ip, ttl, total_length, payload, src_mac, dst_mac)
    return None


def _parse_tcp(
    timestamp: float,
    src_ip: str,
    dst_ip: str,
    ttl: int,
    length: int,
    payload: bytes,
    src_mac: str,
    dst_mac: str,
) -> PacketRecord | None:
    if len(payload) < 20:
        return None
    src_port, dst_port = struct.unpack("!HH", payload[:4])
    flags = payload[13]
    return PacketRecord(
        timestamp=timestamp,
        src_ip=src_ip,
        dst_ip=dst_ip,
        src_port=src_port,
        dst_port=dst_port,
        protocol="TCP",
        length=length,
        ttl=ttl,
        tcp_flags=flags,
        src_mac=src_mac,
        dst_mac=dst_mac,
    )


def _parse_udp(
    timestamp: float,
    src_ip: str,
    dst_ip: str,
    ttl: int,
    length: int,
    payload: bytes,
    src_mac: str,
    dst_mac: str,
) -> PacketRecord | None:
    if len(payload) < 8:
        return None
    src_port, dst_port = struct.unpack("!HH", payload[:4])
    return PacketRecord(
        timestamp=timestamp,
        src_ip=src_ip,
        dst_ip=dst_ip,
        src_port=src_port,
        dst_port=dst_port,
        protocol="UDP",
        length=length,
        ttl=ttl,
        src_mac=src_mac,
        dst_mac=dst_mac,
    )


def _parse_icmp(
    timestamp: float,
    src_ip: str,
    dst_ip: str,
    ttl: int,
    length: int,
    payload: bytes,
    src_mac: str,
    dst_mac: str,
) -> PacketRecord | None:
    if len(payload) < 4:
        return None
    return PacketRecord(
        timestamp=timestamp,
        src_ip=src_ip,
        dst_ip=dst_ip,
        src_port=0,
        dst_port=0,
        protocol="ICMP",
        length=length,
        ttl=ttl,
        icmp_type=payload[0],
        src_mac=src_mac,
        dst_mac=dst_mac,
    )


def _format_mac(raw: bytes) -> str:
    return ":".join(f"{part:02x}" for part in raw)

