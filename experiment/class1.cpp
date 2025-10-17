#include <iostream>
#include <vector>
#include <string>
#include <sstream>
#include <algorithm>

using namespace std;

//基类
class Base {
public:
    virtual ~Base() {}
    virtual void createFromInput() = 0;               // 从键盘输入若干元素创建
    virtual void insertAfterValue(int target, const vector<int>& vals, bool keep_sorted=false) = 0; // 在目标元素后插入多个元素
    virtual void deleteValueAll(int val) = 0;         // 删除值为 val 的所有元素
    virtual void deleteValues(const vector<int>& vals) = 0; // 批量删除不同值（对每个值都删除所有出现）
    virtual void display() const = 0;                 // 显示表中所有元素
};

//顺序表实现（基于 vector）
class SeqList : public Base {
private:
    vector<int> data;
public:
    SeqList() {}
    ~SeqList() override {}

    // 从键盘读取元素，格式：若干整数 token，以单独的 @ 结束
    void createFromInput() override {
        data.clear();
        cout << "请输入若干整数（每个 <100），以 @ 结束：\n";
        while (true) {
            string token;
            if (!(cin >> token)) return;
            if (token == "@") break;
            // 尝试转换为整数
            try {
                int v = stoi(token);
                if (abs(v) >= 100) {
                    cout << "警告: 元素 " << v << " 的绝对值应 <100，已忽略该元素。\n";
                } else {
                    data.push_back(v);
                }
            } catch (...) {
                cout << "非法输入 '" << token << "'，请重新输入或使用 @ 结束。\n";
            }
        }
    }

    // 在 target 后插入 vals（按 vals 的顺序插入）。如果 target 不存在，插入到末尾。
    // keep_sorted==true 时插入完毕后对整个顺序表进行非递减排序
    void insertAfterValue(int target, const vector<int>& vals, bool keep_sorted=false) override {
        if (vals.empty()) return;
        auto it = find(data.begin(), data.end(), target);
        if (it == data.end()) {
            // 目标不存在 -> append to end
            data.insert(data.end(), vals.begin(), vals.end());
        } else {
            // 在第一个匹配目标之后插入一批值
            ++it; // 指向插入位置
            data.insert(it, vals.begin(), vals.end());
        }
        if (keep_sorted) {
            sort(data.begin(), data.end()); // 非递减排序
        }
    }

    // 删除所有值为 val 的元素
    void deleteValueAll(int val) override {
        // 使用 erase-remove idiom 删除所有匹配项
        auto old_size = data.size();
        data.erase(remove(data.begin(), data.end(), val), data.end());
        auto removed = old_size - data.size();
        cout << "已从顺序表删除 " << removed << " 个值为 " << val << " 的元素。\n";
    }

    // 批量删除列表中出现的所有值（对每个 vals[i] 都删除其所有出现）
    void deleteValues(const vector<int>& vals) override {
        for (int v : vals) deleteValueAll(v);
    }

    void display() const override {
        cout << "顺序表内容 (" << data.size() << " 个元素): ";
        if (data.empty()) {
            cout << "[空]\n";
            return;
        }
        for (size_t i = 0; i < data.size(); ++i) {
            if (i) cout << " ";
            cout << data[i];
        }
        cout << "\n";
    }
};

//单链表实现
class LinList : public Base {
private:
    struct Node {
        int val;
        Node* next;
        Node(int v=0): val(v), next(nullptr) {}
    };
    Node* head; // 哨兵头（不存数据）
public:
    LinList() {
        head = new Node(); // 哨兵
    }
    ~LinList() override {
        clear();
        delete head;
    }

    // 清空链表（释放节点）
    void clear() {
        Node* cur = head->next;
        while (cur) {
            Node* tmp = cur;
            cur = cur->next;
            delete tmp;
        }
        head->next = nullptr;
    }

    // 从键盘输入创建链表（追加到尾部）
    void createFromInput() override {
        clear();
        cout << "请输入若干整数（每个 <100），以 @ 结束：\n";
        Node* tail = head;
        while (tail->next) tail = tail->next;
        while (true) {
            string token;
            if (!(cin >> token)) return;
            if (token == "@") break;
            try {
                int v = stoi(token);
                if (abs(v) >= 100) {
                    cout << "警告: 元素 " << v << " 的绝对值应 <100，已忽略该元素。\n";
                    continue;
                }
                Node* n = new Node(v);
                tail->next = n;
                tail = n;
            } catch (...) {
                cout << "非法输入 '" << token << "'，请重新输入或使用 @ 结束。\n";
            }
        }
    }

