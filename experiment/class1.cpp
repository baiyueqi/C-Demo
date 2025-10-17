#include <iostream>
#include <vector>
using namespace std;

// ======================== 基类 ========================
class Base {
public:
    virtual ~Base() {}
    virtual void create() = 0;
    virtual void insertAfter(int target, int value) = 0;
    virtual void deleteValue(int value) = 0;
    virtual void display() const = 0;
    virtual void sortList() = 0;
};

// ======================== 顺序表 ========================
class SeqList : public Base {
private:
    vector<int> data;
public:
    void create() override {
        cout << "请输入整数(<100,@表示结束)：";
        int x;
        while (cin >> x) {
            if (x >= 100) break;
            data.push_back(x);
        }
        sortList();
    }

    void insertAfter(int target, int value) override {
        // 找到所有 target，依次在其后插入 value
        for (size_t i = 0; i < data.size(); ++i) {
            if (data[i] == target) {
                data.insert(data.begin() + i + 1, value);
                ++i; // 跳过新插入的位置
            }
        }
        sortList();
    }

    void deleteValue(int value) override {
        for (auto it = data.begin(); it != data.end(); ) {
            if (*it == value) it = data.erase(it);
            else ++it;
        }
    }

    void sortList() override {
        // 简单冒泡排序（也可用 sort(data.begin(), data.end())）
        for (size_t i = 0; i < data.size(); ++i) {
            for (size_t j = 0; j + 1 < data.size(); ++j) {
                if (data[j] > data[j+1]) {
                    swap(data[j], data[j+1]);
                }
            }
        }
    }

    void display() const override {
        cout << "顺序表元素：";
        for (int v : data) cout << v << " ";
        cout << endl;
    }
};

// ======================== 单链表 ========================
class LinList : public Base {
private:
    struct Node {
        int data;
        Node* next;
        Node(int d) : data(d), next(nullptr) {}
    };
    Node* head;

public:
    LinList() : head(nullptr) {}
    ~LinList() {
        Node* p = head;
        while (p) {
            Node* tmp = p;
            p = p->next;
            delete tmp;
        }
    }

    void create() override {
        cout << "请输入整数(<100,@表示结束)：";
        int x;
        while (cin >> x) {
            if (x >= 100) break;
            Node* node = new Node(x);
            node->next = head;
            head = node;
        }
        sortList();
    }

    void insertAfter(int target, int value) override {
        Node* p = head;
        while (p) {
            if (p->data == target) {
                Node* node = new Node(value);
                node->next = p->next;
                p->next = node;
                p = node->next;  // 跳过刚插入的节点，避免死循环
            } else {
                p = p->next;
            }
        }
        sortList();
    }

    void deleteValue(int value) override {
        while (head && head->data == value) {
            Node* tmp = head;
            head = head->next;
            delete tmp;
        }
        Node* p = head;
        while (p && p->next) {
            if (p->next->data == value) {
                Node* tmp = p->next;
                p->next = tmp->next;
                delete tmp;
            } else {
                p = p->next;
            }
        }
    }

    void sortList() override {
        if (!head || !head->next) return;
        for (Node* i = head; i; i = i->next) {
            for (Node* j = i->next; j; j = j->next) {
                if (i->data > j->data) {
                    swap(i->data, j->data);
                }
            }
        }
    }

    void reverseList() {
        Node* prev = nullptr;
        Node* curr = head;
        while (curr) {
            Node* next = curr->next;
            curr->next = prev;
            prev = curr;
            curr = next;
        }
        head = prev;
    }

    void display() const override {
        cout << "单链表元素：";
        Node* p = head;
        while (p) {
            cout << p->data << " ";
            p = p->next;
        }
        cout << endl;
    }
};

// ======================== 主程序 ========================
int main() {
    Base* p = nullptr;
    int ch;
    cout << "1. 顺序表  2. 单链表\n请选择：";
    cin >> ch;
    if (ch == 1) p = new SeqList();
    else p = new LinList();

    int op;
    do {
        cout << "\n1. 创建表\n2. 插入\n3. 删除\n4. 显示\n5. 链表逆序(仅链表)\n0. 退出\n选择：";
        cin >> op;
        if (op == 1) {
            p->create();
        } else if (op == 2) {
            int target, val, times;
            cout << "请输入要在其后插入的元素值：";
            cin >> target;
            cout << "请输入要插入的值：";
            cin >> val;
            cout << "插入次数：";
            cin >> times;
            for (int i = 0; i < times; ++i) p->insertAfter(target, val);
        } else if (op == 3) {
            int val;
            cout << "请输入要删除的值：";
            cin >> val;
            p->deleteValue(val);
        } else if (op == 4) {
            p->display();
        } else if (op == 5) {
            LinList* lp = dynamic_cast<LinList*>(p);
            if (lp) {
                lp->reverseList();
                cout << "逆序完成。\n";
            } else {
                cout << "当前是顺序表，无法逆序。\n";
            }
        }
    } while (op != 0);

    delete p;
    return 0;
}
