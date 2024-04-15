#ifndef SYMBOL_PROCESS_HPP
#define SYMBOL_PROCESS_HPP
#include <string>
#include <vector>
std::vector<std::vector<std::string>> split_symbols(const std::vector<std::string> &data, int k);
std::vector<std::vector<std::string>> process_symbols(std::vector<std::string> &symbols, int k);

class SymbolConfig {
public:
    std::vector<std::vector<std::string>> symbol_partition;
    int64_t id;
    static SymbolConfig &instance();
private:
    SymbolConfig();
};

#endif