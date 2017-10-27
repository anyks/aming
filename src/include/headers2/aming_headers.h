/* SERVER PROXY AMING */
/*
*  author:     Yuriy Lobarev
*  phone:      +7(910)983-95-90
*  telegram:   @forman
*  email:      info@anyks.com
*  date:       10/27/2017 18:42:35
*  copyright:  © 2017 anyks.com
*/
 
 

#ifndef _HEADERS2_AMING_
#define _HEADERS2_AMING_

#include <map>
#include <regex>
#include <deque>
#include <string>
#include <vector>
#include <fstream>
#include <iostream>
#include <algorithm>
#include <unordered_map>
#include <stdlib.h>
#include "config/conf.h"
#include "nwk/nwk.h"
#include "log/log.h"
#include "http/http.h"
#include "ldap2/ldap.h"
#include "ausers/ausers.h"
#include "general/general.h"


using namespace std;

 
class Headers2 {
	private:
		 
		struct IsNot {
			bool inv;	
			string str;	
		};
		 
		struct Ldap {
			string dn;
			string scope;
			string filter;
		};
		 
		struct Node {
			u_short type;	
			string data;	
		};
		 
		struct Rules {
			string query;				
			string userAgent;			
			vector <uid_t> users;		
			vector <gid_t> groups;		
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
		
		Ldap ldap;
		
		deque <string> names;
		
		LogApp * log = nullptr;
		Config * config = nullptr;
		AUsers * ausers = nullptr;
		
		u_short options = 0x00;
		
		time_t maxUpdate = 0;
		
		time_t lastUpdate = 0;
		
		u_short typeSearch = 0;
		 
		const IsNot isNot(const string str);
		 
		void modifyHeaders(const bool action, const vector <string> headers, HttpData &http);
		 
		void modifyHeaders(const bool action, const vector <string> headers, string &data, HttpData &http);
		 
		void addParams(const map <bool, map <bool, unordered_map <string, Rules>>> * params = nullptr);
		 
		void createRulesList(const Params params);
		 
		void readFromLDAP();
		 
		void readFromFile();
	public:
		 
		const bool checkAvailable(const string name);
		 
		const string getName();
		 
		const vector <string> findHeaders(Client * request, const string method, const Rules * rules = nullptr);
		 
		const vector <string> get(Client * request = nullptr);
		 
		void add(const bool action = false, const bool traffic = false, const string method = "*", void * ctx = nullptr);
		 
		void rm(const gid_t gid, const uid_t uid);
		 
		void read();
		 
		void clear();
		 
		void addName(const string name);
		 
		void setOptions(const u_short options = 0x00);
		 
		void modify(AParams::Client client, HttpData &http);
		 
		void modify(AParams::Client client, string &data);
		 
		Headers2(Config * config = nullptr, LogApp * log = nullptr, AUsers * ausers = nullptr);
};

#endif 