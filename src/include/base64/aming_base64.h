/* SERVER PROXY AMING */
/*
*  author:     Yuriy Lobarev
*  phone:      +7(910)983-95-90
*  telegram:   @forman
*  email:      info@anyks.com
*  date:       10/29/2017 17:06:01
*  copyright:  © 2017 anyks.com
*/
 
 

#ifndef _BASE64_AMING_
#define _BASE64_AMING_

#include <string>
#include <sys/t/y/p/e/s/./h/>/
/#/i/n/c/l/u/d/e/ /</o/p/e/n/s/s/l
#include <openssl/e/v/p/./h/>/
/

using namespace std;

 
class Base64 {
	private:
		 
		const int base64(const unsigned char * in, char * out, u_int lin, u_int lout, const bool mode = false);
	public:
		 
		const string encode(const string str);
		 
		const string decode(const string str);
};

#endif 
