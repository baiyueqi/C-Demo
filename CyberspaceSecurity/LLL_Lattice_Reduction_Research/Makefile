CXX ?= clang++
CXXFLAGS ?= -std=c++17 -O2 -Wall -Wextra -pedantic
CPPFLAGS ?= -Isrc
DEPFLAGS := -MMD -MP

BUILD_DIR := build
SRC_DIR := src
TEST_DIR := tests

LIB_SOURCES := $(SRC_DIR)/lattice.cpp $(SRC_DIR)/gso.cpp $(SRC_DIR)/lll.cpp $(SRC_DIR)/babai.cpp $(SRC_DIR)/reduction_variants.cpp
LIB_OBJECTS := $(patsubst $(SRC_DIR)/%.cpp,$(BUILD_DIR)/%.o,$(LIB_SOURCES))
EXPERIMENT := $(BUILD_DIR)/lll_experiment
TESTS := $(BUILD_DIR)/test_lll_small $(BUILD_DIR)/test_babai_small $(BUILD_DIR)/test_reduction_variants_small

.PHONY: all build demo test clean

all: $(EXPERIMENT) demo

build: $(EXPERIMENT)

$(BUILD_DIR)/.dir:
	mkdir -p $(BUILD_DIR)
	touch $@

$(BUILD_DIR)/%.o: $(SRC_DIR)/%.cpp | $(BUILD_DIR)/.dir
	$(CXX) $(CPPFLAGS) $(CXXFLAGS) $(DEPFLAGS) -c $< -o $@

$(EXPERIMENT): $(LIB_OBJECTS) $(BUILD_DIR)/experiment.o
	$(CXX) $(CXXFLAGS) $^ -o $@

$(BUILD_DIR)/experiment.o: $(SRC_DIR)/experiment.cpp | $(BUILD_DIR)/.dir
	$(CXX) $(CPPFLAGS) $(CXXFLAGS) $(DEPFLAGS) -c $< -o $@

$(BUILD_DIR)/test_lll_small: $(TEST_DIR)/test_lll_small.cpp $(LIB_OBJECTS) | $(BUILD_DIR)/.dir
	$(CXX) $(CPPFLAGS) $(CXXFLAGS) $^ -o $@

$(BUILD_DIR)/test_babai_small: $(TEST_DIR)/test_babai_small.cpp $(LIB_OBJECTS) | $(BUILD_DIR)/.dir
	$(CXX) $(CPPFLAGS) $(CXXFLAGS) $^ -o $@

$(BUILD_DIR)/test_reduction_variants_small: $(TEST_DIR)/test_reduction_variants_small.cpp $(LIB_OBJECTS) | $(BUILD_DIR)/.dir
	$(CXX) $(CPPFLAGS) $(CXXFLAGS) $^ -o $@

test: $(TESTS)
	./$(BUILD_DIR)/test_lll_small
	./$(BUILD_DIR)/test_babai_small
	./$(BUILD_DIR)/test_reduction_variants_small

