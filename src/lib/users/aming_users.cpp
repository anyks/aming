/* SERVER PROXY AMING */
/*
*  author:     Yuriy Lobarev
*  phone:      +7(910)983-95-90
*  telegram:   @forman
*  email:      info@anyks.com
*  date:       11/23/2017 17:50:05
*  copyright:  © 2017 anyks.com
*/
 
 

#include "ausers/ausers.h"


using namespace std;

 
void AUsers::Users::setUserParams(const uid_t uid, const string name, const u_short type){
	 
	auto setParams = [this](const uid_t uid, AParams::Params params, const u_short type){
		
		auto key = make_pair(uid, type);
		
		this->params.insert(pair <pair <uid_t, u_short>, AParams::Params> (key, params));
	};
	
	switch(this->config->proxy.configs){
		
		case AUSERS_TYPE_FILE: setParams(uid, this->ufiles->setParams(uid, name), type); break;
		
		case AUSERS_TYPE_LDAP: setParams(uid, this->uldap->setParams(uid, name), type); break;
		
		case AUSERS_TYPE_FILE_LDAP: {
			
			setParams(uid, this->ufiles->setParams(uid, name), type);
			
			setParams(uid, this->uldap->setParams(uid, name), type);
		} break;
	}
}
 
const gid_t AUsers::Users::getGidByGroupName(const string groupName, const u_short type){
	
	gid_t result = 0;
	
	if(!groupName.empty() && (this->groups != nullptr)){
		
		result = (reinterpret_cast <Groups *> (this->groups))->getIdByName(groupName, type);
	}
	
	return result;
}
 
const string AUsers::Users::getGroupNameByGid(const uid_t gid, const u_short type){
	
	string result;
	
	if((gid > 0) && (this->groups != nullptr)){
		
		result = (reinterpret_cast <Groups *> (this->groups))->getNameById(gid, type);
	}
	
	return result;
}
 
const bool AUsers::Users::update(const u_short type){
	
	bool result = false;
	
	time_t curUpdate = time(nullptr);
	
	if((this->lastUpdate + this->config->proxy.conftime) < curUpdate){
		
		this->params.clear();
		
		this->lastUpdate = curUpdate;
		
		setUserParams(0, "", AMING_NULL);
		 
		auto setParams = [this](vector <AParams::User> users, const u_short type){
			
			for(auto it = users.cbegin(); it != users.cend(); ++it){
				
				setUserParams(it->uid, it->name, type);
			}
		};
		
		const u_short typeUser = (type != AMING_NULL ? type : this->config->auth.services);
		
		switch(typeUser){
			
			case AUSERS_TYPE_FILE: setParams(this->ufiles->readUsers(), AUSERS_TYPE_FILE); break;
			
			case AUSERS_TYPE_PAM: setParams(this->upam->readUsers(), AUSERS_TYPE_PAM); break;
			
			case AUSERS_TYPE_LDAP: setParams(this->uldap->readUsers(), AUSERS_TYPE_LDAP); break;
			
			case AUSERS_TYPE_FILE_PAM: {
				setParams(this->ufiles->readUsers(), AUSERS_TYPE_FILE);
				setParams(this->upam->readUsers(), AUSERS_TYPE_PAM);
			} break;
			
			case AUSERS_TYPE_FILE_LDAP: {
				setParams(this->ufiles->readUsers(), AUSERS_TYPE_FILE);
				setParams(this->uldap->readUsers(), AUSERS_TYPE_LDAP);
			} break;
			
			case AUSERS_TYPE_PAM_LDAP: {
				setParams(this->upam->readUsers(), AUSERS_TYPE_PAM);
				setParams(this->uldap->readUsers(), AUSERS_TYPE_LDAP);
			} break;
			
			case AUSERS_TYPE_FILE_PAM_LDAP: {
				setParams(this->ufiles->readUsers(), AUSERS_TYPE_FILE);
				setParams(this->upam->readUsers(), AUSERS_TYPE_PAM);
				setParams(this->uldap->readUsers(), AUSERS_TYPE_LDAP);
			} break;
		}
		
		result = true;
	}
	
	return result;
}
 
