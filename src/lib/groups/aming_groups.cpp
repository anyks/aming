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

 
template <typename T>
const vector <T> Anyks::concatVectors(const vector <T> &v1, const vector <T> &v2){
	
	vector <T> result;
	
	if(!v1.empty() && !v2.empty()){
		
		result.assign(v1.cbegin(), v1.cend());
		
		for(auto it = v2.cbegin(); it != v2.cend(); ++it){
			
			if(find(v1.begin(), v1.end(), * it) == v1.end()){
				
				result.push_back(* it);
			}
		}
	}
	
	return result;
};
 
void AUsers::Groups::setGroupParams(const gid_t gid, const string name, const u_short type){
	 
	auto setParams = [this](const gid_t gid, AParams::Params params, const u_short type){
		
		auto key = make_pair(gid, type);
		
		this->params.insert(pair <pair <gid_t, u_short>, AParams::Params> (key, params));
	};
	
	switch(this->config->proxy.configs){
		
		case AUSERS_TYPE_FILE: setParams(gid, this->gfiles->setParams(gid, name), type); break;
		
		case AUSERS_TYPE_LDAP: setParams(gid, this->gldap->setParams(gid, name), type); break;
		
		case AUSERS_TYPE_FILE_LDAP: {
			
			setParams(gid, this->gfiles->setParams(gid, name), type);
			
			setParams(gid, this->gldap->setParams(gid, name), type);
		} break;
	}
}
 
const uid_t AUsers::Groups::getUidByUserName(const string userName, const u_short type){
	
	uid_t result = 0;
	
	if(!userName.empty() && (this->users != nullptr)){
		
		result = (reinterpret_cast <Users *> (this->users))->getIdByName(userName, type);
	}
	
	return result;
}
 
const string AUsers::Groups::getUserNameByUid(const uid_t uid, const u_short type){
	
	string result;
	
	if((uid > 0) && (this->users != nullptr)){
		
		result = (reinterpret_cast <Users *> (this->users))->getNameById(uid, type);
	}
	
	return result;
}
 
const bool AUsers::Groups::update(const u_short type){
	
	bool result = false;
	
	time_t curUpdate = time(nullptr);
	
	if((this->lastUpdate + this->config->auth.update) < curUpdate){
		
		this->params.clear();
		
		this->lastUpdate = curUpdate;
		
		setGroupParams(0, "", AMING_NULL);
		 
		auto setParams = [this](vector <AParams::Group> groups, const u_short type){
			
			for(auto it = groups.cbegin(); it != groups.cend(); ++it){
				
				setGroupParams(it->gid, it->name, type);
			}
		};
		
		const u_short typeGroup = (type != AMING_NULL ? type : this->config->auth.services);
		
		switch(typeGroup){
			
			case AUSERS_TYPE_FILE: setParams(this->gfiles->readGroups(), AUSERS_TYPE_FILE); break;
			
			case AUSERS_TYPE_PAM: setParams(this->gpam->readGroups(), AUSERS_TYPE_PAM); break;
			
			case AUSERS_TYPE_LDAP: setParams(this->gldap->readGroups(), AUSERS_TYPE_LDAP); break;
			
			case AUSERS_TYPE_FILE_PAM: {
				setParams(this->gfiles->readGroups(), AUSERS_TYPE_FILE);
				setParams(this->gpam->readGroups(), AUSERS_TYPE_PAM);
			} break;
			
			case AUSERS_TYPE_FILE_LDAP: {
				setParams(this->gfiles->readGroups(), AUSERS_TYPE_FILE);
				setParams(this->gldap->readGroups(), AUSERS_TYPE_LDAP);
			} break;
			
			case AUSERS_TYPE_PAM_LDAP: {
				setParams(this->gpam->readGroups(), AUSERS_TYPE_PAM);
				setParams(this->gldap->readGroups(), AUSERS_TYPE_LDAP);
			} break;
			
			case AUSERS_TYPE_FILE_PAM_LDAP: {
				setParams(this->gfiles->readGroups(), AUSERS_TYPE_FILE);
				setParams(this->gpam->readGroups(), AUSERS_TYPE_PAM);
				setParams(this->gldap->readGroups(), AUSERS_TYPE_LDAP);
			} break;
		}
		
		result = true;
	}
	
	return result;
}
 
