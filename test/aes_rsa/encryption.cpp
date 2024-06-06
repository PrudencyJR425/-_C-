#include <iostream>
#include <random>
#include <string>
#include <openssl/evp.h>
#include <openssl/rand.h>
#include <openssl/err.h>
#include <openssl/aes.h>
#include "encryption.hpp"

using namespace std;
// aes 加密
string aes_opt::aesEncrypt(string plaintext) {
    string ciphertext;
    EVP_CIPHER_CTX *ctx = EVP_CIPHER_CTX_new();
    // 初始化
    if (!EVP_EncryptInit_ex(ctx, EVP_aes_128_cbc(), NULL, 
        (const unsigned char*)aes_opt::get_initKey().c_str(),
        (const unsigned char*)aes_opt::get_initVec().c_str())) {
            cout << "加密初始化失败。" << endl;
            return "";
    }
    // 加密缓冲区
    int ciphertext_len = plaintext.length() + AES_BLOCK_SIZE;
    unsigned char* encrypted = new unsigned char[ciphertext_len];
    int len;
    // 加密
    if (!EVP_EncryptUpdate(ctx, encrypted, &len, (const unsigned char*)plaintext.c_str(), plaintext.length())) {
        cout << "加密失败了。" << endl;
        return "";
    }
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
string aes_opt::aesDecrypt(string ciphertext) {
    string decryptedText;

    EVP_CIPHER_CTX *ctx = EVP_CIPHER_CTX_new();

    // 初始化
    if (!EVP_DecryptInit_ex(ctx, EVP_aes_128_cbc(), NULL, 
        (const unsigned char*)aes_opt::get_initKey().c_str(), 
        (const unsigned char*)aes_opt::get_initVec().c_str())) {
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



int main(){
    aes_opt* aes_option = aes_opt::get_instance();
    cout << "initkey:" << aes_opt::get_initKey() << endl;
    cout << "initVec:" << aes_opt::get_initVec() << endl;

    // 字符串的输入
    string inputString;
    cout << "请输入明文: ";
    getline(std::cin, inputString);

    // AES加密
    string encryptedText = aes_option->aesEncrypt(inputString);
    if (encryptedText.empty()) {
        cout << "AES加密失败了。" << endl;
        return 1;
    }
    cout << "加密的字符串: " << encryptedText << endl;

    // AES解密
    string decryptedText = aes_option->aesDecrypt(encryptedText);
    if (decryptedText.empty()) {
        cout << "AES解码失败了。" << endl;
        return 1;
    }
    cout << "被解码的字符串: " << decryptedText << endl;
    return 0;
}