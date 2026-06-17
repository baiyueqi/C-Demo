# Medium: KeygenMe_3_SWD

## 当前静态入口

- URL: https://crackmes.one/crackme/69fa884dd7ff92e1214c0024
- 文件名: `KeygenMe_3_SWD.exe`
- 样本路径: `Assembly/binaries/medium_keygenme_3_swd/extracted/KeygenMe_3_SWD.exe`
- SHA256: `73758bc4a5fa71719bf031122c903feadfd7d5094bda87d997e3591937463958`
- 平台/架构: Windows PE32+ / x86-64 console
- 类型: 控制台 KeygenMe，输入 Username、Secret code、Verification PIN 后输出校验结果。

## 已确认字符串

- 字符表: `0123456789ABCDEFGHIJKLMNOPQRSTUVWXYZ`，地址 `0x140007420`
- 标题: `KeygenMe No 3`，地址 `0x1400074A0`
- 清屏命令: `cls`，地址 `0x1400074D0`
- 用户名提示: `Username: `，地址 `0x1400074D8`
- Secret code 提示: `Secret code: `，地址 `0x140007500`
- Verification PIN 提示: `Verification PIN: `，地址 `0x140007528`
- Secret code 格式错误重打提示: `\x1b[1A\x1b[2K\rSecret code: `，地址 `0x140007510`
- Verification PIN 格式错误重打提示: `\x1b[1A\x1b[2K\rVerification PIN: `，地址 `0x140007540`
- 结果箭头: ` -> `，地址 `0x140007560` / `0x140007574`
- 输出标签: `Secret code`，地址 `0x140007568`
- 输出标签: `Verification PIN`，地址 `0x140007580`

## 已确认流程

- `0x140002BD0` 附近是主交互流程候选入口。
- `0x140002C41` 读取 Username。
- `0x140002C5E` 调用字符串长度函数，判断用户名长度必须在 3 到 15 之间。
- `0x140001E30` 是用户名字符格式校验，要求非空且所有字符满足 `isalnum`。
- `0x140002CAF` 读取 Secret code。
- `0x140002CCC` 调用 `0x140001EC0` 校验 Secret code 格式。
- `0x140001EC0` 要求 Secret code 长度为 14，`[4]` 和 `[10]` 是 `-`，其他字符满足 `isxdigit`。结合后续 base36 解析逻辑，实际可用字符集应按 `0-9A-Z` 处理。
- `0x140002D08` 读取 Verification PIN。
- `0x140002D2D` 调用 `0x140001F80` 校验 Verification PIN 格式，并把十进制 PIN 转为整数保存到 `[rsp+0x218]`。
- `0x140002D69` 之后进入实际校验逻辑，先调用 `0x140002100` 比较 Username 与 Secret code，再调用 `0x140002910` 比较 Username、Secret code 与 PIN。

## Secret code 算法

### 关键函数

- `0x140001FF0`: base36 字符串转整数。数字字符按 `ch - '0'`，字母字符按 `ch - 0x37`，每轮 `acc = acc * 36 + value`。
- `0x140002100`: Secret code 核心校验函数。
- `0x140003930`: `std::string::substr` 包装函数，用于截取 `XXXX`、`XXXXX`、`XXX` 三段。

### Secret code 分段

程序把 Secret code 拆为 3 段：

| 段 | 下标范围 | 长度 | 进制 |
| --- | --- | --- | --- |
| part1 | `[0:4]` | 4 | base36 |
| part2 | `[5:10]` | 5 | base36 |
| part3 | `[11:14]` | 3 | base36 |

### 用户名派生值

`0x140002100` 前半段会先构造一个用户名副本，并通过 `0x140003530` 对字符逐个调用 `toupper`。因此后续算法使用的是大写化后的用户名；例如输入 `abc` 实际按 `ABC` 参与计算。

`0x140002300` 到 `0x14000239E` 遍历大写化后的用户名，计算 3 个中间值：