const vector <AParams::GroupData> AUsers::Groups::getAllGroups(const u_short type){
	
	vector <AParams::GroupData> result;
	
	const u_short typeGroup = (type != AMING_NULL ? type : this->config->auth.services);
	
	switch(typeGroup){
		
		case AUSERS_TYPE_FILE: result = this->gfiles->getAllGroups(); break;
		
		case AUSERS_TYPE_PAM: result = this->gpam->getAllGroups(); break;
		
		case AUSERS_TYPE_LDAP: result = this->gldap->getAllGroups(); break;
		
		case AUSERS_TYPE_FILE_PAM: {
			
			auto file = this->gfiles->getAllGroups();
			
			auto pam = this->gpam->getAllGroups();
			
			copy(file.begin(), file.end(), back_inserter(result));
			copy(pam.begin(), pam.end(), back_inserter(result));
		} break;
		
		case AUSERS_TYPE_FILE_LDAP: {
			
			auto file = this->gfiles->getAllGroups();
			
			auto ldap = this->gldap->getAllGroups();
			
			copy(file.begin(), file.end(), back_inserter(result));
			copy(ldap.begin(), ldap.end(), back_inserter(result));
		} break;
		
		case AUSERS_TYPE_PAM_LDAP: {
			
			auto pam = this->gpam->getAllGroups();
			
			auto ldap = this->gldap->getAllGroups();
			
			copy(pam.begin(), pam.end(), back_inserter(result));
			copy(ldap.begin(), ldap.end(), back_inserter(result));
		} break;
		
		case AUSERS_TYPE_FILE_PAM_LDAP: {
			
			auto file = this->gfiles->getAllGroups();
			
			auto pam = this->gpam->getAllGroups();
			
			auto ldap = this->gldap->getAllGroups();
			
			copy(file.begin(), file.end(), back_inserter(result));
			copy(pam.begin(), pam.end(), back_inserter(result));
			copy(ldap.begin(), ldap.end(), back_inserter(result));
		} break;
	}
	
	return result;
}
 
const AParams::Params * AUsers::Groups::getParamsById(const gid_t gid, const u_short type){
	
	const AParams::Params * result = nullptr;
	
	if((gid > 0) && !this->params.empty()){
		 
		auto getParams = [this](const gid_t gid, u_short type){
			
			const AParams::Params * result = nullptr;
			
			auto key = make_pair(gid, type);
			
			if(this->params.count(key) > 0){
				
				result = &(this->params.find(key)->second);
			
			} else if(update(type)) {
				
				result = getParamsById(gid, type);
			
			} else {
				
				auto key = make_pair(0, AMING_NULL);
				
				if(this->params.count(key) > 0){
					
					result = &(this->params.find(key)->second);
				}
			}
			
			return result;
		};
		
		const u_short typeGroup = (type != AMING_NULL ? type : this->config->auth.services);
		
		switch(typeGroup){
			
			case AUSERS_TYPE_FILE: result = getParams(gid, AUSERS_TYPE_FILE); break;
			
			case AUSERS_TYPE_PAM: result = getParams(gid, AUSERS_TYPE_PAM); break;
			
			case AUSERS_TYPE_LDAP: result = getParams(gid, AUSERS_TYPE_LDAP); break;
			
			case AUSERS_TYPE_FILE_PAM: {
				
				result = getParams(gid, AUSERS_TYPE_FILE);
				
				if(result == nullptr) result = getParams(gid, AUSERS_TYPE_PAM);
			} break;
			
			case AUSERS_TYPE_FILE_LDAP: {
				
				result = getParams(gid, AUSERS_TYPE_FILE);
				
				if(result == nullptr) result = getParams(gid, AUSERS_TYPE_LDAP);
			} break;
			
			case AUSERS_TYPE_PAM_LDAP: {
				
				result = getParams(gid, AUSERS_TYPE_PAM);
				
				if(result == nullptr) result = getParams(gid, AUSERS_TYPE_LDAP);
			} break;
			
			case AUSERS_TYPE_FILE_PAM_LDAP: {
				
				result = getParams(gid, AUSERS_TYPE_FILE);
				
				if(result == nullptr) result = getParams(gid, AUSERS_TYPE_PAM);
				
				if(result == nullptr) result = getParams(gid, AUSERS_TYPE_LDAP);
			} break;
		}
	}
	
	return result;
}
 
