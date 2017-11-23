/* SERVER PROXY AMING */
/*
*  author:     Yuriy Lobarev
*  phone:      +7(910)983-95-90
*  telegram:   @forman
*  email:      info@anyks.com
*  date:       11/23/2017 17:50:05
*  copyright:  © 2017 anyks.com
*/
 
 

#ifndef _GENERAL_AMING_
#define _GENERAL_AMING_

#include <regex>
#include <pwd.h>
#include <grp.h>
#include <string>
#include <vector>
#include <algorithm>
#include <math.h>
#include <stdlib.h>
#include <unistd.h>
#include <dirent.h>
#include <sys/t/y/p/e/s/./h/>/
/#/i/n/c/l/u/d/e/ /</s/y/s
#include <sys/s/t/a/t/./h/>/
/#/i/n/c/l/u/d/e/ /</o/p/e/n/s/s/l
#include <openssl/s/h/a/./h/>/
/#/i/n/c/l/u/d/e/ /"/d/e/f/a/u/l/t/s


using namespace std;

 
struct Anyks {
	 
	template <typename T>
	static const vector <T> concatVectors(const vector <T> &v1, const vector <T> &v2);
	 
	static void setOptions(const u_short option, u_short &options, const bool flag);
	 
	static void cpydata(const u_char * data, size_t size, size_t &it, void * result);
	 
	static void cpydata(const u_char * data, size_t size, size_t &it, string &result);
	 
	static const string toCase(string str, bool flag = false);
	 
	static const bool isNumber(const string &str);
	 
	static string & rtrim(string &str, const char * t = " \t\n\r\f\v");
	 
	static string & ltrim(string &str, const char * t = " \t\n\r\f\v");
	 
	static string & trim(string &str, const char * t = " \t\n\r\f\v");
	 
	static const bool checkPort(string str);
	 
	static void replace(string &s, const string f, const string r);
	 
	static void split(const string &str, const string delim, vector <string> &v);
	 
	static vector <string> split(const string str, const string delim);
	 
	static uid_t getUid(const char * name);
	 
	static gid_t getGid(const char * name);
	 
	static void setOwner(const char * path, const string user, const string group);
	 
	static void mkDir(const char * path);
	 
	static const int rmDir(const char * path);
	 
	static bool makePath(const char * path, const string user, const string group);
	 
	static bool isDirExist(const char * path);
	 
	static bool isFileExist(const char * path);
	 
	static const string addToPath(const string path, const string file);
	 
	static const string getPathByString(const string path);
	 
	static const string getQueryByString(const string path);
	 
	static const string md5(const string text);
	 
	static const string sha1(const string text);
	 
	static const string sha256(const string text);
	 
	static const string sha512(const string text);
	 
	static const time_t strToTime(const char * date);
	 
	static const string timeToStr(const time_t date);
	 
	static const string strFormat(const char * format, ...);
	 
	static const long getSizeBuffer(const string str);
	 
	static const size_t getBytes(const string str);
	 
	static const size_t getSeconds(const string str);
	 
	static const bool isAddress(const string address);
	 
	static const u_int getTypeAmingByString(const string str);
	 
	static const bool checkDomainByMask(const string domain, const string mask);
};

#endif 