/* SERVER PROXY AMING */
/*
*  author:     Yuriy Lobarev
*  phone:      +7(910)983-95-90
*  telegram:   @forman
*  email:      info@anyks.com
*  date:       10/29/2017 17:06:00
*  copyright:  © 2017 anyks.com
*/


#define _XOPEN_SOURCE

#include <regex>
#include <ctime>
#include <string>
#include <vector>
#include <iostream>
#include <sstream>
#include <math.h>
#include <sys/t/y/p/e/s/./h/>/
/#/i/n/c/l/u/d/e/ /</s/t/d/i/o/./h/>/
/#/i/n/c/l/u/d/e/ /</s/t/d/l/i/b/./h/>/
/
/u/s/i/n/g/ /n/a/m/e/s/p/a/c/e/ /s/t/d/;/
/
/t/y/p/e/d/e/f/ /i/n/t/ /i/n/t/1/2/8/_/t/ /_/_/a/t/t/r/i/b/u/t/e/_/_/(/(/m/o/d/e/(/T/I/)/)/)/;/
/t/y/p/e/d/e/f/ /u/n/s/i/g/n/e/d/ /i/n/t/ /u/i/n/t/1/2/8/_/t/ /_/_/a/t/t/r/i/b/u/t/e/_/_/(/(/m/o/d/e/(/T/I/)/)/)/;/
/

 * IPdata Класс содержащий данные ip адреса
 */
/c/l/a/s/s/ /I/P/d/a/t/a/ /{/
/	/p/r/i/v/a/t/e/:/
/	/	
		u_int ptr[4];
	public:
		 
		void set(u_int ptr1 = 0, u_int ptr2 = 0, u_int ptr3 = 0, u_int ptr4 = 0){
			
			if(ptr1 <= 255) this->ptr[0] = ptr1;
			if(ptr2 <= 255) this->ptr[1] = ptr2;
			if(ptr3 <= 255) this->ptr[2] = ptr3;
			if(ptr4 <= 255) this->ptr[3] = ptr4;
		}
		 
		const u_int * get(){
			
			return ptr;
		}
		 
		const string getStr(){
			
			string result;
			
			for(u_int i = 0; i < 4; i++){
				
				result.append(to_string(this->ptr[i]));
				
				if(i < 3) result.append(".");
			}
			
			return result;
		}
		 
		u_int size(){
			
			return sizeof(this->ptr) / /s/i/z/e/o/f/(/u/_/i/n/t/)/;/
/	/	/}/
/}/;/

 * NLdata Структура содержащая параметры локальных и запрещенных сетей
 */
/s/t/r/u/c/t/ /N/L/d/a/t/a/ /{/
/	/s/t/r/i/n/g/ /i/p/;/	/	
	string network;	
	u_int mask;		
	bool allow;		
};
 
struct NLdata6 {
	string ip;		
	string eip;		
	u_int prefix;	
	bool allow;		
};
 
struct NKdata {
	IPdata ip;		
	IPdata mask;	
	IPdata network;	
};
 
struct NTdata {
	char mask[17];		
	char nmask[17];		
	char smask[17];		
	char emask[17];		
	char invers[17];	
	char format[13];	
	char type[3];		
	float cls;			
	u_int number;		
	u_int sbytes;		
	u_long counts;		
	u_long maxnwk;		
	u_long maxhst;		
	bool notEmpty;		
};

vector <NLdata6> locals6 = {
	{"::1", "", 128, true},
	{"2001::", "", 32, false},
	{"2001:db8::", "", 32, true},
	{"64:ff9b::", "", 96, false},
	{"2002::", "", 16, false},
	{"fe80::", "febf::", 10, true},
	{"fec0::", "feff::", 10, true},
	{"fc00::", "", 7, true},
	{"ff00::", "", 8, false}
};