const AParams::Params * AUsers::Groups::getParamsByName(const string groupName, const u_short type){
	
	if(!groupName.empty() && !this->params.empty()){
		
		const string name = Anyks::toCase(groupName);
		
		const gid_t gid = getIdByName(name, type);
		
		if(gid > 0) return getParamsById(gid, type);
	}
	
	return nullptr;
}
 
const AParams::GroupData AUsers::Groups::getDataById(const gid_t gid, u_short type){
	
	AParams::GroupData result;
	
	if(gid > 0){
		
		const u_short typeGroup = (type != AMING_NULL ? type : this->config->auth.services);
		
		switch(typeGroup){
			
			case AUSERS_TYPE_FILE: result = this->gfiles->getDataById(gid); break;
			
			case AUSERS_TYPE_PAM: result = this->gpam->getDataById(gid); break;
			
			case AUSERS_TYPE_LDAP: result = this->gldap->getDataById(gid); break;
			
			case AUSERS_TYPE_FILE_PAM: {
				
				result = this->gfiles->getDataById(gid);
				
				if(!result.name.empty()) result = this->gpam->getDataById(gid);
			} break;
			
			case AUSERS_TYPE_FILE_LDAP: {
				
				result = this->gfiles->getDataById(gid);
				
				if(!result.name.empty()) result = this->gldap->getDataById(gid);
			} break;
			
			case AUSERS_TYPE_PAM_LDAP: {
				
				result = this->gpam->getDataById(gid);
				
				if(!result.name.empty()) result = this->gldap->getDataById(gid);
			} break;
			
			case AUSERS_TYPE_FILE_PAM_LDAP: {
				
				result = this->gfiles->getDataById(gid);
				
				if(!result.name.empty()) result = this->gpam->getDataById(gid);
				
				if(!result.name.empty()) result = this->gldap->getDataById(gid);
			} break;
		}
	}
	
	return result;
}
 
const AParams::GroupData AUsers::Groups::getDataByName(const string groupName, u_short type){
	
	AParams::GroupData result;
	
	if(!groupName.empty()){
		
		const string name = Anyks::toCase(groupName);
		
		const gid_t gid = getIdByName(name, type);
		
		if(gid > 0) result = getDataById(gid, type);
	}
	
	return result;
}
 
const vector <gid_t> AUsers::Groups::getGroupIdByUser(const uid_t uid, const u_short type){
	
	vector <gid_t> result;
	
	if(uid > 0){
		
		const u_short typeGroup = (type != AMING_NULL ? type : this->config->auth.services);
		
		switch(typeGroup){
			
			case AUSERS_TYPE_FILE: result = this->gfiles->getGroupIdByUser(uid); break;
			
			case AUSERS_TYPE_PAM: result = this->gpam->getGroupIdByUser(uid); break;
			
			case AUSERS_TYPE_LDAP: result = this->gldap->getGroupIdByUser(uid); break;
			
			case AUSERS_TYPE_FILE_PAM: {
				
				result = this->gfiles->getGroupIdByUser(uid);
				
				if(result.empty()) result = this->gpam->getGroupIdByUser(uid);
			} break;
			
			case AUSERS_TYPE_FILE_LDAP: {
				
				result = this->gfiles->getGroupIdByUser(uid);
				
				if(result.empty()) result = this->gldap->getGroupIdByUser(uid);
			} break;
			
			case AUSERS_TYPE_PAM_LDAP: {
				
				result = this->gpam->getGroupIdByUser(uid);
				
				if(result.empty()) result = this->gldap->getGroupIdByUser(uid);
			} break;
			
			case AUSERS_TYPE_FILE_PAM_LDAP: {
				
				result = this->gfiles->getGroupIdByUser(uid);
				
				if(result.empty()) result = this->gpam->getGroupIdByUser(uid);
				
				if(result.empty()) result = this->gldap->getGroupIdByUser(uid);
			} break;
		}
	}
	
	return result;
}
 
