//用多态和虚函数写一个小游戏
// 1. 定义一个基类 Game，包含一个纯虚函数 play()，表示游戏的玩法。
// 2. 定义两个派生类 Chess 和 Poker，分别实现 play() 函数，表示棋类游戏和扑克牌游戏的玩法。
// 3. 在 main 函数中创建 Game 类型的指针，指向 Chess 和 Poker 对象，调用 play() 函数。
// 4. 使用虚析构函数，确保在删除派生类对象时，基类的析构函数也能被调用。
// 5. 使用智能指针管理对象的生命周期，避免内存泄漏。
// 6. 使用异常处理机制，捕获可能的异常并进行处理。
// 7. 使用 STL 容器（如 vector）存储游戏对象，演示容器的使用。
// 8. 使用命名空间来组织代码，避免命名冲突。
// 9. 使用函数重载实现不同参数的 play() 函数，演示函数重载的使用。
// 10. 使用运算符重载实现游戏对象的输出，演示运算符重载的使用。
// 11. 使用 lambda 表达式实现简单的排序功能，演示 lambda 表达式的使用。
// 12. 使用多线程实现游戏的并发执行，演示多线程的使用。
// 13. 使用条件变量实现线程间的同步，演示条件变量的使用。
// 14. 使用互斥锁实现线程间的互斥，演示互斥锁的使用。
// 15. 使用时间函数实现游戏的计时功能，演示时间函数的使用。
// 16. 使用随机数生成器实现游戏的随机性，演示随机数生成器的使用。
// 17. 使用文件操作实现游戏的存档功能，演示文件操作的使用。
#include <iostream>
#include <memory>
#include <vector>
#include <string>
#include <algorithm>
#include <thread>
#include <mutex>
#include <condition_variable>
#include <chrono>
#include <random>
#include <fstream>

namespace GameNamespace {

class Game {
public:
    virtual void play() const = 0; // 纯虚函数
    virtual ~Game() { std::cout << "Game destructor called\n"; } // 虚析构函数
    virtual void play(int level) const { 
        std::cout << "Playing game at level " << level << "\n"; 
    } // 函数重载
    friend std::ostream& operator<<(std::ostream& os, const Game& game) {
        os << "Game object";
        return os;
    } // 运算符重载
};

class Chess : public Game {
public:
    void play() const override {
        std::cout << "Playing Chess\n";
    }
    void play(int level) const override {
        std::cout << "Playing Chess at level " << level << "\n";
    }
    ~Chess() { std::cout << "Chess destructor called\n"; }
};

class Poker : public Game {
public:
    void play() const override {
        std::cout << "Playing Poker\n";
    }
    void play(int level) const override {
        std::cout << "Playing Poker at level " << level << "\n";
    }
    ~Poker() { std::cout << "Poker destructor called\n"; }
};

void sortGames(std::vector<std::shared_ptr<Game>>& games) {
    std::sort(games.begin(), games.end(), [](const std::shared_ptr<Game>& a, const std::shared_ptr<Game>& b) {
        return a.get() < b.get(); // 简单比较指针地址
    });
}

void saveGame(const std::string& filename, const std::string& data) {
    std::ofstream file(filename);
    if (file.is_open()) {
        file << data;
        file.close();
        std::cout << "Game saved to " << filename << "\n";
    } else {
        throw std::ios_base::failure("Failed to open file for saving");
    }
}

void gameThreadFunction(std::shared_ptr<Game> game, std::mutex& mtx, std::condition_variable& cv, bool& ready) {
    std::unique_lock<std::mutex> lock(mtx);
    cv.wait(lock, [&ready]() { return ready; });
    game->play();
}

} // namespace GameNamespace

int main() {
    using namespace GameNamespace;

    try {
        std::vector<std::shared_ptr<Game>> games;
        games.push_back(std::make_shared<Chess>());
        games.push_back(std::make_shared<Poker>());

        for (const auto& game : games) {
            game->play();
        }

        // 演示函数重载
        games[0]->play(2);

        // 演示运算符重载
        std::cout << *games[0] << "\n";

        // 演示 lambda 表达式
        sortGames(games);

        // 演示多线程和条件变量
        std::mutex mtx;
        std::condition_variable cv;
        bool ready = false;

        std::thread t1(gameThreadFunction, games[0], std::ref(mtx), std::ref(cv), std::ref(ready));
        std::thread t2(gameThreadFunction, games[1], std::ref(mtx), std::ref(cv), std::ref(ready));

        {
            std::lock_guard<std::mutex> lock(mtx);
            ready = true;
        }
        cv.notify_all();

        t1.join();
        t2.join();

        // 演示随机数生成器
        std::random_device rd;
        std::mt19937 gen(rd());
        std::uniform_int_distribution<> dis(1, 100);
        std::cout << "Random number: " << dis(gen) << "\n";

        // 演示时间函数
        auto start = std::chrono::high_resolution_clock::now();
        std::this_thread::sleep_for(std::chrono::seconds(1));
        auto end = std::chrono::high_resolution_clock::now();
        std::chrono::duration<double> elapsed = end - start;
        std::cout << "Elapsed time: " << elapsed.count() << " seconds\n";

        // 演示文件操作
        saveGame("game_save.txt", "Game progress data");

    } catch (const std::exception& e) {
        std::cerr << "Exception caught: " << e.what() << "\n";
    }

    return 0;
}