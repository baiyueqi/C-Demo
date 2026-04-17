# 《信息论与编码》上机实验一

## 实验目的

1、熟悉并尝试实现唯一可译码的判定算法；

2、练习实现经典无失真信源编码方法，能够计算编码效率或压缩比；

3、学习编码和解码中的位操作。

---

## 实验要求

1、每位同学需提交实验报告（电子档）一份。应包括如下内容:

1）实习题目；

2）题目分析、相关原理及设计思路；

3）详细步骤和相应截图；

4）实验中遇到的困难及解决方法、上机心得或课程建议；

2、报告电子档文件名必须命名为：信息论与编码实验一-班学号-姓名，可以是 pdf 或 word 格式，文件名中缺少学号姓名可能会造成后续平时成绩漏统计。

3、每位同学需将上机实验报告在 QQ 群作业中提交。

---

## 实验内容

1、实现 LZ-77 译码算法，并使用 zlib 库对若干字符串进行压缩，测试译码算法的正确性，输出压缩率。至少包括对自然对数的压缩和解压缩测试：

2.71828182845904523536

2.718281828459045235360287471352662497757247093699959574966967627724076630353547594571382178525166427427466391

932003059921817413596629043572900334295260

2、实现 LZ-77 编码算法（选做）。

3、实现 LZ-78 编码和解码算法。

4、实现 LZW 编码和解码算法。

5、实现费诺码、香农码、霍夫曼码、算术码中的某一种（选做）。

6、实现唯一可译码的判定算法（选做）。

---

## 附录：LZ-77 固定霍夫曼树编码格式规范

As noted above, encoded data blocks in the "deflate" format consist of sequences of symbols drawn from three conceptually distinct alphabets: either literal bytes, from the alphabet of byte values (0..255), or <length, backward distance> pairs, where the length is drawn from (3..258) and the distance is drawn from (1..32,768). In fact, the literal and length alphabets are merged into a single alphabet (0..285), where values 0..255 represent literal bytes, the value 256 indicates end-of-block, and values 257..285 represent length codes (possibly in conjunction with extra bits following the symbol code) as follows:

Extra Extra Extra

Code Bits Length(s) Code Bits Lengths Code Bits Length(s)

---- ---- ------ ---- ---- ------- ---- ---- -------

257 0 3 267 1 15,16 277 4 67-82  
258 0 4 268 1 17,18 278 4 83-98  
259 0 5 269 2 19-22 279 4 99-114  
260 0 6 270 2 23-26 280 4 115-130  
261 0 7 271 2 27-30 281 5 131-162  
262 0 8 272 2 31-34 282 5 163-194  
263 0 9 273 3 35-42 283 5 195-226  
264 0 10 274 3 43-50 284 5 227-257  
265 1 11,12 275 3 51-58 285 0 258  
266 1 13,14 276 3 59-66  

The extra bits should be interpreted as a machine integer stored with the most-significant bit first, e.g., bits 1110 represent the value 14.

Extra Extra Extra

Code Bits Dist Code Bits Dist Code Bits Distance

---- ---- ---- ---- ---- ------ ---- ---- --------

0 0 1 10 4 33-48 20 9 1025-1536  
1 0 2 11 4 49-64 21 9 1537-2048  
2 0 3 12 5 65-96 22 10 2049-3072  
3 0 4 13 5 97-128 23 10 3073-4096  
4 1 5,6 14 6 129-192 24 11 4097-6144  
5 1 7,8 15 6 193-256 25 11 6145-8192  
6 2 9-12 16 7 257-384 26 12 8193-12288  
7 2 13-16 17 7 385-512 27 12 12289-16384  
8 3 17-24 18 8 513-768 28 13 16385-24576  
9 3 25-32 19 8 769-1024 29 13 24577-32768  

### 3.2.6. Compression with fixed Huffman codes (BTYPE=01)

The Huffman codes for the two alphabets are fixed, and are not represented explicitly in the data. The Huffman code lengths for the literal/length alphabet are:

Lit Value Bits Codes

--------- ---- -----

0 - 143 8 00110000 through 10111111  
144 - 255 9 110010000 through 111111111  
256 - 279 7 0000000 through 0010111  
280 - 287 8 11000000 through 11000111  

The code lengths are sufficient to generate the actual codes, as described above; we show the codes in the table for added clarity. Literal/length values 286-287 will never actually occur in the compressed data, but participate in the code construction.

Distance codes 0-31 are represented by (fixed-length) 5-bit codes, with possible additional bits as shown in the table shown in Paragraph 3.2.5, above. Note that distance codes 30-31 will never actually occur in the compressed data.