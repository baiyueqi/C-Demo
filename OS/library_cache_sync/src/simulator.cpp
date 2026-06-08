#include "simulator.h"

#include <iomanip>
#include <iostream>
#include <random>
#include <sstream>
#include <stdexcept>

namespace {

//初始化信号量
void initSemaphore(sem_t* sem, unsigned int value)
{
    if (sem_init(sem, 0, value) == -1) {//第二个参数 0 表示这个信号量用于同一进程内的多个线程
        throw std::runtime_error("sem_init 初始化失败");
    }
}

//销毁信号量，释放资源
void destroySemaphore(sem_t* sem)
{
    if (sem_destroy(sem) == -1) {
        std::perror("sem_destroy 销毁失败");
    }
}

//生成日志里的线程名字,如读者-01
std::string numberedActor(const std::string& prefix, int zero_based_id)
{
    std::ostringstream out;
    out << prefix << "-" << std::setw(2) << std::setfill('0') << (zero_based_id + 1);
    return out.str();
}

//把书名和版本号拼成字符串,如书名=OS 版本=2
std::string bookVersion(const Book& book)
{
    return "书名=" + book.name + " 版本=" + std::to_string(book.version);
}

} // 匿名命名空间

//构造函数：初始化整个模拟器
Simulator::Simulator(const Config& config)
    : config_(config),//保存配置
      cache_(config.cache_size),//创建指定大小的缓存
      book_names_(database_.allBookNames()),//从数据库获取所有书名，后面随机选择
      reader_ready_(static_cast<std::size_t>(config.readers))//为每个读者创建一个独立的 reader_ready 信号量
{
    // 信号量初值定义了本次运行的同步规则(初始化)
    initSemaphore(&db_sem_, 1);
    initSemaphore(&read_mutex_, 1);
    initSemaphore(&cache_mutex_, 1);
    initSemaphore(&empty_slots_, static_cast<unsigned int>(config.cache_size));//一开始缓存全空
    initSemaphore(&full_slots_, 0);//一开始缓存里没有有效书
    initSemaphore(&cache_unpinned_, 0);//一开始没有释放 pin 的通知
    initSemaphore(&request_mutex_, 1);
    initSemaphore(&request_count_, 0);//一开始没有加载请求
    initSemaphore(&log_mutex_, 1);
    for (sem_t& sem : reader_ready_) {
        initSemaphore(&sem, 0);//每个读者一开始都没有被唤醒
    }

    configure_delays(config_.student_no, config_.delay_min_ms, config_.delay_max_ms);
}

//销毁信号量
Simulator::~Simulator()
{
    for (sem_t& sem : reader_ready_) {
        destroySemaphore(&sem);
    }
    destroySemaphore(&log_mutex_);
    destroySemaphore(&request_count_);
    destroySemaphore(&request_mutex_);
    destroySemaphore(&cache_unpinned_);
    destroySemaphore(&full_slots_);
    destroySemaphore(&empty_slots_);
    destroySemaphore(&cache_mutex_);
    destroySemaphore(&read_mutex_);
    destroySemaphore(&db_sem_);
}

