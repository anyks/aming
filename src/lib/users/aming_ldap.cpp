/* SERVER PROXY AMING */
/*
*  author:     Yuriy Lobarev
*  phone:      +7(910)983-95-90
*  telegram:   @forman
*  email:      info@anyks.com
*  date:       11/23/2017 17:50:05
*  copyright:  © 2017 anyks.com
*/
 
 

#include "users/ldap.h"


using namespace std;

 
const AParams::Params Uldap::createDefaultParams(const uid_t uid, const u_short type){
	
	AParams::Params result;
	
	if(uid > 0){
		
		auto groups = this->getGroupIdByUser(uid, type);
		
		if(!groups.empty()){
			
			gid_t gid = 1000000;
			
			for(auto it = groups.cbegin(); it != groups.cend(); ++it){
				
				if((* it) < gid) gid = (* it);
			}
			
			auto * params = this->getParamsByGid(gid, AUSERS_TYPE_LDAP);
			
			if(params != nullptr){
				
				result.id = uid;
				
				result.options	= params->options;
				result.ipv4		= {params->ipv4.ip, params->ipv4.resolver};
				result.ipv6		= {params->ipv6.ip, params->ipv6.resolver};
				
				result.gzip = {
					params->gzip.vary,
					params->gzip.level,
					params->gzip.length,
					params->gzip.chunk,
					params->gzip.regex,
					params->gzip.vhttp,
					params->gzip.proxied,
					params->gzip.types
				};
				
				result.proxy = {
					params->proxy.reverse,
					params->proxy.transfer,
					params->proxy.forward,
					params->proxy.subnet,
					params->proxy.pipelining,
					params->proxy.redirect
				};
				
				result.connects = {
					params->connects.size,
					params->connects.connect
				};
				
				result.timeouts = {
					params->timeouts.read,
					params->timeouts.write,
					params->timeouts.upgrade
				};
				
				result.buffers = {
					params->buffers.read,
					params->buffers.write
				};
				
				result.keepalive = {
					params->keepalive.keepcnt,
					params->keepalive.keepidle,
					params->keepalive.keepintvl
				};
				
				result.idnt.assign(params->idnt.cbegin(), params->idnt.cend());
			}
		}
	
	} else result = {
		
		uid,
		
		this->config->options, {},
		
		{this->config->ipv4.external, this->config->ipv4.resolver},
		
		{this->config->ipv6.external, this->config->ipv6.resolver},{
		
			this->config->gzip.vary,
			this->config->gzip.level,
			this->config->gzip.length,
			this->config->gzip.chunk,
			this->config->gzip.regex,
			this->config->gzip.vhttp,
			this->config->gzip.proxied,
			this->config->gzip.types
		
		},{
			this->config->proxy.reverse,
			this->config->proxy.transfer,
			this->config->proxy.forward,
			this->config->proxy.subnet,
			this->config->proxy.pipelining,
			this->config->proxy.redirect
		
		},{
			this->config->connects.size,
			this->config->connects.connect
		},{
		
			this->config->timeouts.read,
			this->config->timeouts.write,
			this->config->timeouts.upgrade
		
		},{
			this->config->buffers.read,
			this->config->buffers.write
		
		},{
			this->config->keepalive.keepcnt,
			this->config->keepalive.keepidle,
			this->config->keepalive.keepintvl
		}
	};
	
	return result;
}
 