const vector <gid_t> AUsers::Groups::getGroupIdByUser(const string userName, const u_short type){
	
	vector <gid_t> result;
	
	if(!userName.empty()){
		
		const uid_t uid = getUidByUserName(userName, type);
		
		if(uid > 0) result = getGroupIdByUser(uid, type);
	}
	
	return result;
}
 
const vector <string> AUsers::Groups::getGroupNameByUser(const uid_t uid, const u_short type){
	
	vector <string> result;
	
	if(uid > 0){
		
		const u_short typeGroup = (type != AMING_NULL ? type : this->config->auth.services);
		
		switch(typeGroup){
			
			case AUSERS_TYPE_FILE: result = this->gfiles->getGroupNameByUser(uid); break;
			
			case AUSERS_TYPE_PAM: result = this->gpam->getGroupNameByUser(uid); break;
			
			case AUSERS_TYPE_LDAP: result = this->gldap->getGroupNameByUser(uid); break;
			
			case AUSERS_TYPE_FILE_PAM: {
				
				result = this->gfiles->getGroupNameByUser(uid);
				
				if(result.empty()) result = this->gpam->getGroupNameByUser(uid);
			} break;
			
			case AUSERS_TYPE_FILE_LDAP: {
				
				result = this->gfiles->getGroupNameByUser(uid);
				
				if(result.empty()) result = this->gldap->getGroupNameByUser(uid);
			} break;
			
			case AUSERS_TYPE_PAM_LDAP: {
				
				result = this->gpam->getGroupNameByUser(uid);
				
				if(result.empty()) result = this->gldap->getGroupNameByUser(uid);
			} break;
			
			case AUSERS_TYPE_FILE_PAM_LDAP: {
				
				result = this->gfiles->getGroupNameByUser(uid);
				
				if(result.empty()) result = this->gpam->getGroupNameByUser(uid);
				
				if(result.empty()) result = this->gldap->getGroupNameByUser(uid);
			} break;
		}
	}
	
	return result;
}
 
const vector <string> AUsers::Groups::getGroupNameByUser(const string userName, const u_short type){
	
	vector <string> result;
	
	if(!userName.empty()){
		
		const uid_t uid = getUidByUserName(userName, type);
		
		if(uid > 0) result = getGroupNameByUser(uid, type);
	}
	
	return result;
}
 
const bool AUsers::Groups::checkUser(const gid_t gid, const uid_t uid, const u_short type){
	
	bool result = false;
	
	if((gid > 0) && (uid > 0)){
		
		const u_short typeGroup = (type != AMING_NULL ? type : this->config->auth.services);
		
		switch(typeGroup){
			
			case AUSERS_TYPE_FILE: result = this->gfiles->checkUser(gid, uid); break;
			
			case AUSERS_TYPE_PAM: result = this->gpam->checkUser(gid, uid); break;
			
			case AUSERS_TYPE_LDAP: result = this->gldap->checkUser(gid, uid); break;
			
			case AUSERS_TYPE_FILE_PAM: {
				
				result = this->gfiles->checkUser(gid, uid);
				
				if(!result) result = this->gpam->checkUser(gid, uid);
			} break;
			
			case AUSERS_TYPE_FILE_LDAP: {
				
				result = this->gfiles->checkUser(gid, uid);
				
				if(!result) result = this->gldap->checkUser(gid, uid);
			} break;
			
			case AUSERS_TYPE_PAM_LDAP: {
				
				result = this->gpam->checkUser(gid, uid);
				
				if(!result) result = this->gldap->checkUser(gid, uid);
			} break;
			
			case AUSERS_TYPE_FILE_PAM_LDAP: {
				
				result = this->gfiles->checkUser(gid, uid);
				
				if(!result) result = this->gpam->checkUser(gid, uid);
				
				if(!result) result = this->gldap->checkUser(gid, uid);
			} break;
		}
	}
	
	return result;
}
 
