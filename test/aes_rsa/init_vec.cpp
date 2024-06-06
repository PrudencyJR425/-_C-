#include <iostream>
#include <random>
#include "init_vec.hpp"

unsigned long long InitClass::generateRandomNumber() {
    //随机数引擎
    std::random_device rd;
    std::mt19937_64 gen(rd());
    //生成16位随机整数
    std::uniform_int_distribution<unsigned long long> dist(1000000000000000ULL, 9999999999999999ULL);
    return dist(gen);
}

std::string InitClass::generateCommonKey() {
    //随机数引擎
    std::random_device rd;
    std::mt19937 gen(rd());
    
    //生成16位随机字符串
    const std::string charset = "0123456789abcdefghijklmnopqrstuvwxyzABCDEFGHIJKLMNOPQRSTUVWXYZ";
    std::uniform_int_distribution<std::size_t> dist(0, charset.size() - 1);
    
    std::string result;
    for (int i = 0; i < 16; ++i) {
        result += charset[dist(gen)];
    }
    
    return result;
}