    // 在目标值节点（第一个匹配）之后插入 vals（按顺序），若目标不存在则追加到尾部
    // 不使用下标或 locate(i)
    void insertAfterValue(int target, const vector<int>& vals, bool /*keep_sorted*/ = false) override {
        if (vals.empty()) return;
        Node* cur = head->next;
        Node* found = nullptr;
        while (cur) {
            if (cur->val == target) { found = cur; break; }
            cur = cur->next;
        }
        if (!found) {
            // 追加到尾部
            cur = head;
            while (cur->next) cur = cur->next;
            for (int v : vals) {
                Node* n = new Node(v);
                cur->next = n;
                cur = n;
            }
        } else {
            // 在 found 之后插入一段链（维护顺序为 vals 中给出的顺序）
            Node* next_part = found->next;
            Node* prev = found;
            for (int v : vals) {
                Node* n = new Node(v);
                prev->next = n;
                prev = n;
            }
            prev->next = next_part;
        }
    }

    // 删除链表中所有值为 val 的节点（不使用下标）
    void deleteValueAll(int val) override {
        Node* prev = head;
        Node* cur = head->next;
        int count = 0;
        while (cur) {
            if (cur->val == val) {
                // 删除 cur
                prev->next = cur->next;
                delete cur;
                cur = prev->next;
                ++count;
            } else {
                prev = cur;
                cur = cur->next;
            }
        }
        cout << "已从链表删除 " << count << " 个值为 " << val << " 的节点。\n";
    }

    void deleteValues(const vector<int>& vals) override {
        for (int v : vals) deleteValueAll(v);
    }

    // 原地逆序单链表（不借助额外数组或链表）
    void reverseInPlace() {
        Node* prev = nullptr;
        Node* cur = head->next;
        while (cur) {
            Node* nxt = cur->next;
            cur->next = prev;
            prev = cur;
            cur = nxt;
        }
        // prev 成为新的首节点，链接回哨兵头
        head->next = prev;
    }

    void display() const override {
        cout << "单链表内容: ";
        Node* cur = head->next;
        if (!cur) {
            cout << "[空]\n";
            return;
        }
        bool first = true;
        while (cur) {
            if (!first) cout << " ";
            cout << cur->val;
            first = false;
            cur = cur->next;
        }
        cout << "\n";
    }
};

//辅助函数：从键盘读一串整数直到 @
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

//主交互程序
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
        if (p) { delete p; p = nullptr; } // 切换时释放之前对象

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

        // UI-2: 子菜单（对选中的表执行操作），循环直到返回上级或退出
        bool inner = true;
        while (inner) {
            cout << "\n------ 子菜单 ------\n";
            cout << "1) 表创建（从键盘输入若干元素）\n";
            cout << "2) 元素插入（在特定元素值后插入多个新元素）\n";
            cout << "3) 元素删除（按元素值删除所有相同元素）\n";
            cout << "4) 批量删除（一次性删除多个不同值）\n";
            if (dynamic_cast<LinList*>(p)) cout << "5) 单链表原地逆序并输出（仅对单链表有效）\n";
            cout << "6) 显示表内容\n";
            cout << "7) 返回主页面\n";
            cout << "0) 退出程序\n";
            cout << "请选择操作: ";
            int cd;
            if (!(cin >> cd)) { inner = false; running = false; break; }

            switch (cd) {
            case 1: {
                p->createFromInput();
                break;
            }
            case 2: {
                cout << "请输入目标元素值（将在该元素后插入新元素，如果目标不存在将追加到末尾）： ";
                int target;
                if (!(cin >> target)) { cout << "输入错误。\n"; break; }
                vector<int> newvals = readMultipleIntsFromInput();
                bool keep_sorted = false;
                // 仅对顺序表提供非递减排序选项
                if (dynamic_cast<SeqList*>(p)) {
                    cout << "是否在插入后对顺序表进行非递减排序？(1 是 / 0 否): ";
                    int t; if (cin >> t) keep_sorted = (t != 0);
                }
                p->insertAfterValue(target, newvals, keep_sorted);
                cout << "插入完成。\n";
                break;
            }
            case 3: {
                cout << "请输入要删除的元素值（将删除所有相同值的节点）: ";
                int val;
                if (!(cin >> val)) { cout << "输入错误。\n"; break; }
                p->deleteValueAll(val);
                break;
            }
            case 4: {
                cout << "批量删除：请输入多个要删除的不同元素值，以 @ 结束：\n";
                vector<int> toDelete = readMultipleIntsFromInput();
                p->deleteValues(toDelete);
                break;
            }
            case 5: {
                // 仅 LinList 可用
                LinList* lp = dynamic_cast<LinList*>(p);
                if (!lp) {
                    cout << "仅单链表支持原地逆序操作。\n";
                } else {
                    cout << "正在对单链表原地逆序...\n";
                    lp->reverseInPlace();
                    cout << "逆序完成，当前链表为：\n";
                    lp->display();
                }
                break;
            }
            case 6: {
                p->display();
                break;
            }
            case 7: {
                inner = false; // 返回主页面
                break;
            }
            case 0: {
                inner = false; running = false;
                break;
            }
            default:
                cout << "未知选项，请重试。\n";
            } // switch
        } // inner while
    } // outer while

    if (p) { delete p; p = nullptr; }
    cout << "程序退出。\n";
    return 0;
}
