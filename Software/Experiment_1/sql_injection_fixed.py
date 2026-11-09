import hashlib
import hmac
import os
import sqlite3

DB_NAME = 'secure_user_db.db'
PBKDF2_ITERATIONS = 200_000


def hash_password(password, salt=None):
    """返回十六进制表示的盐值和密码哈希。"""
    if salt is None:
        salt = os.urandom(16)

    digest = hashlib.pbkdf2_hmac(
        'sha256',
        password.encode('utf-8'),
        salt,
        PBKDF2_ITERATIONS
    )
    return salt.hex(), digest.hex()


def init_database():
    try:
        conn = sqlite3.connect(DB_NAME)
        cursor = conn.cursor()
        cursor.execute('''CREATE TABLE IF NOT EXISTS user_info
                        (username TEXT PRIMARY KEY,
                         salt TEXT NOT NULL,
                         password_hash TEXT NOT NULL,
                         role TEXT)''')

        users = [
            ('admin', 'Admin@2026', 'super_admin'),
            ('user1', '123456', 'normal_user'),
        ]

        for username, password, role in users:
            cursor.execute(
                "SELECT username FROM user_info WHERE username=?",
                (username,)
            )
            if cursor.fetchone() is None:
                salt, digest = hash_password(password)
                cursor.execute(
                    "INSERT INTO user_info VALUES (?,?,?,?)",
                    (username, salt, digest, role)
                )

        conn.commit()
    except Exception as e:
        print("初始化失败：", e)
    finally:
        if 'conn' in locals():
            conn.close()


def login(username, password):
    # 修复点：使用参数化查询，用户输入不会再拼接为 SQL 语法的一部分
    sql = "SELECT username, salt, password_hash, role FROM user_info WHERE username=?"
    print("执行SQL语句：", sql)

    try:
        conn = sqlite3.connect(DB_NAME)
        cursor = conn.cursor()
        cursor.execute(sql, (username,))
        row = cursor.fetchone()

        if row is None:
            return None

        username_db, salt_hex, hash_hex, role = row
        salt = bytes.fromhex(salt_hex)
        expected_hash = hash_password(password, salt)[1]

        if hmac.compare_digest(expected_hash, hash_hex):
            return username_db, role
        return None
    except Exception as e:
        print("数据库异常：", e)
        return None
    finally:
        if 'conn' in locals():
            conn.close()


if __name__ == '__main__':
    init_database()
    print("===== 安全修复版后台用户登录系统 =====")
    while True:
        user = input("请输入用户名：").strip()
        if user == 'exit':
            break

        pwd = input("请输入密码：").strip()

        if not user or not pwd:
            print("用户名或密码不能为空！")
            continue

        info = login(user, pwd)
        if info is not None:
            print("登录成功！欢迎进入系统后台")
            print("用户信息：", info)
            break
        else:
            print("用户名或密码错误，登录失败")
        print()