```c
uint32_t sum_weighted = 0;
uint32_t xor_mix = 0;
uint32_t product_mix = 1;

for (size_t i = 0; i < username.length(); i++) {
    uint32_t ch = username[i];
    sum_weighted += ch * (i + 1);
    xor_mix ^= ch + i;
    product_mix = (product_mix * (ch + 3)) % 100000;
}
```

随后生成 3 个目标值：

```c
part1_expected = (sum_weighted ^ 0x5A5A) % 46656;
part2_expected = (xor_mix * 1337 + product_mix) % 60466176;
part3_expected = (product_mix + sum_weighted + xor_mix) % 46656;
```

校验条件：

```c
part1 == part1_expected;
part2 == part2_expected;
part3 == part3_expected;
```

其中 `46656 = 36^3`，`60466176 = 36^5`。输出时把三段分别编码为固定宽度 base36，格式为 `XXXX-XXXXX-XXX`。第一段虽然只会落在 `36^3` 范围内，但程序截取和格式要求是 4 位，所以脚本左侧补 `0`。

## Verification PIN 算法

### 关键函数

- `0x1400025A0`: 根据 Username 和 Secret code 计算 31 位 PIN 候选值。
- `0x1400028D0`: 对候选 PIN 和用户输入 PIN 分别做同一个 32 位变换。
- `0x140002910`: PIN 比较函数，要求变换后的 4 个字节全部相等。
- `0x1400020A0`: 32 位循环左移。
- `0x1400020D0`: 32 位循环右移。

### PIN 输入材料

`0x1400025A0` 也会先把用户名复制为大写形式。`0x140002674` 到 `0x1400026A5` 把输入材料构造成：

```c
material = uppercase(username) + char(secret_code.length() ^ 0x5A) + secret_code;
```

Secret code 固定长度为 14，所以插入字节为 `14 ^ 0x5A = 0x54`，也就是字符 `T`。

### PIN 哈希伪代码

```c
uint32_t a = 0xA3B1C2D3;
uint32_t b = 0x1F2E3D4C;

for (size_t i = 0; i < material.length(); i++) {
    uint32_t ch = material[i];
    a ^= ch + i * 0x11;
    a = rol32(a, i % 5 + 3);
    a += b ^ 0x9E3779B9;
    b ^= a + ch * 0x83;
    b = ror32(b, i % 7 + 2);
    b += (a << 3) ^ 0x7F4A7C15;
    if (i & 1) {
        swap(a, b);
    }
}

uint32_t pin = a ^ b;
pin ^= pin >> 16;
pin *= 0x85EBCA6B;
pin ^= pin >> 13;
pin *= 0xC2B2AE35;
pin ^= pin >> 16;
pin &= 0x7FFFFFFF;
```

`0x140002910` 会对该候选值和用户输入的 PIN 都调用 `0x1400028D0`，再逐字节比较变换结果。直接输出上述 31 位 `pin` 可以稳定通过。

## Keygen

- 脚本: `Assembly/scripts/medium_keygen.py`
- 用法:

```bash
python3 Assembly/scripts/medium_keygen.py Alice
```

示例输出:

```text
Username: Alice
Secret code: 0IMB-040EG-YZL
Verification PIN: 775865310
Self-check: ok
```

## 验证结果

| 编号 | Username | Secret code | Verification PIN | 说明 |
| --- | --- | --- | --- | --- |
| 1 | `abc` | `0I50-02N6V-MB1` | `1846922373` | 最短合法长度边界 |
| 2 | `Alice` | `0IMB-040EG-YZL` | `775865310` | 普通字母用户名 |
| 3 | `Student2026` | `0EOJ-01C98-62D` | `829076332` | 字母数字混合用户名 |

## 下一步分析目标

1. 在 IDA 中把 `0x140002BD0` 命名为 `main_interaction`。
2. 把 `0x140002100` 命名为 `verify_secret_code`。
3. 把 `0x1400025A0` 命名为 `generate_pin_candidate`。
4. 把 `0x140002910` 命名为 `verify_pin`。
5. 用 Windows/x64dbg 实际输入上方 3 组样例，补充成功分支截图。
