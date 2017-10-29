/* SERVER PROXY AMING */
/*
*  author:     Yuriy Lobarev
*  phone:      +7(910)983-95-90
*  telegram:   @forman
*  email:      info@anyks.com
*  date:       10/29/2017 17:06:00
*  copyright:  © 2017 anyks.com
*/
 
 

#include "ldap/ldap.h"


using namespace std;

 
const bool AuthLDAP::authLDAP(LDAP * ld, const string dn, const string password){
	
	int rc = ldap_set_option(ld, LDAP_OPT_PROTOCOL_VERSION, &this->version);
	
	if(rc != LDAP_SUCCESS){
		
		if(this->log) this->log->write(LOG_ERROR, 0, "set version ldap filed: %s", ldap_err2string(rc));
		
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
		
		if(this->log) this->log->write(LOG_ERROR, 0, "ldap auth filed: %s", ldap_err2string(rc));
		
		return false;
	}
	
	return true;
}
 
const bool AuthLDAP::checkUser(const string user, const string password){
	
	bool auth = false;
	
	if(this->enabled){
		
		const char * key = "%v";
		
		size_t pos = this->filter.find(key);
		
		if(pos != string::npos){
			
			char * dn = nullptr;
			
			LDAP * ld = nullptr, * uld = nullptr;
			
			if(ldap_initialize(&ld, this->server.c_str())){
				
				if(this->log) this->log->write(LOG_ERROR, 0, "ldap initialize filed");
				
				return auth;
			}
			
			if(authLDAP(ld, this->config->ldap.binddn, this->config->ldap.bindpw)){
				
				LDAPMessage * result, * e;
				
				string filter = this->filter;
				
				filter = filter.replace(pos, strlen(key), user);
				
				int rc = ldap_search_ext_s(ld, this->userdn.c_str(), this->scope, filter.c_str(), nullptr, 0, nullptr, nullptr, nullptr, 0, &result);
				
				if(rc != LDAP_SUCCESS){
					
					if(this->log) this->log->write(LOG_ERROR, 0, "ldap search filed: %s", ldap_err2string(rc));
					
					ldap_unbind_ext(ld, nullptr, nullptr);
					
					return auth;
				}
				
				for(e = ldap_first_entry(ld, result); e != nullptr; e = ldap_next_entry(ld, e)){
					
					if((dn = ldap_get_dn(ld, e)) != nullptr){
						
						if(ldap_initialize(&uld, this->server.c_str())){
							
							if(this->log) this->log->write(LOG_ERROR, 0, "ldap user initialize filed");
							
							ldap_unbind_ext(ld, nullptr, nullptr);
							
							return auth;
						}
						
						auth = authLDAP(uld, dn, password);
						
						ldap_unbind_ext(uld, nullptr, nullptr);
						
						ldap_memfree(dn);
						
						if(auth) break;
					}
				}
				
				if(result) ldap_msgfree(result);
			}
			
			ldap_unbind_ext(ld, nullptr, nullptr);
		}
	}
	
	return auth;
}
 
AuthLDAP::AuthLDAP(Config * config, LogApp * log){
	
	if(config && config->ldap.enabled
	&& !config->ldap.server.empty()){
		
		this->log = log;
		
		this->config = config;
		
		this->server = this->config->ldap.server;
		
		this->userdn = this->config->ldap.userdn;
		
		this->filter = this->config->ldap.filter;
		
		this->enabled = this->config->ldap.enabled;
		
		switch(this->config->ldap.version){
			
			case 3: this->version = LDAP_VERSION3; break;
			
			default: this->version = LDAP_VERSION2;
		}
		
		if(this->config->ldap.scope.compare("sub") == 0)		this->scope = LDAP_SCOPE_SUBTREE;
		else if(this->config->ldap.scope.compare("one") == 0)	this->scope = LDAP_SCOPE_ONELEVEL;
		else if(this->config->ldap.scope.compare("base") == 0)	this->scope = LDAP_SCOPE_BASE;
		
		LDAP * ld = nullptr;
		
		if(ldap_initialize(&ld, this->server.c_str())){
			
			this->enabled = false;
			
			if(this->log) this->log->write(LOG_ERROR, 0, "ldap initialize filed");
		
		} else {
			
			this->enabled = authLDAP(ld, this->config->ldap.binddn, this->config->ldap.bindpw);
			
			ldap_unbind_ext(ld, nullptr, nullptr);
		}
	}
}
