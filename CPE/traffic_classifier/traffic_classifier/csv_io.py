"""CSV helpers."""

from __future__ import annotations

import csv
from pathlib import Path

from .models import FeatureRow


def write_csv(rows: list[FeatureRow], path: str | Path) -> None:
    """Write feature/classification rows to CSV."""

    if not rows:
        Path(path).write_text("", encoding="utf-8")
        return
    fieldnames = list(rows[0].keys())
    with Path(path).open("w", encoding="utf-8", newline="") as output:
        writer = csv.DictWriter(output, fieldnames=fieldnames)
        writer.writeheader()
        writer.writerows(rows)

