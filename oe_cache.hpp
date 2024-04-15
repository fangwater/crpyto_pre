#ifndef OECACHE_HPP
#define OECACHE_HPP
#include <absl/container/flat_hash_map.h>
#include <absl/log/absl_log.h>
#include <absl/log/log.h>
#include <atomic>
#include <cassert>
#include <cmath>
#include <cstddef>
#include <string_view>
#include <vector>
template<typename T>
class OECache {
public:
    static constexpr bool ODD = true;
    static constexpr bool EVEN = false;
    static std::atomic_bool flag;
    std::vector<std::vector<T>> odd_results;
    std::vector<std::vector<T>> even_results;
    std::vector<std::string> symbols;
    absl::flat_hash_map<std::string, size_t> symbol_index;
    explicit OECache(const std::vector<std::string>& syms);

    void release_odd();
    void release_even();

    template<typename... Args>
    void push_back(std::string_view key, Args &&...args);

    template<typename... Args>
    void push_back(size_t i, Args &&...args);
};

template<typename T>
std::atomic_bool OECache<T>::flag{OECache<T>::ODD};

template<typename T>
OECache<T>::OECache(const std::vector<std::string> &syms) : symbols(syms) {
    odd_results.resize(1000);
    even_results.resize(1000);
    for (int i = 0; i < syms.size(); i++) {
        std::string upper_sym = syms[i];
        std::transform(upper_sym.begin(), upper_sym.end(), upper_sym.begin(),
                       [](unsigned char c) { return std::toupper(c); });
        symbol_index[upper_sym] = i;
    }
    assert(symbol_index.size() == syms.size());
}

template<typename T>
void OECache<T>::release_odd() {
    for (auto &v: odd_results) {
        v.clear();
    }
}

template<typename T>
void OECache<T>::release_even() {
    for (auto &v: even_results) {
        v.clear();
    }
}

template<typename T>
template<typename... Args>
void OECache<T>::push_back(std::string_view key, Args &&...args) {
    auto i = symbol_index[key];
    if (flag == ODD) {
        odd_results[i].emplace_back(std::forward<Args>(args)...);
    } else {
        even_results[i].emplace_back(std::forward<Args>(args)...);
    }
}

template<typename T>
template<typename... Args>
void OECache<T>::push_back(size_t i, Args &&...args) {
    if (flag == ODD) {
        odd_results[i].emplace_back(std::forward<Args>(args)...);
    } else {
        even_results[i].emplace_back(std::forward<Args>(args)...);
    }
}


#endif