const AParams::Params Uldap::setParams(const uid_t uid, const string name){
	
	AParams::Params params = createDefaultParams(uid);
	
	if((uid > 0) && !name.empty()){
		
		ALDAP ldap(this->config, this->log);
		
		const string dn = Anyks::strFormat("ac=%s,%s", name.c_str(), this->config->ldap.dn.configs.c_str());
		
		const string filter = Anyks::strFormat("(&(objectClass=amingConfigs)(amingConfigsType=users)(%s=%u))", "amingConfigsUserId", uid);
		
		const string keys = "amingConfigsConnectsConnect,amingConfigsConnectsSize,amingConfigsGzipChunk,"
							"amingConfigsGzipLength,amingConfigsGzipLevel,amingConfigsGzipProxied,"
							"amingConfigsGzipRegex,amingConfigsGzipResponse,amingConfigsGzipTransfer,"
							"amingConfigsGzipTypes,amingConfigsGzipVary,amingConfigsGzipVhttp,amingConfigsAuth,"
							"amingConfigsIdnt,amingConfigsIpExternal4,amingConfigsIpExternal6,amingConfigsRedirect,"
							"amingConfigsIpResolver4,amingConfigsIpResolver6,amingConfigsKeepAliveCnt,"
							"amingConfigsKeepAliveEnabled,amingConfigsKeepAliveIdle,amingConfigsKeepAliveIntvl,"
							"amingConfigsProxyAgent,amingConfigsProxyConnect,amingConfigsProxyDeblock,"
							"amingConfigsProxyForward,amingConfigsProxyPipelining,amingConfigsProxyReverse,"
							"amingConfigsProxySkill,amingConfigsProxySubnet,amingConfigsProxyTransfer,"
							"amingConfigsProxyUpgrade,amingConfigsSpeedInput,amingConfigsSpeedOutput,"
							"amingConfigsTimeoutsRead,amingConfigsTimeoutsUpgrade,amingConfigsTimeoutsWrite";
		
		auto users = ldap.data(dn, keys, "base", filter);
		
		if(!users.empty()){
			 
			auto getBoolean = [](const string value){
				
				bool check = false;
				
				string param = value;
				
				param = Anyks::toCase(param);
				
				if(param.compare("true") == 0) check = true;
				
				return check;
			};
			
			for(auto it = users.cbegin(); it != users.cend(); ++it){
				
				for(auto dt = it->vals.cbegin(); dt != it->vals.cend(); ++dt){
					
					if(!dt->second.empty()){
						
						if(dt->first.compare("amingConfigsIdnt") == 0) params.idnt = dt->second;
						
						else if(dt->first.compare("amingConfigsIpExternal4") == 0) params.ipv4.ip = dt->second;
						
						else if(dt->first.compare("amingConfigsIpExternal6") == 0) params.ipv6.ip = dt->second;
						
						else if(dt->first.compare("amingConfigsIpResolver4") == 0) params.ipv4.resolver = dt->second;
						
						else if(dt->first.compare("amingConfigsIpResolver6") == 0) params.ipv6.resolver = dt->second;
						
						else if(dt->first.compare("amingConfigsRedirect") == 0) params.proxy.redirect = dt->second;
						
						else if(dt->first.compare("amingConfigsProxyConnect") == 0){
							
							Anyks::setOptions(OPT_CONNECT, params.options, getBoolean(dt->second[0]));
						
						} else if(dt->first.compare("amingConfigsProxyUpgrade") == 0){
							
							Anyks::setOptions(OPT_UPGRADE, params.options, getBoolean(dt->second[0]));
						
						} else if(dt->first.compare("amingConfigsProxyAgent") == 0){
							
							Anyks::setOptions(OPT_AGENT, params.options, getBoolean(dt->second[0]));
						
						} else if(dt->first.compare("amingConfigsProxyDeblock") == 0){
							
							Anyks::setOptions(OPT_DEBLOCK, params.options, getBoolean(dt->second[0]));
						
						} else if(dt->first.compare("amingConfigsGzipTransfer") == 0){
							
							Anyks::setOptions(OPT_GZIP, params.options, getBoolean(dt->second[0]));
						
						} else if(dt->first.compare("amingConfigsGzipResponse") == 0){
							
							Anyks::setOptions(OPT_PGZIP, params.options, getBoolean(dt->second[0]));
						
						} else if(dt->first.compare("amingConfigsKeepAliveEnabled") == 0){
							
							Anyks::setOptions(OPT_KEEPALIVE, params.options, getBoolean(dt->second[0]));
						
						} else if(dt->first.compare("amingConfigsProxySkill") == 0){
							
							const bool check = (dt->second[0].compare("smart") == 0);
							
							Anyks::setOptions(OPT_SMART, params.options, check);
						
						} else if(dt->first.compare("amingConfigsTimeoutsRead") == 0){
							
							params.timeouts.read = (size_t) Anyks::getSeconds(dt->second[0]);
						
						} else if(dt->first.compare("amingConfigsTimeoutsWrite") == 0){
							
							params.timeouts.write = (size_t) Anyks::getSeconds(dt->second[0]);
						
						} else if(dt->first.compare("amingConfigsTimeoutsUpgrade") == 0){
							
							params.timeouts.upgrade = (size_t) Anyks::getSeconds(dt->second[0]);
						
						} else if(dt->first.compare("amingConfigsSpeedInput") == 0){
							
							params.buffers.read = Anyks::getSizeBuffer(dt->second[0]);
						
						} else if(dt->first.compare("amingConfigsSpeedOutput") == 0){
							
							params.buffers.write = Anyks::getSizeBuffer(dt->second[0]);
						
						} else if(dt->first.compare("amingConfigsKeepAliveCnt") == 0){
							
							params.keepalive.keepcnt = (int) ::atoi(dt->second[0].c_str());
						
						} else if(dt->first.compare("amingConfigsKeepAliveIdle") == 0){
							
							params.keepalive.keepidle = (int) ::atoi(dt->second[0].c_str());
						
						} else if(dt->first.compare("amingConfigsKeepAliveIntvl") == 0){
							
							params.keepalive.keepintvl = (int) ::atoi(dt->second[0].c_str());
						
						} else if(dt->first.compare("amingConfigsConnectsSize") == 0){
							
							params.connects.size = Anyks::getBytes(dt->second[0]);
						
						} else if(dt->first.compare("amingConfigsConnectsConnect") == 0){
							
							params.connects.connect = (u_int) ::atoi(dt->second[0].c_str());
						
						} else if(dt->first.compare("amingConfigsProxySubnet") == 0){
							
							params.proxy.subnet = getBoolean(dt->second[0]);
						
						} else if(dt->first.compare("amingConfigsProxyReverse") == 0){
							
							params.proxy.reverse = getBoolean(dt->second[0]);
						
						} else if(dt->first.compare("amingConfigsProxyForward") == 0){
							
							params.proxy.forward = getBoolean(dt->second[0]);
						
						} else if(dt->first.compare("amingConfigsProxyTransfer") == 0){
							
							params.proxy.transfer = getBoolean(dt->second[0]);
						
						} else if(dt->first.compare("amingConfigsProxyPipelining") == 0){
							
							params.proxy.pipelining = getBoolean(dt->second[0]);
						
						} else if(dt->first.compare("amingConfigsGzipRegex") == 0){
							
							params.gzip.regex = dt->second[0];
						
						} else if(dt->first.compare("amingConfigsGzipVary") == 0){
							
							params.gzip.vary = getBoolean(dt->second[0]);
						
						} else if(dt->first.compare("amingConfigsGzipLength") == 0){
							
							params.gzip.length = (int) ::atoi(dt->second[0].c_str());
						
						} else if(dt->first.compare("amingConfigsGzipChunk") == 0){
							
							params.gzip.chunk = Anyks::getBytes(dt->second[0]);
						
						} else if(dt->first.compare("amingConfigsGzipVhttp") == 0){
							
							params.gzip.vhttp = dt->second;
						
						} else if(dt->first.compare("amingConfigsGzipTypes") == 0){
							
							params.gzip.types = dt->second;
						
						} else if(dt->first.compare("amingConfigsGzipProxied") == 0){
							
							params.gzip.proxied = dt->second;
						
						} else if(dt->first.compare("amingConfigsGzipLevel") == 0){
							
							const string gzipLevel = dt->second[0];
							
							if(!gzipLevel.empty()){
								
								if(gzipLevel.compare("default") == 0)		params.gzip.level = Z_DEFAULT_COMPRESSION;
								else if(gzipLevel.compare("best") == 0)		params.gzip.level = Z_BEST_COMPRESSION;
								else if(gzipLevel.compare("speed") == 0)	params.gzip.level = Z_BEST_SPEED;
								else if(gzipLevel.compare("no") == 0)		params.gzip.level = Z_NO_COMPRESSION;
							}
						}
					}
				}
			}
		}
	}
	
	return params;
}
 
