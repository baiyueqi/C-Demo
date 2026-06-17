#!/usr/bin/env python3
"""Verify the Easy crackme password logic recovered from static analysis."""

from __future__ import annotations

import argparse


PASSWORD = "hello"


def check_password(value: str) -> bool:
    if len(value) != 5:
        return False

    for index, char in enumerate(value):
        if char != PASSWORD[index]:
            return False

    return True


def result_text(value: str) -> str:
    return "Access granted!" if check_password(value) else "Access denied!"


def main() -> int:
    parser = argparse.ArgumentParser(
        description="Reproduce the Easy crackme password check."
    )
    parser.add_argument(
        "inputs",
        nargs="*",
        default=["test", "hell", "hello"],
        help="candidate passwords; defaults to three report verification cases",
    )
    args = parser.parse_args()

    for value in args.inputs:
        print(f"{value!r}: {result_text(value)}")

    return 0


if __name__ == "__main__":
    raise SystemExit(main())
