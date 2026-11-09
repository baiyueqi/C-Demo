#include <stdio.h>
#include <stdlib.h>
#include <string.h>

/* 1. 缓冲区溢出：目标缓冲区过小，且使用不安全的 strcpy */
void vuln_buffer_overflow(void)
{
    char buf[8];
    strcpy(buf, "This string is much longer than eight bytes");
    printf("buffer_overflow: %s\n", buf);
}

/* 2. 空指针引用：未判空就解引用 */
void vuln_null_deref(void)
{
    int *p = NULL;
    *p = 42;
    printf("null_deref: %d\n", *p);
}

/* 3. 内存泄漏：动态分配后未释放 */
void vuln_memory_leak(void)
{
    char *p = (char *)malloc(32);
    strcpy(p, "leaked memory");
    printf("memory_leak: %s\n", p);
}

/* 4. 数组越界：访问下标等于数组长度的位置 */
void vuln_array_oob(void)
{
    int arr[5] = {1, 2, 3, 4, 5};
    printf("array_oob: %d\n", arr[5]);
}

/* 5. 未初始化变量：读取未赋值的局部变量 */
void vuln_uninitialized(void)
{
    int value;
    if (value > 0)
    {
        printf("uninitialized: positive\n");
    }
}

/* 6. 释放后使用：free 之后再次访问原指针 */
void vuln_use_after_free(void)
{
    char *p = (char *)malloc(10);
    strcpy(p, "hello");
    free(p);
    printf("use_after_free: %s\n", p);
}

int main(int argc, char **argv)
{
    if (argc > 1 && strcmp(argv[1], "run") == 0)
    {
        vuln_buffer_overflow();
        vuln_null_deref();
        vuln_memory_leak();
        vuln_array_oob();
        vuln_uninitialized();
        vuln_use_after_free();
    }

    printf("vulnerable demo: pass 'run' to execute all vulnerable paths.\n");
    return 0;
}
