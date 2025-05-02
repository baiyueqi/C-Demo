//写出vector的基础成员函数和使用示例
#include <iostream>
#include <vector>

using namespace std;

int main() {
    // 1. 构造函数
    vector<int> v1; // 默认构造函数，创建空向量
    vector<int> v2(5, 10); // 创建包含 5 个值为 10 的元素的向量
    vector<int> v3 = {1, 2, 3, 4, 5}; // 使用初始化列表构造向量

    // 2. 容量相关函数
    cout << "v3 size: " << v3.size() << endl; // 输出向量大小
    cout << "v3 capacity: " << v3.capacity() << endl; // 输出向量容量
    cout << "v3 is empty: " << (v3.empty() ? "yes" : "no") << endl; // 判断是否为空

    // 3. 元素访问函数
    cout << "v3[2]: " << v3[2] << endl; // 使用下标访问元素
    cout << "v3 at(2): " << v3.at(2) << endl; // 使用 at() 访问元素（带边界检查）
    cout << "v3 front: " << v3.front() << endl; // 获取第一个元素
    cout << "v3 back: " << v3.back() << endl; // 获取最后一个元素

    // 4. 修改操作函数
    v3.push_back(6); // 在末尾添加元素 6
    v3.pop_back(); // 移除末尾元素
    v3.insert(v3.begin() + 1, 99); // 在索引 1 处插入元素 99
    v3.erase(v3.begin() + 1); // 移除索引 1 的元素
    v3.clear(); // 清空向量

    // 5. 迭代器相关函数
    for (int i : v2) {
        cout << i << " "; // 使用范围 for 循环遍历向量
    }
    cout << endl;

    return 0;
}