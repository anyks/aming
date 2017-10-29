/* SERVER PROXY AMING */
/*
*  author:     Yuriy Lobarev
*  phone:      +7(910)983-95-90
*  telegram:   @forman
*  email:      info@anyks.com
*  date:       10/29/2017 16:59:43
*  copyright:  © 2017 anyks.com
*/
 
 

#ifndef _AUTH_LDAP_AMING_
#define _AUTH_LDAP_AMING_

#include <string>
#include <vector>
#include <iostream>
#include <ldap.h>
#include <stdio.h>
#include <stdlib.h>
#include "system/system.h"
#include "general/general.h"


using namespace std;

 
class AuthLDAP {
	private:
		
		LogApp * log = nullptr;
		
		Config * config = nullptr;
		
		bool enabled = false;
		
		u_int version = LDAP_VERSION2;
		
		u_int scope = LDAP_SCOPE_SUBTREE;
		
		string server;
		
		string filter;
		
		string userdn;
		 
		const bool authLDAP(LDAP * ld, const string dn, const string password);
	public:
		 
		const bool checkUser(const string user, const string password);
		 
		AuthLDAP(Config * config = nullptr, LogApp * log = nullptr);
};

#endif 