const bool AUsers::Groups::checkUser(const gid_t gid, const string userName, const u_short type){
	
	bool result = false;
	
	if((gid > 0) && !userName.empty()){
		
		const uid_t uid = getUidByUserName(userName, type);
		
		if(uid > 0) result = checkUser(gid, uid, type);
	}
	
	return result;
}
 
const bool AUsers::Groups::checkUser(const string groupName, const uid_t uid, const u_short type){
	
	bool result = false;
	
	if((uid > 0) && !groupName.empty()){
		
		const gid_t gid = getIdByName(groupName, type);
		
		if(gid > 0) result = checkUser(gid, uid, type);
	}
	
	return result;
}
 
const bool AUsers::Groups::checkUser(const string groupName, const string userName, const u_short type){
	
	bool result = false;
	
	if(!groupName.empty() && !userName.empty()){
		
		const gid_t gid = getIdByName(groupName, type);
		
		const uid_t uid = getUidByUserName(userName, type);
		
		if((gid > 0) && (uid > 0)) result = checkUser(gid, uid, type);
	}
	
	return result;
}
 
const bool AUsers::Groups::checkGroupById(const gid_t gid, const u_short type){
	
	bool result = false;
	
	if(gid > 0){
		
		const u_short typeGroup = (type != AMING_NULL ? type : this->config->auth.services);
		
		switch(typeGroup){
			
			case AUSERS_TYPE_FILE: result = this->gfiles->checkGroupById(gid); break;
			
			case AUSERS_TYPE_PAM: result = this->gpam->checkGroupById(gid); break;
			
			case AUSERS_TYPE_LDAP: result = this->gldap->checkGroupById(gid); break;
			
			case AUSERS_TYPE_FILE_PAM: {
				
				result = this->gfiles->checkGroupById(gid);
				
				if(!result) result = this->gpam->checkGroupById(gid);
			} break;
			
			case AUSERS_TYPE_FILE_LDAP: {
				
				result = this->gfiles->checkGroupById(gid);
				
				if(!result) result = this->gldap->checkGroupById(gid);
			} break;
			
			case AUSERS_TYPE_PAM_LDAP: {
				
				result = this->gpam->checkGroupById(gid);
				
				if(!result) result = this->gldap->checkGroupById(gid);
			} break;
			
			case AUSERS_TYPE_FILE_PAM_LDAP: {
				
				result = this->gfiles->checkGroupById(gid);
				
				if(!result) result = this->gpam->checkGroupById(gid);
				
				if(!result) result = this->gldap->checkGroupById(gid);
			} break;
		}
	}
	
	return result;
}
 
const bool AUsers::Groups::checkGroupByName(const string groupName, const u_short type){
	
	return (getIdByName(groupName, type) > 0 ? true : false);
}
 
const gid_t AUsers::Groups::getIdByName(const string groupName, const u_short type){
	
	gid_t result = 0;
	
	if(!groupName.empty()){
		
		const u_short typeGroup = (type != AMING_NULL ? type : this->config->auth.services);
		
		switch(typeGroup){
			
			case AUSERS_TYPE_FILE: result = this->gfiles->getIdByName(groupName); break;
			
			case AUSERS_TYPE_PAM: result = this->gpam->getIdByName(groupName); break;
			
			case AUSERS_TYPE_LDAP: result = this->gldap->getIdByName(groupName); break;
			
			case AUSERS_TYPE_FILE_PAM: {
				
				result = this->gfiles->getIdByName(groupName);
				
				if(result < 1) result = this->gpam->getIdByName(groupName);
			} break;
			
			case AUSERS_TYPE_FILE_LDAP: {
				
				result = this->gfiles->getIdByName(groupName);
				
				if(result < 1) result = this->gldap->getIdByName(groupName);
			} break;
			
			case AUSERS_TYPE_PAM_LDAP: {
				
				result = this->gpam->getIdByName(groupName);
				
				if(result < 1) result = this->gldap->getIdByName(groupName);
			} break;
			
			case AUSERS_TYPE_FILE_PAM_LDAP: {
				
				result = this->gfiles->getIdByName(groupName);
				
				if(result < 1) result = this->gpam->getIdByName(groupName);
				
				if(result < 1) result = this->gldap->getIdByName(groupName);
			} break;
		}
	}
	
	return result;
}
 
