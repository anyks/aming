/* SERVER PROXY AMING */
/*
*  author:     Yuriy Lobarev
*  phone:      +7(910)983-95-90
*  telegram:   @forman
*  email:      info@anyks.com
*  date:       10/29/2017 16:59:43
*  copyright:  © 2017 anyks.com
*/
 
 

#ifndef _USERS_PROXY_AMING_
#define _USERS_PROXY_AMING_

#include <regex>
#include <string>
#include <random>
#include <vector>
#include <iostream>
#include <fstream>
#include <algorithm>
#include <unordered_map>
#include <grp.h>
#include <pwd.h>
#include <ctime>
#include <stdlib.h>
#include <unistd.h>
#include <sys/t/y/p/e/s/./h/>/
/#/i/n/c/l/u/d/e/ /</s/e/c/u/r/i/t/y
#include "ini/ini.h"
#include "nwk/nwk.h"
#include "system/system.h"
#include "general/general.h"
#include "ldap2/ldap.h"


using namespace std;

 
namespace AParams {
	 
	struct Gzip {
		bool vary;					
		int level;					
		long length;				
		size_t chunk;				
		string regex;				
		vector <string> vhttp;		
		vector <string> proxied;	
		vector <string> types;		
	};
	 
	struct IP {
		vector <string> ip;			
		vector <string> resolver;	
	};
	 
	struct Proxy {
		bool reverse;		
		bool transfer;		
		bool forward;		
		bool subnet;		
		bool pipelining;	
	};
	 
	struct Connects {
		size_t size;	
		u_int connect;	
	};
	 
	struct Timeouts {
		size_t read;	
		size_t write;	
		size_t upgrade;	
	};
	 
	struct BufferSize {
		long read;		
		long write;		
	};
	 
	struct Keepalive {
		int keepcnt;	
		int keepidle;	
		int keepintvl;	
	};
	 
	struct Group {
		gid_t gid;		
		string name;	
		string desc;	
		string pass;	
	};
	 
	struct User {
		uid_t uid;		
		string name;	
		string desc;	
		string pass;	
	};
	 
	struct AUser {
		bool auth = false;		
		vector <string> idnt;	
		vector <Group> groups;	
		User user;				
		IP ipv4;				
		IP ipv6;				
		Gzip gzip;				
		Proxy proxy;			
		Connects connects;		
		Timeouts timeouts;		
		BufferSize buffers;		
		Keepalive keepalive;	
	};
	 
	struct Client {
		const string ip;				
		const string mac;				
		const string sip;				
		const AUser * user = nullptr;	
	};
};

 
class AUsers {
	private:
		 
		struct DataGroup {
			gid_t id;						
			u_short options;				
			u_short type;					
			string name;					
			string pass;					
			string auth;					
			string desc;					
			vector <string> idnt;			
			AParams::IP ipv4;				
			AParams::IP ipv6;				
			AParams::Gzip gzip;				
			AParams::Proxy proxy;			
			AParams::Connects connects;		
			AParams::Timeouts timeouts;		
			AParams::BufferSize buffers;	
			AParams::Keepalive keepalive;	
			vector <uid_t> users;	
		};
		 
		struct DataUser {
			uid_t id;						
			u_short options;				
			u_short type;					
			string name;					
			string pass;					
			string auth;					
			string desc;					
			vector <string> idnt;			
			AParams::IP ipv4;				
			AParams::IP ipv6;				
			AParams::Gzip gzip;				
			AParams::Proxy proxy;			
			AParams::Connects connects;		
			AParams::Timeouts timeouts;		
			AParams::BufferSize buffers;	
			AParams::Keepalive keepalive;	
		};
		 
		static void getPasswordsFromFile(const string path, LogApp * log = nullptr, void * object = nullptr, const u_short flag = AMING_USER);
		 
		static const string getPasswordFromFile(const string path, LogApp * log = nullptr, const uid_t uid = -1, const string name = "");
		 
		class Groups {
			private:
				 
				struct Ldap {
					string dnGroup;
					string dnUser;
					string dnConfig;
					string scopeGroup;
					string scopeUser;
					string scopeConfig;
					string filterGroup;
					string filterUser;
					string filterConfig;
				};
				
				time_t maxUpdate = 0;
				
				time_t lastUpdate = 0;
				
				u_short typeSearch = 0;
				
				u_short typeConfigs = 0;
				
				int maxPamGroupsUser = 0;
				
				Ldap ldap;
				
				LogApp * log = nullptr;
				
				Config * config = nullptr;
				
				void * users = nullptr;
				
				unordered_map <gid_t, DataGroup> data;
				 
				void setProxyOptions(const u_short option, u_short &proxyOptions, const bool flag = false);
				 
				void setDataGroupFromLdap(DataGroup &group);
				 
				void setDataGroupFromFile(DataGroup &group, INI * ini = nullptr);
				 
				void setDataGroup(DataGroup &group, INI * ini = nullptr);
				 
				const DataGroup createDefaultData(const gid_t id, const string name);
				 
				const bool readGroupsFromLdap();
				 
				const bool readGroupsFromPam();
				 
				const bool readGroupsFromFile();
				 
				const bool update();
			public:
				 
