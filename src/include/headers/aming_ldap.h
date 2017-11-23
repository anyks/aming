/* SERVER PROXY AMING */
/*
*  author:     Yuriy Lobarev
*  phone:      +7(910)983-95-90
*  telegram:   @forman
*  email:      info@anyks.com
*  date:       11/23/2017 17:50:05
*  copyright:  © 2017 anyks.com
*/
 
 

#ifndef _HEADERS_LDAP_AMING_
#define _HEADERS_LDAP_AMING_

#include <regex>
#include <deque>
#include <string>
#include <vector>
#include <stdlib.h>
#include "log/log.h"
#include "ldap/ldap.h"
#include "config/conf.h"
#include "general/general.h"


using namespace std;

 
class HLdap {
	private:
		 
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
		
		LogApp * log = nullptr;
		Config * config = nullptr;
		
		deque <string> * names = nullptr;
		 
		const string getName();
	public:
		 
		const vector <Params> getParams();
		 
		HLdap(Config * config = nullptr, LogApp * log = nullptr, deque <string> * names = nullptr);
};

#endif 
