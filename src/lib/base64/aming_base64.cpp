/* SERVER PROXY AMING */
/*
*  author:     Yuriy Lobarev
*  phone:      +7(910)983-95-90
*  telegram:   @forman
*  email:      info@anyks.com
*  date:       11/08/2017 16:52:48
*  copyright:  © 2017 anyks.com
*/
 
 

#include "base64/base64.h"


using namespace std;

 
const int Base64::base64(const u_char * in, char * out, u_int lin, u_int lout, const bool mode){
	
	int result = 0;
	
	BIO * b64 = BIO_new(BIO_f_base64());
	BIO * bio = BIO_new(BIO_s_mem());
	
	BIO_set_flags(b64, BIO_FLAGS_BASE64_NO_NL);
	
	BIO_push(b64, bio);
	
	if(!mode){
		
		result = BIO_write(b64, in, lin);
		
		BIO_flush(b64);
		
		if(result) result = BIO_read(bio, out, lout);
	
	} else {
		
		result = BIO_write(bio, in, lin);
		
		BIO_flush(bio);
		
		if(result) result = BIO_read(b64, out, lout);
    }
	
	BIO_free(b64);
	
	return result;
}
 
const string Base64::encode(const string str){
	
	string result;
	
	if(!str.empty()){
		
		char out[256];
		
		const int len = this->base64(reinterpret_cast <const u_char *> (str.c_str()), out, str.length(), sizeof(out), false);
		
		if(len) result = string(out, len);
	}
	
	return result;
}
 
const string Base64::decode(const string str){
	
	string result;
	
	if(!str.empty()){
		
		char out[256];
		
		const int len = this->base64(reinterpret_cast <const u_char *> (str.c_str()), out, str.length(), sizeof(out), true);
		
		if(len) result = string(out, len);
	}
	
	return result;
}