				const vector <const DataGroup *> getAllGroups();
				 
				const DataGroup * getDataById(const gid_t gid);
				 
				const DataGroup * getDataByName(const string groupName);
				 
				const vector <gid_t> getGroupIdByUser(const uid_t uid);
				 
				const vector <gid_t> getGroupIdByUser(const string userName);
				 
				const vector <string> getGroupNameByUser(const uid_t uid);
				 
				const vector <string> getGroupNameByUser(const string userName);
				 
				const bool checkUser(const gid_t gid, const uid_t uid);
				 
				const bool checkUser(const gid_t gid, const string userName);
				 
				const bool checkUser(const string groupName, const uid_t uid);
				 
				const bool checkUser(const string groupName, const string userName);
				 
				const bool checkGroupById(const gid_t gid);
				 
				const bool checkGroupByName(const string groupName);
				 
				const uid_t getUidByUserName(const string userName);
				 
				const gid_t getIdByName(const string groupName);
				 
				const string getUserNameByUid(const uid_t uid);
				 
				const string getNameById(const gid_t gid);
				 
				const vector <string> getNameUsers(const gid_t gid);
				 
				const vector <string> getNameUsers(const string groupName);
				 
				const vector <uid_t> getIdAllUsers();
				 
				const vector <uid_t> getIdUsers(const gid_t gid);
				 
				const vector <uid_t> getIdUsers(const string groupName);
				 
				const bool addUser(const gid_t gid, const uid_t uid);
				 
				const bool addUser(const gid_t gid, const string userName);
				 
				const bool addUser(const string groupName, const uid_t uid);
				 
				const bool addUser(const string groupName, const string userName);
				 
				const bool addGroup(const gid_t gid, const string name);
				 
				void setPassword(const gid_t gid, const string password);
				 
				void setUsers(void * users = nullptr);
				 
				Groups(Config * config = nullptr, LogApp * log = nullptr);
		};
		 
		class Users {
			private:
				 
				struct Ldap {
					string dnUser;
					string dnConfig;
					string scopeUser;
					string scopeConfig;
					string filterUser;
					string filterConfig;
				};
				
				time_t maxUpdate = 0;
				
				time_t lastUpdate = 0;
				
				u_short typeSearch = 0;
				
				u_short typeConfigs = 0;
				
				Ldap ldap;
				
				LogApp * log = nullptr;
				
				Config * config = nullptr;
				
				void * groups = nullptr;
				
				unordered_map <uid_t, DataUser> data;
				 
				void setProxyOptions(const u_short option, u_short &proxyOptions, const bool flag = false);
				 
				void setDataUserFromLdap(DataUser &user);
				 
				void setDataUserFromFile(DataUser &user, INI * ini = nullptr);
				 
				void setDataUser(DataUser &user, INI * ini = nullptr);
				 
				const DataUser createDefaultData(const uid_t id, const string name);
				 
				const bool readUsersFromLdap();
				 
				const bool readUsersFromPam();
				 
				const bool readUsersFromFile();
				 
				const bool update();
			public:
				 
				const vector <const DataUser *> getAllUsers();
				 
				const DataUser * getUserByConnect(const string ip = "", const string mac = "");
				 
				const DataUser * getDataById(const uid_t uid);
				 
				const DataUser * getDataByName(const string userName);
				 
				const bool checkUserById(const uid_t uid);
				 
				const bool checkUserByName(const string userName);
				 
				const uid_t getIdByName(const string userName);
				 
				const string getNameById(const uid_t uid);
				 
				const vector <uid_t> getIdAllUsers();
				 
				void setPassword(const uid_t uid, const string password);
				 
				void setGroups(void * groups = nullptr);
				 
				Users(Config * config = nullptr, LogApp * log = nullptr);
		};
		 
		class Auth {
			private:
				 
				struct Ldap {
					string keyUser;
					string keyGroup;
					string dnUser;
					string dnGroup;
					string scopeUser;
					string scopeGroup;
					string filterUser;
					string filterGroup;
				};
				
				Ldap ldap;
				
				LogApp * log = nullptr;
				
				Config * config = nullptr;
				
				void * groups = nullptr;
				
				void * users = nullptr;
				 
				static int pamconv(int num_msg = 0, const struct pam_message ** msg = nullptr, struct pam_response ** resp = nullptr, void * appdata_ptr = nullptr);
				 
				const bool checkLdap(const uid_t uid, const string password);
				 
				const bool checkPam(const uid_t uid, const string password);
				 
				const bool checkFile(const uid_t uid, const string password);
			public:
				 
				const bool check(const string username = "", const string password = "");
				 
				Auth(Config * config = nullptr, LogApp * log = nullptr, void * groups = nullptr, void * users = nullptr);
		};
		
		LogApp * log = nullptr;
		
		Config * config = nullptr;
		
		Groups * groups = nullptr;
		
		Users * users = nullptr;
		
		Auth * auth = nullptr;
		 
		const AParams::AUser getUser(const uid_t uid);
		 
		const AParams::AUser getUser(const string userName);
	public:
		 
		const vector <const DataGroup *> getAllGroups();
		 
		const vector <const DataUser *> getAllUsers();
		 
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
