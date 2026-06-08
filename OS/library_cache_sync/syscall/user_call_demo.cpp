#include <cerrno>
#include <chrono>
#include <cstdlib>
#include <cstring>
#include <iostream>
#include <sys/syscall.h>
#include <unistd.h>

#if defined(__NR_diag_read) && defined(__NR_diag_load) && defined(__NR_diag_write) && \
    defined(__NR_diag_refresh)

namespace {

struct DiagCall {
    const char* name;
    long number;
};

bool run_diag_call(const DiagCall& call, long long student_no)
{
    errno = 0;
    // 用墙钟时间证明系统调用确实等待了约 1 到 5 秒
    const auto start = std::chrono::steady_clock::now();
    const long result = syscall(call.number, student_no);
    const int saved_errno = errno;
    const auto end = std::chrono::steady_clock::now();
    const auto elapsed_ms =
        std::chrono::duration_cast<std::chrono::milliseconds>(end - start).count();

    if (result == -1) {
        std::cerr << call.name << "(" << student_no << ") 调用失败："
                  << std::strerror(saved_errno) << "，耗时毫秒=" << elapsed_ms << '\n';
        return false;
    }

    std::cout << call.name << "(" << student_no << ") 返回值=" << result
              << "，耗时毫秒=" << elapsed_ms << '\n';
    return true;
}

} // 匿名命名空间

#endif

int main(int argc, char** argv)
{
    if (argc != 2) {
        std::cerr << "用法: " << argv[0] << " <学号>\n";
        return 1;
    }

#if defined(__NR_diag_read) && defined(__NR_diag_load) && defined(__NR_diag_write) && \
    defined(__NR_diag_refresh)
    const long long student_no = std::atoll(argv[1]);
    // 在运行完整模拟器前，先单独验证四个系统调用号
    const DiagCall calls[] = {
        {"diag_read", __NR_diag_read},
        {"diag_load", __NR_diag_load},
        {"diag_write", __NR_diag_write},
        {"diag_refresh", __NR_diag_refresh},
    };

    bool ok = true;
    for (const DiagCall& call : calls) {
        ok = run_diag_call(call, student_no) && ok;
    }
    return ok ? 0 : 1;
#else
    std::cerr << "缺少 diag 系统调用号。\n"
              << "请在启动自定义内核后，使用 -D__NR_diag_read=548 -D__NR_diag_load=549 "
                 "-D__NR_diag_write=550 -D__NR_diag_refresh=551 重新编译。\n";
    return 1;
#endif
}