const vector <AParams::User> Uldap::readUsers(){
	
	vector <AParams::User> result = {{0, "", "", ""}};
	
	ALDAP ldap(this->config, this->log);
	
	const string keys = Anyks::strFormat(
		"%s,%s,%s,%s,%s,%s,%s",
		this->config->ldap.keys.users.uid.c_str(),
		this->config->ldap.keys.users.login.c_str(),
		this->config->ldap.keys.users.firstname.c_str(),
		this->config->ldap.keys.users.lastname.c_str(),
		this->config->ldap.keys.users.secondname.c_str(),
		this->config->ldap.keys.users.desc.c_str(),
		this->config->ldap.keys.users.pass.c_str()
	);
	
	auto users = ldap.data(this->config->ldap.dn.users, keys, this->config->ldap.scope.users, this->config->ldap.filter.users);
	
	if(!users.empty()){
		
		for(auto it = users.cbegin(); it != users.cend(); ++it){
			
			uid_t uid;
			
			string name, desc, pass;
			
			string fname, lname, sname;
			
			for(auto dt = it->vals.cbegin(); dt != it->vals.cend(); ++dt){
				
				if(!dt->second.empty()){
					
					if(dt->first.compare(this->config->ldap.keys.users.login) == 0) name = dt->second[0];
					
					else if(dt->first.compare(this->config->ldap.keys.users.uid) == 0) uid = ::atoi(dt->second[0].c_str());
					
					else if(dt->first.compare(this->config->ldap.keys.users.desc) == 0) desc = dt->second[0];
					
					else if(dt->first.compare(this->config->ldap.keys.users.pass) == 0) pass = dt->second[0];
					
					else if(dt->first.compare(this->config->ldap.keys.users.firstname) == 0) fname = dt->second[0];
					
					else if(dt->first.compare(this->config->ldap.keys.users.lastname) == 0) lname = dt->second[0];
					
					else if(dt->first.compare(this->config->ldap.keys.users.secondname) == 0) sname = dt->second[0];
				}
			}
			
			desc = (!fname.empty() ? (!lname.empty() ? (!sname.empty() ?
				Anyks::strFormat("%s %s %s", lname.c_str(), fname.c_str(), sname.c_str()) :
				Anyks::strFormat("%s %s", fname.c_str(), lname.c_str())) : fname) : !desc.empty() ? desc : name
			);
			
			if((uid > 0) && !name.empty()) result.push_back({uid, name, desc, pass});
		}
	}
	
	return result;
}
 