vector <NLdata> locals = {
	{"0.0.0.0", "0.0.0.0", 8, false},
	{"0.0.0.0", "0.0.0.0", 32, false},
	{"100.64.0.0", "100.64.0.0", 10, false},
	{"169.254.0.0", "169.254.0.0", 16, false},
	{"224.0.0.0", "224.0.0.0", 4, false},
	{"224.0.0.0", "224.0.0.0", 24, false},
	{"224.0.1.0", "224.0.0.0", 8, false},
	{"239.0.0.0", "239.0.0.0", 8, false},
	{"240.0.0.0", "240.0.0.0", 4, false},
	{"255.255.255.255", "255.255.255.255", 32, false},
	{"10.0.0.0", "10.0.0.0", 8, true},
	{"127.0.0.0", "127.0.0.0", 8, true},
	{"172.16.0.0", "172.16.0.0", 12, true},
	{"192.0.0.0", "192.0.0.0", 24, true},
	{"192.0.0.0", "192.0.0.0", 29, true},
	{"192.0.0.170", "192.0.0.170", 32, true},
	{"192.0.0.171", "192.0.0.171", 32, true},
	{"192.0.2.0", "192.0.2.0", 24, true},
	{"192.88.99.0", "192.88.99.0", 24, true},
	{"192.88.99.1", "192.88.99.1", 32, true},
	{"192.168.0.0", "192.168.0.0", 16, true},
	{"198.51.100.0", "198.51.100.0", 24, true},
	{"198.18.0.0", "198.18.0.0", 15, true},
	{"203.0.113.0", "203.0.113.0", 24, true}
};

