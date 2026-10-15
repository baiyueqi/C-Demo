#!/usr/bin/env python3
"""Generate experiment result files for the image encryption report.

This script intentionally uses only the Python standard library so it can run
in the same environment as the C++ project without extra Python packages.
"""

from __future__ import annotations

import math
import struct
import subprocess
import sys
import zlib
from pathlib import Path


ROOT = Path(__file__).resolve().parents[2]
RESULTS = ROOT / "InformationSecurity" / "results"
BINARY = ROOT / "InformationSecurity" / "image_cipher"


def write_pgm_8(path: Path, width: int, height: int, pixels: list[int]) -> None:
    payload = bytes(max(0, min(255, int(p))) for p in pixels)
    with path.open("wb") as f:
        f.write(f"P5\n{width} {height}\n255\n".encode("ascii"))
        f.write(payload)


def read_pgm(path: Path) -> tuple[int, int, int, list[int]]:
    data = path.read_bytes()
    pos = 0

    def next_token() -> bytes:
        nonlocal pos
        while pos < len(data):
            if data[pos : pos + 1] == b"#":
                while pos < len(data) and data[pos] != 10:
                    pos += 1
            elif data[pos] in b" \t\r\n":
                pos += 1
            else:
                start = pos
                while pos < len(data) and data[pos] not in b" \t\r\n":
                    pos += 1
                return data[start:pos]
        raise ValueError("unexpected end of PGM header")

    magic = next_token()
    if magic != b"P5":
        raise ValueError(f"unsupported PGM magic: {magic!r}")

    width = int(next_token())
    height = int(next_token())
    maxval = int(next_token())

    if pos < len(data) and data[pos] == 13:
        pos += 1
        if pos < len(data) and data[pos] == 10:
            pos += 1
    elif pos < len(data) and data[pos] == 10:
        pos += 1
    else:
        raise ValueError("missing raster separator in PGM")

    pixel_count = width * height
    if maxval < 256:
        raw = data[pos : pos + pixel_count]
        if len(raw) != pixel_count:
            raise ValueError("truncated 8-bit PGM raster")
        return width, height, maxval, list(raw)

    raw = data[pos : pos + pixel_count * 2]
    if len(raw) != pixel_count * 2:
        raise ValueError("truncated 16-bit PGM raster")
    pixels = [
        (raw[i * 2] << 8) | raw[i * 2 + 1]
        for i in range(pixel_count)
    ]
    return width, height, maxval, pixels


def write_png_gray(path: Path, width: int, height: int, pixels: list[int]) -> None:
    raw = bytearray()
    for y in range(height):
        raw.append(0)
        start = y * width
        raw.extend(
            max(0, min(255, int(p)))
            for p in pixels[start : start + width]
        )
    _write_png(path, width, height, 8, 0, bytes(raw))


def write_png_rgb(
    path: Path,
    width: int,
    height: int,
    pixels: list[tuple[int, int, int]],
) -> None:
    raw = bytearray()
    for y in range(height):
        raw.append(0)
        start = y * width
        for r, g, b in pixels[start : start + width]:
            raw.extend((max(0, min(255, r)), max(0, min(255, g)), max(0, min(255, b))))
    _write_png(path, width, height, 8, 2, bytes(raw))


def _write_png(
    path: Path,
    width: int,
    height: int,
    bit_depth: int,
    color_type: int,
    raw: bytes,
) -> None:
    def chunk(tag: bytes, payload: bytes) -> bytes:
        return (
            struct.pack(">I", len(payload))
            + tag
            + payload
            + struct.pack(">I", zlib.crc32(tag + payload) & 0xFFFFFFFF)
        )

    signature = b"\x89PNG\r\n\x1a\n"
    ihdr = struct.pack(">IIBBBBB", width, height, bit_depth, color_type, 0, 0, 0)
    idat = zlib.compress(raw, 9)
    path.write_bytes(signature + chunk(b"IHDR", ihdr) + chunk(b"IDAT", idat) + chunk(b"IEND", b""))