const vector <AParams::UserData> AUsers::Users::getAllUsers(const u_short type){
	
	vector <AParams::UserData> result;
	
	const u_short typeUser = (type != AMING_NULL ? type : this->config->auth.services);
	
	switch(typeUser){
		
		case AUSERS_TYPE_FILE: result = this->ufiles->getAllUsers(); break;
		
		case AUSERS_TYPE_PAM: result = this->upam->getAllUsers(); break;
		
		case AUSERS_TYPE_LDAP: result = this->uldap->getAllUsers(); break;
		
		case AUSERS_TYPE_FILE_PAM: {
			
			auto file = this->ufiles->getAllUsers();
			
			auto pam = this->upam->getAllUsers();
			
			copy(file.begin(), file.end(), back_inserter(result));
			copy(pam.begin(), pam.end(), back_inserter(result));
		} break;
		
		case AUSERS_TYPE_FILE_LDAP: {
			
			auto file = this->ufiles->getAllUsers();
			
			auto ldap = this->uldap->getAllUsers();
			
			copy(file.begin(), file.end(), back_inserter(result));
			copy(ldap.begin(), ldap.end(), back_inserter(result));
		} break;
		
		case AUSERS_TYPE_PAM_LDAP: {
			
			auto pam = this->upam->getAllUsers();
			
			auto ldap = this->uldap->getAllUsers();
			
			copy(pam.begin(), pam.end(), back_inserter(result));
			copy(ldap.begin(), ldap.end(), back_inserter(result));
		} break;
		
		case AUSERS_TYPE_FILE_PAM_LDAP: {
			
			auto file = this->ufiles->getAllUsers();
			
			auto pam = this->upam->getAllUsers();
			
			auto ldap = this->uldap->getAllUsers();
			
			copy(file.begin(), file.end(), back_inserter(result));
			copy(pam.begin(), pam.end(), back_inserter(result));
			copy(ldap.begin(), ldap.end(), back_inserter(result));
		} break;
	}
	
	return result;
}
 
const AParams::Params * AUsers::Users::getParamsById(const uid_t uid, const u_short type){
	
	const AParams::Params * result = nullptr;
	
	if(!this->params.empty()){
		
		if(uid > 0){
			 
			auto getParams = [this](const uid_t uid, u_short type){
				
				const AParams::Params * result = nullptr;
				
				auto key = make_pair(uid, type);
				
				if(this->params.count(key) > 0){
					
					result = &(this->params.find(key)->second);
				
				} else if(update(type)) {
					
					result = getParamsById(uid, type);
				
				} else {
					
					auto key = make_pair(0, AMING_NULL);
					
					if(this->params.count(key) > 0){
						
						result = &(this->params.find(key)->second);
					}
				}
				
				return result;
			};
			
			const u_short typeUser = (type != AMING_NULL ? type : this->config->auth.services);
			
			switch(typeUser){
				
				case AUSERS_TYPE_FILE: result = getParams(uid, AUSERS_TYPE_FILE); break;
				
				case AUSERS_TYPE_PAM: result = getParams(uid, AUSERS_TYPE_PAM); break;
				
				case AUSERS_TYPE_LDAP: result = getParams(uid, AUSERS_TYPE_LDAP); break;
				
				case AUSERS_TYPE_FILE_PAM: {
					
					result = getParams(uid, AUSERS_TYPE_FILE);
					
					if(result == nullptr) result = getParams(uid, AUSERS_TYPE_PAM);
				} break;
				
				case AUSERS_TYPE_FILE_LDAP: {
					
					result = getParams(uid, AUSERS_TYPE_FILE);
					
					if(result == nullptr) result = getParams(uid, AUSERS_TYPE_LDAP);
				} break;
				
				case AUSERS_TYPE_PAM_LDAP: {
					
					result = getParams(uid, AUSERS_TYPE_PAM);
					
					if(result == nullptr) result = getParams(uid, AUSERS_TYPE_LDAP);
				} break;
				
				case AUSERS_TYPE_FILE_PAM_LDAP: {
					
					result = getParams(uid, AUSERS_TYPE_FILE);
					
					if(result == nullptr) result = getParams(uid, AUSERS_TYPE_PAM);
					
					if(result == nullptr) result = getParams(uid, AUSERS_TYPE_LDAP);
				} break;
			}
		
		} else {
			
			auto key = make_pair(0, AMING_NULL);
			
			if(this->params.count(key) > 0){
				
				result = &(this->params.find(key)->second);
			}
		}
	}
	
	return result;
}
 
