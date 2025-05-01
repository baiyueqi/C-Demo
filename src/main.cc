// 实现快速排序
// 1. 选择一个基准元素
// 2. 将数组分为两部分：小于基准元素的部分和大于基准元素的部分
// 3. 对这两部分递归进行快速排序
// 4. 合并两部分和基准元素
// 5. 返回排序后的数组
// 6. 时间复杂度：O(nlogn)
// 7. 空间复杂度：O(logn)
#include <iostream>
#include <vector>
#include <algorithm>            

using namespace std;
// 快速排序函数 
void quickSort(vector<int>& arr, int left, int right) {
    if (left >= right) return; // 递归终止条件
    int pivot = arr[left]; // 选择基准元素
    int i = left + 1; // 左指针
    int j = right; // 右指针
    while (i <= j) {
        while (i <= j && arr[i] <= pivot) i++; // 找到第一个大于基准元素的元素
        while (i <= j && arr[j] >= pivot) j--; // 找到第一个小于基准元素的元素
        if (i < j) swap(arr[i], arr[j]); // 交换两个元素
    }
    swap(arr[left], arr[j]); // 将基准元素放到正确的位置
    quickSort(arr, left, j - 1); // 对左半部分递归排序
    quickSort(arr, j + 1, right); // 对右半部分递归排序
}
// 主函数   
int main() {
    vector<int> arr = {3, 6, 8, 10, 1, 2, 1}; // 测试数组
    cout << "原始数组: ";
    for (int i : arr) cout << i << " "; // 输出原始数组
    cout << endl;
    quickSort(arr, 0, arr.size() - 1); // 调用快速排序函数
    cout << "排序后的数组: ";
    for (int i : arr) cout << i << " "; // 输出排序后的数组
    cout << endl;
    return 0; // 返回0表示程序正常结束
}
// 运行结果
// 原始数组: 3 6 8 10 1 2 1
// 排序后的数组: 1 1 2 3 6 8 10
// 复杂度分析
// 时间复杂度：O(nlogn) 