vector <NTdata> masks = {
	{"000.000.000.000\0", "255.0.0.0\0", "0.0.0.0\0", "127.255.255.255\0", "255.255.255.255\0", "С.Х.Х.Х\0", "A\0", 256, 0, 0, 4294967296, 128, 16777214},	
	{"128.000.000.000\0", "255.0.0.0\0", "0.0.0.0\0", "127.255.255.255\0", "127.255.255.255\0", "С.Х.Х.Х\0", "A\0", 128, 1, 0, 2147483648, 128, 16777214},	
	{"192.000.000.000\0", "255.0.0.0\0", "0.0.0.0\0", "127.255.255.255\0", "63.255.255.255\0", "С.Х.Х.Х\0", "A\0", 64, 2, 0, 1073741824, 128, 16777214},	
	{"224.000.000.000\0", "255.0.0.0\0", "0.0.0.0\0", "127.255.255.255\0", "31.255.255.255\0", "С.Х.Х.Х\0", "A\0", 32, 3, 0, 536870912, 128, 16777214},		
	{"240.000.000.000\0", "255.0.0.0\0", "0.0.0.0\0", "127.255.255.255\0", "15.255.255.255\0", "С.Х.Х.Х\0", "A\0", 16, 4, 0, 268435456, 128, 16777214},		
	{"248.000.000.000\0", "255.0.0.0\0", "0.0.0.0\0", "127.255.255.255\0", "7.255.255.255\0", "С.Х.Х.Х\0", "A\0", 8, 5, 0, 134217728, 128, 16777214},		
	{"252.000.000.000\0", "255.0.0.0\0", "0.0.0.0\0", "127.255.255.255\0", "3.255.255.255\0", "С.Х.Х.Х\0", "A\0", 4, 6, 0, 67108864, 128, 16777214},		
	{"254.000.000.000\0", "255.0.0.0\0", "0.0.0.0\0", "127.255.255.255\0", "1.255.255.255\0", "С.Х.Х.Х\0", "A\0", 2, 7, 0, 33554432, 128, 16777214},		
	{"255.000.000.000\0", "255.0.0.0\0", "0.0.0.0\0", "127.255.255.255\0", "0.255.255.255\0", "С.Х.Х.Х\0", "A\0", 1, 8, 0, 16777216, 128, 16777214},		
	{"255.128.000.000\0", "255.255.0.0\0", "128.0.0.0\0", "191.255.255.255\0", "0.127.255.255\0", "С.С.Х.Х\0", "B\0", 128, 9, 10, 8388608, 16384, 65534},	
	{"255.192.000.000\0", "255.255.0.0\0", "128.0.0.0\0", "191.255.255.255\0", "0.63.255.255\0", "С.С.Х.Х\0", "B\0", 64, 10, 10, 4194304, 16384, 65534},	
	{"255.224.000.000\0", "255.255.0.0\0", "128.0.0.0\0", "191.255.255.255\0", "0.31.255.255\0", "С.С.Х.Х\0", "B\0", 32, 11, 10, 2097152, 16384, 65534},	
	{"255.240.000.000\0", "255.255.0.0\0", "128.0.0.0\0", "191.255.255.255\0", "0.15.255.255\0", "С.С.Х.Х\0", "B\0", 16, 12, 10, 1048576, 16384, 65534},	
	{"255.248.000.000\0", "255.255.0.0\0", "128.0.0.0\0", "191.255.255.255\0", "0.7.255.255\0", "С.С.Х.Х\0", "B\0", 8, 13, 10, 524288, 16384, 65534},		
	{"255.252.000.000\0", "255.255.0.0\0", "128.0.0.0\0", "191.255.255.255\0", "0.3.255.255\0", "С.С.Х.Х\0", "B\0", 4, 14, 10, 262144, 16384, 65534},		
	{"255.254.000.000\0", "255.255.0.0\0", "128.0.0.0\0", "191.255.255.255\0", "0.1.255.255\0", "С.С.Х.Х\0", "B\0", 2, 15, 10, 131072, 16384, 65534},		
	{"255.255.000.000\0", "255.255.0.0\0", "128.0.0.0\0", "191.255.255.255\0", "0.0.255.255\0", "С.С.Х.Х\0", "B\0", 1, 16, 10, 65536, 16384, 65534},		
	{"255.255.128.000\0", "255.255.255.0\0", "192.0.0.0\0", "223.255.255.255\0", "0.0.127.255\0", "С.С.С.Х\0", "C\0", 128, 17, 110, 32768, 2097152, 254},	
	{"255.255.192.000\0", "255.255.255.0\0", "192.0.0.0\0", "223.255.255.255\0", "0.0.63.255\0", "С.С.С.Х\0", "C\0", 64, 18, 110, 16384, 2097152, 254},		
	{"255.255.224.000\0", "255.255.255.0\0", "192.0.0.0\0", "223.255.255.255\0", "0.0.31.255\0", "С.С.С.Х\0", "C\0", 32, 19, 110, 8192, 2097152, 254},		
	{"255.255.240.000\0", "255.255.255.0\0", "192.0.0.0\0", "223.255.255.255\0", "0.0.15.255\0", "С.С.С.Х\0", "C\0", 16, 20, 110, 4096, 2097152, 254},		
	{"255.255.248.000\0", "255.255.255.0\0", "192.0.0.0\0", "223.255.255.255\0", "0.0.7.255\0", "С.С.С.Х\0", "C\0", 8, 21, 110, 2048, 2097152, 254},		
	{"255.255.252.000\0", "255.255.255.0\0", "192.0.0.0\0", "223.255.255.255\0", "0.0.3.255\0", "С.С.С.Х\0", "C\0", 4, 22, 110, 1024, 2097152, 254},		
	{"255.255.254.000\0", "255.255.255.0\0", "192.0.0.0\0", "223.255.255.255\0", "0.0.1.255\0", "С.С.С.Х\0", "C\0", 2, 23, 110, 512, 2097152, 254},			
	{"255.255.255.000\0", "255.255.255.0\0", "192.0.0.0\0", "223.255.255.255\0", "0.0.0.255\0", "С.С.С.Х\0", "C\0", 1, 24, 110, 256, 2097152, 254},			
	{"255.255.255.128\0", "255.255.255.0\0", "192.0.0.0\0", "223.255.255.255\0", "0.0.0.127\0", "С.С.С.Х\0", "C\0", 0.5, 25, 110, 128, 2097152, 127},		
	{"255.255.255.192\0", "255.255.255.0\0", "192.0.0.0\0", "223.255.255.255\0", "0.0.0.63\0", "С.С.С.Х\0", "C\0", 0.25, 26, 110, 64, 2097152, 	63},		
	{"255.255.255.224\0", "255.255.255.0\0", "192.0.0.0\0", "223.255.255.255\0", "0.0.0.31\0", "С.С.С.Х\0", "C\0", 0.125, 27, 110, 32, 2097152, 31},		
	{"255.255.255.240\0", "255.255.255.0\0", "192.0.0.0\0", "223.255.255.255\0", "0.0.0.15\0", "С.С.С.Х\0", "C\0", 0.0625, 28, 110, 16, 2097152, 15},		
	{"255.255.255.248\0", "255.255.255.0\0", "192.0.0.0\0", "223.255.255.255\0", "0.0.0.7\0", "С.С.С.Х\0", "C\0", 0.03125, 29, 110, 8, 2097152, 7},			
	{"255.255.255.252\0", "255.255.255.0\0", "192.0.0.0\0", "223.255.255.255\0", "0.0.0.3\0", "С.С.С.Х\0", "C\0", 0.015625, 30, 110, 4, 2097152, 3},		
	{"255.255.255.254\0", "255.255.255.0\0", "192.0.0.0\0", "223.255.255.255\0", "0.0.0.1\0", "С.С.С.Х\0", "C\0", 0.0078125, 31, 110, 2, 2097152, 1},		
	{"255.255.255.255\0", "255.255.255.0\0", "192.0.0.0\0", "223.255.255.255\0", "0.0.0.0\0", "С.С.С.Х\0", "C\0", 0.00390625, 32, 110, 1, 2097152, 	0}		
};
 
