/* SERVER PROXY AMING */
/*
*  author:     Yuriy Lobarev
*  phone:      +7(910)983-95-90
*  telegram:   @forman
*  email:      info@anyks.com
*  date:       10/29/2017 17:06:01
*  copyright:  © 2017 anyks.com
*/
 
 

#ifndef _GENERAL_AMING_
#define _GENERAL_AMING_

#include <regex>
#include <pwd.h>
#include <grp.h>
#include <string>
#include <vector>
#include <math.h>
#include <stdlib.h>
#include <unistd.h>
#include <dirent.h>
#include <sys/t/y/p/e/s/./h/>/
/#/i/n/c/l/u/d/e/ /</s/y/s
#include <sys/s/t/a/t/./h/>/
/#/i/n/c/l/u/d/e/ /</o/p/e/n/s/s/l
#include <openssl/s/h/a/./h/>/
/

#define AMING_NULL 0x00
#define AMING_IP 0x01
#define AMING_NETWORK 0x02
#define AMING_IPV4 0x03
#define AMING_IPV6 0x04
#define AMING_DOMAIN 0x05
#define AMING_ADDRESS 0x06
#define AMING_MAC 0x07
#define AMING_USER 0x08
#define AMING_GROUP 0x09
#define AMING_HTTP_ACTION 0x10
#define AMING_HTTP_METHOD 0x11
#define AMING_HTTP_TRAFFIC 0x12


#define PROXY_HTTP_METHODS "OPTIONS|GET|HEAD|POST|PUT|PATCH|DELETE|TRACE|CONNECT"


using namespace std;

 
namespace Anyks { 
	 
	void cpydata(const u_char * data, size_t size, size_t &it, void * result);
	 
	void cpydata(const u_char * data, size_t size, size_t &it, string &result);
	 
	const string toCase(string str, bool flag = false);
	 
	const bool isNumber(const string &str);
	 
	string & rtrim(string &str, const char * t = " \t\n\r\f\v");
	 
	string & ltrim(string &str, const char * t = " \t\n\r\f\v");
	 
	string & trim(string &str, const char * t = " \t\n\r\f\v");
	 
	const bool checkPort(string str);
	 
	void replace(string &s, const string f, const string r);
	 
	void split(const string &str, const string delim, vector <string> &v);
	 
	vector <string> split(const string str, const string delim);
	 
	uid_t getUid(const char * name);
	 
	gid_t getGid(const char * name);
	 
	void setOwner(const char * path, const string user, const string group);
	 
	void mkDir(const char * path);
	 
	const int rmDir(const char * path);
	 
	bool makePath(const char * path, const string user, const string group);
	 
	bool isDirExist(const char * path);
	 
	bool isFileExist(const char * path);
	 
	const string addToPath(const string path, const string file);
	 
	const string getPathByString(const string path);
	 
	const string getQueryByString(const string path);
	 
	const string md5(const string text);
	 
	const string sha1(const string text);
	 
	const string sha256(const string text);
	 
	const string sha512(const string text);
	 
	const time_t strToTime(const char * date);
	 
	const string timeToStr(const time_t date);
	 
	const long getSizeBuffer(const string str);
	 
	const size_t getBytes(const string str);
	 
	const size_t getSeconds(const string str);
	 
	const bool isAddress(const string address);
	 
	const u_int getTypeAmingByString(const string str);
	 
	const bool checkDomainByMask(const string domain, const string mask);
}

#endif 