const AParams::Params * AUsers::Users::getParamsByName(const string userName, const u_short type){
	
	if(!userName.empty() && !this->params.empty()){
		
		const string name = Anyks::toCase(userName);
		
		const uid_t uid = getIdByName(name, type);
		
		if(uid > 0) return getParamsById(uid, type);
	}
	
	return nullptr;
}
 
const AParams::Params * AUsers::Users::getDataByConnect(const string ip, const string mac, const u_short type){
	
	const AParams::Params * result = getParamsById(0, type);
	
	if((!ip.empty() || !mac.empty()) && !this->params.empty()){
		
		Network nwk;
		
		u_int nettype = (!ip.empty() ? nwk.checkNetworkByIp(ip) : 0);
		
		for(auto it = this->params.cbegin(); it != this->params.cend(); ++it){
			
			bool _ip = false;
			
			bool _mac = false;
			
			for(auto iit = it->second.idnt.cbegin(); iit != it->second.idnt.cend(); ++iit){
				
				const string id = Anyks::toCase(* iit);
				
				const u_int idnt = Anyks::getTypeAmingByString(id);
				
				switch(idnt){
					case AMING_IPV4: {
						
						if((nettype == 4) && (ip.compare(id) == 0)) _ip = true;
					} break;
					case AMING_IPV6: {
						
						if(((nettype == 6) && nwk.compareIP6(ip, id))
						|| (nwk.isV4ToV6(ip) && (Anyks::toCase(ip).compare(id) == 0))) _ip = true;
					} break;
					case AMING_NETWORK: {
						
						if((nettype == 4) && nwk.checkIPByNetwork(ip, id)) _ip = true;
						
						else if((nettype == 6) && nwk.checkIPByNetwork6(ip, id)) _ip = true;
					} break;
					case AMING_MAC: {
						
						if(!mac.empty() && (id.compare(Anyks::toCase(mac)) == 0)) _mac = true;
					} break;
				}
			}
			
			if(_ip && _mac) return &(it->second);
			
			else if((_ip || _mac) && (result == nullptr)) result = &(it->second);
		}
	}
	
	return result;
}
 
const AParams::UserData AUsers::Users::getDataById(const uid_t uid, const u_short type){
	
	AParams::UserData result;
	
	if(uid > 0){
		
		const u_short typeUser = (type != AMING_NULL ? type : this->config->auth.services);
		
		switch(typeUser){
			
			case AUSERS_TYPE_FILE: result = this->ufiles->getDataById(uid); break;
			
			case AUSERS_TYPE_PAM: result = this->upam->getDataById(uid); break;
			
			case AUSERS_TYPE_LDAP: result = this->uldap->getDataById(uid); break;
			
			case AUSERS_TYPE_FILE_PAM: {
				
				result = this->ufiles->getDataById(uid);
				
				if(result.name.empty()) result = this->upam->getDataById(uid);
			} break;
			
			case AUSERS_TYPE_FILE_LDAP: {
				
				result = this->ufiles->getDataById(uid);
				
				if(result.name.empty()) result = this->uldap->getDataById(uid);
			} break;
			
			case AUSERS_TYPE_PAM_LDAP: {
				
				result = this->upam->getDataById(uid);
				
				if(result.name.empty()) result = this->uldap->getDataById(uid);
			} break;
			
			case AUSERS_TYPE_FILE_PAM_LDAP: {
				
				result = this->ufiles->getDataById(uid);
				
				if(result.name.empty()) result = this->upam->getDataById(uid);
				
				if(result.name.empty()) result = this->uldap->getDataById(uid);
			} break;
		}
	}
	
	return result;
}
 
const AParams::UserData AUsers::Users::getDataByName(const string userName, const u_short type){
	
	AParams::UserData result;
	
	if(!userName.empty()){
		
		const string name = Anyks::toCase(userName);
		
		const uid_t uid = getIdByName(name, type);
		
		if(uid > 0) result = getDataById(uid, type);
	}
	
	return result;
}
 
