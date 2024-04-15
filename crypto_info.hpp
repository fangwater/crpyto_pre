#ifndef CRYPTO_INFO_HPP
#define CRYPTO_INFO_HPP
#include <cstdint>
struct CryptoIncrementOrderBookInfo {
    int64_t timestamp;
    bool is_snapshot;
    char side;
    float price;
    float amount;
    CryptoIncrementOrderBookInfo(int64_t _timestamp, bool _is_snapshot, char _side, float _price, float _amount)
        : timestamp(_timestamp), is_snapshot(_is_snapshot), side(_side), price(_price), amount(_amount) {}
};

struct CryptoTradeInfo {
    int64_t timestamp;
    char side;
    float price;
    float amount;
    CryptoTradeInfo(int64_t _timestamp, char _side, float _price, float _amount)
        : timestamp(_timestamp), side(_side), price(_price), amount(_amount) {}
};
#endif