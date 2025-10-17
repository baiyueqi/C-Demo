#ifndef LINLIST_H
#define LINLIST_H

#include "Base.h"
#include <iostream>

using namespace std;

// 单链表类
class LinList : public Base {
private:
    struct Node {
        int val;
        Node* next;
        Node(int v=0): val(v), next(nullptr) {}
    };
    Node* head;

public:
    LinList() {
        head = new Node(); // 哨兵头结点
    }
    ~LinList() override {
        clear();
        delete head;
    }

    void clear() {
        Node* cur = head->next;
        while (cur) {
            Node* tmp = cur;
            cur = cur->next;
            delete tmp;
        }
        head->next = nullptr;
    }

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

    void insertAfterValue(int target, const vector<int>& vals, bool /*keep_sorted*/ = false) override {
        if (vals.empty()) return;
        Node* cur = head->next;
        Node* found = nullptr;
        while (cur) {
            if (cur->val == target) { found = cur; break; }
            cur = cur->next;
        }
        if (!found) {
            cur = head;
            while (cur->next) cur = cur->next;
            for (int v : vals) {
                Node* n = new Node(v);
                cur->next = n;
                cur = n;
            }
        } else {
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

    void deleteValueAll(int val) override {
        Node* prev = head;
        Node* cur = head->next;
        int count = 0;
        while (cur) {
            if (cur->val == val) {
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

    void reverseInPlace() {
        Node* prev = nullptr;
        Node* cur = head->next;
        while (cur) {
            Node* nxt = cur->next;
            cur->next = prev;
            prev = cur;
            cur = nxt;
        }
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

#endif
