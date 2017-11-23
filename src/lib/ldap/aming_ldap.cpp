/* SERVER PROXY AMING */
/*
*  author:     Yuriy Lobarev
*  phone:      +7(910)983-95-90
*  telegram:   @forman
*  email:      info@anyks.com
*  date:       11/23/2017 17:50:05
*  copyright:  © 2017 anyks.com
*/
 
 

#include "ldap/ldap.h"


using namespace std;

 
const u_int ALDAP::getScope(const string scope){
	
	u_int result = LDAP_SCOPE_SUBTREE;
	
	if(!scope.empty()){
		
		if(scope.compare("sub") == 0)		result = LDAP_SCOPE_SUBTREE;
		else if(scope.compare("one") == 0)	result = LDAP_SCOPE_ONELEVEL;
		else if(scope.compare("base") == 0)	result = LDAP_SCOPE_BASE;
    }
	
	return result;
}
 
const bool ALDAP::auth(LDAP * ld, const string dn, const string password){
	
	int rc = ldap_set_option(ld, LDAP_OPT_PROTOCOL_VERSION, &this->version);
	
	if(rc != LDAP_SUCCESS){
		
		if(this->log != nullptr) this->log->write(LOG_ERROR, 0, "set version ldap filed: %s", ldap_err2string(rc));
		
		return false;
	}
	
	#if defined(LDAP_API_FEATURE_X_OPENLDAP) && LDAP_API_FEATURE_X_OPENLDAP >= 20300
		
		struct berval cred = {0, nullptr};
		
		cred.bv_len = password.length();
		
		cred.bv_val = new char[cred.bv_len + 1];
		
		memset(cred.bv_val, 0, cred.bv_len);
		
		strcpy(cred.bv_val, password.c_str());
		
		rc = ldap_sasl_bind_s(ld, dn.c_str(), LDAP_SASL_SIMPLE, &cred, nullptr, nullptr, nullptr);
		
		delete [] cred.bv_val;
		
		memset(&cred, 0, sizeof(cred));
	
	#else
		
		rc = ldap_simple_bind_s(ld, dn.c_str(), password.c_str());
	#endif
	
	if(rc != LDAP_SUCCESS){
		
		if(this->log != nullptr) this->log->write(LOG_ERROR, 0, "ldap auth filed: %s", ldap_err2string(rc));
		
		return false;
	}
	
	return true;
}
 
const string ALDAP::getServer(){
	
	if(!this->servers.empty()){
		
		if(this->servers.size() == 1) return this->servers[0];
		
		else {
			
			srand(time(0));
			
			return this->servers[rand() % this->servers.size()];
		}
	}
	
	return string();
}
 
const bool ALDAP::checkAuth(const string dn, const string password, const string scope, const string filter){
	
	bool auth = false;
	
	if(!dn.empty() && !password.empty() && !scope.empty() && !filter.empty()){
		
		LDAP * ld = nullptr, * uld = nullptr;
		
		if(ldap_initialize(&ld, getServer().c_str())){
			
			if(this->log != nullptr) this->log->write(LOG_ERROR, 0, "ldap initialize filed");
			
			return auth;
		}
		
		if(this->auth(ld, this->config->ldap.binddn, this->config->ldap.bindpw)){
			
			char * udn = nullptr;
			
			LDAPMessage * result, * e;
			
			const u_int cscope = getScope(scope);
			
			int rc = ldap_search_ext_s(ld, dn.c_str(), cscope, filter.c_str(), nullptr, 0, nullptr, nullptr, nullptr, 0, &result);
			
			if(rc != LDAP_SUCCESS){
				
				if(this->log != nullptr) this->log->write(LOG_ERROR, 0, "ldap search filed: %s", ldap_err2string(rc));
				
				ldap_unbind_ext(ld, nullptr, nullptr);
				
				return auth;
			}
			
			for(e = ldap_first_entry(ld, result); e != nullptr; e = ldap_next_entry(ld, e)){
				
				if((udn = ldap_get_dn(ld, e)) != nullptr){
					
					if(ldap_initialize(&uld, getServer().c_str())){
						
						if(this->log != nullptr) this->log->write(LOG_ERROR, 0, "ldap user initialize filed");
						
						ldap_unbind_ext(ld, nullptr, nullptr);
						
						return auth;
					}
					
					auth = this->auth(uld, udn, password);
					
					ldap_unbind_ext(uld, nullptr, nullptr);
					
					ldap_memfree(udn);
					
					if(auth) break;
				}
			}
			
			if(result) ldap_msgfree(result);
		}
		
		ldap_unbind_ext(ld, nullptr, nullptr);
	}
	
	return auth;
}
 
