/* SERVER PROXY AMING */
/*
*  author:     Yuriy Lobarev
*  phone:      +7(910)983-95-90
*  telegram:   @forman
*  email:      info@anyks.com
*  date:       10/29/2017 17:06:00
*  copyright:  © 2017 anyks.com
*/
#include <stdio.h>
#include <string>
#include <sys/t/y/p/e/s/./h/>/
/#/i/n/c/l/u/d/e/ /</o/p/e/n/s/s/l
#include <openssl/s/h/a/./h/>/
/

using namespace std;



 
const string md5(const string text){
	
	string result;
	
	if(!text.empty()){
		
		u_char digest[16];
		
		MD5_CTX ctx;
		
		MD5_Init(&ctx);
		
		MD5_Update(&ctx, text.c_str(), text.length());
		
		MD5_Final(digest, &ctx);
		
		char mdString[33];
		
		memset(mdString, 0, 33);
		
		for(u_short i = 0; i < 16; i++) sprintf(&mdString[i * 2], "%02x", u_int(digest[i]));
		
		result = mdString;
	}
	
	return result;
}

 
const string sha1(const string text){
	
	string result;
	
	if(!text.empty()){
		
		u_char digest[20];
		
		SHA_CTX ctx;
		
		SHA1_Init(&ctx);
		
		SHA1_Update(&ctx, text.c_str(), text.length());
		
		SHA1_Final(digest, &ctx);
		
		char mdString[41];
		
		memset(mdString, 0, 41);
		
		for(u_short i = 0; i < 20; i++) sprintf(&mdString[i * 2], "%02x", u_int(digest[i]));
		
		result = mdString;
	}
	
	return result;
}

 
const string sha256(const string text){
	
	string result;
	
	if(!text.empty()){
		
		u_char digest[32];
		
		SHA256_CTX ctx;
		
		SHA256_Init(&ctx);
		
		SHA256_Update(&ctx, text.c_str(), text.length());
		
		SHA256_Final(digest, &ctx);
		
		char mdString[65];
		
		memset(mdString, 0, 65);
		
		for(u_short i = 0; i < 32; i++) sprintf(&mdString[i * 2], "%02x", u_int(digest[i]));
		
		result = mdString;
	}
	
	return result;
}

 
const string sha512(const string text){
	
	string result;
	
	if(!text.empty()){
		
		u_char digest[64];
		
		SHA512_CTX ctx;
		
		SHA512_Init(&ctx);
		
		SHA512_Update(&ctx, text.c_str(), text.length());
		
		SHA512_Final(digest, &ctx);
		
		char mdString[129];
		
		memset(mdString, 0, 129);
		
		for(u_short i = 0; i < 64; i++) sprintf(&mdString[i * 2], "%02x", u_int(digest[i]));
		
		result = mdString;
	}
	
	return result;
}


int main() {
   
    printf("md5 digest: %s\n", sha512("user").c_str());
 
    return 0;
}
