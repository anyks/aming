/* SERVER PROXY AMING */
/*
*  author:     Yuriy Lobarev
*  phone:      +7(910)983-95-90
*  telegram:   @forman
*  email:      info@anyks.com
*  date:       10/29/2017 17:06:01
*  copyright:  © 2017 anyks.com
*/
 
 

#ifndef _NETWORK_AMING_
#define _NETWORK_AMING_

#include <regex>
#include <string>
#include <vector>
#include <iostream>
#include <sstream>
#include <math.h>
#include <sys/t/y/p/e/s/./h/>/
/#/i/n/c/l/u/d/e/ /"/g/e/n/e/r/a/l


using namespace std;

 
class IPdata {
	private:
		
		u_int ptr[4];
	public:
		 
		void set(u_int ptr1 = 0, u_int ptr2 = 0, u_int ptr3 = 0, u_int ptr4 = 0);
		 
		const u_int * get();
		 
		const string getStr();
		 
		u_int size();
		 
		IPdata();
};
 
struct NTdata {
	string mask;	
	string nmask;	
	string smask;	
	string emask;	
	string invers;	
	string format;	
	string type;	
	float cls;		
	u_int number;	
	u_int sbytes;	
	u_long counts;	
	u_long maxnwk;	
	u_long maxhst;	
	bool notEmpty;	
};
 
struct NLdata {
	string ip;		
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
 
class Network {
	private:
		
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
			{"000.000.000.000", "255.0.0.0", "0.0.0.0", "127.255.255.255", "255.255.255.255", "С.Х.Х.Х", "A", 256, 0, 0, 4294967296, 128, 16777214, false},	
			{"128.000.000.000", "255.0.0.0", "0.0.0.0", "127.255.255.255", "127.255.255.255", "С.Х.Х.Х", "A", 128, 1, 0, 2147483648, 128, 16777214, false},	
			{"192.000.000.000", "255.0.0.0", "0.0.0.0", "127.255.255.255", "63.255.255.255", "С.Х.Х.Х", "A", 64, 2, 0, 1073741824, 128, 16777214, false},	
			{"224.000.000.000", "255.0.0.0", "0.0.0.0", "127.255.255.255", "31.255.255.255", "С.Х.Х.Х", "A", 32, 3, 0, 536870912, 128, 16777214, false},	
			{"240.000.000.000", "255.0.0.0", "0.0.0.0", "127.255.255.255", "15.255.255.255", "С.Х.Х.Х", "A", 16, 4, 0, 268435456, 128, 16777214, false},	
			{"248.000.000.000", "255.0.0.0", "0.0.0.0", "127.255.255.255", "7.255.255.255", "С.Х.Х.Х", "A", 8, 5, 0, 134217728, 128, 16777214, false},		
			{"252.000.000.000", "255.0.0.0", "0.0.0.0", "127.255.255.255", "3.255.255.255", "С.Х.Х.Х", "A", 4, 6, 0, 67108864, 128, 16777214, false},		
			{"254.000.000.000", "255.0.0.0", "0.0.0.0", "127.255.255.255", "1.255.255.255", "С.Х.Х.Х", "A", 2, 7, 0, 33554432, 128, 16777214, false},		
			{"255.000.000.000", "255.0.0.0", "0.0.0.0", "127.255.255.255", "0.255.255.255", "С.Х.Х.Х", "A", 1, 8, 0, 16777216, 128, 16777214, false},		
			{"255.128.000.000", "255.255.0.0", "128.0.0.0", "191.255.255.255", "0.127.255.255", "С.С.Х.Х", "B", 128, 9, 10, 8388608, 16384, 65534, false},	
			{"255.192.000.000", "255.255.0.0", "128.0.0.0", "191.255.255.255", "0.63.255.255", "С.С.Х.Х", "B", 64, 10, 10, 4194304, 16384, 65534, false},	
			{"255.224.000.000", "255.255.0.0", "128.0.0.0", "191.255.255.255", "0.31.255.255", "С.С.Х.Х", "B", 32, 11, 10, 2097152, 16384, 65534, false},	
			{"255.240.000.000", "255.255.0.0", "128.0.0.0", "191.255.255.255", "0.15.255.255", "С.С.Х.Х", "B", 16, 12, 10, 1048576, 16384, 65534, false},	
			{"255.248.000.000", "255.255.0.0", "128.0.0.0", "191.255.255.255", "0.7.255.255", "С.С.Х.Х", "B", 8, 13, 10, 524288, 16384, 65534, false},		
			{"255.252.000.000", "255.255.0.0", "128.0.0.0", "191.255.255.255", "0.3.255.255", "С.С.Х.Х", "B", 4, 14, 10, 262144, 16384, 65534, false},		
			{"255.254.000.000", "255.255.0.0", "128.0.0.0", "191.255.255.255", "0.1.255.255", "С.С.Х.Х", "B", 2, 15, 10, 131072, 16384, 65534, false},		
			{"255.255.000.000", "255.255.0.0", "128.0.0.0", "191.255.255.255", "0.0.255.255", "С.С.Х.Х", "B", 1, 16, 10, 65536, 16384, 65534, false},		
			{"255.255.128.000", "255.255.255.0", "192.0.0.0", "223.255.255.255", "0.0.127.255", "С.С.С.Х", "C", 128, 17, 110, 32768, 2097152, 254, false},	
			{"255.255.192.000", "255.255.255.0", "192.0.0.0", "223.255.255.255", "0.0.63.255", "С.С.С.Х", "C", 64, 18, 110, 16384, 2097152, 254, false},	
			{"255.255.224.000", "255.255.255.0", "192.0.0.0", "223.255.255.255", "0.0.31.255", "С.С.С.Х", "C", 32, 19, 110, 8192, 2097152, 254, false},		
			{"255.255.240.000", "255.255.255.0", "192.0.0.0", "223.255.255.255", "0.0.15.255", "С.С.С.Х", "C", 16, 20, 110, 4096, 2097152, 254, false},		
			{"255.255.248.000", "255.255.255.0", "192.0.0.0", "223.255.255.255", "0.0.7.255", "С.С.С.Х", "C", 8, 21, 110, 2048, 2097152, 254, false},		
			{"255.255.252.000", "255.255.255.0", "192.0.0.0", "223.255.255.255", "0.0.3.255", "С.С.С.Х", "C", 4, 22, 110, 1024, 2097152, 254, false},		
			{"255.255.254.000", "255.255.255.0", "192.0.0.0", "223.255.255.255", "0.0.1.255", "С.С.С.Х", "C", 2, 23, 110, 512, 2097152, 254, false},		
			{"255.255.255.000", "255.255.255.0", "192.0.0.0", "223.255.255.255", "0.0.0.255", "С.С.С.Х", "C", 1, 24, 110, 256, 2097152, 254, false},		
			{"255.255.255.128", "255.255.255.0", "192.0.0.0", "223.255.255.255", "0.0.0.127", "С.С.С.Х", "C", 0.5, 25, 110, 128, 2097152, 127, false},		
			{"255.255.255.192", "255.255.255.0", "192.0.0.0", "223.255.255.255", "0.0.0.63", "С.С.С.Х", "C", 0.25, 26, 110, 64, 2097152, 	63, false},		
			{"255.255.255.224", "255.255.255.0", "192.0.0.0", "223.255.255.255", "0.0.0.31", "С.С.С.Х", "C", 0.125, 27, 110, 32, 2097152, 31, false},		
			{"255.255.255.240", "255.255.255.0", "192.0.0.0", "223.255.255.255", "0.0.0.15", "С.С.С.Х", "C", 0.0625, 28, 110, 16, 2097152, 15, false},		
			{"255.255.255.248", "255.255.255.0", "192.0.0.0", "223.255.255.255", "0.0.0.7", "С.С.С.Х", "C", 0.03125, 29, 110, 8, 2097152, 7, false},		
			{"255.255.255.252", "255.255.255.0", "192.0.0.0", "223.255.255.255", "0.0.0.3", "С.С.С.Х", "C", 0.015625, 30, 110, 4, 2097152, 3, false},		
			{"255.255.255.254", "255.255.255.0", "192.0.0.0", "223.255.255.255", "0.0.0.1", "С.С.С.Х", "C", 0.0078125, 31, 110, 2, 2097152, 1, false},		
			{"255.255.255.255", "255.255.255.0", "192.0.0.0", "223.255.255.255", "0.0.0.0", "С.С.С.Х", "C", 0.00390625, 32, 110, 1, 2097152, 	0, false}	
		};
		 
