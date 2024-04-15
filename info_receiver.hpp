#ifndef INFO_RECEIVER
#define INFO_RECEIVER
#include <cstddef>
#include <nlohmann/json.hpp>
#include <simdjson.h>
#include <stdexcept>
#include <string>
#include <string_view>
#include <absl/container/flat_hash_map.h>
#include <vector>
#include <websocketpp/client.hpp>
#include <websocketpp/config/asio_client.hpp>
#include <fmt/format.h>
#include "oe_cache.hpp"
#include "crypto_info.hpp"
#include "symbols_process.hpp"
template <typename T>
std::string get_message_type() {
    if constexpr (std::is_same_v<T, CryptoTradeInfo>) {
        return "trade";
    } else if constexpr (std::is_same_v<T, CryptoIncrementOrderBookInfo>) {
        return "depth@100ms";
    } else {
        throw std::runtime_error("Unsupport message type");
    }
}

template<typename T>
class InfoReceiver {
public:
    simdjson::dom::parser parser;
    OECache<T> data_buffer;
    explicit InfoReceiver(const std::vector<std::string>& symbols):data_buffer(symbols){};
    nlohmann::json get_subscribe_message() {
        std::vector<std::string> params;
        for (const auto &symbol: data_buffer.symbols) {
            params.push_back(fmt::format("{}@{}", symbol, get_message_type<T>()));
        }
        nlohmann::json subscribe_message = {
                {"method", "SUBSCRIBE"},
                {"params", params},
                {"id", SymbolConfig::instance().id}};
        return subscribe_message;
    }
    void process_trade(const simdjson::dom::element &doc) {
        try {
            auto symbol = doc["s"].get_string().value();
            auto event_time = doc["E"].get_int64().value();
            float price = std::stof(doc["p"].get_c_str().value());
            float amount = std::stof(doc["q"].get_c_str().value());
            auto trade_time = doc["T"].get_int64().value();
            //买方是否是做市方。如true，则此次成交是一个主动卖出单，否则是一个主动买入单。
            auto side = doc["m"].get_bool().value() ? 'S' : 'B';
            data_buffer.push_back(
                    symbol, trade_time, side, price, amount);
        } catch (const std::exception &e) {
            std::cerr << "Failed to extract data: " << e.what() << std::endl;
        }
    }
    void process_increment_orderbook(const simdjson::dom::element &doc) {
        try {
            std::string_view symbol = doc["s"].get_string().value();
            int64_t event_time = doc["E"].get_int64().value();
            size_t index = data_buffer.symbol_index[symbol];
            // 买单数组
            simdjson::dom::array bids = doc["b"].get_array();
            for (auto bid: bids) {
                float price = std::stof(bid.at(0).get_c_str().value());
                float amount = std::stof(bid.at(1).get_c_str().value());
                data_buffer.push_back(
                    index, event_time, false, 'B', price, amount
                );
            }
            // 卖单数组
            simdjson::dom::array asks = doc["a"].get_array();
            for (auto ask: asks) {
                float price = std::stof(ask.at(0).get_c_str().value());
                float amount = std::stof(ask.at(1).get_c_str().value());
                data_buffer.push_back(
                    index, event_time, false, 'S', price, amount);
            }

        } catch (const std::exception &e) {
            std::cerr << "Failed to extract data: " << e.what() << std::endl;
        }
    }

    void process_message(std::string_view json_sv) {
        simdjson::dom::element doc;
        auto error = parser.parse(json_sv).get(doc);
        if (error) {
            std::cerr << "Parsing failed: " << error << std::endl;
            return;
        }
        if constexpr (std::is_same_v<CryptoTradeInfo,T>) {
            process_trade(doc);
        } else if constexpr (std::is_same_v<CryptoIncrementOrderBookInfo,T>) {
            process_increment_orderbook(doc);
        }
    }
};

#endif