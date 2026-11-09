# 编译器，默认使用 clang++。
CXX ?= clang++

# 所有目标共同使用的编译选项。
CXXFLAGS ?= -std=c++17

# fs_auditor：文件系统审计程序。
FS_AUDITOR_SRC = main/main.cpp src/fs_auditor.cpp src/thread_pool.cpp src/report.cpp
FS_AUDITOR_HDR = include/fs_auditor.h include/thread_pool.h include/report.h

# myls：自定义 ls 程序。
MYLS_SRC = main/myls_main.cpp src/myls.cpp
MYLS_HDR = include/myls.h

# reverse：文本逆序程序。
REVERSE_SRC = main/reverse_main.cpp src/reverse.cpp
REVERSE_HDR = include/reverse.h

.PHONY: all clean fs_auditor myls reverse

# 默认构建所有可执行文件。
all: fs_auditor myls reverse

fs_auditor: $(FS_AUDITOR_SRC) $(FS_AUDITOR_HDR)
	$(CXX) $(CXXFLAGS) -pthread $(FS_AUDITOR_SRC) -o $@

myls: $(MYLS_SRC) $(MYLS_HDR)
	$(CXX) $(CXXFLAGS) $(MYLS_SRC) -o $@

reverse: $(REVERSE_SRC) $(REVERSE_HDR)
	$(CXX) $(CXXFLAGS) $(REVERSE_SRC) -o $@

# 删除 make 生成的二进制文件。
clean:
	rm -f fs_auditor myls reverse