const string AUsers::Groups::getNameById(const gid_t gid, const u_short type){
	
	string result;
	
	if(gid > 0){
		
		const u_short typeGroup = (type != AMING_NULL ? type : this->config->auth.services);
		
		switch(typeGroup){
			
			case AUSERS_TYPE_FILE: result = this->gfiles->getNameById(gid); break;
			
			case AUSERS_TYPE_PAM: result = this->gpam->getNameById(gid); break;
			
			case AUSERS_TYPE_LDAP: result = this->gldap->getNameById(gid); break;
			
			case AUSERS_TYPE_FILE_PAM: {
				
				result = this->gfiles->getNameById(gid);
				
				if(result.empty()) result = this->gpam->getNameById(gid);
			} break;
			
			case AUSERS_TYPE_FILE_LDAP: {
				
				result = this->gfiles->getNameById(gid);
				
				if(result.empty()) result = this->gldap->getNameById(gid);
			} break;
			
			case AUSERS_TYPE_PAM_LDAP: {
				
				result = this->gpam->getNameById(gid);
				
				if(result.empty()) result = this->gldap->getNameById(gid);
			} break;
			
			case AUSERS_TYPE_FILE_PAM_LDAP: {
				
				result = this->gfiles->getNameById(gid);
				
				if(result.empty()) result = this->gpam->getNameById(gid);
				
				if(result.empty()) result = this->gldap->getNameById(gid);
			} break;
		}
	}
	
	return result;
}
 
const vector <string> AUsers::Groups::getNameUsers(const gid_t gid, const u_short type){
	
	vector <string> result;
	
	if(gid > 0){
		
		const u_short typeGroup = (type != AMING_NULL ? type : this->config->auth.services);
		
		switch(typeGroup){
			
			case AUSERS_TYPE_FILE: result = this->gfiles->getNameUsers(gid); break;
			
			case AUSERS_TYPE_PAM: result = this->gpam->getNameUsers(gid); break;
			
			case AUSERS_TYPE_LDAP: result = this->gldap->getNameUsers(gid); break;
			
			case AUSERS_TYPE_FILE_PAM: {
				
				auto file = this->gfiles->getNameUsers(gid);
				
				auto pam = this->gpam->getNameUsers(gid);
				
				result = Anyks::concatVectors <string> (file, pam);
			} break;
			
			case AUSERS_TYPE_FILE_LDAP: {
				
				auto file = this->gfiles->getNameUsers(gid);
				
				auto ldap = this->gldap->getNameUsers(gid);
				
				result = Anyks::concatVectors <string> (file, ldap);
			} break;
			
			case AUSERS_TYPE_PAM_LDAP: {
				
				auto pam = this->gpam->getNameUsers(gid);
				
				auto ldap = this->gldap->getNameUsers(gid);
				
				result = Anyks::concatVectors <string> (pam, ldap);
			} break;
			
			case AUSERS_TYPE_FILE_PAM_LDAP: {
				
				auto file = this->gfiles->getNameUsers(gid);
				
				auto pam = this->gpam->getNameUsers(gid);
				
				auto ldap = this->gldap->getNameUsers(gid);
				
				vector <string> tmp = Anyks::concatVectors <string> (file, pam);
				
				result = Anyks::concatVectors <string> (tmp, ldap);
			} break;
		}
	}
	
	return result;
}
 
const vector <string> AUsers::Groups::getNameUsers(const string groupName, const u_short type){
	
	vector <string> result;
	
	if((users != nullptr) && !groupName.empty()){
		
		const u_short typeGroup = (type != AMING_NULL ? type : this->config->auth.services);
		
		const gid_t gid = getIdByName(groupName, typeGroup);
		
		if(gid > 0) result = getNameUsers(gid, typeGroup);
	}
	
	return result;
}
 
