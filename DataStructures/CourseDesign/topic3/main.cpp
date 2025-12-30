#include "TrafficConsultSystem.h"
#include <iostream>

using namespace std;

/**
 * @brief 程序主函数
 * @return 程序退出码（0-正常，1-异常）
 * 
 * @note 程序功能：
 * 1. 创建交通咨询系统实例
 * 2. 初始化系统（加载数据）
 * 3. 运行系统主循环
 * 4. 异常处理
 */
int main() {
    try {
        // 创建系统实例
        TrafficConsultSystem system;
        
        // 初始化系统（加载数据）
        system.initialize();
        
        // 运行系统主循环
        system.run();
        
        cout << "程序正常结束" << endl;
        
    } catch (const exception& e) {
        // 异常处理
        cerr << "程序异常: " << e.what() << endl;
        cerr << "程序异常终止，请检查数据文件是否完整。" << endl;
        return 1;
    } catch (...) {
        // 未知异常处理
        cerr << "发生未知异常" << endl;
        return 2;
    }
    
    return 0;
}