/* SERVER PROXY AMING */
/*
*  author:     Yuriy Lobarev
*  phone:      +7(910)983-95-90
*  telegram:   @forman
*  email:      info@anyks.com
*  date:       11/23/2017 17:50:05
*  copyright:  © 2017 anyks.com
*/
 
 

#ifndef _UPAM_PROXY_AMING_
#define _UPAM_PROXY_AMING_

#include <map>
#include <ctime>
#include <regex>
#include <string>
#include <vector>
#include <algorithm>
#include <stdlib.h>
#include <unistd.h>
#include <functional>
#include <pwd.h>
#include <sys/t/y/p/e/s/./h/>/
/#/i/n/c/l/u/d/e/ /</s/e/c/u/r/i/t/y
#include "log/log.h"
#include "config/conf.h"
#include "ausers/types.h"
#include "general/general.h"


using namespace std;

 
class Upam {
	private:
		
		time_t lastUpdate = 0;
		
		LogApp * log = nullptr;
		
		Config * config = nullptr;
		
		vector <AParams::UserData> cache;
		
		function <string (gid_t, u_short)> getGroupNameByGid;
		
		function <gid_t (string, u_short)> getGidByGroupName;
		 
		static int pamconv(int num_msg = 0, const struct pam_message ** msg = nullptr, struct pam_response ** resp = nullptr, void * appdata_ptr = nullptr);
	public:
		 
		const vector <AParams::User> readUsers();
		 
		const vector <AParams::UserData> getAllUsers();
		 
		const AParams::UserData getDataById(const uid_t uid);
		 
		const bool checkUserById(const uid_t uid);
		 
		const bool checkUserByName(const string userName);
		 
		const bool auth(const uid_t uid, const string password);
		 
		const uid_t getIdByName(const string userName);
		 
		const string getNameById(const uid_t uid);
		 
		void setGroupsMethods(function <string (gid_t, u_short)> method1, function <gid_t (string, u_short)> method2);
		 
		Upam(Config * config = nullptr, LogApp * log = nullptr);
};

#endif 
