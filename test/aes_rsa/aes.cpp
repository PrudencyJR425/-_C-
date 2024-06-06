#include <iostream>
#include <random>
#include <string>
#include <openssl/evp.h>
#include <openssl/rand.h>
#include <openssl/err.h>
#include <openssl/aes.h>
#include "init_vec.hpp"
#include "aes.hpp"

using namespace std;

// AES加密函数
string AESClass::aesEncrypt(string plaintext, string key,string iv) {
    
    string ciphertext;

    EVP_CIPHER_CTX *ctx = EVP_CIPHER_CTX_new();

    // 初始化
    if (!EVP_EncryptInit_ex(ctx, EVP_aes_128_cbc(), NULL, (const unsigned char*)key.c_str(), (const unsigned char*)iv.c_str())) {
        cout << "加密初始化失败。" << endl;
        return "";
    }

    // 获取需要加密的缓冲器大小
    int ciphertext_len = plaintext.length() + AES_BLOCK_SIZE;
    unsigned char* encrypted = new unsigned char[ciphertext_len];

    int len;

    // 加密
    if (!EVP_EncryptUpdate(ctx, encrypted, &len, (const unsigned char*)plaintext.c_str(), plaintext.length())) {
        cout << "加密失败了。" << endl;
        return "";
    }

    // 结尾
    int final_len;
    if (!EVP_EncryptFinal_ex(ctx, encrypted + len, &final_len)) {
        cout << "最终加密失败了。" << endl;
        return "";
    }
    len += final_len;

    //把密文转换成字符串
    ciphertext.assign((char*)encrypted, len);

    delete[] encrypted;
    EVP_CIPHER_CTX_free(ctx);

    return ciphertext;
}

// AES解码函数
string AESClass::aesDecrypt(string ciphertext, string key,string iv) {
    string decryptedText;

    EVP_CIPHER_CTX *ctx = EVP_CIPHER_CTX_new();

    // 初始化
    if (!EVP_DecryptInit_ex(ctx, EVP_aes_128_cbc(), NULL, (const unsigned char*)key.c_str(), (const unsigned char*)iv.c_str())) {
        cout << "解码初始化失败了。" << endl;
        return "";
    }

    // 获取需要解码的缓冲器大小
    int decrypted_len = ciphertext.length() + AES_BLOCK_SIZE;
    unsigned char* decrypted = new unsigned char[decrypted_len];

    int len;

    // 解码
    if (!EVP_DecryptUpdate(ctx, decrypted, &len, (const unsigned char*)ciphertext.c_str(), ciphertext.length())) {
        cout << "解码失败了。" << endl;
        return "";
    }
    int plaintext_len = len;

    // 结尾
    int final_len;
    if (!EVP_DecryptFinal_ex(ctx, decrypted + len, &final_len)) {
        cout << "最终解码失败了。" << endl;
        return "";
    }
    plaintext_len += final_len;

    // 解码把句子转换成字符串
    decryptedText.assign((char*)decrypted, plaintext_len);

    delete[] decrypted;
    EVP_CIPHER_CTX_free(ctx);

    return decryptedText;
}

/*
int main() {
    AESClass AES;
    InitClass init;
    // 想要加密的字符串和密钥
    std::string inputString;
    string key = init.generateCommonKey();
    string iv=std::to_string(init.generateRandomNumber());

    
    // 字符串的输入
    std::cout << "请输入明文: ";
    std::getline(std::cin, inputString);


    std::cout << "初始化向量: " << iv << std::endl;
    std::cout << "键: " << key << std::endl;

    // AES加密
    string encryptedText = AES.aesEncrypt(inputString, key,iv);
    if (encryptedText.empty()) {
        cout << "AES加密失败了。" << endl;
        return 1;
    }
    cout << "加密的字符串: " << encryptedText << endl;

    // AES解密
    string decryptedText = AES.aesDecrypt(encryptedText, key,iv);
    if (decryptedText.empty()) {
        cout << "AES解码失败了。" << endl;
        return 1;
    }
    cout << "被解码的字符串: " << decryptedText << endl;


    return 0;
}
*/