demo: $(EXPERIMENT)
	@mkdir -p results
	@printf '\n== 第一阶段：LLL 基线约化 ==\n'
	@./$(EXPERIMENT) --algorithm lll --dimension 8 --bits 6 --delta 0.99 --eta 0.51 --strategy suffix | awk -F= 'BEGIN{label["dimension"]="格维数"; label["ambientDimension"]="环境维数"; label["delta"]="Lovasz 参数 delta"; label["eta"]="尺寸约化参数 eta"; label["strategy"]="GSO 更新策略"; label["runtimeMs"]="运行时间(ms)"; label["swapCount"]="交换次数"; label["sizeReductionCount"]="尺寸约化次数"; label["gsoUpdateCount"]="GSO 更新量"; label["firstVectorNorm"]="首向量范数"; label["rootHermiteFactor"]="根 Hermite 因子"; label["orthogonalityDefect"]="正交缺陷"; label["validReduced"]="是否通过 LLL 校验"} NF==2 {v=$$2; if (v=="true") v="是"; else if (v=="false") v="否"; else if (v=="suffix") v="后缀更新"; else if (v=="full") v="全量重算"; printf "%-24s %s\n", label[$$1], v}'
	@printf '\n== 第二阶段：参数扫描与推荐 ==\n'
	@./$(EXPERIMENT) --algorithm scan --dimension 6 --bits 5 --repeat 1 --output results/make_lll_scan.csv > results/make_scan_summary.txt
	@awk '/^wrote / {print "已写入: " $$2} /^validRuns=/ {sub("validRuns=", "有效运行: "); print} /^qualityRecommendation:/ {sub("qualityRecommendation: ", "质量优先推荐: "); gsub("strategy=full", "策略=全量重算"); gsub("strategy=suffix", "策略=后缀更新"); gsub("rootHermiteFactor=", "RHF="); gsub("runtimeMs=", "耗时(ms)="); print} /^speedRecommendation:/ {sub("speedRecommendation: ", "速度优先推荐: "); gsub("strategy=full", "策略=全量重算"); gsub("strategy=suffix", "策略=后缀更新"); gsub("rootHermiteFactor=", "RHF="); gsub("runtimeMs=", "耗时(ms)="); print}' results/make_scan_summary.txt
	@printf '\n-- 参数扫描部分结果：delta / eta / 策略 / 耗时 / RHF / 校验 --\n'
	@awk -F, 'NR==1 {printf "%-8s %-8s %-10s %-12s %-18s %-8s\n", "delta", "eta", "策略", "耗时(ms)", "RHF", "校验"} NR>1 && NR<=11 {strategy=$$9; valid=$$18; if (strategy=="suffix") strategy="后缀更新"; else if (strategy=="full") strategy="全量重算"; if (valid=="true") valid="通过"; else if (valid=="false") valid="失败"; printf "%-8s %-8s %-10s %-12s %-18s %-8s\n", $$7, $$8, strategy, $$10, $$16, valid}' results/make_lll_scan.csv
	@printf '\n== 第三阶段：Babai 优化前后对比 ==\n'
	@./$(EXPERIMENT) --algorithm babai-compare --dimension 5 --bits 5 --repeat 1 --max-candidates 256 --output results/make_babai_comparison.csv >/dev/null
	@awk -F, 'function basisName(v){if(v=="raw") return "原始基"; if(v=="lll-reduced") return "LLL约化基"; return v} function methodName(v){if(v=="nearest-plane") return "最近平面"; if(v=="randomized") return "随机舍入"; if(v=="tail-enumeration") return "尾部枚举"; if(v=="local-search") return "局部搜索"; return v} NR==1 {printf "%-12s %-14s %-14s %-12s %-10s %-10s\n", "输入基", "方法", "距离", "近似比", "精确命中", "候选数"} NR>1 {hit=$$10; if(hit=="true") hit="是"; else if(hit=="false") hit="否"; printf "%-12s %-14s %-14s %-12s %-10s %-10s\n", basisName($$1), methodName($$2), $$8, $$9, hit, $$11}' results/make_babai_comparison.csv
	@printf '\n== 第四阶段：LLL 与高级约化原型对比 ==\n'
	@./$(EXPERIMENT) --algorithm advanced --dimension 5 --bits 5 --repeat 1 --max-candidates 256 --output results/make_advanced_comparison.csv >/dev/null
	@awk -F, 'function algName(v){if(v=="lll") return "LLL"; if(v=="mini-bkz") return "mini-BKZ"; if(v=="deep-lll-postpass") return "DeepLLL后处理"; if(v=="pot-lll-postpass") return "PotLLL后处理"; return v} NR==1 {printf "%-18s %-12s %-10s %-10s %-18s %-18s %-8s\n", "算法", "耗时(ms)", "候选数", "插入数", "RHF", "正交缺陷", "校验"} NR>1 {valid=$$17; if(valid=="true") valid="通过"; else if(valid=="false") valid="失败"; printf "%-18s %-12s %-10s %-10s %-18s %-18s %-8s\n", algName($$1), $$6, $$7, $$8, $$15, $$16, valid}' results/make_advanced_comparison.csv
	@printf '\nCSV 文件也已写入 results/，后续可用于画图和报告。\n'

clean:
	rm -rf $(BUILD_DIR)

-include $(BUILD_DIR)/*.d
