// 自定义 ls 命令的数据结构和目录列表接口声明。
#ifndef MYLS_H
#define MYLS_H

#include <string>

struct MyLsOptions {
    bool show_all = false;    // 对应 -a：显示隐藏文件
    bool long_format = false; // 对应 -l：使用长格式
    bool recursive = false;   // 对应 -R：递归列出子目录
    bool show_inode = false;  // 对应 -i：显示 inode 号
};

/*
 * 列出指定目录内容。
 *
 * path:         要列出的目录或文件路径
 * options:      控制显示方式和递归行为的选项
 * print_header: 为 true 时，在输出内容前打印目录标题
 */
void list_dir(const std::string& path,
              const MyLsOptions& options,
              bool print_header);

#endif
