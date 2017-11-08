/* SERVER PROXY AMING */
/*
*  author:     Yuriy Lobarev
*  phone:      +7(910)983-95-90
*  telegram:   @forman
*  email:      info@anyks.com
*  date:       11/08/2017 16:52:48
*  copyright:  © 2017 anyks.com
*/
 
 

#include "ausers/ausers.h"


using namespace std;

 
void AUsers::getPasswordsFromFile(const string path, LogApp * log, void * object, const u_short flag){
	
	if(!path.empty() && (object != nullptr)
	&& ((flag == AMING_USER) || (flag == AMING_GROUP))){
		
		if(Anyks::getTypeAmingByString(path) == AMING_ADDRESS){
			
			if(Anyks::isFileExist(path.c_str())){
				
				ifstream file(path.c_str());
				
				if(file.is_open()){
					 
					auto readData = [&file](void * object, void * data, const u_short flag){
						
						string filedata;
						
						while(file.good()){
							
							getline(file, filedata);
							
							if(!filedata.empty()){
								
								smatch match;
								
								regex e("^([A-Za-z]+|\\d+)\\:((?:CL|MD5|SHA1|SHA256|SHA512)\\:.{3,128})$", regex::ECMAScript | regex::icase);
								
								regex_search(filedata, match, e);
								
								if(!match.empty()){
									
									const string subject = Anyks::toCase(match[1].str());
									
									switch(flag){
										
										case AMING_GROUP: {
											
											auto * groups = reinterpret_cast <const vector <const AParams::GroupData> *> (data);
											
											const gid_t gid = (Anyks::isNumber(subject) ? ::atoi(subject.c_str()) : 0);
											
											for(auto it = groups->cbegin(); it != groups->cend(); ++it){
												
												if(((* it).type == AUSERS_TYPE_FILE) && (((gid > 0)
												&& ((* it).gid == gid))
												|| (subject.compare(Anyks::toCase((* it).name)) == 0))) (reinterpret_cast <Gfiles *> (object))->setPassword(gid, match[2].str());
											}
										} break;
										
										case AMING_USER: {
											
											auto * users = reinterpret_cast <const vector <const AParams::UserData> *> (data);
											
											const uid_t uid = (Anyks::isNumber(subject) ? ::atoi(subject.c_str()) : 0);
											
											for(auto it = users->cbegin(); it != users->cend(); ++it){
												
												if(((* it).type == AUSERS_TYPE_FILE) && (((uid > 0)
												&& ((* it).uid == uid))
												|| (subject.compare(Anyks::toCase((* it).name)) == 0))) (reinterpret_cast <Ufiles *> (object))->setPassword(uid, match[2].str());
											}
										} break;
									}
								}
							}
						}
						
						return true;
					};
					
					switch(flag){
						
						case AMING_GROUP: {
							
							auto data = (reinterpret_cast <Gfiles *> (object))->getAllGroups();
							
							readData(object, &data, flag);
						} break;
						
						case AMING_USER: {
							
							auto data = (reinterpret_cast <Ufiles *> (object))->getAllUsers();
							
							readData(object, &data, flag);
						} break;
					}
					
					file.close();
				
				} else if(log != nullptr) log->write(LOG_ERROR, 0, "password file (%s) is cannot open", path.c_str());
			
			} else if(log != nullptr) log->write(LOG_ERROR, 0, "password file (%s) does not exist", path.c_str());
		}
	}
}
 
