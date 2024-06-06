#include <openssl/rsa.h>
#include <openssl/pem.h>
#include <openssl/err.h>
#include <iostream>
#include <string>
#include "rsa.hpp"

//密钥的生成
RSA* RSAClass::createRSAKeyPair() {
    int keyLength = 2048;
    //公开指数(通常是rsa_f4)
    unsigned long e = RSA_F4; 

    RSA* rsa = RSA_generate_key(keyLength, e, NULL, NULL);
    if (rsa == NULL) {
        std::cerr << "密钥的生成失败了。" << std::endl;
        return NULL;
    }

    return rsa;
}

//以PEM形式获取公开密匙
std::string RSAClass::getPublicKey(RSA* rsa) {
    BIO* bio = BIO_new(BIO_s_mem());
    PEM_write_bio_RSA_PUBKEY(bio, rsa);
    
    size_t pubKeyLen = BIO_pending(bio);
    char* pubKey = new char[pubKeyLen + 1];
    BIO_read(bio, pubKey, pubKeyLen);
    pubKey[pubKeyLen] = '\0';

    std::string publicKey(pubKey);
    delete[] pubKey;
    BIO_free_all(bio);

    return publicKey;
}

//以PEM形式获取秘密密匙
std::string RSAClass::getPrivateKey(RSA* rsa) {
    BIO* bio = BIO_new(BIO_s_mem());
    PEM_write_bio_RSAPrivateKey(bio, rsa, NULL, NULL, 0, NULL, NULL);

    size_t privKeyLen = BIO_pending(bio);
    char* privKey = new char[privKeyLen + 1];
    BIO_read(bio, privKey, privKeyLen);
    privKey[privKeyLen] = '\0';

    std::string privateKey(privKey);
    delete[] privKey;
    BIO_free_all(bio);

    return privateKey;
}

//信息的加密
std::string RSAClass::encryptMessage(RSA* rsa, const std::string& message) {
    size_t rsaLen = RSA_size(rsa);
    unsigned char* encryptedMessage = new unsigned char[rsaLen];

    int result = RSA_public_encrypt(message.length(), 
                                    reinterpret_cast<const unsigned char*>(message.c_str()), 
                                    encryptedMessage, 
                                    rsa, 
                                    RSA_PKCS1_PADDING);

    if (result == -1) {
        char* err = new char[130];
        ERR_load_crypto_strings();
        ERR_error_string(ERR_get_error(), err);
        std::cerr << "加密失败了。 " << err << std::endl;
        delete[] err;
        return "";
    }

    std::string encryptedString(reinterpret_cast<char*>(encryptedMessage), result);
    delete[] encryptedMessage;

    return encryptedString;
}

//消息的解码
std::string RSAClass::decryptMessage(RSA* rsa, const std::string& encryptedMessage) {
    size_t rsaLen = RSA_size(rsa);
    unsigned char* decryptedMessage = new unsigned char[rsaLen];

    int result = RSA_private_decrypt(encryptedMessage.length(), 
                                     reinterpret_cast<const unsigned char*>(encryptedMessage.c_str()), 
                                     decryptedMessage, 
                                     rsa, 
                                     RSA_PKCS1_PADDING);

    if (result == -1) {
        char* err = new char[130];
        ERR_load_crypto_strings();
        ERR_error_string(ERR_get_error(), err);
        std::cerr << "解码失败了。" << err << std::endl;
        delete[] err;
        return "";
    }

    std::string decryptedString(reinterpret_cast<char*>(decryptedMessage), result);
    delete[] decryptedMessage;

    return decryptedString;
}
/*
int main() {
    RSAClass RRSA;
    //密钥生成
    RSA* rsa = RRSA.createRSAKeyPair();
    if (rsa == NULL) {
        return -1;
    }

    
    std::string message ;
    // 字符串的输入
    std::cout << "请输入明文: ";
    std::getline(std::cin,  message);

    // 公开密匙和秘密密匙的获取和显示
    std::string publicKey = RRSA.getPublicKey(rsa);
    std::string privateKey = RRSA.getPrivateKey(rsa);
    std::cout << "\n公开密匙:\n" << publicKey << std::endl;
    std::cout << "私有密钥:\n" << privateKey << std::endl;

    std::string encryptedMessage = RRSA.encryptMessage(rsa, message);
    std::cout << "加密的字符串:\n-----BEGIN ENCRYPTED STRING-----\n" << encryptedMessage <<"\n-----END ENCRYPTED STRING-----\n"<< std::endl;

    // 消息的解码
    std::string decryptedMessage = RRSA.decryptMessage(rsa, encryptedMessage);
    std::cout << "被解码的字符串: " << decryptedMessage << std::endl;

    // RSA对象的释放
    RSA_free(rsa);

    return 0;
}
*/