string toCase(string str, bool flag = false){
	
	transform(str.begin(), str.end(), str.begin(), (flag ? ::toupper : ::tolower));
	
	return str;
}
 
string & rtrim(string &str, const char * t = " \t\n\r\f\v"){
	str.erase(str.find_last_not_of(t) + 1);
	return str;
}
 
string & ltrim(string &str, const char * t = " \t\n\r\f\v"){
	str.erase(0, str.find_first_not_of(t));
	return str;
}
 
string & trim(string &str, const char * t = " \t\n\r\f\v"){
	return ltrim(rtrim(str, t), t);
}
 
vector <string> split(const string str, const string delim){
	
	vector <string> result;
	
	string value = str;
	
	value = trim(value);
	
	if(!value.empty()){
		string data;
		string::size_type i = 0;
		string::size_type j = value.find(delim);
		u_int len = delim.length();
		
		while(j != string::npos){
			data = value.substr(i, j - i);
			result.push_back(trim(data));
			i = ++j + (len - 1);
			j = value.find(delim, j);
			if(j == string::npos){
				data = value.substr(i, value.length());
				result.push_back(trim(data));
			}
		}
		
		if(result.empty()) result.push_back(value);
	}
	
	return result;
}
 
NTdata getMaskByNumber(u_int value){
	
	NTdata mask;
	
	if(value < masks.size()){
		
		mask = masks[value];
		
		mask.notEmpty = true;
	}
	
	return mask;
}
 
void setZerroToStrIp(string &str){
	
	if(str.length() < 3){
		
		for(u_int i = 0; i < (4 - str.length()); i++) str = string("0") + str;
	}
}
 
NTdata getMaskByString(string value){
	
	NTdata mask;
	
	smatch match;
	
	regex e(
		"^(\\d{1,3})\\.(\\d{1,3})\\.(\\d{1,3})\\.(\\d{1,3})$",
		regex::ECMAScript | regex::icase
	);
	
	regex_search(value, match, e);
	
	if(!match.empty()){
		
		string pir_1 = match[1].str();
		string pir_2 = match[2].str();
		string pir_3 = match[3].str();
		string pir_4 = match[4].str();
		
		setZerroToStrIp(pir_1);
		setZerroToStrIp(pir_2);
		setZerroToStrIp(pir_3);
		setZerroToStrIp(pir_4);
		
		string ip = (pir_1 + string(".") + pir_2 + string(".") + pir_3 + string(".") + pir_4);
		
		for(u_int i = 0; i < masks.size(); i++){
			
			if(ip.compare(masks[i].mask) == 0){
				
				mask = masks[i];
				
				mask.notEmpty = true;
				
				break;
			}
		}
	}
	
	return mask;
}
 
IPdata getDataIp(string ip){
	
	IPdata ipdata;
	
	smatch match;
	
	regex e(
		"^(\\d{1,3})\\.(\\d{1,3})\\.(\\d{1,3})\\.(\\d{1,3})$",
		regex::ECMAScript | regex::icase
	);
	
	regex_search(ip, match, e);
	
	if(!match.empty()){
		
		ipdata.set(
			u_int(::atoi(match[1].str().c_str())),
			u_int(::atoi(match[2].str().c_str())),
			u_int(::atoi(match[3].str().c_str())),
			u_int(::atoi(match[4].str().c_str()))
		);
	}
	
	return ipdata;
}
 