//主运行流程
int Simulator::run()
{
    started_at_ = std::chrono::steady_clock::now();
    logLine("主线程", "启动 读者数=" + std::to_string(config_.readers) +
                         " 写者数=" + std::to_string(config_.writers) +
                         " 缓存容量=" + std::to_string(config_.cache_size) +
                         " 每线程轮次=" + std::to_string(config_.rounds));

    // 先启动缓存管理器，保证读者缓存未命中后能立即被处理
    pthread_t manager_thread{};
    if (pthread_create(&manager_thread, nullptr, &Simulator::cacheManagerEntry, this) != 0) {
        throw std::runtime_error("创建缓存管理器线程失败");
    }

    std::vector<pthread_t> reader_threads(static_cast<std::size_t>(config_.readers));
    std::vector<pthread_t> writer_threads(static_cast<std::size_t>(config_.writers));
    std::vector<ThreadArg> reader_args(static_cast<std::size_t>(config_.readers));
    std::vector<ThreadArg> writer_args(static_cast<std::size_t>(config_.writers));

    for (int i = 0; i < config_.readers; ++i) {
        reader_args[static_cast<std::size_t>(i)] = ThreadArg{this, i};
        if (pthread_create(&reader_threads[static_cast<std::size_t>(i)], nullptr,
                           &Simulator::readerEntry,
                           &reader_args[static_cast<std::size_t>(i)]) != 0) {
            throw std::runtime_error("创建读者线程失败");
        }
    }

    for (int i = 0; i < config_.writers; ++i) {
        writer_args[static_cast<std::size_t>(i)] = ThreadArg{this, i};
        if (pthread_create(&writer_threads[static_cast<std::size_t>(i)], nullptr,
                           &Simulator::writerEntry,
                           &writer_args[static_cast<std::size_t>(i)]) != 0) {
            throw std::runtime_error("创建写者线程失败");
        }
    }

    for (pthread_t thread : reader_threads) {
        pthread_join(thread, nullptr);//主线程等待子线程结束
    }
    for (pthread_t thread : writer_threads) {
        pthread_join(thread, nullptr);
    }

    // 读者和写者都结束后，发送一个哨兵请求唤醒并结束缓存管理器
    sendShutdown();
    pthread_join(manager_thread, nullptr);

    logLine("主线程", "运行结束");
    return 0;
}

//pthread 需要的静态入口函数readerEntry,writerEntry,cacheManagerEntry
void* Simulator::readerEntry(void* arg)
{
    ThreadArg* thread_arg = static_cast<ThreadArg*>(arg);
    thread_arg->simulator->readerThread(thread_arg->id);//用静态函数做桥接
    return nullptr;
}

void* Simulator::writerEntry(void* arg)
{
    ThreadArg* thread_arg = static_cast<ThreadArg*>(arg);
    thread_arg->simulator->writerThread(thread_arg->id);
    return nullptr;
}

void* Simulator::cacheManagerEntry(void* arg)
{
    Simulator* simulator = static_cast<Simulator*>(arg);
    simulator->cacheManagerThread();
    return nullptr;
}

//读者线程
void Simulator::readerThread(int reader_id)
{
    const std::string actor = numberedActor("读者", reader_id);
    std::mt19937 gen(config_.random_seed + static_cast<unsigned int>(reader_id * 101 + 17));

    for (int round = 1; round <= config_.rounds; ++round) {
        const std::string book_name = chooseBook(gen);
        logLine(actor, "第" + std::to_string(round) + "轮 请求书名=" + book_name);

        enterReadSection(reader_id);//读者进入读区

        // 内部用 read_count_ + db_sem_ 实现读者优先
        // 读者只在持锁时复制缓存/数据库数据；耗时读操作发生在复制后，
        // 这样可以减少锁占用并保留并发性
        Book copied;
        bool copied_from_cache = false;
        while (!copied_from_cache) {
            P(&cache_mutex_);//读者短暂拿 cache_mutex_，只做缓存查找和复制
            const bool hit = cache_.findAndCopy(book_name, copied);
            const std::string cache_state = cache_.snapshot();
            V(&cache_mutex_);

            if (hit) {
                logLine(actor, "缓存命中 " + bookVersion(copied) + " 缓存=" + cache_state);
                copied_from_cache = true;
                break;
            }
            
            //缓存 miss
            const int request_id = next_request_id_.fetch_add(1);
            logLine(actor, "缓存未命中 书名=" + book_name +
                               " 入队 请求编号=" + std::to_string(request_id));

            // 提交缓存未命中请求时不持有 cache_mutex_，避免阻塞缓存管理器加载
            P(&request_mutex_);
            request_queue_.push(LoadRequest::normal(reader_id, book_name, request_id));//生成唯一请求编号
            V(&request_mutex_);//把加载请求放入队列，并通知缓存管理器
            V(&request_count_);

            // 每个读者都有独立信号量，缓存管理器可以准确唤醒对应读者
            P(&reader_ready_[static_cast<std::size_t>(reader_id)]);

            P(&cache_mutex_);
            const bool found = cache_.findPinnedAndCopy(book_name, request_id, copied);//被唤醒后的二次读取
            if (found) {
                // 复制完成后释放保护标记，该缓存槽后续才允许被淘汰
                cache_.releasePin(book_name, request_id);
                V(&cache_unpinned_);//释放 pin，并通知可能等待的缓存管理器
            }
            const std::string loaded_cache_state = cache_.snapshot();
            V(&cache_mutex_);

            if (found) {
                logLine(actor, "加载后读取 " + bookVersion(copied) +
                                   " 缓存=" + loaded_cache_state);
                copied_from_cache = true;
            } else {
                logLine(actor, "加载完成但未找到受保护缓存副本 书名=" + book_name +
                                   "，重新查询缓存");
            }
        }

        logLine(actor, "开始阅读 " + bookVersion(copied));
        delay_read(config_.student_no);//阅读延时
        logLine(actor, "阅读完成 " + bookVersion(copied));

        leaveReadSection(reader_id);
    }

    logLine(actor, "退出");
}