def make_test_image(width: int = 256, height: int = 256) -> list[int]:
    pixels = [0] * (width * height)

    def set_pixel(x: int, y: int, value: int) -> None:
        if 0 <= x < width and 0 <= y < height:
            pixels[y * width + x] = max(0, min(255, value))

    for y in range(height):
        for x in range(width):
            set_pixel(x, y, 35 + int(185 * x / max(1, width - 1)))

    # White square.
    for y in range(28, 92):
        for x in range(24, 88):
            set_pixel(x, y, 245)

    # Dark square.
    for y in range(150, 220):
        for x in range(150, 220):
            set_pixel(x, y, 18)

    # Circle and ring.
    center_x, center_y = 128, 128
    for y in range(height):
        for x in range(width):
            d2 = (x - center_x) ** 2 + (y - center_y) ** 2
            if 34 * 34 <= d2 <= 38 * 38:
                set_pixel(x, y, 15)
            elif 46 * 46 <= d2 <= 50 * 50:
                set_pixel(x, y, 235)

    # Checkerboard.
    for y in range(12, 76):
        for x in range(170, 234):
            if ((x // 8) + (y // 8)) % 2 == 0:
                set_pixel(x, y, 20)
            else:
                set_pixel(x, y, 225)

    return pixels


def make_odd_test_image(width: int = 250, height: int = 251) -> list[int]:
    pixels = [0] * (width * height)
    for y in range(height):
        for x in range(width):
            band = ((x // 24) + (y // 24)) % 2
            value = 25 if band == 0 else 225
            pixels[y * width + x] = max(0, min(255, value + (x % 9)))
    return pixels


def run(cmd: list[str], log_path: Path) -> None:
    completed = subprocess.run(
        cmd,
        cwd=ROOT,
        text=True,
        stdout=subprocess.PIPE,
        stderr=subprocess.STDOUT,
        check=False,
    )
    log_path.write_text(completed.stdout, encoding="utf-8")
    if completed.returncode != 0:
        print(completed.stdout, file=sys.stderr)
        raise SystemExit(f"command failed: {' '.join(cmd)}")


def histogram_image(pixels: list[int], width: int = 640, height: int = 360) -> list[tuple[int, int, int]]:
    image = [(255, 255, 255)] * (width * height)

    def set_px(x: int, y: int, color: tuple[int, int, int]) -> None:
        if 0 <= x < width and 0 <= y < height:
            image[y * width + x] = color

    def draw_line(
        x0: int,
        y0: int,
        x1: int,
        y1: int,
        color: tuple[int, int, int],
    ) -> None:
        dx = abs(x1 - x0)
        dy = -abs(y1 - y0)
        sx = 1 if x0 < x1 else -1
        sy = 1 if y0 < y1 else -1
        err = dx + dy
        while True:
            set_px(x0, y0, color)
            if x0 == x1 and y0 == y1:
                break
            e2 = 2 * err
            if e2 >= dy:
                err += dy
                x0 += sx
            if e2 <= dx:
                err += dx
                y0 += sy

    counts = [0] * 256
    for p in pixels:
        if 0 <= p <= 255:
            counts[p] += 1

    left, right, top, bottom = 55, 20, 25, 35
    plot_w = width - left - right
    plot_h = height - top - bottom
    max_count = max(counts)

    draw_line(left, top, left, height - bottom, (20, 20, 20))
    draw_line(left, height - bottom, width - right, height - bottom, (20, 20, 20))

    bar_w = max(1, plot_w // 256)
    for value, count in enumerate(counts):
        if count == 0:
            continue
        bar_h = max(1, int(round(count * plot_h / max_count)))
        x0 = left + value * plot_w // 256
        x1 = min(width - right - 1, x0 + bar_w)
        y0 = height - bottom - bar_h
        y1 = height - bottom
        for y in range(y0, y1):
            for x in range(x0, x1 + 1):
                set_px(x, y, (90, 90, 90))

    return image


def lorenz_trajectory() -> tuple[list[float], list[float], list[float], list[float]]:
    a, b, c, r = 10.0, 8.0 / 3.0, 28.0, -1.0
    h = 0.001
    discard = 2000
    samples = 22000

    def deriv(state: tuple[float, float, float, float]) -> tuple[float, float, float, float]:
        x, y, z, w = state
        return (
            a * (y - x) + w,
            c * x - y - x * z,
            x * y - b * z,
            -y * z + r * w,
        )

    def rk4(state: tuple[float, float, float, float]) -> tuple[float, float, float, float]:
        k1 = deriv(state)
        k2 = deriv(tuple(state[i] + h * 0.5 * k1[i] for i in range(4)))
        k3 = deriv(tuple(state[i] + h * 0.5 * k2[i] for i in range(4)))
        k4 = deriv(tuple(state[i] + h * k3[i] for i in range(4)))
        return tuple(
            state[i] + h / 6.0 * (k1[i] + 2 * k2[i] + 2 * k3[i] + k4[i])
            for i in range(4)
        )

    state = (1.0, 1.0, 1.0, 1.0)
    for _ in range(discard):
        state = rk4(state)

    xs: list[float] = []
    ys: list[float] = []
    zs: list[float] = []
    ws: list[float] = []
    for _ in range(samples):
        state = rk4(state)
        xs.append(state[0])
        ys.append(state[1])
        zs.append(state[2])
        ws.append(state[3])
    return xs, ys, zs, ws


def phase_plot_image(width: int = 1200, height: int = 400) -> list[tuple[int, int, int]]:
    xs, ys, zs, ws = lorenz_trajectory()
    pairs = [(xs, ys), (xs, zs), (xs, ws)]
    panel_count = len(pairs)
    gap = 35
    margin = 45
    panel_w = (width - 2 * margin - (panel_count - 1) * gap) // panel_count
    panel_h = height - 2 * margin
    image = [(255, 255, 255)] * (width * height)

    def set_px(x: int, y: int, color: tuple[int, int, int]) -> None:
        if 0 <= x < width and 0 <= y < height:
            image[y * width + x] = color

    def draw_line(
        x0: int,
        y0: int,
        x1: int,
        y1: int,
        color: tuple[int, int, int],
    ) -> None:
        dx = abs(x1 - x0)
        dy = -abs(y1 - y0)
        sx = 1 if x0 < x1 else -1
        sy = 1 if y0 < y1 else -1
        err = dx + dy
        while True:
            set_px(x0, y0, color)
            if x0 == x1 and y0 == y1:
                break
            e2 = 2 * err
            if e2 >= dy:
                err += dy
                x0 += sx
            if e2 <= dx:
                err += dx
                y0 += sy

    for panel_index, (a, b) in enumerate(pairs):
        left = margin + panel_index * (panel_w + gap)
        top = margin
        right = left + panel_w
        bottom = top + panel_h
        min_a, max_a = min(a), max(a)
        min_b, max_b = min(b), max(b)
        span_a = max(1e-12, max_a - min_a)
        span_b = max(1e-12, max_b - min_b)

        draw_line(left, top, left, bottom, (20, 20, 20))
        draw_line(left, bottom, right, bottom, (20, 20, 20))

        previous: tuple[int, int] | None = None
        for i in range(len(a)):
            px = left + int((a[i] - min_a) / span_a * (panel_w - 1))
            py = bottom - int((b[i] - min_b) / span_b * (panel_h - 1))
            if previous is not None:
                draw_line(previous[0], previous[1], px, py, (35, 70, 190))
            previous = (px, py)

    return image


def entropy(pixels: list[int], maxval: int = 255) -> float:
    counts = [0] * (maxval + 1)
    for p in pixels:
        if 0 <= p <= maxval:
            counts[p] += 1
    total = len(pixels)
    value = 0.0
    for count in counts:
        if count == 0:
            continue
        p = count / total
        value -= p * math.log2(p)
    return value


def adjacent_correlation(pixels: list[int], width: int, height: int, horizontal: bool = True) -> float:
    xs: list[float] = []
    ys: list[float] = []
    if horizontal:
        for y in range(height):
            for x in range(width - 1):
                xs.append(float(pixels[y * width + x]))
                ys.append(float(pixels[y * width + x + 1]))
    else:
        for y in range(height - 1):
            for x in range(width):
                xs.append(float(pixels[y * width + x]))
                ys.append(float(pixels[(y + 1) * width + x]))

    n = len(xs)
    if n == 0:
        return 0.0
    mean_x = sum(xs) / n
    mean_y = sum(ys) / n
    cov = sum((x - mean_x) * (y - mean_y) for x, y in zip(xs, ys))
    var_x = sum((x - mean_x) ** 2 for x in xs)
    var_y = sum((y - mean_y) ** 2 for y in ys)
    if var_x == 0.0 or var_y == 0.0:
        return 0.0
    return cov / math.sqrt(var_x * var_y)


def mse(a: list[int], b: list[int]) -> float:
    if len(a) != len(b):
        raise ValueError("MSE requires equal-length images")
    return sum((x - y) ** 2 for x, y in zip(a, b)) / len(a)


def psnr(a: list[int], b: list[int]) -> float:
    error = mse(a, b)
    if error == 0.0:
        return float("inf")
    return 10.0 * math.log10((255.0 * 255.0) / error)


def main() -> None:
    RESULTS.mkdir(parents=True, exist_ok=True)

    plain_path = RESULTS / "plain_256.pgm"
    plain_odd_path = RESULTS / "plain_odd.pgm"
    cipher_path = RESULTS / "cipher_256.pgm"
    preview_path = RESULTS / "preview_256.pgm"
    recovered_path = RESULTS / "recovered_256.pgm"
    wrong_path = RESULTS / "wrong_key_256.pgm"
    cipher_odd_path = RESULTS / "cipher_odd.pgm"
    preview_odd_path = RESULTS / "preview_odd.pgm"
    recovered_odd_path = RESULTS / "recovered_odd.pgm"

    plain = make_test_image()
    plain_odd = make_odd_test_image()
    write_pgm_8(plain_path, 256, 256, plain)
    write_pgm_8(plain_odd_path, 250, 251, plain_odd)

    run(
        [
            str(BINARY),
            "test",
            str(plain_path),
            str(cipher_path),
            str(recovered_path),
            "--preview",
            str(preview_path),
        ],
        RESULTS / "test_256.log",
    )
    run(
        [
            str(BINARY),
            "decrypt",
            str(cipher_path),
            str(wrong_path),
            "--x0",
            "1.000001",
        ],
        RESULTS / "wrong_key_256.log",
    )
    run(
        [
            str(BINARY),
            "test",
            str(plain_odd_path),
            str(cipher_odd_path),
            str(recovered_odd_path),
            "--preview",
            str(preview_odd_path),
        ],
        RESULTS / "test_odd.log",
    )

    _, _, _, preview = read_pgm(preview_path)
    _, _, _, recovered = read_pgm(recovered_path)
    _, _, _, wrong = read_pgm(wrong_path)
    _, _, _, preview_odd = read_pgm(preview_odd_path)
    _, _, _, recovered_odd = read_pgm(recovered_odd_path)

    write_png_gray(RESULTS / "plain_256.png", 256, 256, plain)
    write_png_gray(RESULTS / "preview_256.png", 256, 256, preview)
    write_png_gray(RESULTS / "recovered_256.png", 256, 256, recovered)
    write_png_gray(RESULTS / "wrong_key_256.png", 256, 256, wrong)
    write_png_gray(RESULTS / "plain_odd.png", 250, 251, plain_odd)
    write_png_gray(RESULTS / "preview_odd.png", 250, 251, preview_odd)
    write_png_gray(RESULTS / "recovered_odd.png", 250, 251, recovered_odd)

    write_png_rgb(RESULTS / "plain_hist.png", 640, 360, histogram_image(plain))
    write_png_rgb(RESULTS / "cipher_hist.png", 640, 360, histogram_image(preview))
    write_png_rgb(RESULTS / "lorenz_phase.png", 1200, 400, phase_plot_image())

    lines = [
        "# 实验数据摘要",
        "",
        "## 256×256 测试图",
        "",
        "| 指标 | 数值 |",
        "|---|---:|",
        f"| 明文信息熵 | {entropy(plain):.6f} bit |",
        f"| 密文预览信息熵 | {entropy(preview):.6f} bit |",
        f"| 密文预览水平相关系数 | {adjacent_correlation(preview, 256, 256, True):.6f} |",
        f"| 密文预览垂直相关系数 | {adjacent_correlation(preview, 256, 256, False):.6f} |",
        f"| 恢复图像 MSE | {mse(plain, recovered):.6f} |",
        f"| 恢复图像 PSNR | {psnr(plain, recovered):.6f} dB |",
        f"| 错误密钥恢复 MSE | {mse(plain, wrong):.6f} |",
        f"| 错误密钥恢复 PSNR | {psnr(plain, wrong):.6f} dB |",
        "",
        "## 250×251 非 8 倍数尺寸测试图",
        "",
        "| 指标 | 数值 |",
        "|---|---:|",
        f"| 密文预览信息熵 | {entropy(preview_odd):.6f} bit |",
        f"| 密文预览水平相关系数 | {adjacent_correlation(preview_odd, 250, 251, True):.6f} |",
        f"| 密文预览垂直相关系数 | {adjacent_correlation(preview_odd, 250, 251, False):.6f} |",
        f"| 恢复图像 MSE | {mse(plain_odd, recovered_odd):.6f} |",
        f"| 恢复图像 PSNR | {psnr(plain_odd, recovered_odd):.6f} dB |",
        "",
    ]
    (RESULTS / "metrics_summary.md").write_text("\n".join(lines), encoding="utf-8")
    print(f"generated report assets under {RESULTS}")


if __name__ == "__main__":
    main()
