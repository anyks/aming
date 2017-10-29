/* SERVER PROXY AMING */
/*
*  author:     Yuriy Lobarev
*  phone:      +7(910)983-95-90
*  telegram:   @forman
*  email:      info@anyks.com
*  date:       10/29/2017 17:06:01
*  copyright:  © 2017 anyks.com
*/
 
 

#ifndef _HEADERS_AMING_
#define _HEADERS_AMING_

#include <regex>
#include <deque>
#include <string>
#include <vector>
#include <fstream>
#include <iostream>
#include <unordered_map>
#include <stdlib.h>
#include "config/conf.h"
#include "nwk/nwk.h"
#include "log/log.h"
#include "http/http.h"
#include "general/general.h"


using namespace std;

 
class Headers {
	private:
		 
		struct IsNot {
			bool inv;	
			string str;	
		};
		 
		struct Params {
			u_short utype;				
			u_short stype;				
			string action;				
			string route;				
			string server;				
			string path;				
			string regex;				
			vector <string> methods;	
			vector <string> headers;	
		};
		
		deque <string> names;
		
		LogApp * log = nullptr;
		
		Config * config = nullptr;
		
		u_short options = 0x00;
		
		unordered_map <string, vector <Params>> rules;
		 
		vector <Params> get(const string client, bool addGeneral = true);
		 
		void add(const string client, Params params);
		 
		void rm(const string client);
		 
		void read();
		 
		void modifyHeaders(const string server, vector <Params> rules, HttpData &http);
		 
		void modifyHeaders(const string server, vector <Params> rules, string &data, HttpData &http);
		 
		const IsNot isNot(const string str);
		 
		const u_short checkTypeId(const string str);
		 
		const bool isIpV4(const string ip);
		 
		const bool isIpV6(const string ip);
		 
		const bool isIp(const string ip);
		 
		const bool isMac(const string mac);
		 
		const bool isDomain(const string domain);
		 
		const bool isLogin(const string login);
		 
		const bool checkAvailable(const string name);
		 
		const string getName();
	public:
		 
		void clear();
		 
		void addName(const string name);
		 
		void setOptions(const u_short options = 0x00);
		 
		void modify(const string ip, const string mac, const string server, HttpData &http);
		 
		void modify(const string ip, const string mac, const string server, string &data);
		 
		Headers(Config * config = nullptr, LogApp * log = nullptr);
};

#endif 