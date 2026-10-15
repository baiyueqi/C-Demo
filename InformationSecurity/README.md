# 基于超混沌 Lorenz 系统的 DCT 域图像加密

本目录实现课程实验的加密与解密程序。算法采用：

- 8 x 8 二维离散余弦变换 DCT；
- 四维超混沌 Lorenz 系统；
- 4 阶 Runge-Kutta 法生成混沌序列；
- DCT 系数置乱和模加扩散；
- 密文保存为 16-bit PGM 系数图，便于无损反解。

## 文件结构

- `include/common.h`：公共常量和工具函数；
- `include/pgm.h` / `src/pgm.cpp`：二进制 PGM 读写；
- `include/chaos.h` / `src/chaos.cpp`：超混沌 Lorenz 系统、RK4、置乱和扩散序列生成；
- `include/dct.h` / `src/dct.cpp`：8 x 8 DCT / IDCT；
- `include/cipher.h` / `src/cipher.cpp`：加密、解密、预览图生成；
- `include/metrics.h` / `src/metrics.cpp`：MSE、PSNR、信息熵、相邻像素相关系数；
- `src/main.cpp`：命令行入口。

## 构建

在仓库根目录进入 `nix develop` 后执行：

```bash
make -C InformationSecurity
```

或在 `InformationSecurity` 目录下：

```bash
make
```

## 使用

输入图像使用 8-bit 二进制 PGM，即 P5 格式。

```bash
./image_cipher encrypt input.pgm cipher.pgm --preview preview.pgm
./image_cipher decrypt cipher.pgm recovered.pgm
./image_cipher test input.pgm cipher.pgm recovered.pgm --preview preview.pgm
```

默认 Lorenz 初值为 `(1.0, 1.0, 1.0, 1.0)`，可以通过以下选项修改：

```bash
--x0 1.0 --y0 1.0 --z0 1.0 --w0 1.0
```

加密和解密必须使用同一组初值和参数。

## 说明

密文是 DCT 系数图，不是空间域经过 IDCT 后的普通灰度图。程序额外生成 `_preview.pgm`，把密文系数线性归一化到 8-bit，便于查看其类噪声效果。解密时读取 16-bit 密文系数图，因此可以精确还原置换和扩散前的整数量化 DCT 系数。

由于加密端使用 `q = 1` 的系数量化，恢复图像是原始图像的分块 DCT 量化近似，PSNR 通常会很高，但不是逐像素比特级无损。
