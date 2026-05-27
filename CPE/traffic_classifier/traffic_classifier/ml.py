"""Small KNN model helpers using only the Python standard library."""

from __future__ import annotations

import csv
import json
import math
from pathlib import Path
from typing import Any

from .models import FeatureRow

FEATURE_COLUMNS = [
    "protocol_tcp",
    "protocol_udp",
    "protocol_icmp",
    "src_port",
    "dst_port",
    "packet_count",
    "total_bytes",
    "avg_packet_size",
    "max_packet_size",
    "min_packet_size",
    "duration",
    "avg_interval",
    "interval_variance",
    "avg_ttl",
    "syn_count",
    "ack_count",
    "fin_count",
    "rst_count",
    "psh_count",
    "unique_ports",
]


def train_knn(csv_paths: list[str | Path], k: int = 3, label_overrides: dict[str, str] | None = None) -> dict[str, Any]:
    """Build a normalized KNN model from labeled feature CSV files."""

    samples = []
    label_overrides = label_overrides or {}
    for path in csv_paths:
        path_obj = Path(path)
        override = label_overrides.get(str(path_obj)) or label_overrides.get(path_obj.name) or label_overrides.get(path_obj.stem)
        with Path(path).open(encoding="utf-8", newline="") as input_file:
            for row in csv.DictReader(input_file):
                label = override or row.get("label", "").strip()
                if not label:
                    continue
                samples.append({"features": _row_vector(row), "label": label})

    if not samples:
        raise ValueError("no labeled rows found")

    means, scales = _fit_scaler([sample["features"] for sample in samples])
    return {
        "type": "knn",
        "k": max(1, min(k, len(samples))),
        "feature_columns": FEATURE_COLUMNS,
        "means": means,
        "scales": scales,
        "samples": [
            {"features": _normalize(sample["features"], means, scales), "label": sample["label"]}
            for sample in samples
        ],
    }


def save_model(model: dict[str, Any], path: str | Path) -> None:
    Path(path).parent.mkdir(parents=True, exist_ok=True)
    Path(path).write_text(json.dumps(model, indent=2), encoding="utf-8")


def load_model(path: str | Path) -> dict[str, Any]:
    return json.loads(Path(path).read_text(encoding="utf-8"))


def predict_row(model: dict[str, Any], row: FeatureRow) -> tuple[str, float]:
    """Predict one feature row and return label plus vote confidence."""

    vector = _normalize(_row_vector(row), model["means"], model["scales"])
    distances = [
        (_euclidean(vector, sample["features"]), sample["label"])
        for sample in model["samples"]
    ]
    distances.sort(key=lambda item: item[0])
    neighbors = distances[: int(model["k"])]
    if neighbors[0][0] == 0:
        return neighbors[0][1], 1.0

    votes: dict[str, float] = {}
    for distance, label in neighbors:
        votes[label] = votes.get(label, 0.0) + 1.0 / distance
    label, score = max(votes.items(), key=lambda item: item[1])
    return label, score / sum(votes.values())


def apply_model(model: dict[str, Any], rows: list[FeatureRow], confidence_floor: float = 0.6) -> list[FeatureRow]:
    results = []
    for row in rows:
        label, confidence = predict_row(model, row)
        enriched = dict(row)
        if _should_use_rule_label(row, label, confidence, confidence_floor):
            enriched["ml_label"] = row["label"]
            enriched["ml_confidence"] = row["confidence"]
            enriched["ml_reason"] = "fallback to rule classifier"
        else:
            enriched["ml_label"] = label
            enriched["ml_confidence"] = round(confidence, 6)
            enriched["ml_reason"] = "knn nearest-neighbor vote"
        results.append(enriched)
    return results


def _row_vector(row: dict[str, Any]) -> list[float]:
    protocol = str(row.get("protocol", "")).upper()
    values = {
        "protocol_tcp": 1.0 if protocol == "TCP" else 0.0,
        "protocol_udp": 1.0 if protocol == "UDP" else 0.0,
        "protocol_icmp": 1.0 if protocol == "ICMP" else 0.0,
    }
    return [values.get(column, float(row.get(column) or 0.0)) for column in FEATURE_COLUMNS]


def _fit_scaler(vectors: list[list[float]]) -> tuple[list[float], list[float]]:
    columns = list(zip(*vectors))
    means = [sum(column) / len(column) for column in columns]
    scales = []
    for column, mean in zip(columns, means):
        variance = sum((value - mean) ** 2 for value in column) / len(column)
        scale = math.sqrt(variance)
        scales.append(scale if scale > 0 else 1.0)
    return means, scales


def _normalize(vector: list[float], means: list[float], scales: list[float]) -> list[float]:
    return [(value - mean) / scale for value, mean, scale in zip(vector, means, scales)]


def _euclidean(left: list[float], right: list[float]) -> float:
    return math.sqrt(sum((a - b) ** 2 for a, b in zip(left, right)))


def _should_use_rule_label(row: FeatureRow, ml_label: str, ml_confidence: float, confidence_floor: float) -> bool:
    rule_label = str(row.get("label", ""))
    rule_confidence = float(row.get("confidence", 0.0) or 0.0)
    if not rule_label or rule_label == "Unknown":
        return ml_confidence < confidence_floor
    if ml_confidence < confidence_floor:
        return True
    if ml_label == "Port_Scan" and rule_label in {"Web", "DNS", "ICMP", "SSH", "Small_UDP"}:
        return True
    if rule_confidence >= 0.85 and ml_label != rule_label:
        return True
    return False
