/* SERVER PROXY AMING */
/*
*  author:     Yuriy Lobarev
*  phone:      +7(910)983-95-90
*  telegram:   @forman
*  email:      info@anyks.com
*  date:       11/23/2017 17:50:05
*  copyright:  © 2017 anyks.com
*/
 
 

#ifndef _GPAM_PROXY_AMING_
#define _GPAM_PROXY_AMING_

#include <map>
#include <ctime>
#include <regex>
#include <string>
#include <vector>
#include <algorithm>
#include <stdlib.h>
#include <unistd.h>
#include <functional>
#include <grp.h>
#include <pwd.h>
#include <sys/t/y/p/e/s/./h/>/
/#/i/n/c/l/u/d/e/ /"/l/o/g
#include "config/conf.h"
#include "ausers/types.h"
#include "general/general.h"


using namespace std;

 
class Gpam {
	private:
		
		time_t lastUpdate = 0;
		
		LogApp * log = nullptr;
		
		Config * config = nullptr;
		
		vector <AParams::GroupData> cache;
		
		function <string (uid_t, u_short)> getUserNameByUid;
		
		function <uid_t (string, u_short)> getUidByUserName;
	public:
		 
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
		 
		Gpam(Config * config = nullptr, LogApp * log = nullptr);
};

#endif 