//写者线程
void Simulator::writerThread(int writer_id)
{
    const std::string actor = numberedActor("写者", writer_id);
    std::mt19937 gen(config_.random_seed + static_cast<unsigned int>(writer_id * 211 + 53));

    for (int round = 1; round <= config_.rounds; ++round) {
        const std::string book_name = chooseBook(gen);
        logLine(actor, "第" + std::to_string(round) + "轮 等待写入 书名=" + book_name);

        // db_sem_ 是数据库互斥的核心。读者组作为整体持有它，写者单独持有它
        // db_sem_ 让写者独占数据库，并阻止新的读者进入读区
        P(&db_sem_);//只要读者组占用了 db_sem_，写者就会阻塞。只要有一个写者占用了 db_sem_，其他写者和读者也不能进入。

        logLine(actor, "开始更新数据库 书名=" + book_name);
        //更新数据库
        Book updated = database_.writeBook(book_name, writer_id + 1);
        delay_write(config_.student_no);
        logLine(actor, "数据库更新完成 " + bookVersion(updated));

        P(&cache_mutex_);
        // 只刷新缓存中已有的副本；写操作不会强制把所有书加入缓存
        const bool refreshed = cache_.updateIfPresent(updated);
        const std::string cache_state = cache_.snapshot();
        V(&cache_mutex_);

        if (refreshed) {
            logLine(actor, "开始刷新缓存 " + bookVersion(updated) +
                               " 缓存=" + cache_state);
            delay_refresh(config_.student_no);
            logLine(actor, "缓存刷新完成 " + bookVersion(updated));
        } else {
            logLine(actor, "缓存中没有副本 书名=" + book_name + "，跳过刷新");
        }
        
        //刷新时仍持有 db_sem_,在数据库更新和缓存刷新之间，不允许读者进入
        V(&db_sem_);
        logLine(actor, "离开写区 书名=" + book_name);
    }

    logLine(actor, "退出");
}

