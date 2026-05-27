"""Train a small KNN traffic classifier from labeled CSV files."""

from __future__ import annotations

import argparse

from .ml import save_model, train_knn


def _parse_labels(values: list[str]) -> dict[str, str]:
    labels = {}
    for value in values:
        if "=" not in value:
            raise ValueError(f"invalid --label value: {value}")
        key, label = value.split("=", 1)
        labels[key.strip()] = label.strip()
    return labels


def main() -> int:
    parser = argparse.ArgumentParser(description="Train a KNN model from labeled traffic CSV files")
    parser.add_argument("csv", nargs="+", help="Input feature CSV files with a label column")
    parser.add_argument("--model", default="models/knn_model.json", help="Output model path")
    parser.add_argument(
        "--label",
        action="append",
        default=[],
        help="Override labels by file path, file name, or stem, for example video_features.csv=Video",
    )
    parser.add_argument("-k", type=int, default=3, help="Number of nearest neighbors")
    args = parser.parse_args()

    try:
        model = train_knn(args.csv, k=args.k, label_overrides=_parse_labels(args.label))
        save_model(model, args.model)
    except (OSError, ValueError) as exc:
        print(f"error: {exc}")
        return 1

    labels = sorted({sample["label"] for sample in model["samples"]})
    print(f"model: {args.model}")
    print(f"samples: {len(model['samples'])}")
    print(f"labels: {', '.join(labels)}")
    print(f"k: {model['k']}")
    return 0


if __name__ == "__main__":
    raise SystemExit(main())