bool checkMask(IPdata ip, IPdata mask){
	
	for(u_int i = 0; i < ip.size(); i++){
		
		u_int msk = mask.get()[i];
		
		if(msk && (ip.get()[i] > msk)) return false;
	}
	
	return true;
}
 
bool checkMaskByString(string ip, string mask){
	
	IPdata ipdata = getDataIp(ip);
	
	IPdata maskdata = getDataIp(mask);
	
	return checkMask(ipdata, maskdata);
}
 
bool checkMaskByNumber(string ip, u_int mask){
	
	IPdata ipdata = getDataIp(ip);
	
	IPdata maskdata = getDataIp(getMaskByNumber(mask).mask);
	
	return checkMask(ipdata, maskdata);
}
 
IPdata imposeMask(IPdata ip, IPdata mask){
	
	IPdata network;
	
	u_int ptr[4];
	
	for(u_int i = 0; i < ip.size(); i++){
		
		ptr[i] = ip.get()[i];
		
		if(ptr[i] > mask.get()[i]) ptr[i] = 0;
	}
	
	network.set(ptr[0], ptr[1], ptr[2], ptr[3]);
	
	return network;
}
 
NKdata getNetwork(string str){
	
	NKdata result;
	
	smatch match;
	
	regex e(
		"^(\\d{1,3}\\.\\d{1,3}\\.\\d{1,3}\\.\\d{1,3})(?:\\/(\\d+|\\d{1,3}\\.\\d{1,3}\\.\\d{1,3}\\.\\d{1,3}))?$",
		regex::ECMAScript | regex::icase
	);
	
	regex_search(str, match, e);
	
	if(!match.empty()){
		
		string ip, mask;
		
		ip = match[1].str();
		
		mask = match[2].str();
		
		if(mask.empty()) mask = "0";
		
		if(mask.find(".") == string::npos){
			
			mask = getMaskByNumber(::atoi(mask.c_str())).mask;
		}
		
		string network;
		
		IPdata ipdata = getDataIp(ip);
		
		IPdata maskdata = getDataIp(mask);
		
		IPdata networkdata = imposeMask(ipdata, maskdata);
		
		result = {ipdata, maskdata, networkdata};
	}
	
	return result;
}
 
int isLocal(const string ip){
	
	IPdata ipdata = getDataIp(ip);
	
	for(u_int i = 0; i < locals.size(); i++){
		
		if(checkMaskByNumber(ip, locals[i].mask)){
			
			string mask = getMaskByNumber(locals[i].mask).mask;
			
			IPdata maskdata = getDataIp(mask);
			
			IPdata networkdata = imposeMask(ipdata, maskdata);
			
			if(networkdata.getStr()
			.compare(locals[i].network) == 0){
				
				if(locals[i].allow) return 0;
				
				else return -1;
			}
		}
	}
	
	return 1;
}

 
string getLow1Ip6(const string ip){
	
	string ipv6;
	
	smatch match;
	
	regex e(
		"^\\[?([ABCDEFabcdef\\d]{4})\\:([ABCDEFabcdef\\d]{4})"
		"\\:([ABCDEFabcdef\\d]{4})\\:([ABCDEFabcdef\\d]{4})"
		"\\:([ABCDEFabcdef\\d]{4})\\:([ABCDEFabcdef\\d]{4})"
		"\\:([ABCDEFabcdef\\d]{4})\\:([ABCDEFabcdef\\d]{4})\\]?$",
		regex::ECMAScript | regex::icase
	);
	
	regex_search(ip, match, e);
	
	if(!match.empty()){
		
		string str;
		
		u_int len = match.size();
		
		regex e("^(0+)([ABCDEFabcdef\\d]+)$");
		
		for(u_int i = 1; i < len; i++){
			
			str = match[i].str();
			
			if(str.compare("0000") == 0) str = "0";
			
			else str = regex_replace(str, e, "$2");
			
			ipv6.append(str);
			
			if(i < (len - 1)) ipv6.append(":");
		}
	}
	
	return ipv6;
}
 
