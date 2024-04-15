#ifndef PERIODRECEIVER_HPP
#define PERIODRECEIVER_HPP
#include <cstdint>
#include <nlohmann/json.hpp>
#include <simdjson.h>
#include <string>
#include <thread>
#include <vector>
#include <websocketpp/client.hpp>
#include <websocketpp/config/asio_client.hpp>
#include <fmt/format.h>
#include "oe_cache.hpp"
#include <absl/log/absl_log.h>
#include <absl/log/log.h>
#include "absl/time/time.h"
#include "info_client.hpp"


template<typename T>
class PeriodReceiver{
public:
    InfoClient<T> info_client;
    explicit PeriodReceiver(const std::vector<std::string> &symbols, std::string url, int period);
    CLIENT signal_client;
    simdjson::dom::parser parser;
private:
    int period_count;
    int period;
    void on_open(websocketpp::connection_hdl hdl);
    void on_message(websocketpp::connection_hdl hdl, websocketpp::config::asio_client::message_type::ptr msg);
public:
    void run() {
        websocketpp::lib::error_code ec;
        CLIENT::connection_ptr con = signal_client.get_connection(info_client.url, ec);
        if (ec) {
            std::cout << "could not create connection because: " << ec.message() << std::endl;
            return;
        }
        con->set_open_handshake_timeout(10000);// 10秒
        signal_client.connect(con);
        //启动消息线程
        std::jthread info_receiver([this] {
                info_client.run();
            }
        );
        signal_client.run();
    }
};

template<typename T>
PeriodReceiver<T>::PeriodReceiver(const std::vector<std::string> &symbols, std::string url, int period)
    : info_client(symbols, url), period(period){
    using websocketpp::lib::bind;
    using websocketpp::lib::placeholders::_1;
    using websocketpp::lib::placeholders::_2;
    signal_client.set_tls_init_handler(bind(&on_tls_init, url.c_str(), _1));
    // 设置打开连接的处理程序
    signal_client.set_open_handler(bind(&PeriodReceiver::on_open, this, _1));
    // 设置消息处理程序
    signal_client.set_message_handler(bind(&PeriodReceiver::on_message, this, _1, _2));
    signal_client.init_asio();
    period_count = 0;
}

template<typename T>
void PeriodReceiver<T>::PeriodReceiver::on_open(websocketpp::connection_hdl hdl) {
    std::vector<std::string> params;
    auto &signal_symbols = SymbolConfig::instance().symbol_partition[0];
    for (const auto &symbol: signal_symbols) {
        params.push_back(fmt::format("{}@{}", symbol, get_message_type<T>()));
    }
    nlohmann::json subscribe_message = {
            {"method", "SUBSCRIBE"},
            {"params", params},
            {"id", SymbolConfig::instance().id}};
    signal_client.send(hdl, subscribe_message.dump(), websocketpp::frame::opcode::text);
}

template<typename T>
void PeriodReceiver<T>::PeriodReceiver::on_message(websocketpp::connection_hdl hdl, websocketpp::config::asio_client::message_type::ptr msg) {
    if (msg->get_opcode() == websocketpp::frame::opcode::ping) {
        std::string payload = msg->get_payload();
        try {
            signal_client.pong(hdl, payload);
        } catch (const websocketpp::lib::error_code &e) {
            std::cerr << "Pong reply failed: " << e.message() << std::endl;
        }
    } else {
        std::string_view json_sv = msg->get_payload();
        simdjson::dom::element doc;
        auto error = parser.parse(json_sv).get(doc);
        if (error) {
            std::cerr << "Parsing failed: " << error << std::endl;
            return;
        }
        int64_t time = 0;
        try {
            time = doc["E"].get_int64().value();
            static int64_t prev_tp = (time / period) * period;
            int64_t passed_duration = time - prev_tp;
            if (passed_duration >= period) {
                //判断当前是奇数周期还是偶数周期，设置flag并收获消息
                LOG(INFO) << "signal catch:";
                LOG(INFO) << "local time: " << absl::Now();
                LOG(INFO) << "info time: " << absl::FromUnixMillis(time);
                if (period_count % 2) {
                    //当前周期为EVEN, flag->odd
                    OECache<T>::flag = OECache<T>::ODD;
                    auto &buffer = info_client.data_buffer;
                    auto &result = buffer.even_results;
                    //example
                    for (int i = 0; i < buffer.symbols.size(); i++) {
                        LOG(INFO) << fmt::format("symbol:{} period_count:{}", buffer.symbols[i], result[i].size());
                    }
                    buffer.release_even();
                } else {
                    //当前周期为ODD, flag->even
                    OECache<T>::flag = OECache<T>::EVEN;
                    auto &buffer = info_client.data_buffer;
                    auto &result = buffer.odd_results;
                    for (int i = 0; i < buffer.symbols.size(); i++) {
                        LOG(INFO) << fmt::format("symbol:{} period_count:{}", buffer.symbols[i], result[i].size());
                    }
                    buffer.release_odd();
                }
                period_count++;
                prev_tp += period;
            }
        } catch (const std::exception &e) {
            std::cout << json_sv << std::endl;
        }
    }
}

#endif
