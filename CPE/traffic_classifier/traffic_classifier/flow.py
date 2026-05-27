"""Flow aggregation."""

from __future__ import annotations

from collections import OrderedDict
from collections.abc import Iterable

from .models import Flow, FlowKey, PacketRecord


def build_flows(packets: Iterable[PacketRecord]) -> list[Flow]:
    """Group packets into bidirectional flows."""

    flows: OrderedDict[FlowKey, Flow] = OrderedDict()
    for packet in packets:
        key = FlowKey.from_packet(packet)
        if key not in flows:
            flows[key] = Flow(key=key)
        flows[key].add(packet)
    return list(flows.values())

