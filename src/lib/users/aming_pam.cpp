/* SERVER PROXY AMING */
/*
*  author:     Yuriy Lobarev
*  phone:      +7(910)983-95-90
*  telegram:   @forman
*  email:      info@anyks.com
*  date:       11/08/2017 16:52:48
*  copyright:  © 2017 anyks.com
*/
 
 

#include "users/pam.h"


using namespace std;

 
const vector <AParams::User> Upam::readUsers(){
	
	vector <AParams::User> result = {{0, "", "", ""}};
	
	const time_t curUpdate = time(nullptr);
	
	if((this->lastUpdate + this->config->auth.update) < curUpdate){
		
		smatch match;
		
		this->cache.clear();
		
		this->lastUpdate = curUpdate;
		
		struct passwd * pw = nullptr;
		
		regex e("\\/(?:sh|bash)$", regex::ECMAScript | regex::icase);
		
		while((pw = getpwent()) != nullptr){
			
			if(pw->pw_uid > 0){
				
				string shell = pw->pw_shell;
				
				regex_search(shell, match, e);
				
				if(!match.empty()){
					
					result.push_back({
						uid_t(pw->pw_uid),
						string(pw->pw_name),
						string(pw->pw_name),
						string(pw->pw_passwd)
					});
					
					this->cache.push_back({
						uid_t(pw->pw_uid),
						u_short(AUSERS_TYPE_PAM),
						string(pw->pw_name),
						string(pw->pw_name),
						string(pw->pw_passwd)
					});
				}
			}
		}
	
	} else if(!this->cache.empty()) {
		
		for(auto it = this->cache.cbegin(); it != this->cache.cend(); ++it){
			
			result.push_back({it->uid, it->name, it->desc, it->pass});
		}
	}
	
	return result;
}
 
const vector <AParams::UserData> Upam::getAllUsers(){
	
	vector <AParams::UserData> result;
	
	const time_t curUpdate = time(nullptr);
	
	if((this->lastUpdate + this->config->auth.update) < curUpdate){
		
		smatch match;
		
		this->cache.clear();
		
		this->lastUpdate = curUpdate;
		
		struct passwd * pw = nullptr;
		
		regex e("\\/(?:sh|bash)$", regex::ECMAScript | regex::icase);
		
		while((pw = getpwent()) != nullptr){
			
			if(pw->pw_uid > 0){
				
				string shell = pw->pw_shell;
				
				regex_search(shell, match, e);
				
				if(!match.empty()){
					
					result.push_back({
						uid_t(pw->pw_uid),
						u_short(AUSERS_TYPE_PAM),
						string(pw->pw_name),
						string(pw->pw_name),
						string(pw->pw_passwd)
					});
				}
			}
		}
		
		this->cache.assign(result.cbegin(), result.cend());
	
	} else if(!this->cache.empty()) result.assign(this->cache.cbegin(), this->cache.cend());
	
	return result;
}
 
const AParams::UserData Upam::getDataById(const uid_t uid){
	
	AParams::UserData result;
	
	if(uid > 0){
		
		auto users = getAllUsers();
		
		for(auto it = users.cbegin(); it != users.cend(); ++it){
			
			if(it->uid == uid){
				
				result = * it;
				
				break;
			}
		}
	}
	
	return result;
}
 
const bool Upam::checkUserById(const uid_t uid){
	
	return !(getNameById(uid)).empty();
}
 
const bool Upam::checkUserByName(const string userName){
	
	return (getIdByName(userName) > 0 ? true : false);
}
 
const uid_t Upam::getIdByName(const string userName){
	
	uid_t result = 0;
	
	if(!userName.empty()){
		
		auto users = getAllUsers();
		
		for(auto it = users.cbegin(); it != users.cend(); ++it){
			
			if(userName.compare(it->name) == 0){
				
				result = it->uid;
				
				break;
			}
		}
	}
	
	return result;
}
 
const string Upam::getNameById(const uid_t uid){
	
	string result;
	
	if(uid > 0){
		
		auto users = getAllUsers();
		
		for(auto it = users.cbegin(); it != users.cend(); ++it){
			
			if(uid == it->uid){
				
				result = it->name;
				
				break;
			}
		}
	}
	
	return result;
}
 
void Upam::setGroupsMethods(function <string (gid_t, u_short)> method1, function <gid_t (string, u_short)> method2){
	
	this->getGroupNameByGid = method1;
	
	this->getGidByGroupName = method2;
}
 
Upam::Upam(Config * config, LogApp * log){
	
	if(config != nullptr){
		
		this->log = log;
		
		this->config = config;
	}
}
