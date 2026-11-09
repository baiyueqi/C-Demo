// 声明审计报告输出函数和程序用法函数。
#ifndef FS_AUDITOR_REPORT_H
#define FS_AUDITOR_REPORT_H

#include <string>

struct AuditStats;

// 将扫描结果写入指定报告文件。
void write_report(const std::string& report_path,
                  const std::string& target,
                  const std::string& extension_filter,
                  const AuditStats& stats);

// 输出 fs_auditor 的命令行用法。
void print_usage(const char* prog);

#endif
