/* SERVER PROXY AMING */
/*
*  author:     Yuriy Lobarev
*  phone:      +7(910)983-95-90
*  telegram:   @forman
*  email:      info@anyks.com
*  date:       11/23/2017 17:50:05
*  copyright:  © 2017 anyks.com
*/
 
 

#ifndef _ULDAP_PROXY_AMING_
#define _ULDAP_PROXY_AMING_

#include <map>
#include <string>
#include <vector>
#include <stdlib.h>
#include <unistd.h>
#include <functional>
#include <sys/t/y/p/e/s/./h/>/
/#/i/n/c/l/u/d/e/ /"/l/o/g
#include "config/conf.h"
#include "ausers/types.h"
#include "general/general.h"
#include "ldap/ldap.h"


using namespace std;

 
class Uldap {
	private:
		
		LogApp * log = nullptr;
		
		Config * config = nullptr;
		
		function <string (gid_t, u_short)> getGroupNameByGid;
		
		function <gid_t (string, u_short)> getGidByGroupName;
		
		function <const AParams::Params * (const gid_t gid, const u_short type)> getParamsByGid;
		
		function <const vector <gid_t> (const uid_t uid, const u_short type)> getGroupIdByUser;
		
		function <const AParams::GroupData (const gid_t gid, const u_short type)> getDataByGid;
		 
		const AParams::Params createDefaultParams(const uid_t uid, const u_short type = AMING_NULL);
	public:
		 
		const AParams::Params setParams(const uid_t uid, const string name = "");
		 
		const vector <AParams::User> readUsers();
		 
		const vector <AParams::UserData> getAllUsers();
		 
		const AParams::UserData getDataById(const uid_t uid);
		 
		const bool checkUserById(const uid_t uid);
		 
		const bool checkUserByName(const string userName);
		 
		const bool auth(const uid_t uid, const string password);
		 
		const uid_t getIdByName(const string userName);
		 
		const string getNameById(const uid_t uid);
		 
		void setGroupsMethods(function <string (gid_t, u_short)> method1, function <gid_t (string, u_short)> method2);
		 
		void setParamsMethod(function <const AParams::Params * (const gid_t gid, const u_short type)> method);
		 
		void setGidsMethod(function <const vector <gid_t> (const uid_t uid, const u_short type)> method);
		 
		void setGroupDataMethod(function <const AParams::GroupData (const gid_t gid, const u_short type)> method);
		 
		Uldap(Config * config = nullptr, LogApp * log = nullptr);
};

#endif 
