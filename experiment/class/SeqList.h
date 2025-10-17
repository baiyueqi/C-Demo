#ifndef SEQLIST_H
#define SEQLIST_H

#include "Base.h"
#include <iostream>
#include <algorithm>

using namespace std;

// 顺序表类
class SeqList : public Base {
private:
    vector<int> data;
public:
    SeqList() {}
    ~SeqList() override {}

    void createFromInput() override {
        data.clear();
        cout << "请输入若干整数（每个 <100），以 @ 结束：\n";
        while (true) {
            string token;
            if (!(cin >> token)) return;
            if (token == "@") break;
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

    void insertAfterValue(int target, const vector<int>& vals, bool keep_sorted=false) override {
        if (vals.empty()) return;
        auto it = find(data.begin(), data.end(), target);
        if (it == data.end()) {
            data.insert(data.end(), vals.begin(), vals.end());
        } else {
            ++it;
            data.insert(it, vals.begin(), vals.end());
        }
        if (keep_sorted) {
            sort(data.begin(), data.end());
        }
    }

    void deleteValueAll(int val) override {
        auto old_size = data.size();
        data.erase(remove(data.begin(), data.end(), val), data.end());
        auto removed = old_size - data.size();
        cout << "已从顺序表删除 " << removed << " 个值为 " << val << " 的元素。\n";
    }

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

#endif
