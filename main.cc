#include "crypto_info.hpp"
#include "period_receiver.hpp"
#include "symbols_process.hpp"
#include <thread>

int main() {
    std::string url = "wss://vps.mgsd.cc:8090/ws";
    // for (int i = 0; i < SymbolConfig::instance().symbol_partition.size(); i++) {
    //     std::jthread t([i] {

    //     });
    // }
    // auto &main_sym = SymbolConfig::instance().symbol_partition[0];
    // PeriodReceiver<CryptoTradeInfo> trade_client(SymbolConfig::instance().symbol_partition[0], url, 3 * 1000);
    // trade_client.run();
    auto &sym = SymbolConfig::instance().symbol_partition[0];
    PeriodReceiver<CryptoIncrementOrderBookInfo> orderbook_client(sym, url, 3 * 1000);
    orderbook_client.run();
}

// 
// #include <folly/io/async/AsyncTimeout.h>
// #include <folly/io/async/EventBase.h>
// #include <iostream>

// class PeriodicTask : public folly::AsyncTimeout {
// public:
//     PeriodicTask(folly::EventBase *eventBase, uint32_t intervalMillis)
//         : folly::AsyncTimeout(eventBase), interval_(intervalMillis) {}

//     void timeoutExpired() noexcept override {
//         std::cout << "Periodic task triggered." << std::endl;
//         reinterpret_cast<folly::AsyncTimeout *>(this)->scheduleTimeout(interval_);// 注意这里使用 this 指针
//     }

//     void start() {
//         reinterpret_cast<folly::AsyncTimeout *>(this)->scheduleTimeout(interval_);// 注意这里使用 this 指针
//     }

// private:
//     uint32_t interval_;// 定时器触发间隔，毫秒
// };

// int main() {
//     folly::EventBase eventBase;
//     PeriodicTask task(&eventBase, 1000);// 设置定时器，每1000毫秒触发一次

//     task.start();           // 启动周期性任务
//     eventBase.loopForever();// 启动事件循环

//     return 0;
// }