		void setZerroToStrIp(string &str);
		 
		void rmZerroToStrIp(string &str);
		 
		const bool checkMask(IPdata ip, IPdata mask);
		 
		const string getLow1Ip6(const string ip);
		 
		const string getLow2Ip6(const string ip);
		 
		const string setLow1Ip6(const string ip);
		 
		const string setLow2Ip6(const string ip);
	public:
		 
		NTdata getMaskByNumber(u_int value);
		 
		NTdata getMaskByString(string value);
		 
		IPdata getDataIp(string ip);
		 
		const bool checkRange6(const string ip, const string bip, const string eip);
		 
		const bool checkMaskByString(string ip, string mask);
		 
		const bool checkMaskByNumber(string ip, u_int mask);
		 
		const bool checkIPByNetwork(const string ip, const string nwk);
		 
		const bool checkIPByNetwork6(const string ip, const string nwk);
		 
		const bool compareIP6(const string ip1, const string ip2);
		 
		IPdata imposeMask(IPdata ip, IPdata mask);
		 
		NKdata getNetwork(string str);
		 
		const string setLowIp(const string ip);
		 
		const string getLowIp(const string ip);
		 
		const string imposePrefix6(const string ip6, u_int prefix);
		 
		const string getLowIp6(const string ip);
		 
		const string setLowIp6(const string ip);
		 
		const string getIPByNetwork(const string nwk);
		 
		__uint64_t strIp6ToHex64(const string ip);
		 
		const u_int checkNetworkByIp(const string ip);
		 
		const u_int getPrefixByNetwork(const string nwk);
		 
		const int isLocal(const string ip);
		 
		const int isLocal6(const string ip);
};

#endif 