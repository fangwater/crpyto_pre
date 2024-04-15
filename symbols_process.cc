#include "symbols_process.hpp"
#include <algorithm>
#include "utils.hpp"

SymbolConfig &SymbolConfig::instance() {
    static SymbolConfig symbol_config;
    return symbol_config;
}

SymbolConfig::SymbolConfig() {
    auto symbol_config = open_json_file("./symbols.json");
    int k = symbol_config["partition_num"];
    std::vector<std::string> symbols = symbol_config["symbols"];
    symbol_partition = process_symbols(symbols, k);
    id = symbol_config["id"];
}

std::vector<std::vector<std::string>> split_symbols(const std::vector<std::string> &data, int k) {
    std::vector<std::vector<std::string>> result(k);
    int n = data.size();
    int base_size = n / k;
    int remainder = n % k;

    auto it = data.begin();
    for (int i = 0; i < k; ++i) {
        int current_size = base_size + (i < remainder ? 1 : 0);
        result[i].insert(result[i].end(), it, it + current_size);
        it += current_size;
    }
    return result;
}

// 主算法实现
std::vector<std::vector<std::string>> process_symbols(std::vector<std::string> &symbols, int k) {
    // 确定关键符号
    std::vector<std::string> key_symbols = {"btcusdt", "ethusdt", "bnbusdt"};
    std::vector<std::string> main_symbols;
    std::vector<std::string> other_symbols;

    // 检查是否存在必要的关键符号
    for (const auto &sym: key_symbols) {
        if (std::find(symbols.begin(), symbols.end(), sym) == symbols.end()) {
            throw std::runtime_error("Missing key symbol: " + sym);
        }
    }

    // 分类符号
    for (auto &sym: symbols) {
        if (std::find(key_symbols.begin(), key_symbols.end(), sym) != key_symbols.end()) {
            main_symbols.push_back(std::move(sym));// 直接移动到主列表
        } else {
            other_symbols.push_back(std::move(sym));
        }
    }

    // 将其他符号分割为 k 份
    std::vector<std::vector<std::string>> splitted_others = split_symbols(other_symbols, k);

    // 将主符号作为第一个子向量
    splitted_others.insert(splitted_others.begin(), main_symbols);

    return splitted_others;
}