const string AUsers::getPasswordFromFile(const string path, LogApp * log, const uid_t uid, const string name){
	
	string result = path;
	
	if(((uid > 0) || !name.empty()) && !path.empty()){
		
		if(Anyks::getTypeAmingByString(path) == AMING_ADDRESS){
			
			if(Anyks::isFileExist(path.c_str())){
				
				string filedata;
				
				ifstream file(path.c_str());
				
				if(file.is_open()){
					
					while(file.good()){
						
						getline(file, filedata);
						
						if(!filedata.empty()){
							
							smatch match;
							
							regex e("^([A-Za-z]+|\\d+)\\:((?:CL|MD5|SHA1|SHA256|SHA512)\\:.{3,128})$", regex::ECMAScript | regex::icase);
							
							regex_search(filedata, match, e);
							
							if(!match.empty()){
								
								const string user = Anyks::toCase(match[1].str());
								
								if((Anyks::isNumber(user) && (uid_t(::atoi(user.c_str())) == uid))
								|| (Anyks::toCase(name).compare(user) == 0)){
									
									result = match[2].str();
									
									break;
								}
							}
						}
					}
					
					file.close();
				
				} else if(log != nullptr) log->write(LOG_ERROR, 0, "password file (%s) is cannot open", path.c_str());
			
			} else if(log != nullptr) log->write(LOG_ERROR, 0, "password file (%s) does not exist", path.c_str());
		}
	}
	
	return result;
}
 
const AParams::AUser AUsers::getUser(const uid_t uid){
	
	AParams::AUser result;
	
	if((uid > 0) && (this->users != nullptr) && (this->groups != nullptr)){
		
		auto user = this->users->getDataById(uid);
		
		if(user.uid > 0){
			
			auto gits = this->groups->getGroupIdByUser(uid, user.type);
			
			for(auto it = gits.cbegin(); it != gits.cend(); it++){
				
				auto group = this->groups->getDataById(* it, user.type);
				
				if(group.gid > 0){
					
					result.groups.push_back({group.gid, group.name, group.desc, group.pass});
				}
			}
			
			result.user = {user.uid, user.name, user.desc, user.pass};
			
			auto * params = this->users->getParamsById(user.uid, user.type);
			
			if(params != nullptr){
				
				result.idnt = params->idnt;
				result.ipv4 = params->ipv4;
				result.ipv6 = params->ipv6;
				result.gzip = params->gzip;
				result.proxy = params->proxy;
				result.connects = params->connects;
				result.timeouts = params->timeouts;
				result.buffers = params->buffers;
				result.keepalive = params->keepalive;
			}
		}
	}
	
	return result;
}
 
const AParams::AUser AUsers::getUser(const string userName){
	
	AParams::AUser result;
	
	if(!userName.empty()){
		
		const uid_t uid = (this->users != nullptr ? this->users->getIdByName(userName) : 0);
		
		if(uid > 0) result = getUser(uid);
	}
	
	return result;
}

 
const vector <AParams::GroupData> AUsers::getAllGroups(const u_short type){
	
	vector <AParams::GroupData> result;
	
	if(this->groups != nullptr) result = this->groups->getAllGroups(type);
	
	return result;
}
 
const vector <AParams::UserData> AUsers::getAllUsers(const u_short type){
	
	vector <AParams::UserData> result;
	
	if(this->users != nullptr) result = this->users->getAllUsers(type);
	
	return result;
}
 
const vector <uid_t> AUsers::getIdUsersInGroup(const gid_t gid){
	
	vector <uid_t> result;
	
	if((gid > 0) && (this->groups != nullptr)){
		
		result = this->groups->getIdUsers(gid);
	}
	
	return result;
}
 
const vector <uid_t> AUsers::getIdUsersInGroup(const string groupName){
	
	vector <uid_t> result;
	
	if(!groupName.empty() && (this->groups != nullptr)){
		
		result = this->groups->getIdUsers(groupName);
	}
	
	return result;
}
 
const vector <string> AUsers::getNameUsersInGroup(const gid_t gid){
	
	vector <string> result;
	
	if((gid > 0) && (this->groups != nullptr)){
		
		result = this->groups->getNameUsers(gid);
	}
	
	return result;
}
 
const vector <string> AUsers::getNameUsersInGroup(const string groupName){
	
	vector <string> result;
	
	if(!groupName.empty() && (this->groups != nullptr)){
		
		result = this->groups->getNameUsers(groupName);
	}
	
	return result;
}
 
const string AUsers::getGroupNameByGid(const gid_t gid){
	
	string result;
	
	if(this->groups != nullptr) result = this->groups->getNameById(gid);
	
	return result;
}
 
