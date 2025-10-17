#ifndef BASE_H
#define BASE_H

#include <vector>
using namespace std;

// 抽象基类 Base：定义通用接口
class Base {
public:
    virtual ~Base() {}
    virtual void createFromInput() = 0;                              // 从键盘输入若干元素创建
    virtual void insertAfterValue(int target, const vector<int>& vals, bool keep_sorted=false) = 0;
    virtual void deleteValueAll(int val) = 0;                         // 删除值为 val 的所有元素
    virtual void deleteValues(const vector<int>& vals) = 0;           // 批量删除
    virtual void display() const = 0;                                 // 显示元素
};

#endif
