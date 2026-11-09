#include <stdio.h>
#include <stdlib.h>
#include <string.h>

/* 1. 修复缓冲区溢出：使用 snprintf 并限制写入长度 */
void fixed_buffer_overflow(void)
{
    const char *long_text = "This string is much longer than eight bytes";
    char buf[8];
    snprintf(buf, sizeof(buf), "%s", long_text);
    printf("buffer_overflow_fixed: %s\n", buf);
}

/* 2. 修复空指针引用：解引用前判空 */
void fixed_null_deref(void)
{
    int value = 42;
    int *p = &value;
    if (p != NULL)
    {
        printf("null_deref_fixed: %d\n", *p);
    }
    else
    {
        printf("null_deref_fixed: pointer is null\n");
    }
}

/* 3. 修复内存泄漏：使用完毕及时释放 */
void fixed_memory_leak(void)
{
    char *p = (char *)malloc(32);
    if (p == NULL)
    {
        return;
    }
    snprintf(p, 32, "%s", "released memory");
    printf("memory_leak_fixed: %s\n", p);
    free(p);
}

/* 4. 修复数组越界：下标严格限制在 [0, 4] */
void fixed_array_oob(void)
{
    int arr[5] = {1, 2, 3, 4, 5};
    for (int i = 0; i < 5; ++i)
    {
        printf("array_oob_fixed[%d] = %d\n", i, arr[i]);
    }
}

/* 5. 修复未初始化变量：声明时显式初始化 */
void fixed_uninitialized(void)
{
    int value = 0;
    if (value > 0)
    {
        printf("uninitialized_fixed: positive\n");
    }
    else
    {
        printf("uninitialized_fixed: non-positive\n");
    }
}

/* 6. 修复释放后使用：在释放前完成访问，释放后置空 */
void fixed_use_after_free(void)
{
    char *p = (char *)malloc(10);
    if (p == NULL)
    {
        return;
    }
    snprintf(p, 10, "%s", "hello");
    printf("use_after_free_fixed: %s\n", p);
    free(p);
    p = NULL;
}

int main(void)
{
    fixed_buffer_overflow();
    fixed_null_deref();
    fixed_memory_leak();
    fixed_array_oob();
    fixed_uninitialized();
    fixed_use_after_free();
    return 0;
}