const bool AUsers::Users::checkUserById(const uid_t uid, const u_short type){
	
	bool result = false;
	
	if(uid > 0){
		
		const u_short typeUser = (type != AMING_NULL ? type : this->config->auth.services);
		
		switch(typeUser){
			
			case AUSERS_TYPE_FILE: result = this->ufiles->checkUserById(uid); break;
			
			case AUSERS_TYPE_PAM: result = this->upam->checkUserById(uid); break;
			
			case AUSERS_TYPE_LDAP: result = this->uldap->checkUserById(uid); break;
			
			case AUSERS_TYPE_FILE_PAM: {
				
				result = this->ufiles->checkUserById(uid);
				
				if(!result) result = this->upam->checkUserById(uid);
			} break;
			
			case AUSERS_TYPE_FILE_LDAP: {
				
				result = this->ufiles->checkUserById(uid);
				
				if(!result) result = this->uldap->checkUserById(uid);
			} break;
			
			case AUSERS_TYPE_PAM_LDAP: {
				
				result = this->upam->checkUserById(uid);
				
				if(!result) result = this->uldap->checkUserById(uid);
			} break;
			
			case AUSERS_TYPE_FILE_PAM_LDAP: {
				
				result = this->ufiles->checkUserById(uid);
				
				if(!result) result = this->upam->checkUserById(uid);
				
				if(!result) result = this->uldap->checkUserById(uid);
			} break;
		}
	}
	
	return result;
}
 
const bool AUsers::Users::checkUserByName(const string userName, const u_short type){
	
	return (getIdByName(userName, type) > 0 ? true : false);
}
 
const bool AUsers::Users::auth(const string username, const string password, const u_short type){
	
	bool result = false;
	
	if(!username.empty() && !password.empty() && this->config->auth.enabled){
		
		const u_short typeUser = (type != AMING_NULL ? type : this->config->auth.services);
		
		auto user = getDataByName(username, typeUser);
		
		if(user.uid > 0){
			
			switch(user.type){
				
				case AUSERS_TYPE_FILE: result = this->ufiles->auth(user.uid, password);	break;
				
				case AUSERS_TYPE_PAM: result = this->upam->auth(user.uid, password);	break;
				
				case AUSERS_TYPE_LDAP: result = this->uldap->auth(user.uid, password);	break;
			}
		}
	
	} else if(!this->config->auth.enabled){
		
		const u_short typeUser = (type != AMING_NULL ? type : this->config->auth.services);
		
		result = checkUserByName(username, typeUser);
	}
	
	return result;
}
 
const uid_t AUsers::Users::getIdByName(const string userName, const u_short type){
	
	uid_t result = 0;
	
	if(!userName.empty()){
		
		const u_short typeUser = (type != AMING_NULL ? type : this->config->auth.services);
		
		switch(typeUser){
			
			case AUSERS_TYPE_FILE: result = this->ufiles->getIdByName(userName); break;
			
			case AUSERS_TYPE_PAM: result = this->upam->getIdByName(userName); break;
			
			case AUSERS_TYPE_LDAP: result = this->uldap->getIdByName(userName); break;
			
			case AUSERS_TYPE_FILE_PAM: {
				
				result = this->ufiles->getIdByName(userName);
				
				if(result < 1) result = this->upam->getIdByName(userName);
			} break;
			
			case AUSERS_TYPE_FILE_LDAP: {
				
				result = this->ufiles->getIdByName(userName);
				
				if(result < 1) result = this->uldap->getIdByName(userName);
			} break;
			
			case AUSERS_TYPE_PAM_LDAP: {
				
				result = this->upam->getIdByName(userName);
				
				if(result < 1) result = this->uldap->getIdByName(userName);
			} break;
			
			case AUSERS_TYPE_FILE_PAM_LDAP: {
				
				result = this->ufiles->getIdByName(userName);
				
				if(result < 1) result = this->upam->getIdByName(userName);
				
				if(result < 1) result = this->uldap->getIdByName(userName);
			} break;
		}
	}
	
	return result;
}
 
