/* SERVER PROXY AMING */
/*
*  author:     Yuriy Lobarev
*  phone:      +7(910)983-95-90
*  telegram:   @forman
*  email:      info@anyks.com
*  date:       11/23/2017 17:50:05
*  copyright:  © 2017 anyks.com
*/
 
 

#ifndef _USERS_PROXY_AMING_
#define _USERS_PROXY_AMING_

#include <map>
#include <regex>
#include <string>
#include <vector>
#include <iostream>
#include <stdlib.h>
#include <unistd.h>
#include <sys/t/y/p/e/s/./h/>/
/#/i/n/c/l/u/d/e/ /"/n/w/k
#include "ausers/types.h"
#include "general/general.h"
#include "groups/pam.h"
#include "groups/files.h"
#include "groups/ldap.h"
#include "users/pam.h"
#include "users/files.h"
#include "users/ldap.h"


using namespace std;

 
class AUsers {
	private:
		 
		static void getPasswordsFromFile(const string path, LogApp * log = nullptr, void * object = nullptr, const u_short flag = AMING_USER);
		 
		static const string getPasswordFromFile(const string path, LogApp * log = nullptr, const uid_t uid = 0, const string name = "");
		 
		class Groups {
			private:
				
				time_t lastUpdate = 0;
				
				LogApp * log = nullptr;
				
				Config * config = nullptr;
				
				void * users = nullptr;
				
				Gpam * gpam = nullptr;
				Gldap * gldap = nullptr;
				Gfiles * gfiles = nullptr;
				
				map <pair <gid_t, u_short>, AParams::Params> params;
				 
				void setGroupParams(const gid_t gid, const string name, const u_short type = AMING_NULL);
				 
				const uid_t getUidByUserName(const string userName, const u_short type = AMING_NULL);
				 
				const string getUserNameByUid(const uid_t uid, const u_short type = AMING_NULL);
				 
				const bool update(const u_short type = AMING_NULL);
			public:
				 
				const vector <AParams::GroupData> getAllGroups(const u_short type = AMING_NULL);
				 
				const AParams::Params * getParamsById(const gid_t gid, const u_short type = AMING_NULL);
				 
				const AParams::Params * getParamsByName(const string groupName, const u_short type = AMING_NULL);
				 
				const AParams::GroupData getDataById(const gid_t gid, const u_short type = AMING_NULL);
				 
				const AParams::GroupData getDataByName(const string groupName, const u_short type = AMING_NULL);
				 
				const vector <gid_t> getGroupIdByUser(const uid_t uid, const u_short type = AMING_NULL);
				 
				const vector <gid_t> getGroupIdByUser(const string userName, const u_short type = AMING_NULL);
				 
				const vector <string> getGroupNameByUser(const uid_t uid, const u_short type = AMING_NULL);
				 
				const vector <string> getGroupNameByUser(const string userName, const u_short type = AMING_NULL);
				 
				const bool checkUser(const gid_t gid, const uid_t uid, const u_short type = AMING_NULL);
				 
				const bool checkUser(const gid_t gid, const string userName, const u_short type = AMING_NULL);
				 
				const bool checkUser(const string groupName, const uid_t uid, const u_short type = AMING_NULL);
				 
				const bool checkUser(const string groupName, const string userName, const u_short type = AMING_NULL);
				 
				const bool checkGroupById(const gid_t gid, const u_short type = AMING_NULL);
				 
				const bool checkGroupByName(const string groupName, const u_short type = AMING_NULL);
				 
				const gid_t getIdByName(const string groupName, const u_short type = AMING_NULL);
				 
				const string getNameById(const gid_t gid, const u_short type = AMING_NULL);
				 
				const vector <string> getNameUsers(const gid_t gid, const u_short type = AMING_NULL);
				 
				const vector <string> getNameUsers(const string groupName, const u_short type = AMING_NULL);
				 
				const vector <uid_t> getIdAllUsers(const u_short type = AMING_NULL);
				 
				const vector <uid_t> getIdUsers(const gid_t gid, const u_short type = AMING_NULL);
				 
				const vector <uid_t> getIdUsers(const string groupName, const u_short type = AMING_NULL);
				 
