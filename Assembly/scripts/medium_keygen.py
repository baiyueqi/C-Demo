#!/usr/bin/env python3
"""Keygen for Assembly Medium sample: KeygenMe_3_SWD."""

from __future__ import annotations

import argparse
import re


ALPHABET = "0123456789ABCDEFGHIJKLMNOPQRSTUVWXYZ"
SECRET_RE = re.compile(r"^[0-9A-Z]{4}-[0-9A-Z]{5}-[0-9A-Z]{3}$")
MASK32 = 0xFFFFFFFF


def u32(value: int) -> int:
    return value & MASK32


def rol32(value: int, bits: int) -> int:
    value = u32(value)
    return u32((value << bits) | (value >> (32 - bits)))


def ror32(value: int, bits: int) -> int:
    value = u32(value)
    return u32((value >> bits) | (value << (32 - bits)))


def base36_encode(value: int, width: int) -> str:
    if value < 0:
        raise ValueError("base36 value must be non-negative")

    digits = "0"
    if value:
        parts: list[str] = []
        while value:
            value, remainder = divmod(value, 36)
            parts.append(ALPHABET[remainder])
        digits = "".join(reversed(parts))

    if len(digits) > width:
        raise ValueError(f"value does not fit in {width} base36 digits")
    return digits.rjust(width, "0")


def base36_decode(text: str) -> int:
    value = 0
    for char in text:
        value = value * 36 + ALPHABET.index(char)
    return value


def validate_username(username: str) -> None:
    if not 3 <= len(username) <= 15:
        raise ValueError("username length must be 3..15")
    if not all(char in ALPHABET or "a" <= char <= "z" for char in username):
        raise ValueError("username must contain only ASCII alphanumeric characters")


def username_metrics(username: str) -> tuple[int, int, int]:
    username = username.upper()
    sum_weighted = 0
    xor_mix = 0
    product_mix = 1

    for index, char in enumerate(username):
        value = ord(char)
        sum_weighted = u32(sum_weighted + value * (index + 1))
        xor_mix = u32(xor_mix ^ (value + index))
        product_mix = u32((product_mix * (value + 3)) % 100000)

    part1 = u32((sum_weighted ^ 0x5A5A) % 46656)
    part2 = u32((xor_mix * 1337 + product_mix) % 60466176)
    part3 = u32((product_mix + sum_weighted + xor_mix) % 46656)
    return part1, part2, part3


def generate_secret_code(username: str) -> str:
    part1, part2, part3 = username_metrics(username)
    return (
        f"{base36_encode(part1, 4)}-"
        f"{base36_encode(part2, 5)}-"
        f"{base36_encode(part3, 3)}"
    )


def decode_secret_code(secret_code: str) -> tuple[int, int, int]:
    if not SECRET_RE.fullmatch(secret_code):
        raise ValueError("secret code must match XXXX-XXXXX-XXX using 0-9A-Z")
    return (
        base36_decode(secret_code[:4]),
        base36_decode(secret_code[5:10]),
        base36_decode(secret_code[11:14]),
    )


def verify_secret_code(username: str, secret_code: str) -> bool:
    return username_metrics(username) == decode_secret_code(secret_code)


def generate_pin(username: str, secret_code: str) -> int:
    hash_a = 0xA3B1C2D3
    hash_b = 0x1F2E3D4C
    material = username.upper() + chr(len(secret_code) ^ 0x5A) + secret_code

    for index, char in enumerate(material):
        value = ord(char)
        hash_a = u32(hash_a ^ (value + index * 0x11))
        hash_a = rol32(hash_a, index % 5 + 3)
        hash_a = u32(hash_a + (hash_b ^ 0x9E3779B9))
        hash_b = u32(hash_b ^ u32(hash_a + value * 0x83))
        hash_b = ror32(hash_b, index % 7 + 2)
        hash_b = u32(hash_b + (u32(hash_a << 3) ^ 0x7F4A7C15))
        if index & 1:
            hash_a, hash_b = hash_b, hash_a

    mixed = u32(hash_a ^ hash_b)
    mixed = u32(mixed ^ (mixed >> 16))
    mixed = u32(mixed * 0x85EBCA6B)
    mixed = u32(mixed ^ (mixed >> 13))
    mixed = u32(mixed * 0xC2B2AE35)
    mixed = u32(mixed ^ (mixed >> 16))
    return mixed & 0x7FFFFFFF


def main() -> int:
    parser = argparse.ArgumentParser(
        description="Generate Secret code and Verification PIN for KeygenMe_3_SWD."
    )
    parser.add_argument("username", help="3..15 alphanumeric characters")
    args = parser.parse_args()

    validate_username(args.username)
    secret_code = generate_secret_code(args.username)
    pin = generate_pin(args.username, secret_code)

    print(f"Username: {args.username}")
    print(f"Secret code: {secret_code}")
    print(f"Verification PIN: {pin}")
    print(f"Self-check: {'ok' if verify_secret_code(args.username, secret_code) else 'failed'}")
    return 0


if __name__ == "__main__":
    raise SystemExit(main())
