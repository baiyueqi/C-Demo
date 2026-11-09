/* 使用命名管道 FIFO 在父子进程间传递消息，并用 SIGUSR1 通知子进程读取。 */
// 第 2 题：使用命名管道 FIFO 在父子进程间传递消息。
#define _POSIX_C_SOURCE 200809L

#include "../include/task2_fifo.h"

#include <errno.h>
#include <fcntl.h>
#include <signal.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <unistd.h>

static volatile sig_atomic_t got_sigusr1 = 0;

static void handle_sigusr1(int signo) {
    // 子进程收到 SIGUSR1 后设置标志。
    (void)signo;
    got_sigusr1 = 1;
}

static const char *FIFO_PATH = "work3_fifo";

static const char *MESSAGE =
    "Message sent through a named pipe.\n"
    "The parent writes this data into the FIFO,\n"
    "then sends SIGUSR1 to the child.\n";

int run_task2_fifo(void) {
    // 先删除可能残留的 FIFO，再创建新的命名管道。
    sigset_t block_set;
    sigset_t old_set;
    sigset_t empty_set;

    (void)unlink(FIFO_PATH);
    if (mkfifo(FIFO_PATH, 0666) < 0 && errno != EEXIST) {
        perror("mkfifo");
        return 1;
    }

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
        // 子进程：打开 FIFO 读端，等待信号后读取并输出父进程写入的内容。
        struct sigaction sa;

        memset(&sa, 0, sizeof(sa));
        sa.sa_handler = handle_sigusr1;
        sigemptyset(&sa.sa_mask);
        sa.sa_flags = 0;

        if (sigaction(SIGUSR1, &sa, NULL) < 0) {
            perror("sigaction");
            _exit(1);
        }

        int fd = open(FIFO_PATH, O_RDONLY);
        if (fd < 0) {
            perror("child open fifo for read");
            _exit(1);
        }

        sigemptyset(&empty_set);
        // 使用 sigsuspend 等待父进程发送 SIGUSR1。
        while (!got_sigusr1) {
            sigsuspend(&empty_set);
        }

        // 收到信号后从 FIFO 读取数据并输出。
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
            perror("read fifo");
        }

        close(fd);
        _exit(n < 0 ? 1 : 0);
    }

    // 父进程：打开 FIFO 写端，写入消息，再通知子进程读取。
    int fd = open(FIFO_PATH, O_WRONLY);
    if (fd < 0) {
        perror("parent open fifo for write");
        return 1;
    }

    size_t msg_len = strlen(MESSAGE);
    if (write(fd, MESSAGE, msg_len) != (ssize_t)msg_len) {
        perror("write fifo");
        close(fd);
        return 1;
    }

    if (close(fd) < 0) {
        perror("close fifo");
        return 1;
    }

    // 写完后发送 SIGUSR1 通知子进程读取 FIFO。
    if (kill(pid, SIGUSR1) < 0) {
        perror("kill");
        return 1;
    }

    int status = 0;
    // 等待子进程结束并检查退出状态。
    if (waitpid(pid, &status, 0) < 0) {
        perror("waitpid");
        return 1;
    }

    (void)unlink(FIFO_PATH);
    sigprocmask(SIG_SETMASK, &old_set, NULL);

    if (WIFEXITED(status) && WEXITSTATUS(status) == 0) {
        return 0;
    }

    return 1;
}
