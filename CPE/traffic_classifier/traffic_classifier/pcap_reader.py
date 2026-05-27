"""Minimal pcap/pcapng reader implemented with the Python standard library."""

from __future__ import annotations

import struct
from dataclasses import dataclass
from pathlib import Path
from typing import Iterator


class PcapError(ValueError):
    """Raised when a capture file cannot be parsed."""


@dataclass(frozen=True)
class RawPacket:
    """Raw captured frame plus timestamp metadata."""

    timestamp: float
    data: bytes


PCAPNG_SECTION_HEADER = 0x0A0D0D0A
PCAPNG_INTERFACE_DESCRIPTION = 0x00000001
PCAPNG_ENHANCED_PACKET = 0x00000006
PCAPNG_SIMPLE_PACKET = 0x00000003


def read_capture(path: str | Path) -> list[RawPacket]:
    """Read a .pcap or .pcapng file into raw packets."""

    data = Path(path).read_bytes()
    if len(data) < 4:
        raise PcapError("capture file is too small")
    if data[:4] == b"\x0a\x0d\x0d\x0a":
        return list(_read_pcapng(data))
    return list(_read_pcap(data))


def _read_pcap(data: bytes) -> Iterator[RawPacket]:
    if len(data) < 24:
        raise PcapError("pcap file is too small")

    magic = data[:4]
    if magic == b"\xd4\xc3\xb2\xa1":
        endian = "<"
        fraction_scale = 1_000_000
    elif magic == b"\xa1\xb2\xc3\xd4":
        endian = ">"
        fraction_scale = 1_000_000
    elif magic == b"\x4d\x3c\xb2\xa1":
        endian = "<"
        fraction_scale = 1_000_000_000
    elif magic == b"\xa1\xb2\x3c\x4d":
        endian = ">"
        fraction_scale = 1_000_000_000
    else:
        raise PcapError("unsupported pcap magic number")

    offset = 24
    while offset + 16 <= len(data):
        ts_sec, ts_frac, incl_len, _orig_len = struct.unpack_from(f"{endian}IIII", data, offset)
        offset += 16
        packet = data[offset : offset + incl_len]
        if len(packet) != incl_len:
            break
        offset += incl_len
        yield RawPacket(ts_sec + ts_frac / fraction_scale, packet)


def _read_pcapng(data: bytes) -> Iterator[RawPacket]:
    offset = 0
    endian = "<"
    interfaces: list[dict[str, int]] = []

    while offset + 12 <= len(data):
        block_type_le, total_length_le = struct.unpack_from("<II", data, offset)
        if total_length_le < 12 or offset + total_length_le > len(data):
            break

        if block_type_le == PCAPNG_SECTION_HEADER:
            byte_order_magic = data[offset + 8 : offset + 12]
            if byte_order_magic == b"\x4d\x3c\x2b\x1a":
                endian = "<"
            elif byte_order_magic == b"\x1a\x2b\x3c\x4d":
                endian = ">"
            else:
                raise PcapError("unsupported pcapng byte-order magic")
            block_type, total_length = struct.unpack_from(f"{endian}II", data, offset)
        else:
            block_type, total_length = struct.unpack_from(f"{endian}II", data, offset)

        body_start = offset + 8
        body_end = offset + total_length - 4
        body = data[body_start:body_end]

        if block_type == PCAPNG_INTERFACE_DESCRIPTION and len(body) >= 8:
            linktype, _reserved, snaplen = struct.unpack_from(f"{endian}HHI", body, 0)
            interfaces.append({"linktype": linktype, "snaplen": snaplen, "tsresol": 1_000_000})
        elif block_type == PCAPNG_ENHANCED_PACKET and len(body) >= 20:
            interface_id, ts_high, ts_low, captured_len, _packet_len = struct.unpack_from(
                f"{endian}IIIII", body, 0
            )
            packet = body[20 : 20 + captured_len]
            if len(packet) == captured_len:
                tsresol = interfaces[interface_id]["tsresol"] if interface_id < len(interfaces) else 1_000_000
                timestamp_raw = (ts_high << 32) | ts_low
                yield RawPacket(timestamp_raw / tsresol, packet)
        elif block_type == PCAPNG_SIMPLE_PACKET and len(body) >= 4:
            packet_len = struct.unpack_from(f"{endian}I", body, 0)[0]
            packet = body[4 : 4 + packet_len]
            if packet:
                yield RawPacket(0.0, packet)

        offset += total_length