const vector <AParams::UserData> Uldap::getAllUsers(){
	
	vector <AParams::UserData> result;
	
	ALDAP ldap(this->config, this->log);
	
	const string keys = Anyks::strFormat(
		"%s,%s,%s,%s,%s,%s,%s",
		this->config->ldap.keys.users.uid.c_str(),
		this->config->ldap.keys.users.login.c_str(),
		this->config->ldap.keys.users.firstname.c_str(),
		this->config->ldap.keys.users.lastname.c_str(),
		this->config->ldap.keys.users.secondname.c_str(),
		this->config->ldap.keys.users.desc.c_str(),
		this->config->ldap.keys.users.pass.c_str()
	);
	
	auto users = ldap.data(this->config->ldap.dn.users, keys, this->config->ldap.scope.users, this->config->ldap.filter.users);
	
	if(!users.empty()){
		
		for(auto it = users.cbegin(); it != users.cend(); ++it){
			
			uid_t uid;
			
			string name, desc, pass;
			
			string fname, lname, sname;
			
			for(auto dt = it->vals.cbegin(); dt != it->vals.cend(); ++dt){
				
				if(!dt->second.empty()){
					
					if(dt->first.compare(this->config->ldap.keys.users.login) == 0) name = dt->second[0];
					
					else if(dt->first.compare(this->config->ldap.keys.users.uid) == 0) uid = ::atoi(dt->second[0].c_str());
					
					else if(dt->first.compare(this->config->ldap.keys.users.desc) == 0) desc = dt->second[0];
					
					else if(dt->first.compare(this->config->ldap.keys.users.pass) == 0) pass = dt->second[0];
					
					else if(dt->first.compare(this->config->ldap.keys.users.firstname) == 0) fname = dt->second[0];
					
					else if(dt->first.compare(this->config->ldap.keys.users.lastname) == 0) lname = dt->second[0];
					
					else if(dt->first.compare(this->config->ldap.keys.users.secondname) == 0) sname = dt->second[0];
				}
			}
			
			desc = (!fname.empty() ? (!lname.empty() ? (!sname.empty() ?
				Anyks::strFormat("%s %s %s", lname.c_str(), fname.c_str(), sname.c_str()) :
				Anyks::strFormat("%s %s", fname.c_str(), lname.c_str())) : fname) : !desc.empty() ? desc : name
			);
			
			if((uid > 0) && !name.empty()) result.push_back({uid, AUSERS_TYPE_LDAP, name, desc, pass});
		}
	}
	
	return result;
}
 