string getLow2Ip6(const string ip){
	
	string ipv6;
	
	smatch match;
	
	regex e(
		"^\\[?([ABCDEFabcdef\\d]{1,4})\\:([ABCDEFabcdef\\d]{1,4})"
		"\\:([ABCDEFabcdef\\d]{1,4})\\:([ABCDEFabcdef\\d]{1,4})"
		"\\:([ABCDEFabcdef\\d]{1,4})\\:([ABCDEFabcdef\\d]{1,4})"
		"\\:([ABCDEFabcdef\\d]{1,4})\\:([ABCDEFabcdef\\d]{1,4})\\]?$",
		regex::ECMAScript | regex::icase
	);
	
	regex_search(ip, match, e);
	
	if(!match.empty()){
		
		string str = ip;
		
		vector <string> fstr;
		
		regex e("(?:^|\\:)([0\\:]+)(?:\\:|$)");
		
		while(true){
			
			regex_search(str, match, e);
			
			if(!match.empty()){
				
				string delim = match[0].str();
				
				size_t pos = str.find(delim);
				
				if(pos != string::npos){
					
					str.replace(pos, delim.length(), "");
				}
				
				fstr.push_back(delim);
			
			} else break;
		}
		
		if(!fstr.empty()){
			
			u_int max = 0, index = -1;
			
			for(u_int i = 0; i < fstr.size(); i++){
				
				size_t len = fstr[i].length();
				
				if(len > max){
					
					max = len;
					
					index = i;
				}
			}
			
			regex e(fstr[index]);
			
			ipv6 = regex_replace(ip, e, "::");
		
		} else ipv6 = str;
	}
	
	return ipv6;
}
 
string getLowIp6(const string ip){
	
	string str = getLow1Ip6(ip);
	
	if(str.empty()) str = ip;
	
	return getLow2Ip6(str);
}
 
string setLow2Ip6(const string ip){
	
	string ipv6;
	
	string str = ip;
	
	size_t pos = str.find("::");
	
	if(pos != string::npos){
		
		vector <string> fstr;
		
		smatch match;
		
		regex e("([ABCDEFabcdef\\d]{1,4})", regex::ECMAScript | regex::icase);
		
		while(true){
			
			regex_search(str, match, e);
			
			if(!match.empty()){
				
				string delim = match[1].str();
				
				size_t pos = str.find(delim);
				
				if(pos != string::npos){
					
					str.replace(pos, delim.length(), "");
				}
				
				fstr.push_back(delim);
			
			} else break;
		}
		
		u_int lhex = fstr.size();
		
		if(lhex < 8){
			
			string mask = ":";
			
			for(u_int i = 0; i < (8 - lhex); i++) mask.append("0:");
			
			ipv6 = ip;
			
			ipv6.replace(pos, 2, mask);
			
			regex e("(?:^\\[?\\:|\\:\\]?$)");
			
			ipv6 = regex_replace(ipv6, e, "");
		}
	}
	
	return ipv6;
}
 
string setLow1Ip6(const string ip){
	
	string ipv6;
	
	smatch match;
	
	regex e(
		"^\\[?([ABCDEFabcdef\\d]{1,4})\\:([ABCDEFabcdef\\d]{1,4})"
		"\\:([ABCDEFabcdef\\d]{1,4})\\:([ABCDEFabcdef\\d]{1,4})"
		"\\:([ABCDEFabcdef\\d]{1,4})\\:([ABCDEFabcdef\\d]{1,4})"
		"\\:([ABCDEFabcdef\\d]{1,4})\\:([ABCDEFabcdef\\d]{1,4})\\]?$",
		regex::ECMAScript | regex::icase
	);
	
	regex_search(ip, match, e);
	
	if(!match.empty()){
		
		string str = ip;
		
		vector <string> fstr;
		
		smatch match;
		
		regex e("([ABCDEFabcdef\\d]{1,4})", regex::ECMAScript | regex::icase);
		
		while(true){
			
			regex_search(str, match, e);
			
			if(!match.empty()){
				
				string delim = match[1].str();
				
				size_t pos = str.find(delim);
				
				if(pos != string::npos){
					
					str.replace(pos, delim.length(), "");
				}
				
				fstr.push_back(delim);
			
			} else break;
		}
		
		u_int len = fstr.size();
		
		for(u_int i = 0; i < len; i++){
			
			u_int size = fstr[i].length();
			
			if(size < 4){
				
				for(u_int j = 0; j < (4 - size); j++) fstr[i] = (string("0") + fstr[i]);
			}
			
			ipv6.append(fstr[i]);
			
			if(i < (len - 1)) ipv6.append(":");
		}
	}
	
	return ipv6;
}
 
