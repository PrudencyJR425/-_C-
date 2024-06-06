#ifndef ENCRYPTION_H
#define ENCRYPTION_H
#include <string>
class aes_opt
{
public:
    //懒汉单例模式
    static aes_opt* get_instance(){
        static aes_opt aes_option;
        return &aes_option;
    };
    static std::string get_initKey(){
        return init_key;
    }
    static std::string get_initVec(){
        return init_vec;
    }
    // aes 加密
    static std::string aesEncrypt(std::string plaintext);
    // aes 解密
    static std::string aesDecrypt(std::string plaintext);
private:
    aes_opt(){};
    //静态常量aes密钥
    static const std::string init_key;
    //静态常量aes初始向量
    static const std::string init_vec;
};

#endif