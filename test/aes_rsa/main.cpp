#include <openssl/rsa.h>
#include <openssl/pem.h>
#include <openssl/err.h>
#include <iostream>
#include <string>
#include "init_vec.hpp"
#include "aes.hpp"
#include "rsa.hpp"

//编译需要连接库 : -lssl 和 -lcrypto
using namespace std;

int main(){

    /*********************************************/
    /*******************----A-----****************/
    /*********************************************/

    //密钥，初始化向量生成类" init_vec .hpp"
    InitClass init;
    // AES密码方法类"aes.hpp"
    AESClass AES;
    // RSA密码方法类"rsa.hpp"
    RSAClass rsaclass;

    // 字符串的输入
    string message ;
    cout << "\n请输入n信息: ";
    getline(cin,  message);

    // 生成公共密钥
    string commonkey = init.generateCommonKey();
    cout << "\n公用密钥: " << commonkey << endl;

    // 生成初始化向量
    string  iv=to_string(init.generateRandomNumber());
    cout << "\n初始化向量: " << iv << endl;
    
    // 用公用密钥加密信息
    string encryptedMessage = AES.aesEncrypt(message, commonkey,iv);
    if (encryptedMessage.empty()) {
       cout << "\nAES加密失败了。" << endl;
        return 1;
    }
    cout << "\n加密的信息: " << encryptedMessage << endl;

    // 生成rsa公匙，私匙
    // 键对的生成
    RSA* rsa = rsaclass.createRSAKeyPair();
    if (rsa == NULL) {
        return -1;
    }
    string publicKey = rsaclass.getPublicKey(rsa);
    string privateKey = rsaclass.getPrivateKey(rsa);
    cout << "\n公钥:\n" << publicKey << endl;
    cout << "私钥:\n" << privateKey << endl;

    // 用rsa公匙加密eas公匙，初始化向量
    // 加密eas公匙
    string encryptedcommonkey = rsaclass.encryptMessage(rsa, commonkey);
    cout << "\n加密的eas密钥:\n-----BEGIN ENCRYPTED STRING-----\n" << encryptedcommonkey <<"\n-----END ENCRYPTED STRING-----\n"<< endl;
    //加密初始化向量
    string encryptediv = rsaclass.encryptMessage(rsa, iv);
    cout << "\n加密初始化向量:\n-----BEGIN ENCRYPTED STRING-----\n" << encryptediv <<"\n-----END ENCRYPTED STRING-----\n"<< endl;

    /*********************************************/
    /*********************************************/
    /*********************************************/



    /*********************************************/
    /*******************----B-----****************/
    /*********************************************/
    /*接收侧:rsa私匙，加密的eas公匙，加密的初始化向量，加密的消息*/
    /*encryptedcommonkey,encryptediv,encryptedMessage*/
    
    //eas公共密匙，初始化向量解密
    string decryptedcommonkey = rsaclass.decryptMessage(rsa, encryptedcommonkey);
    cout << "\n被解码的公共密钥: " << decryptedcommonkey << endl;
    string decryptediv = rsaclass.decryptMessage(rsa, encryptediv);
    cout << "\n经解码的初始化向量: " << decryptediv << endl;

    //用解密的公共密钥解密信息
    string decryptedMessage = AES.aesDecrypt(encryptedMessage, decryptedcommonkey,decryptediv);
    if (decryptedMessage.empty()) {
        cout << "\nAES解码失败了。" << endl;
        return 1;
    }
    cout << "\n被解码的消息: " << decryptedMessage << endl;

    /*********************************************/
    /*********************************************/
    /*********************************************/

}