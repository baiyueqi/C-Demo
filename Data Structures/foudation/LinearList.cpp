#include <iostream>
#include <cstdlib>
using namespace std;

/* ========= 常量定义 ========= */
#define MAXSIZE 100
#define OK 1
#define ERROR 0

/* ========= 类型定义 ========= */
typedef int ElemType;

/* ========= 顺序表结构 ========= */
typedef struct {
    ElemType* elem;   // 存储空间基地址
    int length;       // 当前长度
} SqList;

/* ========= 基本操作 ========= */

// 初始化线性表
bool InitList(SqList &L)
{
    L.elem = new ElemType[MAXSIZE];
    if (!L.elem) return ERROR;
    L.length = 0;
    return OK;
}

// 销毁线性表
void DestroyList(SqList &L)
{
    if (L.elem) {
        delete[] L.elem;
        L.elem = NULL;
    }
    L.length = 0;
}

// 清空线性表
void ClearList(SqList &L)
{
    L.length = 0;
}

// 判断是否为空
bool ListEmpty(SqList L)
{
    return L.length == 0;
}

// 获取长度
int ListLength(SqList L)
{
    return L.length;
}

// 按位取值
bool GetElem(SqList L, int i, ElemType &e)
{
    if (i < 1 || i > L.length) return ERROR;
    e = L.elem[i - 1];
    return OK;
}

// 按值查找
int LocateElem(SqList L, ElemType e)
{
    for (int i = 0; i < L.length; i++)
        if (L.elem[i] == e)
            return i + 1;
    return 0;
}

// 按位插入
bool ListInsert(SqList &L, int i, ElemType e)
{
    if (i < 1 || i > L.length + 1) return ERROR;
    if (L.length >= MAXSIZE) return ERROR;

    for (int j = L.length; j >= i; j--)
        L.elem[j] = L.elem[j - 1];

    L.elem[i - 1] = e;
    L.length++;
    return OK;
}

// 按位删除
bool ListDelete(SqList &L, int i, ElemType &e)
{
    if (i < 1 || i > L.length) return ERROR;

    e = L.elem[i - 1];
    for (int j = i; j < L.length; j++)
        L.elem[j - 1] = L.elem[j];

    L.length--;
    return OK;
}

// 输出线性表
void PrintList(SqList L)
{
    if (ListEmpty(L)) {
        cout << "线性表为空" << endl;
        return;
    }
    for (int i = 0; i < L.length; i++)
        cout << L.elem[i] << " ";
    cout << endl;
}

/* ========= 主函数测试 ========= */
int main()
{
    SqList L;
    InitList(L);

    // 插入元素
    ListInsert(L, 1, 10);
    ListInsert(L, 2, 20);
    ListInsert(L, 3, 30);
    ListInsert(L, 2, 15);

    cout << "当前线性表：";
    PrintList(L);

    // 删除元素
    ElemType e;
    ListDelete(L, 3, e);
    cout << "删除的元素是：" << e << endl;

    cout << "删除后线性表：";
    PrintList(L);

    // 查找
    int pos = LocateElem(L, 20);
    cout << "元素20的位置：" << pos << endl;

    // 获取长度
    cout << "线性表长度：" << ListLength(L) << endl;

    DestroyList(L);
    return 0;
}
