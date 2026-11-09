import sqlite3

DB_NAME = 'user_db.db'


def login(username, password):
    sql = "SELECT * FROM user_info WHERE username='" + username + "' AND password='" + password + "'"
    print("执行SQL语句：", sql)

    try:
        conn = sqlite3.connect(DB_NAME)
        cursor = conn.cursor()
        cursor.execute(sql)
        result = cursor.fetchone()
        return result
    except Exception as e:
        print("数据库异常：", e)
        return None
    finally:
        if 'conn' in locals():
            conn.close()


def init_database():
    try:
        conn = sqlite3.connect(DB_NAME)
        cursor = conn.cursor()
        cursor.execute('''CREATE TABLE IF NOT EXISTS user_info
                        (username TEXT PRIMARY KEY, password TEXT, role TEXT)''')

        # 用户密码存储
        cursor.execute("INSERT OR IGNORE INTO user_info VALUES ('admin','Admin@2026','super_admin')")
        cursor.execute("INSERT OR IGNORE INTO user_info VALUES ('user1','123456','normal_user')")

        conn.commit()
    except Exception as e:
        print("初始化失败：", e)
    finally:
        if 'conn' in locals():
            conn.close()


if __name__ == '__main__':
    init_database()
    print("===== 后台用户登录系统 =====")
    while True:
        user = input("请输入用户名：").strip()
        if user == 'exit':
            break

        pwd = input("请输入密码：").strip()

        # 空值校验
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
