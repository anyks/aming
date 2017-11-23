/* SERVER PROXY AMING */
/*
*  author:     Yuriy Lobarev
*  phone:      +7(910)983-95-90
*  telegram:   @forman
*  email:      info@anyks.com
*  date:       11/23/2017 17:50:05
*  copyright:  © 2017 anyks.com
*/
 
 

#ifndef _PROXY_LDAP_AMING_
#define _PROXY_LDAP_AMING_

#include <string>
#include <vector>
#include <random>
#include <ctime>
#include <iostream>
#include <unordered_map>
#include <ldap.h>
#include <stdio.h>
#include <stdlib.h>
#include "log/log.h"
#include "config/conf.h"
#include "general/general.h"


using namespace std;

 
class ALDAP {
	private:
		 
        struct Data {
			string dn;
			string key;
			unordered_map <string, vector <string>> vals;
		};
        
		LogApp * log = nullptr;
		
		Config * config = nullptr;
		
		u_int version = LDAP_VERSION2;
		
		vector <string> servers;
		 
		const u_int getScope(const string scope);
		 
		const bool auth(LDAP * ld, const string dn, const string password);
		 
		const string getServer();
	public:
		 
		const bool checkAuth(const string dn, const string password, const string scope = "", const string filter = "(objectClass=top)");
		 
		const vector <Data> data(const string dn, const string key, const string scope = "", const string filter = "(objectClass=top)");
		 
		void setServer(const string server);
		 
		ALDAP(Config * config = nullptr, LogApp * log = nullptr);
};

#endif 
