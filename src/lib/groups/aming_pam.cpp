/* SERVER PROXY AMING */
/*
*  author:     Yuriy Lobarev
*  phone:      +7(910)983-95-90
*  telegram:   @forman
*  email:      info@anyks.com
*  date:       11/23/2017 17:50:05
*  copyright:  © 2017 anyks.com
*/
 
 

#include "groups/pam.h"


using namespace std;

 
const vector <AParams::Group> Gpam::readGroups(){
	
	vector <AParams::Group> result = {{0, "", "", ""}};
	
	const time_t curUpdate = time(nullptr);
	
	if((this->lastUpdate + this->config->proxy.conftime) < curUpdate){
		
		smatch match;
		
		this->cache.clear();
		
		this->lastUpdate = curUpdate;
		
		map <gid_t, AParams::GroupData> groups;
		
		struct passwd * pw = nullptr;
		
		regex e("\\/(?:sh|bash)$", regex::ECMAScript | regex::icase);
		
		while((pw = getpwent()) != nullptr){
			
			if(pw->pw_uid > 0){
				
				string shell = pw->pw_shell;
				
				regex_search(shell, match, e);
				
				if(!match.empty()){
					
					int maxGroupsUser = this->config->auth.gmaxpam;
					
					int * userGroups = new int[(const int) maxGroupsUser];
					
					struct group * gr = nullptr;
					
					if(getgrouplist(pw->pw_name, pw->pw_gid, userGroups, &maxGroupsUser) == -1){
						
						this->log->write(LOG_ERROR, 0, "groups from user = %s [%s] not found", pw->pw_name, pw->pw_gecos);
					
					} else {
						
						for(int i = 0; i < maxGroupsUser; i++){
							
							gr = getgrgid(userGroups[i]);
							
							if(gr != nullptr){
								
								if(groups.count(gr->gr_gid) > 0){
									
									groups.find(gr->gr_gid)->second.users.push_back(pw->pw_uid);
								
								} else {
									
									result.push_back({
										gid_t(gr->gr_gid),
										string(gr->gr_name),
										string(gr->gr_name),
										string(gr->gr_passwd)
									});
									
									AParams::GroupData group = {
										gid_t(gr->gr_gid),
										u_short(AUSERS_TYPE_PAM),
										string(gr->gr_name),
										string(gr->gr_name),
										string(gr->gr_passwd),
										{uid_t(pw->pw_uid)}
									};
									
									groups.insert(pair <gid_t, AParams::GroupData> (gr->gr_gid, group));
								}
							
							} else this->log->write(LOG_ERROR, 0, "group [%i] from user = %s [%s] not found", userGroups[i], pw->pw_name, pw->pw_gecos);
						}
					}
					
					delete [] userGroups;
				}
			}
		}
		
		endpwent();
		
		for(auto it = groups.cbegin(); it != groups.cend(); ++it) this->cache.push_back(it->second);
	
	} else if(!this->cache.empty()) {
		
		for(auto it = this->cache.cbegin(); it != this->cache.cend(); ++it){
			
			result.push_back({it->gid, it->name, it->desc, it->pass});
		}
	}
	
	return result;
}
 
const vector <AParams::GroupData> Gpam::getAllGroups(){
	
	vector <AParams::GroupData> result;
	
	const time_t curUpdate = time(nullptr);
	
	if((this->lastUpdate + this->config->proxy.conftime) < curUpdate){
		
		this->cache.clear();
		
		this->lastUpdate = curUpdate;
		
		map <gid_t, AParams::GroupData> groups;
		
		struct passwd * pw = nullptr;
		
		while((pw = getpwent()) != nullptr){
			
			if(pw->pw_uid > 0){
				
				smatch match;
				
				regex e("\\/(?:sh|bash)$", regex::ECMAScript | regex::icase);
				
				string shell = pw->pw_shell;
				
				regex_search(shell, match, e);
				
				if(!match.empty()){
					
					int maxGroupsUser = this->config->auth.gmaxpam;
					
					int * userGroups = new int[(const int) maxGroupsUser];
					
					if(getgrouplist(pw->pw_name, pw->pw_gid, userGroups, &maxGroupsUser) == -1){
						
						this->log->write(LOG_ERROR, 0, "groups from user = %s [%s] not found", pw->pw_name, pw->pw_gecos);
					
					} else {
						
						for(int i = 0; i < maxGroupsUser; i++){
							
							struct group * gr = getgrgid(userGroups[i]);
							
							if(gr != nullptr){
								
								if(groups.count(gr->gr_gid) > 0){
									
									groups.find(gr->gr_gid)->second.users.push_back(pw->pw_uid);
								
								} else {
									
									AParams::GroupData group = {
										gid_t(gr->gr_gid),
										u_short(AUSERS_TYPE_PAM),
										string(gr->gr_name),
										string(gr->gr_name),
										string(gr->gr_passwd),
										{uid_t(pw->pw_uid)}
									};
									
									groups.insert(pair <gid_t, AParams::GroupData> (gr->gr_gid, group));
								}
							
							} else this->log->write(LOG_ERROR, 0, "group [%i] from user = %s [%s] not found", userGroups[i], pw->pw_name, pw->pw_gecos);
						}
					}
					
					delete [] userGroups;
				}
			}
		}
		
		endpwent();
		
		for(auto it = groups.cbegin(); it != groups.cend(); ++it) result.push_back(it->second);
		
		this->cache.assign(result.cbegin(), result.cend());
	
	} else if(!this->cache.empty()) result.assign(this->cache.cbegin(), this->cache.cend());
	
	return result;
}
 