const AParams::UserData Uldap::getDataById(const uid_t uid){
	
	AParams::UserData result;
	
	if(uid > 0){
		
		ALDAP ldap(this->config, this->log);
		
		const string filter = Anyks::strFormat("(&%s(%s=%u))", this->config->ldap.filter.users.c_str(), this->config->ldap.keys.users.uid.c_str(), uid);
		
		const string keys = Anyks::strFormat(
			"%s,%s,%s,%s,%s,%s",
			this->config->ldap.keys.users.login.c_str(),
			this->config->ldap.keys.users.firstname.c_str(),
			this->config->ldap.keys.users.lastname.c_str(),
			this->config->ldap.keys.users.secondname.c_str(),
			this->config->ldap.keys.users.desc.c_str(),
			this->config->ldap.keys.users.pass.c_str()
		);
		
		auto users = ldap.data(this->config->ldap.dn.users, keys, this->config->ldap.scope.users, filter);
		
		if(!users.empty()){
			
			for(auto it = users.cbegin(); it != users.cend(); ++it){
				
				string name, desc, pass;
				
				string fname, lname, sname;
				
				for(auto dt = it->vals.cbegin(); dt != it->vals.cend(); ++dt){
					
					if(!dt->second.empty()){
						
						if(dt->first.compare(this->config->ldap.keys.users.login) == 0) name = dt->second[0];
						
						else if(dt->first.compare(this->config->ldap.keys.users.desc) == 0) desc = dt->second[0];
						
						else if(dt->first.compare(this->config->ldap.keys.users.pass) == 0) pass = dt->second[0];
						
						else if(dt->first.compare(this->config->ldap.keys.users.firstname) == 0) fname = dt->second[0];
						
						else if(dt->first.compare(this->config->ldap.keys.users.lastname) == 0) lname = dt->second[0];
						
						else if(dt->first.compare(this->config->ldap.keys.users.secondname) == 0) sname = dt->second[0];
					}
				}
				
				desc = (!fname.empty() ? (!lname.empty() ? (!sname.empty() ?
					Anyks::strFormat("%s %s %s", lname.c_str(), fname.c_str(), sname.c_str()) :
					Anyks::strFormat("%s %s", fname.c_str(), lname.c_str())) : fname) : !desc.empty() ? desc : name
				);
				
				result = {uid, AUSERS_TYPE_LDAP, name, desc, pass};
				
				break;
			}
		}
	}
	
	return result;
}
 
const bool Uldap::checkUserById(const uid_t uid){
	
	return !(getNameById(uid)).empty();
}
 
const bool Uldap::checkUserByName(const string userName){
	
	return (getIdByName(userName) > 0 ? true : false);
}
 