string setLowIp6(const string ip){
	
	string str = setLow2Ip6(ip);
	
	if(str.empty()) str = ip;
	
	return setLow1Ip6(str);
}
 
const string imposePrefix6(const string ip6, u_int prefix){
	
	string str;
	
	if(prefix){
		
		str = setLowIp6(ip6);
		
		if(!str.empty()){
			
			if(prefix < 128){
				
				string ip = str;
				
				vector <string> fstr;
				
				smatch match;
				
				regex e("([ABCDEFabcdef\\d]{4})", regex::ECMAScript | regex::icase);
				
				while(true){
					
					regex_search(ip, match, e);
					
					if(!match.empty()){
						
						string delim = match[1].str();
						
						size_t pos = ip.find(delim);
						
						if(pos != string::npos){
							
							ip.replace(pos, delim.length(), "");
						}
						
						fstr.push_back(delim);
					
					} else break;
				}
				
				u_int fprefix = prefix;
				
				while(fmod(fprefix, 4)) fprefix++;
				
				int len = (fprefix / /1/6/)/;/
/	/	/	/	
				if(!len) len = 1;
				
				str.clear();
				
				for(u_int i = 0; i < len; i++){
					
					str.append(fstr[i]);
					
					str.append(":");
				}
				
				for(u_int i = len; i < 8; i++){
					
					str.append("0000");
					
					if(i < 7) str.append(":");
				}
			}
		}
		
		str = getLowIp6(str);
	}
	
	return str;
}
 
__uint64_t strIp6ToHex64(const string ip){
	
	__uint64_t result = 0;
	
	stringstream strm;
	
	regex e("[^ABCDEFabcdef\\d]", regex::ECMAScript | regex::icase);
	
	string str = regex_replace(ip, e, "");
	
	if(str.length() > 16) str = str.erase(15, str.length());
	
	strm << str;
	
	strm >> std::hex >> result;
	
	return result;
}
 
bool checkRange6(const string ip, const string bip, const string eip){
	
	bool result = false;
	
	if(!ip.empty() && !bip.empty() && !eip.empty()){
		
		__uint64_t nip	= strIp6ToHex64(setLowIp6(ip));
		__uint64_t nbip	= strIp6ToHex64(setLowIp6(bip));
		__uint64_t neip	= strIp6ToHex64(setLowIp6(eip));
		
		result = ((nip >= nbip) && (nip <= neip));
	}
	
	return result;
}
 
int isLocal6(const string ip){
	
	int result = 1;
	
	bool compare = false;
	
	for(u_int i = 0; i < locals6.size(); i++){
		
		if(!locals6[i].eip.empty())
			
			compare = checkRange6(ip, locals6[i].ip, locals6[i].eip);
		else {
			
			string network = toCase(setLowIp6(locals6[i].ip));
			
			string ipv6 = imposePrefix6(ip, locals6[i].prefix);
			
			ipv6 = toCase(setLowIp6(ipv6));
			
			vector <string> mip = split(ipv6, ":");
			
			if(mip[0].compare("0000") == 0) compare = true;
			
			else {
				
				vector <string> nwk = split(network, ":");
				
				for(u_int j = 0; j < mip.size(); j++){
					
					if((mip[j].compare(nwk[j]) == 0)
					|| (nwk[j].compare("0000") == 0)) compare = true;
					else {
						
						compare = false;
						
						break;
					}
				}
			}
		}
		
		if(compare) result = (!locals6[i].allow ? -1 : 0);
	}
	
	return result;
}
 
