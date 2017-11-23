/* SERVER PROXY AMING */
/*
*  author:     Yuriy Lobarev
*  phone:      +7(910)983-95-90
*  telegram:   @forman
*  email:      info@anyks.com
*  date:       11/23/2017 17:50:05
*  copyright:  © 2017 anyks.com
*/
 
 

#ifndef _GFILES_PROXY_AMING_
#define _GFILES_PROXY_AMING_

#include <map>
#include <ctime>
#include <string>
#include <vector>
#include <stdlib.h>
#include <unistd.h>
#include <functional>
#include <sys/t/y/p/e/s/./h/>/
/#/i/n/c/l/u/d/e/ /"/i/n/i
#include "log/log.h"
#include "config/conf.h"
#include "ausers/types.h"
#include "general/general.h"


using namespace std;

 
class Gfiles {
	private:
		
		time_t lastUpdate = 0;
		
		LogApp * log = nullptr;
		
		Config * config = nullptr;
		
		map <gid_t, AParams::GroupData> groups;
		
		function <string (uid_t, u_short)> getUserNameByUid;
		
		function <uid_t (string, u_short)> getUidByUserName;
		
		function <void (const string, LogApp * log, void * object, const u_short)> getPasswords;
		
		function <const string (const string, LogApp * log, const uid_t, const string)> getPassword;
		 
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
		 
		void setPassword(const gid_t gid, const string password);
		 
		void setUsersMethods(function <string (uid_t, u_short)> method1, function <uid_t (string, u_short)> method2);
		 
		void setPasswordsMethod(function <void (const string, LogApp * log, void * object, const u_short)> method);
		 
		void setPasswordMethod(function <const string (const string, LogApp * log, const uid_t, const string)> method);
		 
		Gfiles(Config * config = nullptr, LogApp * log = nullptr);
};

#endif 
