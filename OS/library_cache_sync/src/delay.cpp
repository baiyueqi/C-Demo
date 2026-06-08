//封装四类模拟耗时函数
#include "common.h"

#include <atomic>
#include <chrono>
#include <functional>
#include <random>
#include <thread>

#if defined(USE_DIAG_SYSCALL)
#include <sys/syscall.h>
#include <unistd.h>
#endif

namespace {

int g_min_ms = 1000;//最小延时，默认 1000 毫秒
int g_max_ms = 5000;//最大延时，默认 5000 毫秒
std::atomic<unsigned int> g_delay_counter{0};//调用计数器，多线程安全

//普通模式：用用户态 sleep_for 模拟延时
void user_delay(long long student_no, int op_offset)
{
    // 加入线程 id 和调用序号，避免并发用户态延时完全相同,如果只用学号作为随机种子，那么每次生成的随机数可能一样，读、写、加载、刷新看起来就不够随机
    const unsigned int call_id = g_delay_counter.fetch_add(1);//获取本次调用编号
    const auto thread_hash = static_cast<unsigned int>(
        std::hash<std::thread::id>{}(std::this_thread::get_id()));//获取当前线程 id 的哈希
    const unsigned int seed = static_cast<unsigned int>(student_no) +
                              static_cast<unsigned int>(op_offset * 1009) +
                              call_id * 7919U + thread_hash;//生成随机种子

    std::mt19937 gen(seed);
    std::uniform_int_distribution<int> dist(g_min_ms, g_max_ms);//用 mt19937 随机数引擎生成延时时间
    std::this_thread::sleep_for(std::chrono::milliseconds(dist(gen)));//睡眠对应毫秒数
}

//系统调用模式：编译时定义 USE_DIAG_SYSCALL 后，优先调用自定义 Linux 系统调用
#if defined(USE_DIAG_SYSCALL)//fallback,如果没有启动自定义内核，或者系统调用号没注册成功，程序仍然可以退回用户态 sleep，保证题目一功能可以运行
void syscall_delay_or_fallback(long long student_no, int op_offset)
{
#if defined(__NR_diag_read) && defined(__NR_diag_load) && defined(__NR_diag_write) && defined(__NR_diag_refresh)
    long result = -1;
    // op_offset 把课程中的四类延时函数映射到四个自定义系统调用
    switch (op_offset) {
    case 1:
        result = syscall(__NR_diag_read, student_no);
        break;
    case 2:
        result = syscall(__NR_diag_load, student_no);
        break;
    case 3:
        result = syscall(__NR_diag_write, student_no);
        break;
    case 4:
        result = syscall(__NR_diag_refresh, student_no);
        break;
    default:
        break;
    }
    if (result == 0) {
        return;
    }
#endif
    // 即使没有加载自定义内核，也保留题目一的用户态运行能力
    user_delay(student_no, op_offset);
}
#endif

//统一判断当前编译模式
void operation_delay(long long student_no, int op_offset)
{
#if defined(USE_DIAG_SYSCALL)
    syscall_delay_or_fallback(student_no, op_offset);
#else
    user_delay(student_no, op_offset);
#endif
}

} // 匿名命名空间

//设置用户态延时范围
void configure_delays(long long, int min_ms, int max_ms)
{
    g_min_ms = min_ms;
    g_max_ms = max_ms;
}

//对外接口
void delay_read(long long student_no)// 读者读取耗时
{
    operation_delay(student_no, 1);
}

void delay_load(long long student_no)// 缓存管理器加载耗时
{
    operation_delay(student_no, 2);
}

void delay_write(long long student_no)// 写者写数据库耗时
{
    operation_delay(student_no, 3);
}

void delay_refresh(long long student_no)// 写者刷新缓存耗时

{
    operation_delay(student_no, 4);
}
