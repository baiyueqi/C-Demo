#include <iostream>
#include <algorithm>
using namespace std;

// ========================== 抽象基类 ==========================
class Base {
public:
    virtual void create() = 0;                        // 创建
    virtual void insert(int value, int afterVal) = 0; // 插入（多插）
    virtual void deleteElem(int value) = 0;           // 删除
    virtual void display() const = 0;                // 显示
    virtual ~Base() {}
};

// ========================== 顺序表类 ==========================
class SeqList : public Base {
private:
    int data[100];
    int length;
public:
    SeqList() { length = 0; }

    // 创建顺序表
    void create() override {
        cout << "请输入元素（<100），以 @ 结束：" << endl;
        string s;
        while (cin >> s) {
            if (s == "@") break;
            int num = stoi(s);
            if (length < 100) data[length++] = num;
        }
        sort(data, data + length); // 初始建表后保持非递减排序
    }

    // 插入：在指定值后插入多个元素，插完排序
    void insert(int, int afterVal) override {
        cout << "请输入要插入的多个值，以 @ 结束：" << endl;
        string s;
        int insertArr[100];
        int insertLen = 0;
        while (cin >> s) {
            if (s == "@") break;
            insertArr[insertLen++] = stoi(s);
        }

        // 找到最后一个 afterVal 的位置
        int pos = -1;
        for (int i = 0; i < length; i++) {
            if (data[i] == afterVal) pos = i;
        }
        if (pos == -1) pos = length - 1; // 没找到就插到尾

        // 批量插入
        for (int k = 0; k < insertLen; k++) {
            if (length >= 100) { cout << "表满！无法插入\n"; break; }
            for (int i = length; i > pos + 1 + k; i--) {
                data[i] = data[i - 1];
            }
            data[pos + 1 + k] = insertArr[k];
            length++;
        }

        sort(data, data + length);
        cout << "插入完成，当前顺序表：";
        display();
    }

    // 删除：按值删除所有相同元素
    void deleteElem(int value) override {
        int newLen = 0;
        for (int i = 0; i < length; i++) {
            if (data[i] != value) {
                data[newLen++] = data[i];
            }
        }
        if (newLen == length) {
            cout << "未找到元素 " << value << "！\n";
        } else {
            length = newLen;
            cout << "已删除所有值为 " << value << " 的元素\n";
        }
    }

    // 显示
    void display() const override {
        cout << "顺序表元素：";
        for (int i = 0; i < length; i++) {
            cout << data[i] << " ";
        }
        cout << endl;
    }

    ~SeqList() {
        // 顺序表使用静态数组，不需要特别析构
    }
};

// ========================== 单链表类 ==========================
struct Node {
    int data;
    Node* next;
    Node(int d = 0) : data(d), next(nullptr) {}
};

class LinList : public Base {
private:
    Node* head;
public:
    LinList() {
        head = new Node(); // 头结点
    }

    // 创建单链表
    void create() override {
        cout << "请输入元素（<100），以 @ 结束：" << endl;
        string s;
        Node* tail = head;
        while (cin >> s) {
            if (s == "@") break;
            int num = stoi(s);
            Node* p = new Node(num);
            tail->next = p;
            tail = p;
        }
        sortList();
    }

    // 插入：在指定值节点后插入多个元素，并保持排序
    void insert(int, int afterVal) override {
        cout << "请输入要插入的多个值，以 @ 结束：" << endl;
        string s;
        int insertArr[100];
        int insertLen = 0;
        while (cin >> s) {
            if (s == "@") break;
            insertArr[insertLen++] = stoi(s);
        }

        // 找到最后一个 afterVal 节点
        Node* p = head->next;
        Node* pos = nullptr;
        while (p) {
            if (p->data == afterVal) pos = p;
            p = p->next;
        }

        // 插入多个节点
        Node* insertAfter = pos;
        if (!insertAfter) { // 没找到，就插到尾
            insertAfter = head;
            while (insertAfter->next) insertAfter = insertAfter->next;
        }

        for (int k = 0; k < insertLen; k++) {
            Node* newNode = new Node(insertArr[k]);
            newNode->next = insertAfter->next;
            insertAfter->next = newNode;
            insertAfter = newNode;  // 每次插入在上一次插入的后面
        }

        sortList();
        cout << "插入完成，当前链表：";
        display();
    }

    // 删除：按值删除所有相同节点
    void deleteElem(int value) override {
        Node* p = head;
        bool found = false;
        while (p->next) {
            if (p->next->data == value) {
                Node* temp = p->next;
                p->next = temp->next;
                delete temp;
                found = true;
            } else {
                p = p->next;
            }
        }
        if (!found) cout << "未找到元素 " << value << "！\n";
        else cout << "已删除所有值为 " << value << " 的节点\n";
    }

    // 显示
    void display() const override {
        cout << "链表元素：";
        Node* p = head->next;
        while (p) {
            cout << p->data << " ";
            p = p->next;
        }
        cout << endl;
    }

    // 链表逆序输出（不借助辅助数组）
    void reverse() {
        Node* prev = nullptr;
        Node* curr = head->next;
        while (curr) {
            Node* next = curr->next;
            curr->next = prev;
            prev = curr;
            curr = next;
        }
        head->next = prev;
        cout << "链表已逆序，当前元素顺序：";
        display();
    }

    // 对链表排序（非递减）
    void sortList() {
        if (!head->next || !head->next->next) return;
        for (Node* i = head->next; i; i = i->next) {
            for (Node* j = i->next; j; j = j->next) {
                if (i->data > j->data) swap(i->data, j->data);
            }
        }
    }

    // 析构
    ~LinList() {
        Node* p = head;
        while (p) {
            Node* temp = p;
            p = p->next;
            delete temp;
        }
    }
};

// ========================== 主程序 ==========================
int main() {
    Base* p = nullptr;
    int ch;

    while (true) {
        cout << "\n=========== 主菜单 ===========" << endl;
        cout << "1. 创建顺序表" << endl;
        cout << "2. 创建单链表" << endl;
        cout << "3. 插入元素(多插)" << endl;
        cout << "4. 删除元素" << endl;
        cout << "5. 单链表逆序输出" << endl;
        cout << "6. 显示所有元素" << endl;
        cout << "0. 退出系统" << endl;
        cout << "==============================" << endl;
        cout << "请选择：";
        cin >> ch;

        switch (ch) {
        case 1:
            delete p;
            p = new SeqList();
            p->create();
            break;
        case 2:
            delete p;
            p = new LinList();
            p->create();
            break;
        case 3: {
            if (!p) { cout << "请先创建表！\n"; break; }
            int after;
            cout << "插入到哪个值之后："; cin >> after;
            p->insert(0, after);
            break;
        }
        case 4: {
            if (!p) { cout << "请先创建表！\n"; break; }
            int val;
            cout << "输入要删除的值："; cin >> val;
            p->deleteElem(val);
            break;
        }
        case 5: {
            LinList* lp = dynamic_cast<LinList*>(p);
            if (lp) lp->reverse();
            else cout << "顺序表不支持逆序输出\n";
            break;
        }
        case 6:
            if (p) p->display();
            else cout << "请先创建表！\n";
            break;
        case 0:
            delete p;
            return 0;
        default:
            cout << "无效选项，请重新输入！\n";
        }
    }
}