const AParams::GroupData Gpam::getDataById(const gid_t gid){
	
	AParams::GroupData result;
	
	if(gid > 0){
		
		auto groups = getAllGroups();
		
		for(auto it = groups.cbegin(); it != groups.cend(); ++it){
			
			if(it->gid == gid){
				
				result = * it;
				
				break;
			}
		}
	}
	
	return result;
}
 
const vector <gid_t> Gpam::getGroupIdByUser(const uid_t uid){
	
	vector <gid_t> result;
	
	if(uid > 0){
		
		auto groups = getAllGroups();
		
		for(auto it = groups.cbegin(); it != groups.cend(); ++it){
			
			auto users = it->users;
			
			if(find(users.begin(), users.end(), uid) != users.end()){
				
				result.push_back(it->gid);
			}
		}
	}
	
	return result;
}
 
const vector <string> Gpam::getGroupNameByUser(const uid_t uid){
	
	vector <string> result;
	
	if(uid > 0){
		
		auto groups = getAllGroups();
		
		for(auto it = groups.cbegin(); it != groups.cend(); ++it){
			
			auto users = it->users;
			
			if(find(users.begin(), users.end(), uid) != users.end()){
				
				result.push_back(it->name);
			}
		}
	}
	
	return result;
}
 
const bool Gpam::checkUser(const gid_t gid, const uid_t uid){
	
	bool result = false;
	
	if((gid > 0) && (uid > 0)){
		
		auto users = getIdUsers(gid);
		
		if(find(users.begin(), users.end(), uid) != users.end()) result = true;
	}
	
	return result;
}
 
const bool Gpam::checkUser(const string groupName, const uid_t uid){
	
	bool result = false;
	
	if((uid > 0) && !groupName.empty()){
		
		const gid_t gid = getIdByName(groupName);
		
		if(gid > 0) result = checkUser(gid, uid);
	}
	
	return result;
}
 
const bool Gpam::checkGroupById(const gid_t gid){
	
	return !(getNameById(gid)).empty();
}
 
const bool Gpam::checkGroupByName(const string groupName){
	
	return (getIdByName(groupName) > 0 ? true : false);
}
 
const gid_t Gpam::getIdByName(const string groupName){
	
	gid_t result = 0;
	
	if(!groupName.empty()){
		
		auto groups = getAllGroups();
		
		for(auto it = groups.cbegin(); it != groups.cend(); ++it){
			
			if(groupName.compare(it->name) == 0){
				
				result = it->gid;
				
				break;
			}
		}
	}
	
	return result;
}
 
const string Gpam::getNameById(const gid_t gid){
	
	string result;
	
	if(gid > 0){
		
		auto groups = getAllGroups();
		
		for(auto it = groups.cbegin(); it != groups.cend(); ++it){
			
			if(gid == it->gid){
				
				result = it->name;
				
				break;
			}
		}
	}
	
	return result;
}
 
const vector <string> Gpam::getNameUsers(const gid_t gid){
	
	vector <string> result;
	
	if(gid > 0){
		
		auto users = getIdUsers(gid);
		
		for(auto it = users.cbegin(); it != users.cend(); ++it){
			
			const string userName = this->getUserNameByUid(* it, AUSERS_TYPE_PAM);
			
			if(!userName.empty()) result.push_back(userName);
		}
	}
	
	return result;
}
 
const vector <uid_t> Gpam::getIdUsers(const gid_t gid){
	
	vector <uid_t> result;
	
	auto groups = getAllGroups();
	
	for(auto it = groups.cbegin(); it != groups.cend(); ++it){
		
		if(gid == it->gid){
			
			result.assign(it->users.begin(), it->users.end());
			
			break;
		}
	}
	
	return result;
}
 
void Gpam::setUsersMethods(function <string (uid_t, u_short)> method1, function <uid_t (string, u_short)> method2){
	
	this->getUserNameByUid = method1;
	
	this->getUidByUserName = method2;
}
 
Gpam::Gpam(Config * config, LogApp * log){
	
	if(config != nullptr){
		
		this->log = log;
		
		this->config = config;
	}
}