const bool Uldap::auth(const uid_t uid, const string password){
	
	bool result = false;
	
	if((uid > 0) && !password.empty()){
		
		const string filter = Anyks::strFormat("(&%s(%s=%u))", this->config->ldap.filter.users.c_str(), this->config->ldap.keys.users.uid.c_str(), uid);
		
		ALDAP ldap(this->config, this->log);
		
		result = ldap.checkAuth(this->config->ldap.dn.users, password, this->config->ldap.scope.users, filter);
		
		if(!result){
			
			auto gids = this->getGroupIdByUser(uid, AUSERS_TYPE_LDAP);
			
			if(!gids.empty()){
				
				for(auto gid = gids.cbegin(); gid != gids.cend(); ++gid){
					
					auto group = this->getDataByGid(* gid, AUSERS_TYPE_LDAP);
					
					if(!group.name.empty()){
						
						const string dn = Anyks::strFormat("%s=%s,%s", this->config->ldap.keys.groups.login.c_str(), Anyks::toCase(group.name).c_str(), this->config->ldap.dn.groups.c_str());
						
						result = ldap.checkAuth(dn, password, "base", this->config->ldap.filter.groups);
						
						if(result) break;
					}
				}
			}
		}
	}
	
	return result;
}
 
const uid_t Uldap::getIdByName(const string userName){
	
	uid_t result = 0;
	
	if(!userName.empty()){
		
		ALDAP ldap(this->config, this->log);
		
		const string dn = Anyks::strFormat("%s=%s,%s", this->config->ldap.keys.users.login.c_str(), userName.c_str(), this->config->ldap.dn.users.c_str());
		
		auto users = ldap.data(dn, this->config->ldap.keys.users.uid, "base", this->config->ldap.filter.users);
		
		if(!users.empty()){
			
			for(auto it = users.cbegin(); it != users.cend(); ++it){
				
				for(auto dt = it->vals.cbegin(); dt != it->vals.cend(); ++dt){
					
					if(!dt->second.empty() && (dt->first.compare(this->config->ldap.keys.users.uid) == 0)){
						
						return ::atoi(dt->second[0].c_str());
					}
				}
			}
		}
	}
	
	return result;
}
 
const string Uldap::getNameById(const uid_t uid){
	
	string result;
	
	if(uid > 0){
		
		ALDAP ldap(this->config, this->log);
		
		const string filter = Anyks::strFormat("(&%s(%s=%u))", this->config->ldap.filter.users.c_str(), this->config->ldap.keys.users.uid.c_str(), uid);
		
		auto users = ldap.data(this->config->ldap.dn.users, this->config->ldap.keys.users.login, this->config->ldap.scope.users, filter);
		
		if(!users.empty()){
			
			for(auto it = users.cbegin(); it != users.cend(); ++it){
				
				for(auto dt = it->vals.cbegin(); dt != it->vals.cend(); ++dt){
					
					if(!dt->second.empty() && (dt->first.compare(this->config->ldap.keys.users.login) == 0)) return dt->second[0];
				}
			}
		}
	}
	
	return result;
}
 
void Uldap::setGroupsMethods(function <string (gid_t, u_short)> method1, function <gid_t (string, u_short)> method2){
	
	this->getGroupNameByGid = method1;
	
	this->getGidByGroupName = method2;
}
 
void Uldap::setParamsMethod(function <const AParams::Params * (const gid_t gid, const u_short type)> method){
	
	this->getParamsByGid = method;
}
 
void Uldap::setGidsMethod(function <const vector <gid_t> (const uid_t uid, const u_short type)> method){
	
	this->getGroupIdByUser = method;
}
 
void Uldap::setGroupDataMethod(function <const AParams::GroupData (const gid_t gid, const u_short type)> method){
	
	this->getDataByGid = method;
}
 
Uldap::Uldap(Config * config, LogApp * log){
	
	if(config != nullptr){
		
		this->log = log;
		
		this->config = config;
	}
}
