/* SERVER PROXY AMING */
/*
*  author:     Yuriy Lobarev
*  phone:      +7(910)983-95-90
*  telegram:   @forman
*  email:      info@anyks.com
*  date:       11/23/2017 17:50:05
*  copyright:  © 2017 anyks.com
*/
 
 

#ifndef _HEADERS_AMING_
#define _HEADERS_AMING_

#include <map>
#include <regex>
#include <deque>
#include <string>
#include <vector>
#include <algorithm>
#include <unordered_map>
#include <stdlib.h>
#include "headers/files.h"
#include "headers/ldap.h"
#include "nwk/nwk.h"
#include "http/http.h"
#include "ausers/ausers.h"


using namespace std;

 
class Headers {
	private:
		 
		struct IsNot {
			bool inv;	
			string str;	
		};
		 
		struct Node {
			u_short type;	
			string data;	
		};
		 
		struct Rules {
			string query;				
			string userAgent;			
			vector <uid_t> users;		
			vector <uid_t> bsers;		
			vector <gid_t> groups;		
			vector <gid_t> broups;		
			vector <Node> clients;		
			vector <Node> servers;		
			vector <string> paths;		
			vector <string> headers;	
		};
		 
		struct Params {
			string query;				
			string userAgent;			
			vector <string> actions;	
			vector <string> traffic;	
			vector <string> clients;	
			vector <string> servers;	
			vector <string> methods;	
			vector <string> paths;		
			vector <string> users;		
			vector <string> groups;		
			vector <string> headers;	
		};
		 
		struct Client {
			bool action;
			bool traffic;
			uid_t user;
			vector <gid_t> groups;
			string ip;
			string mac;
			string sip;
			string domain;
			string agent;
			string path;
			string query;
			string method;
		};
		 
		 
		map <
			bool,
			map <
				bool,
				unordered_map <
					string,
					Rules
				>
			>
		> rules;
		
		deque <string> names;
		
		LogApp * log = nullptr;
		Config * config = nullptr;
		AUsers * ausers = nullptr;
		
		time_t lastUpdate = 0;
		
		u_short options = AMING_NULL;
		 
		const IsNot isNot(const string str);
		 
		void modifyHeaders(const bool action, const vector <string> headers, HttpData &http);
		 
		void modifyHeaders(const bool action, const vector <string> headers, string &data, HttpData &http);
		 
		void addParams(const map <bool, map <bool, unordered_map <string, Rules>>> * params = nullptr);
		 
		void createRulesList(void * ctx = nullptr);
		 
		void read(const u_short type = AMING_NULL);
		 
		const vector <string> findHeaders(Client * request, const string method, const Rules * rules = nullptr);
		 
		const vector <string> get(Client * request = nullptr);
	public:
		 
		void clear();
		 
		void rm(const bool action, const bool traffic);
		 
		void add(const bool action = false, const bool traffic = false, const string method = "*", void * ctx = nullptr);
		 
		void addName(const string name);
		 
		void setOptions(const u_short options = AMING_NULL);
		 
		void modify(AParams::Client client, HttpData &http);
		 
		void modify(AParams::Client client, string &data);
		 
		Headers(Config * config = nullptr, LogApp * log = nullptr, AUsers * ausers = nullptr);
};

#endif 
