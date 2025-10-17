#include <iostream>
#include <vector>
#include <string>
#include <algorithm>
#include <cctype>
using namespace std;

// ---------- 辅助：判断 token 是否整数并转换 ----------
bool tryParseInt(const string &s, int &out) {
    if (s.empty()) return false;
    size_t i = 0;
    if (s[0] == '+' || s[0] == '-') {
        if (s.size() == 1) return false;
        i = 1;
    }
    for (; i < s.size(); ++i) if (!isdigit((unsigned char)s[i])) return false;
    try {
        out = stoi(s);
        return true;
    } catch (...) {
        return false;
    }
}

// ================= 基类 =================
class Base {
public:
    virtual ~Base() {}
    virtual void create() = 0;
    // 在 target 值后插入多个值（由交互读取，以 @ 结束）
    virtual void insertAfterTarget(int target) = 0;
    virtual void deleteValue(int value) = 0;
    virtual void display() const = 0;
    virtual void sortList() = 0; // 把排序功能抽出来
};

// ================= 顺序表 =================
class SeqList : public Base {
private:
    vector<int> data;
public:
    void create() override {
        cout << "请输入整数序列，以 @ 结束（例如：1 2 3 @）:\n";
        data.clear();
        string token;
        while (cin >> token) {
            if (token == "@") break;
            int v;
            if (tryParseInt(token, v)) data.push_back(v);
            else cout << "忽略非法输入: " << token << endl;
        }
        // 创建时不排序
        cout << "创建完成，当前顺序表：\n";
        display();
    }

    // 交互读取多个要插入的值（以 @ 结束），然后在 target 的最后一次出现之后依次插入
    void insertAfterTarget(int target) override {
        cout << "输入要插入的多个值，以 @ 结束（例如：4 5 6 @）:\n";
        vector<int> ins;
        string token;
        while (cin >> token) {
            if (token == "@") break;
            int v;
            if (tryParseInt(token, v)) ins.push_back(v);
            else cout << "忽略非法输入: " << token << endl;
        }
        if (ins.empty()) {
            cout << "没有有效插入值，操作取消。\n";
            return;
        }

        // 找到 target 的最后一次位置
        int pos = -1;
        for (int i = 0; i < (int)data.size(); ++i) if (data[i] == target) pos = i;

        if (pos == -1) {
            // 如果没找到 target，插到尾部（按题意也可以）
            data.insert(data.end(), ins.begin(), ins.end());
        } else {
            // 插入到 pos 之后：保持顺序依次插入
            data.insert(data.begin() + pos + 1, ins.begin(), ins.end());
        }

        cout << "插入完成，当前顺序表（未排序）：\n";
        display();
        
        sortList();
        cout << "排序后，当前顺序表：\n";
        display();
    }

    void deleteValue(int value) override {
        auto it = remove(data.begin(), data.end(), value);
        if (it == data.end()) {
            cout << "未找到值 " << value << "。\n";
        } else {
            data.erase(it, data.end());
            cout << "已删除所有值为 " << value << " 的元素。\n";
        }
    }

    void display() const override {
        cout << "顺序表元素：";
        for (int v : data) cout << v << " ";
        cout << "\n";
    }

    void sortList() override {
        sort(data.begin(), data.end());
    }
};

// ================= 单链表 =================
class LinList : public Base {
private:
    struct Node {
        int val;
        Node* next;
        Node(int v=0): val(v), next(nullptr) {}
    };
    Node* head; // 哨兵头结点，head->next 为首真实节点

public:
    LinList() {
        head = new Node(); // 哨兵
    }
    ~LinList() {
        Node* p = head;
        while (p) {
            Node* t = p;
            p = p->next;
            delete t;
        }
    }

    void create() override {
        cout << "请输入整数序列，以 @ 结束（例如：1 2 3 @）:\n";
        // 清空原链表（保留哨兵）
        Node* p = head->next;
        while (p) {
            Node* t = p;
            p = p->next;
            delete t;
        }
        head->next = nullptr;

        // 尾插保持输入顺序
        Node* tail = head;
        string token;
        while (cin >> token) {
            if (token == "@") break;
            int v;
            if (tryParseInt(token, v)) {
                Node* node = new Node(v);
                tail->next = node;
                tail = node;
            } else {
                cout << "忽略非法输入: " << token << endl;
            }
        }
        // 创建时不排序
        cout << "创建完成，当前链表：\n";
        display();
    }