				void setUsers(void * users = nullptr);
				 
				void run();
				 
				Groups(Config * config = nullptr, LogApp * log = nullptr);
				 
				~Groups();
		};
		 
		class Users {
			private:
				
				time_t lastUpdate = 0;
				
				LogApp * log = nullptr;
				
				Config * config = nullptr;
				
				void * groups = nullptr;
				
				Upam * upam = nullptr;
				Uldap * uldap = nullptr;
				Ufiles * ufiles = nullptr;
				
				map <pair <uid_t, u_short>, AParams::Params> params;
				 
				void setUserParams(const uid_t uid, const string name, const u_short type = AMING_NULL);
				 
				const gid_t getGidByGroupName(const string groupName, const u_short type = AMING_NULL);
				 
				const string getGroupNameByGid(const uid_t gid, const u_short type = AMING_NULL);
				 
				const bool update(const u_short type = AMING_NULL);
			public:
				 
				const vector <AParams::UserData> getAllUsers(const u_short type = AMING_NULL);
				 
				const AParams::Params * getParamsById(const uid_t uid, const u_short type = AMING_NULL);
				 
				const AParams::Params * getParamsByName(const string userName, const u_short type = AMING_NULL);
				 
				const AParams::Params * getDataByConnect(const string ip, const string mac, const u_short type = AMING_NULL);
				 
				const AParams::UserData getDataById(const uid_t uid, const u_short type = AMING_NULL);
				 
				const AParams::UserData getDataByName(const string userName, const u_short type = AMING_NULL);
				 
				const bool checkUserById(const uid_t uid, const u_short type = AMING_NULL);
				 
				const bool checkUserByName(const string userName, const u_short type = AMING_NULL);
				 
				const bool auth(const string username, const string password, const u_short type = AMING_NULL);
				 
				const uid_t getIdByName(const string userName, const u_short type = AMING_NULL);
				 
				const string getNameById(const uid_t uid, const u_short type = AMING_NULL);
				 
				const vector <uid_t> getIdAllUsers(const u_short type = AMING_NULL);
				 
				void setGroups(void * groups = nullptr);
				 
				void run();
				 
				Users(Config * config = nullptr, LogApp * log = nullptr);
				 
				~Users();
		};
		
		LogApp * log = nullptr;
		
		Config * config = nullptr;
		
		Groups * groups = nullptr;
		
		Users * users = nullptr;
		 
		const AParams::AUser getUser(const uid_t uid);
		 
		const AParams::AUser getUser(const string userName);
	public:
		 
		const vector <AParams::GroupData> getAllGroups(const u_short type = AMING_NULL);
		 
		const vector <AParams::UserData> getAllUsers(const u_short type = AMING_NULL);
		 
		const vector <uid_t> getIdUsersInGroup(const gid_t gid);
		 
		const vector <uid_t> getIdUsersInGroup(const string groupName);
		 
		const vector <string> getNameUsersInGroup(const gid_t gid);
		 
		const vector <string> getNameUsersInGroup(const string groupName);
		 
		const string getGroupNameByGid(const gid_t gid);
		 
		const string getUserNameByUid(const uid_t uid);
		 
		const gid_t getGidByName(const string groupName);
		 
		const uid_t getUidByName(const string userName);
		 
		const u_short getOptionsByUid(const uid_t uid);
		 
		const u_short getOptionsByUserName(const string userName);
		 
		const bool checkUserInGroup(const gid_t gid, const uid_t uid);
		 
		const bool checkUserInGroup(const gid_t gid, const string userName);
		 
		const bool checkUserInGroup(const string groupName, const uid_t uid);
		 
		const bool checkUserInGroup(const string groupName, const string userName);
		 
		const bool checkGroupById(const gid_t gid);
		 
		const bool checkUserById(const uid_t uid);
		 
		const AParams::AUser searchUser(const string ip = "", const string mac = "");
		 
		const AParams::AUser authenticate(const string login, const string pass);
		 
		AUsers(Config * config = nullptr, LogApp * log = nullptr);
		 
		~AUsers();
};

#endif 