const vector <uid_t> AUsers::Groups::getIdAllUsers(const u_short type){
	
	vector <uid_t> result;
	
	const u_short typeGroup = (type != AMING_NULL ? type : this->config->auth.services);
	
	auto groups = getAllGroups(typeGroup);
	
	if(!groups.empty()){
		
		for(auto it = groups.cbegin(); it != groups.cend(); ++it){
			
			copy((* it).users.cbegin(), (* it).users.cend(), back_inserter(result));
		}
		
		sort(result.begin(), result.end());
		
		result.resize(unique(result.begin(), result.end()) - result.begin());
	}
	
	return result;
}
 
const vector <uid_t> AUsers::Groups::getIdUsers(const gid_t gid, const u_short type){
	
	vector <uid_t> result;
	
	if(gid > 0){
		
		const u_short typeGroup = (type != AMING_NULL ? type : this->config->auth.services);
		
		switch(typeGroup){
			
			case AUSERS_TYPE_FILE: result = this->gfiles->getIdUsers(gid); break;
			
			case AUSERS_TYPE_PAM: result = this->gpam->getIdUsers(gid); break;
			
			case AUSERS_TYPE_LDAP: result = this->gldap->getIdUsers(gid); break;
			
			case AUSERS_TYPE_FILE_PAM: {
				
				auto file = this->gfiles->getIdUsers(gid);
				
				auto pam = this->gpam->getIdUsers(gid);
				
				result = Anyks::concatVectors <uid_t> (file, pam);
			} break;
			
			case AUSERS_TYPE_FILE_LDAP: {
				
				auto file = this->gfiles->getIdUsers(gid);
				
				auto ldap = this->gldap->getIdUsers(gid);
				
				result = Anyks::concatVectors <uid_t> (file, ldap);
			} break;
			
			case AUSERS_TYPE_PAM_LDAP: {
				
				auto pam = this->gpam->getIdUsers(gid);
				
				auto ldap = this->gldap->getIdUsers(gid);
				
				result = Anyks::concatVectors <uid_t> (pam, ldap);
			} break;
			
			case AUSERS_TYPE_FILE_PAM_LDAP: {
				
				auto file = this->gfiles->getIdUsers(gid);
				
				auto pam = this->gpam->getIdUsers(gid);
				
				auto ldap = this->gldap->getIdUsers(gid);
				
				vector <uid_t> tmp = Anyks::concatVectors <uid_t> (file, pam);
				
				result = Anyks::concatVectors <uid_t> (tmp, ldap);
			} break;
		}
	}
	
	return result;
}
 
const vector <uid_t> AUsers::Groups::getIdUsers(const string groupName, const u_short type){
	
	vector <uid_t> result;
	
	if(!groupName.empty()){
		
		const gid_t gid = getIdByName(groupName, type);
		
		if(gid > 0) result = getIdUsers(gid, type);
	}
	
	return result;
}
 
void AUsers::Groups::setUsers(void * users){
	
	this->users = users;
	 
	auto userNameByUid = [this](const uid_t uid, const u_short type){
		
		return getUserNameByUid(uid, type);
	};
	 
	auto uidByUserName = [this](const string userName, const u_short type){
		
		return getUidByUserName(userName, type);
	};
	
	this->gpam->setUsersMethods(userNameByUid, uidByUserName);
	this->gldap->setUsersMethods(userNameByUid, uidByUserName);
	this->gfiles->setUsersMethods(userNameByUid, uidByUserName);
	
	this->gfiles->setPasswordMethod(AUsers::getPasswordFromFile);
	this->gfiles->setPasswordsMethod(AUsers::getPasswordsFromFile);
	
	update();
}
 
AUsers::Groups::Groups(Config * config, LogApp * log){
	
	if(config != nullptr){
		
		this->log = log;
		
		this->config = config;
		
		this->gpam = new Gpam(this->config, this->log);
		this->gldap = new Gldap(this->config, this->log);
		this->gfiles = new Gfiles(this->config, this->log);
	}
}
 
AUsers::Groups::~Groups(){
	
	if(this->gpam != nullptr)	delete this->gpam;
	if(this->gldap != nullptr)	delete this->gldap;
	if(this->gfiles != nullptr)	delete this->gfiles;
}