    // 在 target 的最后一处之后插入多个值（由交互读取）
    void insertAfterTarget(int target) override {
        cout << "输入要插入的多个值，以 @ 结束（例如：4 5 6 @）:\n";
        vector<int> ins;
        string token;
        while (cin >> token) {
            if (token == "@") break;
            int v;
            if (tryParseInt(token, v)) ins.push_back(v);
            else cout << "忽略非法输入: " << token << endl;
        }
        if (ins.empty()) {
            cout << "没有有效插入值，操作取消。\n";
            return;
        }

        // 找到 target 的最后一个节点
        Node* p = head->next;
        Node* pos = nullptr;
        while (p) {
            if (p->val == target) pos = p;
            p = p->next;
        }

        Node* insertPoint = pos;
        if (!insertPoint) {
            // 没找到 target，则插在尾部
            insertPoint = head;
            while (insertPoint->next) insertPoint = insertPoint->next;
        }

        // 依次插入
        for (int v : ins) {
            Node* node = new Node(v);
            node->next = insertPoint->next;
            insertPoint->next = node;
            insertPoint = node; // 下次在刚插入的节点后插入
        }

        cout << "插入完成，当前链表（未排序）：\n";
        display();
        
        sortList();
        cout << "排序后，当前链表：\n";
        display();
    }

    void deleteValue(int value) override {
        // 删除头部连续等于 value 的节点
        while (head->next && head->next->val == value) {
            Node* t = head->next;
            head->next = t->next;
            delete t;
        }
        // 删除中间其它节点
        Node* p = head;
        bool found = false;
        while (p->next) {
            if (p->next->val == value) {
                Node* t = p->next;
                p->next = t->next;
                delete t;
                found = true;
            } else {
                p = p->next;
            }
        }
        if (!found && !(head->next && head->next->val == value)) {
            cout << "未找到值 " << value << "。\n";
        } else {
            cout << "已删除所有值为 " << value << " 的节点。\n";
        }
    }

    void display() const override {
        cout << "链表元素：";
        Node* p = head->next;
        while (p) {
            cout << p->val << " ";
            p = p->next;
        }
        cout << "\n";
    }

    // 原地交换节点值实现排序（稳定性不是重点）
    void sortList() override {
        // 取出节点到 vector 中（只为了排序值并回写），避免复杂的链表插入排序
        vector<int> vals;
        for (Node* p = head->next; p; p = p->next) vals.push_back(p->val);
        sort(vals.begin(), vals.end());
        Node* p = head->next;
        int i = 0;
        while (p) {
            p->val = vals[i++];
            p = p->next;
        }
    }

    // 逆序（就地指针翻转）
    void reverseList() {
        Node* prev = nullptr;
        Node* curr = head->next;
        while (curr) {
            Node* nxt = curr->next;
            curr->next = prev;
            prev = curr;
            curr = nxt;
        }
        head->next = prev;
    }
};

// ================= 主程序 =================
int main() {
    Base* list = nullptr;
    cout << "请选择要创建的数据结构：1. 顺序表  2. 单链表\n输入 1 或 2：";
    int t;
    while (!(cin >> t) || (t != 1 && t != 2)) {
        cout << "请输入 1 或 2：";
        cin.clear();
        string dummy; cin >> dummy;
    }
    if (t == 1) list = new SeqList();
    else list = new LinList();

    while (true) {
        cout << "\n----- 菜单 -----\n";
        cout << "1. 创建表（从头输入，@ 结束）\n";
        cout << "2. 插入元素（在指定值后插入多个，以 @ 结束）\n";
        cout << "3. 删除元素（按值删除所有）\n";
        cout << "4. 显示\n";
        cout << "5. 排序\n";
        cout << "6. 链表逆序（仅单链表有效）\n";
        cout << "0. 退出\n";
        cout << "请选择：";

        int op;
        if (!(cin >> op)) {
            cin.clear();
            string bad; cin >> bad;
            cout << "非法输入，重试。\n";
            continue;
        }

        if (op == 0) break;
        if (op == 1) {
            list->create();
        } else if (op == 2) {
            cout << "指定在哪个值之后插入（若未找到则插到尾部），请输入目标值：";
            string tok; cin >> tok;
            int target;
            if (!tryParseInt(tok, target)) {
                cout << "目标值必须为整数，操作取消。\n";
                continue;
            }
            list->insertAfterTarget(target);
        } else if (op == 3) {
            cout << "请输入要删除的值：";
            string tok; cin >> tok;
            int val;
            if (!tryParseInt(tok, val)) {
                cout << "必须输入整数。\n";
                continue;
            }
            list->deleteValue(val);
        } else if (op == 4) {
            list->display();
        } else if (op == 5) {
            list->sortList();
            cout << "排序完成：\n";
            list->display();
        } else if (op == 6) {
            LinList* lp = dynamic_cast<LinList*>(list);
            if (lp) {
                lp->reverseList();
                cout << "链表已逆序（就地翻转）。\n";
                lp->display();
            } else {
                cout << "当前是顺序表，无法逆序（该操作仅对单链表有效）。\n";
            }
        } else {
            cout << "无效选项。\n";
        }
    }

    delete list;
    cout << "程序退出。\n";
    return 0;
}