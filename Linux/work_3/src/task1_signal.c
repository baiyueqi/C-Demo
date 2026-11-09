/* 父进程写文件，子进程阻塞等待 SIGUSR1，收到信号后读取并输出文件内容。 */
// 第 1 题：父进程写文件，用 SIGUSR1 通知子进程读取并显示。
#define _POSIX_C_SOURCE 200809L

#include "../include/task1_signal.h"

#include <errno.h>
#include <fcntl.h>
#include <signal.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <unistd.h>

static volatile sig_atomic_t got_sigusr1 = 0;

static void handle_sigusr1(int signo) {
    // 信号处理函数只负责设置一个可安全访问的标志。
    (void)signo;
    got_sigusr1 = 1;
}

static const char *DATA_FILE = "data_from_parent.txt";

static const char *MESSAGE =
    "Hello from parent process!\n"
    "This is the second line written by the parent.\n"
    "Parent finishes writing, then notifies child via SIGUSR1.\n";

int run_task1_signal(void) {
    // 先阻塞 SIGUSR1，确保父子进程都准备好后再接收信号。
    sigset_t block_set;
    sigset_t old_set;
    sigset_t empty_set;

    sigemptyset(&block_set);
    sigaddset(&block_set, SIGUSR1);
    if (sigprocmask(SIG_BLOCK, &block_set, &old_set) < 0) {
        perror("sigprocmask");
        return 1;
    }

    pid_t pid = fork();
    if (pid < 0) {
        perror("fork");
        return 1;
    }

    if (pid == 0) {
        // 子进程：安装信号处理器，等待 SIGUSR1 后再读取父进程写好的文件。
        struct sigaction sa;

        memset(&sa, 0, sizeof(sa));
        sa.sa_handler = handle_sigusr1;
        sigemptyset(&sa.sa_mask);
        sa.sa_flags = 0;

        if (sigaction(SIGUSR1, &sa, NULL) < 0) {
            perror("sigaction");
            _exit(1);
        }

        sigemptyset(&empty_set);
        // 使用 sigsuspend 等待信号，避免忙等占用 CPU。
        while (!got_sigusr1) {
            sigsuspend(&empty_set);
        }

        // 收到信号后再打开父进程写好的文件并输出内容。
        int fd = open(DATA_FILE, O_RDONLY);
        if (fd < 0) {
            perror("open for read");
            _exit(1);
        }

        char buf[256];
        ssize_t n;
        while ((n = read(fd, buf, sizeof(buf))) > 0) {
            if (write(STDOUT_FILENO, buf, (size_t)n) < 0) {
                perror("write to stdout");
                close(fd);
                _exit(1);
            }
        }

        if (n < 0) {
            perror("read data file");
        }

        close(fd);
        _exit(n < 0 ? 1 : 0);
    }

    // 父进程：写入数据文件，然后发送 SIGUSR1 通知子进程。
    int fd = open(DATA_FILE, O_WRONLY | O_CREAT | O_TRUNC, 0644);
    if (fd < 0) {
        perror("open for write");
        return 1;
    }

    size_t msg_len = strlen(MESSAGE);
    if (write(fd, MESSAGE, msg_len) != (ssize_t)msg_len) {
        perror("write data file");
        close(fd);
        return 1;
    }

    if (close(fd) < 0) {
        perror("close data file");
        return 1;
    }

    // 文件写完后发送 SIGUSR1 通知子进程。
    if (kill(pid, SIGUSR1) < 0) {
        perror("kill");
        return 1;
    }

    int status = 0;
    // 等待子进程结束，并检查其退出状态。
    if (waitpid(pid, &status, 0) < 0) {
        perror("waitpid");
        return 1;
    }

    sigprocmask(SIG_SETMASK, &old_set, NULL);

    if (WIFEXITED(status) && WEXITSTATUS(status) == 0) {
        return 0;
    }

    return 1;
}