//缓存管理器线程
void Simulator::cacheManagerThread()
{
    logLine("缓存管理器", "启动");

    while (true) {
        // request_count_ 是生产者-消费者模型中的待处理请求数量
        P(&request_count_);//如果队列没有请求，缓存管理器会阻塞在这里

        LoadRequest req;
        //取出请求
        P(&request_mutex_);
        const bool has_request = request_queue_.pop(req);
        V(&request_mutex_);

        if (!has_request) {
            continue;
        }

        //处理 Shutdown,主线程最后发送一个关闭请求，让缓存管理器正常退出
        if (req.type == LoadRequest::Type::Shutdown) {
            logLine("缓存管理器", "收到关闭请求");
            break;
        }

        logLine("缓存管理器", "开始加载 书名=" + req.book_name +
                                 " 读者=" + std::to_string(req.reader_id + 1) +
                                 " 请求编号=" + std::to_string(req.request_id));

        // 数据库到缓存的耗时加载在 cache_mutex_ 外执行，避免长时间占锁
        Book loaded = database_.readBook(req.book_name);//加载书籍
        delay_load(config_.student_no);//这个加载延时在 cache_mutex_ 外面,数据库到缓存的耗时加载不占用缓存锁，避免阻塞其他读者查缓存

        while (true) {
            P(&cache_mutex_);

            // 同一本书已在缓存中：更新缓存并为当前读者请求加保护标记
            if (cache_.updateExistingAndPin(loaded, req.request_id)) {
                const std::string cache_state = cache_.snapshot();
                V(&cache_mutex_);
                logLine("缓存管理器", "更新已有缓存 " + bookVersion(loaded) +
                                         " 缓存=" + cache_state);
                break;
            }

            //有空槽
            if (cache_.hasFreeSlot()) {
                // 等待 empty_slots_ 时不能持有 cache_mutex_，避免死锁
                V(&cache_mutex_);
                P(&empty_slots_);
                P(&cache_mutex_);
                cache_.insertIntoFreeSlotAndPin(loaded, req.request_id);
                V(&full_slots_);
                const std::string cache_state = cache_.snapshot();
                V(&cache_mutex_);
                logLine("缓存管理器", "载入空闲缓存槽 " + bookVersion(loaded) +
                                         " 缓存=" + cache_state);
                break;
            }

            // 缓存已满：如果存在未被保护的 LRU 项，则替换它
            if (cache_.replaceLruAndPin(loaded, req.request_id)) {
                const std::string cache_state = cache_.snapshot();
                V(&cache_mutex_);
                logLine("缓存管理器", "替换LRU缓存槽 " + bookVersion(loaded) +
                                         " 缓存=" + cache_state);
                break;
            }

            V(&cache_mutex_);
            logLine("缓存管理器", "缓存已满且全部受保护，等待释放保护 书名=" +
                                     req.book_name);

            // 所有缓存项都被读者保护，等待某个读者释放保护标记
            P(&cache_unpinned_);
        }

        V(&reader_ready_[static_cast<std::size_t>(req.reader_id)]);//唤醒读者
    }

    logLine("缓存管理器", "退出");
}

//进入读区,读者优先算法
void Simulator::enterReadSection(int reader_id)
{
    bool first_reader = false;
    int active_readers = 0;

    // 读者优先入口：第一个读者获取 db_sem_，从而阻塞写者
    P(&read_mutex_);
    if (read_count_ == 0) {
        first_reader = true;
        P(&db_sem_);
    }
    ++read_count_;
    active_readers = read_count_;
    V(&read_mutex_);

    logLine(numberedActor("读者", reader_id),
            std::string("进入读区 当前读者数=") +
                std::to_string(active_readers) +
                (first_reader ? " 首个读者已锁定数据库" : ""));
}

//离开读区
void Simulator::leaveReadSection(int reader_id)
{
    bool last_reader = false;
    int active_readers = 0;

    // 最后一个读者释放 db_sem_，允许一个等待中的写者进入
    P(&read_mutex_);
    --read_count_;
    active_readers = read_count_;
    if (read_count_ == 0) {
        last_reader = true;
        V(&db_sem_);
    }
    V(&read_mutex_);

    logLine(numberedActor("读者", reader_id),
            std::string("离开读区 当前读者数=") +
                std::to_string(active_readers) +
                (last_reader ? " 最后读者已释放数据库" : ""));
}

//发送关闭请求,主线程通知缓存管理器退出
void Simulator::sendShutdown()
{
    // 复用请求队列，让缓存管理器沿正常等待路径退出
    P(&request_mutex_);
    request_queue_.push(LoadRequest::shutdown());
    V(&request_mutex_);
    V(&request_count_);
}

//随机选书(让读者或写者随机选择一本书操作)
std::string Simulator::chooseBook(std::mt19937& gen) const
{
    std::uniform_int_distribution<std::size_t> dist(0, book_names_.size() - 1);
    return book_names_[dist(gen)];
}

//日志函数,串行化日志输出
void Simulator::logLine(const std::string& actor, const std::string& message)
{
    P(&log_mutex_);
    std::cout << "[时间=" << std::setw(6) << std::setfill('0') << elapsedMs() << "毫秒] "
              << "[" << actor << "] " << message << '\n';
    std::cout.flush();
    V(&log_mutex_);
}

//时间戳,计算程序启动到当前的毫秒数，显示在线程日志里
long Simulator::elapsedMs() const
{
    const auto now = std::chrono::steady_clock::now();
    return std::chrono::duration_cast<std::chrono::milliseconds>(now - started_at_).count();
}