const vector <ALDAP::Data> ALDAP::data(const string dn, const string key, const string scope, const string filter){
	
	vector <Data> data;
	
	if(!dn.empty() && !filter.empty() && !scope.empty() && !key.empty()){
		
		LDAP * ld = nullptr;
		
		if(ldap_initialize(&ld, getServer().c_str())){
			
			if(this->log != nullptr) this->log->write(LOG_ERROR, 0, "ldap initialize filed");
			
			return data;
		}
		
		if(auth(ld, this->config->ldap.binddn, this->config->ldap.bindpw)){
			
			LDAPMessage * result, * e;
			
			const u_int cscope = getScope(scope);
			
			int rc = ldap_search_ext_s(ld, dn.c_str(), cscope, filter.c_str(), nullptr, 0, nullptr, nullptr, nullptr, 0, &result);
			
			if(rc != LDAP_SUCCESS){
				
				if(this->log != nullptr) this->log->write(LOG_ERROR, 0, "ldap search filed: %s", ldap_err2string(rc));
				
				ldap_unbind_ext(ld, nullptr, nullptr);
				
				return data;
			}
			
			char * cdn = nullptr;
			
			for(e = ldap_first_entry(ld, result); e != nullptr; e = ldap_next_entry(ld, e)){
				
				if((cdn = ldap_get_dn(ld, e)) != nullptr){
					
					unordered_map <string, vector <string>> vals;
					
					vector <string> keys = Anyks::split(key, ",");
					
					for(auto it = keys.cbegin(); it != keys.cend(); ++it){
						
						const string key = * it;
						
						if(!key.empty()){
							
							char ** params = ldap_get_values(ld, e, key.c_str());
							
							for(u_int i = 0; i < ldap_count_values(params); i++){
								
								if(vals.count(key)){
									
									vals.find(key)->second.push_back(params[i]);
								
								} else vals.insert(pair <string, vector <string>>(key, {params[i]}));
							}
						}
					}
					
					data.push_back({cdn, key, vals});
					
					ldap_memfree(cdn);
				}
			}
			
			if(result) ldap_msgfree(result);
		}
		
		ldap_unbind_ext(ld, nullptr, nullptr);
	}
	
	return data;
}
 
void ALDAP::setServer(const string server){
	
	if(!server.empty()) this->servers.push_back(server);
}
 
ALDAP::ALDAP(Config * config, LogApp * log){
	
	if((config != nullptr) && !config->ldap.servers.empty()){
		
		this->log = log;
		
		this->config = config;
		
		this->servers.assign(this->config->ldap.servers.cbegin(), this->config->ldap.servers.cend());
		
		switch(this->config->ldap.version){
			
			case 3: this->version = LDAP_VERSION3; break;
			
			default: this->version = LDAP_VERSION2;
		}
		
		LDAP * ld = nullptr;
		
		if(ldap_initialize(&ld, getServer().c_str())){
			
			if(this->log != nullptr) this->log->write(LOG_ERROR, 0, "ldap initialize filed");
		
		} else {
			
			if(auth(ld, this->config->ldap.binddn, this->config->ldap.bindpw)){
				
				ldap_unbind_ext(ld, nullptr, nullptr);
			}
		}
	}
}
