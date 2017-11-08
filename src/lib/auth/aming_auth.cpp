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

 
int AUsers::Auth::pamconv(int num_msg, const struct pam_message ** msg, struct pam_response ** resp, void * appdata_ptr){
	
	if(num_msg <= 0 || (appdata_ptr == nullptr)) return PAM_CONV_ERR;
	
	struct pam_response * reply = new struct pam_response;
	
	if(reply == nullptr) return PAM_CONV_ERR;
	
	reply[0].resp_retcode = 0;
	
	reply[0].resp = strdup(reinterpret_cast <const char *> (appdata_ptr));
	
	* resp = reply;
	
	reply = nullptr;
	
	return PAM_SUCCESS;
}
 
const bool AUsers::Auth::checkLdap(const uid_t uid, const string password){
	
	bool result = false;
	
	if((uid > 0) && !password.empty() && (this->users != nullptr)){
		
		const string login = (reinterpret_cast <Users *> (this->users))->getNameById(uid, AUSERS_TYPE_LDAP);
		
		if(!login.empty()){
			
			const string dn = Anyks::strFormat("%s=%s,%s", this->ldap.keyUser.c_str(), Anyks::toCase(login).c_str(), this->ldap.dnUser.c_str());
			
			ALDAP ldap(this->config, this->log);
			
			result = ldap.checkAuth(dn, password, this->ldap.scopeUser, this->ldap.filterUser);
			
			if(!result && (this->groups != nullptr)){
				
				Groups * groups = reinterpret_cast <Groups *> (this->groups);
				
				if(groups != nullptr){
					
					auto gids = groups->getGroupIdByUser(uid, AUSERS_TYPE_LDAP);
					
					if(!gids.empty()){
						
						for(auto gid = gids.cbegin(); gid != gids.cend(); ++gid){
							
							auto group = groups->getDataById(* gid, AUSERS_TYPE_LDAP);
							
							if(!group.name.empty()){
								
								const string dn = Anyks::strFormat("%s=%s,%s", this->ldap.keyGroup.c_str(), Anyks::toCase(group.name).c_str(), this->ldap.dnGroup.c_str());
								
								result = ldap.checkAuth(dn, password, this->ldap.scopeGroup, this->ldap.filterGroup);
								
								if(result) break;
							}
						}
					}
				}
			}
		}
	}
	
	return result;
}
 
const bool AUsers::Auth::checkPam(const uid_t uid, const string password){
	
	bool result = false;
	
	if((uid > 0) && !password.empty()){
		
		const string name = (reinterpret_cast <Users *> (this->users))->getNameById(uid, AUSERS_TYPE_PAM);
		
		if(!name.empty()){
			
			const struct pam_conv conv = {
				&pamconv,
				strdup(password.c_str())
			};
			
			pam_handle_t * pamh = nullptr;
			
			if(pam_start("su", name.c_str(), &conv, &pamh) == PAM_SUCCESS){
				
				if(pam_authenticate(pamh, 0) == PAM_SUCCESS) result = true;
				
				pam_end(pamh, PAM_SUCCESS);
			}
		}
	}
	
	return result;
}
 
const bool AUsers::Auth::checkFile(const uid_t uid, const string password){
	
	bool result = false;
	
	if((uid > 0) && !password.empty()){
		
		auto user = (reinterpret_cast <Users *> (this->users))->getDataById(uid, AUSERS_TYPE_FILE);
		
		if(user.uid > 0){
			 
			auto checkPassword = [](const string password1, const string password2){
				
				bool result = false;
				
				smatch match;
				
				
				
				
				
				
				regex e("(?:(CL)\\:(.+)|(MD5)\\:(\\w{32})|(SHA1)\\:(\\w{40})|(SHA256)\\:(\\w{64})|(SHA512)\\:(\\w{128}))", regex::ECMAScript | regex::icase);
				
				regex_search(password1, match, e);
				
				if(!match.empty()){
					
					if(!match[1].str().empty()) result = (password2.compare(match[2].str()) == 0);
					
					else if(!match[3].str().empty()) result = (Anyks::md5(password2).compare(match[4].str()) == 0);
					
					else if(!match[5].str().empty()) result = (Anyks::sha1(password2).compare(match[6].str()) == 0);
					
					else if(!match[7].str().empty()) result = (Anyks::sha256(password2).compare(match[8].str()) == 0);
					
					else if(!match[9].str().empty()) result = (Anyks::sha512(password2).compare(match[10].str()) == 0);
				}
				
				return result;
			};
			
			result = checkPassword(user.pass, user.name + password);
			
			if(!result && (this->groups != nullptr)){
				
				Groups * groups = reinterpret_cast <Groups *> (this->groups);
				
				if(groups != nullptr){
					
					auto gids = groups->getGroupIdByUser(user.uid, AUSERS_TYPE_FILE);
					
					if(!gids.empty()){
						
						for(auto gid = gids.cbegin(); gid != gids.cend(); ++gid){
							
							auto group = groups->getDataById(* gid, AUSERS_TYPE_FILE);
							
							if(!group.name.empty()){
								
								result = checkPassword(group.pass, group.name + password);
								
								if(result) break;
							}
						}
					}
				}
			}
		}
	}
	
	return result;
}
 
const bool AUsers::Auth::check(const string username, const string password){
	
	bool result = false;
	
	if(!username.empty() && !password.empty() && (this->users != nullptr) && this->config->auth.enabled){
		
		auto user = (reinterpret_cast <Users *> (this->users))->getDataByName(username);
		
		if(user.uid > 0){
			
			switch(user.type){
				
				case 0: result = checkFile(user.uid, password);	break;
				
				case 1: result = checkPam(user.uid, password);	break;
				
				case 2: result = checkLdap(user.uid, password);	break;
			}
		}
	
	} else if(!this->config->auth.enabled) result = true;
	
	return result;
}
 
AUsers::Auth::Auth(Config * config, LogApp * log, void * groups, void * users){
	
	if((config != nullptr) && (groups != nullptr) && (users != nullptr)){
		
		this->log = log;
		
		this->config = config;
		
		this->groups = groups;
		
		this->users = users;
		
		this->ldap = {
			"uid",
			"cn",
			"ou=users,dc=agro24,dc=dev",
			"ou=groups,dc=agro24,dc=dev",
			"one",
			"one",
			"(&(!(agro24CoJpDismissed=TRUE))(objectClass=inetOrgPerson))",
			"(objectClass=posixGroup)"
		};
	}
}