bool checkIPByNetwork6(const string ip, const string nwk){
	
	bool compare = false;
	
	smatch match;
	
	regex e("^([\\s\\S]+)\\/(\\d+)$", regex::ECMAScript | regex::icase);
	
	regex_search(nwk, match, e);
	
	if(!match.empty()){
		
		string network = toCase(setLowIp6(match[1].str()));
		
		string ipv6 = imposePrefix6(ip, ::atoi(match[2].str().c_str()));
		
		ipv6 = toCase(setLowIp6(ipv6));
		
		vector <string> mip = split(ipv6, ":");
		
		if(mip[0].compare("0000") == 0) compare = true;
		
		else {
			
			vector <string> nwk = split(network, ":");
			
			for(u_int j = 0; j < mip.size(); j++){
				
				if((mip[j].compare(nwk[j]) == 0)
				|| (nwk[j].compare("0000") == 0)) compare = true;
				else {
					
					compare = false;
					
					break;
				}
			}
		}
	}
	
	return compare;
}

 
bool checkIPByNetwork(const string ip, const string nwk){
	
	IPdata ipdata = getDataIp(ip);
	
	NKdata nwkdata = getNetwork(nwk);
	
	IPdata networkdata = imposeMask(ipdata, nwkdata.mask);
	
	if(networkdata.getStr()
	.compare(nwkdata.network.getStr()) == 0) return true;
	
	return false;
}

class A {
	private:
		int d = 153;
	public:
		
		
		

		function <void (int)> remove;

		void m(){
			
			this->remove(10);
		}
};

class B {
	public:
		int m = 13;
	private:
		int d = 153;
	public:
		
		void f2(int x){
			cout << x << endl;
		}

		void f1(){
			A t;

			t.remove = [this](int x){
				
				f2(x);
			};

			t.m();
		};
		
};


int f1 (int a) {
    cout << "f1: " << a << endl;
    return a*2;
}

void f2 (int (* ff) (int), int a) {
    int i = ff (a);
    cout << "f2: " << i << endl;

}

void f3(void * ctx, size_t param){
	memcpy(ctx, &param, sizeof(param));
}

int main(int len, char * buff[]){
	
	B d;

	d.f1();

	 

	

	

	

	
	

	f2(f1, 5);


	 
	 
	
	
	
	

	

	

	

	

	 
  	
  	cout << " **************1 " << isLocal6("[2a03:b0c0:3:d0::42e1:c001]") << endl; 

  	cout << " **************2 " << isLocal6("[2a00:1450:4001:824::200e]") << endl; 



	std::stringstream stream;
	stream << std::hex << 154;
	std::string result( stream.str() );

	std::stringstream ss;
	ss << "your id is " << std::hex << 0x0daffa0;
	const std::string s = ss.str();

	std::stringstream str;
	string s1 = "9a";
	str << s1;
	int value;
	str >> std::hex >> value;

	cout << " ========== " << result << " == " << s << " == " << value << endl;





	
	struct tm tm;
	char buf[255];

	memset(&tm, 0, sizeof(struct tm));
	strptime("Wed, 15 Mar 2017 14:10:08 GMT", "%a, %d %b %Y %X %Z", &tm);
	strftime(buf, sizeof(buf), "%A, %d %b %Y %H:%M:%S %Z", &tm);

	time_t seconds = mktime(&tm);

	cout << " ++++++++++++++1 " << buf << " == " << seconds << endl;

	
	

	char buf2[255];
	memset(buf2, 0, sizeof(buf2));
	struct tm * now_tm = gmtime(&seconds);
	strftime(buf2, sizeof(buf), "%a, %d %b %Y %X %OZ", now_tm);

	cout << " ++++++++++++++2 " << buf2 << endl;


	struct DM {
		size_t k;
		size_t m;
		size_t d;
		size_t p = 18434353;
	};

	DM dm;
	dm.k = 77;
	dm.m = 324;
	dm.d = 1123;

	unsigned char * raw_data = reinterpret_cast <unsigned char *> (&dm);

	
	for(size_t i = 0, k = 0; i < sizeof(DM); i += sizeof(size_t), k++){
		size_t d;
		memcpy(&d, raw_data + i, sizeof(size_t));
		cout << "----------- " << k << " == " << d << endl;
	}
	
	string data1 = "Hello World";

	unsigned char * raw_data1 = reinterpret_cast <unsigned char *> (strdup(data1.data()));

	string data2 = reinterpret_cast <char *> (raw_data1);

	cout << " --------- " << data2 << endl;

	size_t par = 13;

	f3(&par, 22);

	cout << " =========== " << par << endl;

	 

	return 0;
}