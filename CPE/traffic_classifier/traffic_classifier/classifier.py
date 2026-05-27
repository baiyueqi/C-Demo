"""Rule-based classifier used before a trained ML model is available."""

from __future__ import annotations

from .models import Classification, FeatureRow

ATTACK_LABELS = {"SYN_Flood", "TCP_Probe", "Port_Scan"}
PORT_SCAN_MIN_PROBES = 5


def classify_rule_based(row: FeatureRow) -> Classification:
    """Classify a flow using transparent rules suitable for early demos."""

    protocol = str(row["protocol"])
    ports = {int(row["src_port"]), int(row["dst_port"])}
    packet_count = int(row["packet_count"])
    syn_count = int(row["syn_count"])
    ack_count = int(row["ack_count"])
    fin_count = int(row["fin_count"])
    rst_count = int(row["rst_count"])
    avg_size = float(row["avg_packet_size"])

    if protocol == "ICMP":
        return Classification("ICMP", 0.95, "ICMP protocol")

    if 53 in ports and protocol in {"UDP", "TCP"}:
        return Classification("DNS", 0.92, "port 53 DNS traffic")

    if protocol == "TCP" and syn_count >= 20 and syn_count / max(ack_count, 1) >= 4:
        return Classification("SYN_Flood", 0.82, "many SYN packets with few ACK packets")

    if protocol == "TCP" and 22 in ports:
        return Classification("SSH", 0.85, "TCP port 22")

    if protocol == "TCP" and (80 in ports or 443 in ports):
        return Classification("Web", 0.88, "HTTP/HTTPS port")

    if protocol == "UDP" and packet_count <= 4 and avg_size < 200:
        return Classification("Small_UDP", 0.55, "small short UDP flow")

    if protocol == "TCP" and packet_count <= 3 and syn_count > 0 and fin_count == 0 and rst_count == 0:
        return Classification("TCP_Probe", 0.55, "short TCP handshake-like flow")

    return Classification("Unknown", 0.35, "no strong rule matched")


def is_attack_label(label: str) -> bool:
    """Return whether a classification label should raise an alert."""

    return label in ATTACK_LABELS


def classify_rows(rows: list[FeatureRow]) -> list[FeatureRow]:
    """Attach label, confidence, and reason columns to feature rows."""

    results = []
    for row in rows:
        classification = classify_rule_based(row)
        enriched = dict(row)
        enriched["label"] = classification.label
        enriched["confidence"] = classification.confidence
        enriched["reason"] = classification.reason
        results.append(enriched)
    return _mark_port_scans(results)


def _mark_port_scans(rows: list[FeatureRow]) -> list[FeatureRow]:
    probes_by_pair: dict[tuple[str, str], set[int]] = {}

    for row in rows:
        if row["label"] != "TCP_Probe":
            continue
        pair = (str(row["src_ip"]), str(row["dst_ip"]))
        probes_by_pair.setdefault(pair, set()).add(int(row["dst_port"]))

    scan_pairs = {
        pair: ports for pair, ports in probes_by_pair.items() if len(ports) >= PORT_SCAN_MIN_PROBES
    }
    if not scan_pairs:
        return rows

    for row in rows:
        pair = (str(row["src_ip"]), str(row["dst_ip"]))
        if row["label"] == "TCP_Probe" and pair in scan_pairs:
            row["label"] = "Port_Scan"
            row["confidence"] = 0.8
            row["reason"] = f"TCP probes to {len(scan_pairs[pair])} destination ports"
    return rows
