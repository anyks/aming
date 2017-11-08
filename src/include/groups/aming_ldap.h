/* SERVER PROXY AMING */
/*
*  author:     Yuriy Lobarev
*  phone:      +7(910)983-95-90
*  telegram:   @forman
*  email:      info@anyks.com
*  date:       11/08/2017 16:52:48
*  copyright:  © 2017 anyks.com
*/
 
 

#ifndef _GLDAP_PROXY_AMING_
#define _GLDAP_PROXY_AMING_

#include <map>
#include <string>
#include <vector>
#include <stdlib.h>
#include <unistd.h>
#include <functional>
#include <sys/t/y/p/e/s/./h/>/
/#/i/n/c/l/u/d/e/ /"/a/u/s/e/r/s
#include "system/system.h"
#include "general/general.h"
#include "ldap/ldap.h"


using namespace std;

 
class Gldap {
	private:
		
		LogApp * log = nullptr;
		
		Config * config = nullptr;
		
		function <string (uid_t, u_short)> getUserNameByUid;
		
		function <uid_t (string, u_short)> getUidByUserName;
		 
		const AParams::Params createDefaultParams(const gid_t gid);
	public:
		 
		const AParams::Params setParams(const gid_t gid, const string name = "");
		 
		const vector <AParams::Group> readGroups();
		 
		const vector <AParams::GroupData> getAllGroups();
		 
		const AParams::GroupData getDataById(const gid_t gid);
		 
		const vector <gid_t> getGroupIdByUser(const uid_t uid);
		 
		const vector <string> getGroupNameByUser(const uid_t uid);
		 
		const bool checkUser(const gid_t gid, const uid_t uid);
		 
		const bool checkUser(const string groupName, const uid_t uid);
		 
		const bool checkGroupById(const gid_t gid);
		 
		const bool checkGroupByName(const string groupName);
		 
		const gid_t getIdByName(const string groupName);
		 
		const string getNameById(const gid_t gid);
		 
		const vector <string> getNameUsers(const gid_t gid);
		 
		const vector <uid_t> getIdUsers(const gid_t gid);
		 
		void setUsersMethods(function <string (uid_t, u_short)> method1, function <uid_t (string, u_short)> method2);
		 
		Gldap(Config * config = nullptr, LogApp * log = nullptr);
};

#endif 