const string AUsers::getUserNameByUid(const uid_t uid){
	
	string result;
	
	if(this->users != nullptr) result = this->users->getNameById(uid);
	
	return result;
}
 
const gid_t AUsers::getGidByName(const string groupName){
	
	gid_t result = 0;
	
	if(this->groups != nullptr) result = this->groups->getIdByName(groupName);
	
	return result;
}
 
const uid_t AUsers::getUidByName(const string userName){
	
	uid_t result = 0;
	
	if(this->users != nullptr) result = this->users->getIdByName(userName);
	
	return result;
}
 
const u_short AUsers::getOptionsByUid(const uid_t uid){
	
	u_short result = AMING_NULL;
	
	if(this->users != nullptr){
		
		auto * params = this->users->getParamsById(uid);
		
		if(params != nullptr) result = params->options;
	}
	
	return result;
}
 
const u_short AUsers::getOptionsByUserName(const string userName){
	
	u_short result = AMING_NULL;
	
	if(this->users != nullptr){
		
		auto * user = this->users->getParamsByName(userName);
		
		if(user != nullptr) result = user->options;
	}
	
	return result;
}
 
const bool AUsers::checkUserInGroup(const gid_t gid, const uid_t uid){
	
	bool result = false;
	
	if((gid > 0) && (uid > 0) && (this->groups != nullptr)){
		
		result = this->groups->checkUser(gid, uid);
	}
	
	return result;
}
 
const bool AUsers::checkUserInGroup(const gid_t gid, const string userName){
	
	bool result = false;
	
	if((gid > 0) && !userName.empty() && (this->groups != nullptr)){
		
		result = this->groups->checkUser(gid, userName);
	}
	
	return result;
}
 
const bool AUsers::checkUserInGroup(const string groupName, const uid_t uid){
	
	bool result = false;
	
	if((uid > 0) && !groupName.empty() && (this->groups != nullptr)){
		
		result = this->groups->checkUser(groupName, uid);
	}
	
	return result;
}
 
const bool AUsers::checkUserInGroup(const string groupName, const string userName){
	
	bool result = false;
	
	if(!userName.empty() && !groupName.empty() && (this->groups != nullptr)){
		
		result = this->groups->checkUser(groupName, userName);
	}
	
	return result;
}
 
const bool AUsers::checkGroupById(const gid_t gid){
	
	bool result = false;
	
	if((gid > 0) && (this->groups != nullptr)){
		
		result = this->groups->checkGroupById(gid);
	}
	
	return result;
}
 
const bool AUsers::checkUserById(const uid_t uid){
	
	bool result = false;
	
	if((uid > 0) && (this->users != nullptr)){
		
		result = this->users->checkUserById(uid);
	}
	
	return result;
}
 
const AParams::AUser AUsers::searchUser(const string ip, const string mac){
	
	AParams::AUser result;
	
	if(!ip.empty() && !mac.empty()){
		
		auto * user = this->users->getDataByConnect(ip, mac);
		
		if(user != nullptr){
			
			result = getUser(user->id);
			
			result.auth = true;
		}
	}
	
	return result;
}
 
const AParams::AUser AUsers::authenticate(const string login, const string pass){
	
	AParams::AUser result;
	
	if(!login.empty() && !pass.empty() && (this->auth != nullptr)){
		
		if(this->auth->check(login, pass)){
			
			result = getUser(login);
			
			result.auth = true;
		}
	}
	
	return result;
}
 
AUsers::AUsers(Config * config, LogApp * log){
	
	if(config != nullptr){
		
		this->log = log;
		
		this->config = config;
		
		this->groups = new Groups(this->config, this->log);
		
		this->users = new Users(this->config, this->log);
		
		if((this->users != nullptr) && (this->groups != nullptr)){
			
			this->groups->setUsers(this->users);
			
			this->users->setGroups(this->groups);
			
			this->auth = new Auth(this->config, this->log, this->groups, this->users);
		}
	}
}
 
AUsers::~AUsers(){
	
	if(this->auth != nullptr)	delete this->auth;
	if(this->users != nullptr)	delete this->users;
	if(this->groups != nullptr)	delete this->groups;
}