const string AUsers::Users::getNameById(const uid_t uid, const u_short type){
	
	string result;
	
	if(uid > 0){
		
		const u_short typeUser = (type != AMING_NULL ? type : this->config->auth.services);
		
		switch(typeUser){
			
			case AUSERS_TYPE_FILE: result = this->ufiles->getNameById(uid); break;
			
			case AUSERS_TYPE_PAM: result = this->upam->getNameById(uid); break;
			
			case AUSERS_TYPE_LDAP: result = this->uldap->getNameById(uid); break;
			
			case AUSERS_TYPE_FILE_PAM: {
				
				result = this->ufiles->getNameById(uid);
				
				if(result.empty()) result = this->upam->getNameById(uid);
			} break;
			
			case AUSERS_TYPE_FILE_LDAP: {
				
				result = this->ufiles->getNameById(uid);
				
				if(result.empty()) result = this->uldap->getNameById(uid);
			} break;
			
			case AUSERS_TYPE_PAM_LDAP: {
				
				result = this->upam->getNameById(uid);
				
				if(result.empty()) result = this->uldap->getNameById(uid);
			} break;
			
			case AUSERS_TYPE_FILE_PAM_LDAP: {
				
				result = this->ufiles->getNameById(uid);
				
				if(result.empty()) result = this->upam->getNameById(uid);
				
				if(result.empty()) result = this->uldap->getNameById(uid);
			} break;
		}
	}
	
	return result;
}
 
const vector <uid_t> AUsers::Users::getIdAllUsers(const u_short type){
	
	vector <uid_t> result;
	
	const u_short typeUser = (type != AMING_NULL ? type : this->config->auth.services);
	
	auto users = getAllUsers(typeUser);
	
	if(!users.empty()){
		
		for(auto it = users.cbegin(); it != users.cend(); ++it){
			
			result.push_back(it->uid);
		}
	}
	
	return result;
}
 
void AUsers::Users::setGroups(void * groups){
	
	if(groups != nullptr){
		
		this->groups = groups;
		 
		auto groupNameByGid = [this](const uid_t gid, const u_short type){
			
			return getGroupNameByGid(gid, type);
		};
		 
		auto gidByGroupName = [this](const string groupName, const u_short type){
			
			return getGidByGroupName(groupName, type);
		};
		 
		auto getParamsById = [this](const gid_t gid, const u_short type){
			
			const AParams::Params * result = nullptr;
			
			if((gid > 0) && (this->groups != nullptr)){
				
				result = (reinterpret_cast <Groups *> (this->groups))->getParamsById(gid, type);
			}
			
			return result;
		};
		 
		auto getGroupIdByUser = [this](const uid_t uid, const u_short type){
			
			vector <gid_t> result;
			
			if((uid > 0) && (this->groups != nullptr)){
				
				result = (reinterpret_cast <Groups *> (this->groups))->getGroupIdByUser(uid, type);
			}
			
			return result;
		};
		 
		auto getDataByGid = [this](const gid_t gid, const u_short type){
			
			AParams::GroupData result;
			
			if((gid > 0) && (this->groups != nullptr)){
				
				result = (reinterpret_cast <Groups *> (this->groups))->getDataById(gid, type);
			}
			
			return result;
		};
		
		this->upam->setGroupsMethods(groupNameByGid, gidByGroupName);
		this->uldap->setGroupsMethods(groupNameByGid, gidByGroupName);
		this->ufiles->setGroupsMethods(groupNameByGid, gidByGroupName);
		
		this->ufiles->setPasswordMethod(AUsers::getPasswordFromFile);
		this->ufiles->setPasswordsMethod(AUsers::getPasswordsFromFile);
		
		this->uldap->setParamsMethod(getParamsById);
		this->uldap->setGidsMethod(getGroupIdByUser);
		this->uldap->setGroupDataMethod(getDataByGid);
		this->ufiles->setParamsMethod(getParamsById);
		this->ufiles->setGidsMethod(getGroupIdByUser);
		this->ufiles->setGroupDataMethod(getDataByGid);
	}
}
 
void AUsers::Users::run(){
	
	if((this->groups != nullptr) && this->params.empty()) update();
}
 
AUsers::Users::Users(Config * config, LogApp * log){
	
	if(config != nullptr){
		
		this->log = log;
		
		this->config = config;
		
		this->upam = new Upam(this->config, this->log);
		this->uldap = new Uldap(this->config, this->log);
		this->ufiles = new Ufiles(this->config, this->log);
	}
}
 
AUsers::Users::~Users(){
	
	if(this->upam != nullptr)	delete this->upam;
	if(this->uldap != nullptr)	delete this->uldap;
	if(this->ufiles != nullptr)	delete this->ufiles;
}
