/* SERVER PROXY AMING */
/*
*  author:     Yuriy Lobarev
*  phone:      +7(910)983-95-90
*  telegram:   @forman
*  email:      info@anyks.com
*  date:       11/23/2017 17:50:05
*  copyright:  © 2017 anyks.com
*/
 
 

#include "headers/ldap.h"

 
const string HLdap::getName(){
	
	string result;
	
	if(!this->names->empty()){
		
		result = * this->names->cbegin();
	}
	
	return result;
}
 
const vector <HLdap::Params> HLdap::getParams(){
	
	vector <Params> result;
	
	const string name = getName();
	
	if(!name.empty()){
		
		ALDAP ldap(this->config, this->log);
		
		const string dn = Anyks::strFormat("ah=%s,%s", name.c_str(), this->config->ldap.dn.headers.c_str());
		
		const string lParams =	"amingHeadersUser,amingHeadersGroup,amingHeadersAgent,"
								"amingHeadersAction,amingHeadersTraffic,amingHeadersMethod,"
								"amingHeadersPath,amingHeadersQuery,amingHeadersHeaders,"
								"amingHeadersClientDomain,amingHeadersClientMac,"
								"amingHeadersClientIp4,amingHeadersClientIp6,"
								"amingHeadersClientNetwork4,amingHeadersClientNetwork6,"
								"amingHeadersServerDomain,amingHeadersServerMac,"
								"amingHeadersServerIp4,amingHeadersServerIp6,"
								"amingHeadersServerNetwork4,amingHeadersServerNetwork6";
		
		auto data = ldap.data(dn, lParams, "base", "(objectClass=amingHeaders)");
		
		if(!data.empty()){
			
			for(auto it = data.cbegin(); it != data.cend(); ++it){
				
				const string str_all = "*";
				
				const vector <string> list_all = {str_all};
				
				Params params = {
					str_all,
					str_all,
					list_all,
					list_all,
					list_all,
					list_all,
					list_all,
					list_all,
					list_all,
					list_all,
					list_all
				};
				
				for(auto dt = it->vals.cbegin(); dt != it->vals.cend(); ++dt){
					
					if(!dt->second.empty()){
						
						if(dt->first.compare("amingHeadersUser") == 0) params.users = dt->second;
						
						else if(dt->first.compare("amingHeadersGroup") == 0) params.groups = dt->second;
						
						else if(dt->first.compare("amingHeadersMethod") == 0) params.methods = dt->second;
						
						else if(dt->first.compare("amingHeadersPath") == 0) params.paths = dt->second;
						
						else if(dt->first.compare("amingHeadersHeaders") == 0) params.headers = dt->second;
						
						else if(dt->first.compare("amingHeadersQuery") == 0) params.query = dt->second[0];
						
						else if(dt->first.compare("amingHeadersAgent") == 0) params.userAgent = dt->second[0];
						
						else if(dt->first.compare("amingHeadersAction") == 0){
							
							vector <string> actions;
							
							for(auto at = dt->second.cbegin(); at != dt->second.cend(); ++at){
								
								string action = * at;
								
								action = Anyks::toCase(action);
								
								if(action.compare("true") == 0)			actions.push_back("add");
								else if(action.compare("false") == 0)	actions.push_back("rm");
							}
							
							params.actions = actions;
						}
						
						else if(dt->first.compare("amingHeadersTraffic") == 0){
							
							vector <string> traffic;
							
							for(auto tt = dt->second.cbegin(); tt != dt->second.cend(); ++tt){
								
								string param = * tt;
								
								param = Anyks::toCase(param);
								
								if(param.compare("true") == 0)			traffic.push_back("in");
								else if(param.compare("false") == 0)	traffic.push_back("out");
							}
							
							params.traffic = traffic;
						}
						
						else if((dt->first.compare("amingHeadersClientDomain") == 0)
						|| (dt->first.compare("amingHeadersClientMac") == 0)
						|| (dt->first.compare("amingHeadersClientIp4") == 0)
						|| (dt->first.compare("amingHeadersClientIp6") == 0)
						|| (dt->first.compare("amingHeadersClientNetwork4") == 0)
						|| (dt->first.compare("amingHeadersClientNetwork6") == 0)){
							
							if(find(params.clients.begin(), params.clients.end(), str_all) != params.clients.end()){
								
								params.clients.clear();
							}
							
							for(auto dom = dt->second.cbegin(); dom != dt->second.cend(); ++dom){
								
								params.clients.push_back(* dom);
							}
						}
						
						else if((dt->first.compare("amingHeadersServerDomain") == 0)
						|| (dt->first.compare("amingHeadersServerMac") == 0)
						|| (dt->first.compare("amingHeadersServerIp4") == 0)
						|| (dt->first.compare("amingHeadersServerIp6") == 0)
						|| (dt->first.compare("amingHeadersServerNetwork4") == 0)
						|| (dt->first.compare("amingHeadersServerNetwork6") == 0)){
							
							if(find(params.servers.begin(), params.servers.end(), str_all) != params.servers.end()){
								
								params.servers.clear();
							}
							
							for(auto dom = dt->second.cbegin(); dom != dt->second.cend(); ++dom){
								
								params.servers.push_back(* dom);
							}
						}
					}
				}
				
				result.push_back(params);
			}
		}
	
	} else if(this->log != nullptr) this->log->write(LOG_ERROR, 0, "headers params is not found");
	
	return result;
}
 
HLdap::HLdap(Config * config, LogApp * log, deque <string> * names){
	
	if((config != nullptr) && (names != nullptr)){
		
		this->log = log;
		
		this->config = config;
		
		this->names = names;
	}
}
