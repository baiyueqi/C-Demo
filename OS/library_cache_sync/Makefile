CXX ?= clang++
CPPFLAGS ?= -I include
CXXFLAGS ?= -std=c++17 -pthread -Wall -Wextra -O2

BUILD_DIR := build
TARGET := $(BUILD_DIR)/library_cache_sync
SYSCALL_DEMO := $(BUILD_DIR)/user_call_demo

# WSL2 自定义内核实验中使用的默认系统调用号
STUDENT_NO ?= 20241001655
DIAG_NR_READ ?= 548
DIAG_NR_LOAD ?= 549
DIAG_NR_WRITE ?= 550
DIAG_NR_REFRESH ?= 551
DIAG_SYSCALL_DEFS := -DUSE_DIAG_SYSCALL \
  -D__NR_diag_read=$(DIAG_NR_READ) -D__NR_diag_load=$(DIAG_NR_LOAD) \
  -D__NR_diag_write=$(DIAG_NR_WRITE) -D__NR_diag_refresh=$(DIAG_NR_REFRESH)

SRC := $(wildcard src/*.cpp)
OBJ := $(patsubst src/%.cpp,$(BUILD_DIR)/%.o,$(SRC))

.PHONY: all clean run run-quick run-demo run-cache-one run-writers-only run-examples \
  build-diag run-diag-demo run-diag-smoke

all: $(TARGET) $(SYSCALL_DEMO)

$(BUILD_DIR):
	mkdir -p $@

$(TARGET): $(OBJ)
	$(CXX) $(CXXFLAGS) $^ -o $@

$(BUILD_DIR)/%.o: src/%.cpp include/*.h | $(BUILD_DIR)
	$(CXX) $(CPPFLAGS) $(CXXFLAGS) -c $< -o $@

$(SYSCALL_DEMO): syscall/user_call_demo.cpp | $(BUILD_DIR)
	$(CXX) $(CXXFLAGS) $< -o $@

run: $(TARGET)
	$(TARGET)

# 题目一快速演示：使用很短的用户态延时，便于快速检查同步逻辑
run-quick: $(TARGET)
	@printf '\n=== 快速验证运行：延时很短，适合先看功能 ===\n'
	@$(TARGET) \
	  --readers 3 \
	  --writers 1 \
	  --cache-size 2 \
	  --rounds 2 \
	  --student-no 20241001655 \
	  --delay-min-ms 1 \
	  --delay-max-ms 3 \
	  --seed 42

# 题目一验收演示：使用默认 1 到 5 秒延时
run-demo: $(TARGET)
	@printf '\n=== 验收演示运行：使用默认 1 到 5 秒随机延时 ===\n'
	@$(TARGET) \
	  --readers 8 \
	  --writers 3 \
	  --cache-size 5 \
	  --rounds 5 \
	  --student-no 20241001655

run-cache-one: $(TARGET)
	@printf '\n=== 边界用例：缓存容量为 1，观察频繁替换但不死锁 ===\n'
	@$(TARGET) \
	  --readers 3 \
	  --writers 0 \
	  --cache-size 1 \
	  --rounds 2 \
	  --delay-min-ms 1 \
	  --delay-max-ms 2 \
	  --seed 42

run-writers-only: $(TARGET)
	@printf '\n=== 边界用例：无读者、多写者，观察写者独占 ===\n'
	@$(TARGET) \
	  --readers 0 \
	  --writers 2 \
	  --cache-size 2 \
	  --rounds 2 \
	  --delay-min-ms 1 \
	  --delay-max-ms 2 \
	  --seed 9

run-examples: $(TARGET)
	@$(MAKE) --no-print-directory run-quick
	@$(MAKE) --no-print-directory run-demo
	@$(MAKE) --no-print-directory run-cache-one
	@$(MAKE) --no-print-directory run-writers-only

# 题目二构建：启用自定义 diag 系统调用
build-diag:
	@$(MAKE) --no-print-directory clean
	@$(MAKE) --no-print-directory CXXFLAGS="$(CXXFLAGS) $(DIAG_SYSCALL_DEFS)" all

# 单独验证系统调用：直接调用 diag_read/load/write/refresh
run-diag-demo: build-diag
	$(SYSCALL_DEMO) $(STUDENT_NO)

# 小规模集成运行：使用系统调用延时路径
run-diag-smoke: build-diag
	$(TARGET) \
	  --readers 1 \
	  --writers 1 \
	  --cache-size 1 \
	  --rounds 1 \
	  --student-no $(STUDENT_NO) \
	  --seed 42

clean:
	rm -rf $(BUILD_DIR)
