#include <iostream>
#include <string>
#include "Base.h"
#include "SeqList.h"
#include "LinList.h"
using namespace std;

vector<int> readMultipleIntsFromInput() {
    vector<int> vals;
    cout << "请输入要操作的若干整数，以 @ 结束：\n";
    while (true) {
        string token;
        if (!(cin >> token)) break;
        if (token == "@") break;
        try {
            int v = stoi(token);
            vals.push_back(v);
        } catch (...) {
            cout << "非法输入 '" << token << "'，请重新输入或使用 @ 结束。\n";
        }
    }
    return vals;
}

int main() {
    ios::sync_with_stdio(false);
    cin.tie(nullptr);

    Base* p = nullptr;
    bool running = true;

    while (running) {
        cout << "主页面：请选择表类型并创建（1 顺序表, 2 单链表, 0 退出）： ";
        int ch;
        if (!(cin >> ch)) break;
        if (ch == 0) break;
        if (p) { delete p; p = nullptr; }

        if (ch == 1) {
            p = new SeqList();
            cout << "已选择顺序表 (SeqList)。\n";
        } else if (ch == 2) {
            p = new LinList();
            cout << "已选择单链表 (LinList)。\n";
        } else {
            cout << "无效选择，请重新选择。\n";
            continue;
        }

        bool inner = true;
        while (inner) {
            cout << "\n------ 子菜单 ------\n";
            cout << "1) 表创建\n";
            cout << "2) 元素插入\n";
            cout << "3) 元素删除\n";
            cout << "4) 批量删除\n";
            if (dynamic_cast<LinList*>(p)) cout << "5) 单链表逆序\n";
            cout << "6) 显示表内容\n";
            cout << "7) 返回主页面\n";
            cout << "0) 退出程序\n";
            cout << "请选择操作: ";
            int cd;
            if (!(cin >> cd)) { inner = false; running = false; break; }

            switch (cd) {
            case 1:
                p->createFromInput();
                break;
            case 2: {
                cout << "请输入目标元素值: ";
                int target;
                if (!(cin >> target)) break;
                auto vals = readMultipleIntsFromInput();
                bool keep_sorted = false;
                if (dynamic_cast<SeqList*>(p)) {
                    cout << "是否在插入后排序？(1 是 / 0 否): ";
                    int t; cin >> t; keep_sorted = (t != 0);
                }
                p->insertAfterValue(target, vals, keep_sorted);
                break;
            }
            case 3: {
                cout << "请输入要删除的元素值: ";
                int v;
                if (!(cin >> v)) break;
                p->deleteValueAll(v);
                break;
            }
            case 4: {
                auto vals = readMultipleIntsFromInput();
                p->deleteValues(vals);
                break;
            }
            case 5: {
                LinList* lp = dynamic_cast<LinList*>(p);
                if (lp) {
                    lp->reverseInPlace();
                    cout << "逆序完成，链表内容如下：\n";
                    lp->display();
                } else {
                    cout << "顺序表不支持逆序。\n";
                }
                break;
            }
            case 6:
                p->display();
                break;
            case 7:
                inner = false;
                break;
            case 0:
                inner = false; running = false;
                break;
            default:
                cout << "未知选项。\n";
            }
        }
    }

    if (p) delete p;
    cout << "程序退出。\n";
    return 0;
}
