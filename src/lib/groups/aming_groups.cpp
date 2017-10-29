/* SERVER PROXY AMING */
/*
*  author:     Yuriy Lobarev
*  phone:      +7(910)983-95-90
*  telegram:   @forman
*  email:      info@anyks.com
*  date:       10/29/2017 16:59:43
*  copyright:  © 2017 anyks.com
*/
 
 

#include "ausers/ausers.h"


using namespace std;

 
void AUsers::Groups::setProxyOptions(const u_short option, u_short &proxyOptions, const bool flag){
	
	u_short options = proxyOptions;
	
	if(flag) options = options | option;
	
	else {
		
		options = options ^ option;
		
		if(options > proxyOptions) options = proxyOptions;
	}
	
	proxyOptions = options;
}
 
void AUsers::Groups::setDataGroupFromLdap(AUsers::DataGroup &group){
	
	const char * key = "%g";
	
	size_t pos = this->ldap.filterConfig.find(key);
	
	if(pos != string::npos){
		
		ALDAP ldap(this->config, this->log);
		
		string filter = this->ldap.filterConfig;
		
		filter = filter.replace(pos, strlen(key), to_string(group.id));
		
		const string dn = (string("ac=") + this->config->proxy.name + string(",") + this->ldap.dnConfig);
		
		const string params =	"amingConfigsConnectsConnect,amingConfigsConnectsSize,amingConfigsGzipChunk,"
								"amingConfigsGzipLength,amingConfigsGzipLevel,amingConfigsGzipProxied,"
								"amingConfigsGzipRegex,amingConfigsGzipResponse,amingConfigsGzipTransfer,"
								"amingConfigsGzipTypes,amingConfigsGzipVary,amingConfigsGzipVhttp,amingConfigsAuth,"
								"amingConfigsIdnt,amingConfigsIpExternal4,amingConfigsIpExternal6,"
								"amingConfigsIpResolver4,amingConfigsIpResolver6,amingConfigsKeepAliveCnt,"
								"amingConfigsKeepAliveEnabled,amingConfigsKeepAliveIdle,amingConfigsKeepAliveIntvl,"
								"amingConfigsProxyAgent,amingConfigsProxyConnect,amingConfigsProxyDeblock,"
								"amingConfigsProxyForward,amingConfigsProxyPipelining,amingConfigsProxyReverse,"
								"amingConfigsProxySkill,amingConfigsProxySubnet,amingConfigsProxyTransfer,"
								"amingConfigsProxyUpgrade,amingConfigsSpeedInput,amingConfigsSpeedOutput,"
								"amingConfigsTimeoutsRead,amingConfigsTimeoutsUpgrade,amingConfigsTimeoutsWrite";
		
		auto groups = ldap.data(dn, params, this->ldap.scopeConfig, filter);
		
		if(!groups.empty()){
			 
			auto getBoolean = [](const string value){
				
				bool check = false;
				
				string param = value;
				
				param = Anyks::toCase(param);
				
				if(param.compare("true") == 0) check = true;
				
				return check;
			};
			
			for(auto it = groups.cbegin(); it != groups.cend(); ++it){
				
				for(auto dt = it->vals.cbegin(); dt != it->vals.cend(); ++dt){
					
					if(!dt->second.empty()){
						
						if(dt->first.compare("amingConfigsIdnt") == 0) group.idnt = dt->second;
						
						else if(dt->first.compare("amingConfigsIpExternal4") == 0) group.ipv4.ip = dt->second;
						
						else if(dt->first.compare("amingConfigsIpExternal6") == 0) group.ipv6.ip = dt->second;
						
						else if(dt->first.compare("amingConfigsIpResolver4") == 0) group.ipv4.resolver = dt->second;
						
						else if(dt->first.compare("amingConfigsIpResolver6") == 0) group.ipv6.resolver = dt->second;
						
						else if(dt->first.compare("amingConfigsAuth") == 0) group.auth = Anyks::toCase(dt->second[0]);
						
						else if(dt->first.compare("amingConfigsProxyConnect") == 0){
							
							setProxyOptions(OPT_CONNECT, group.options, getBoolean(dt->second[0]));
						
						} else if(dt->first.compare("amingConfigsProxyUpgrade") == 0){
							
							setProxyOptions(OPT_UPGRADE, group.options, getBoolean(dt->second[0]));
						
						} else if(dt->first.compare("amingConfigsProxyAgent") == 0){
							
							setProxyOptions(OPT_AGENT, group.options, getBoolean(dt->second[0]));
						
						} else if(dt->first.compare("amingConfigsProxyDeblock") == 0){
							
							setProxyOptions(OPT_DEBLOCK, group.options, getBoolean(dt->second[0]));
						
						} else if(dt->first.compare("amingConfigsGzipTransfer") == 0){
							
							setProxyOptions(OPT_GZIP, group.options, getBoolean(dt->second[0]));
						
						} else if(dt->first.compare("amingConfigsGzipResponse") == 0){
							
							setProxyOptions(OPT_PGZIP, group.options, getBoolean(dt->second[0]));
						
						} else if(dt->first.compare("amingConfigsKeepAliveEnabled") == 0){
							
							setProxyOptions(OPT_KEEPALIVE, group.options, getBoolean(dt->second[0]));
						
						} else if(dt->first.compare("amingConfigsProxySkill") == 0){
							
							const bool check = (dt->second[0].compare("smart") == 0);
							
							setProxyOptions(OPT_SMART, group.options, check);
						
						} else if(dt->first.compare("amingConfigsTimeoutsRead") == 0){
							
							group.timeouts.read = (size_t) Anyks::getSeconds(dt->second[0]);
						
						} else if(dt->first.compare("amingConfigsTimeoutsWrite") == 0){
							
							group.timeouts.write = (size_t) Anyks::getSeconds(dt->second[0]);
						
						} else if(dt->first.compare("amingConfigsTimeoutsUpgrade") == 0){
							
							group.timeouts.upgrade = (size_t) Anyks::getSeconds(dt->second[0]);
						
						} else if(dt->first.compare("amingConfigsSpeedInput") == 0){
							
							group.buffers.read = Anyks::getSizeBuffer(dt->second[0]);
						
						} else if(dt->first.compare("amingConfigsSpeedOutput") == 0){
							
							group.buffers.write = Anyks::getSizeBuffer(dt->second[0]);
						
						} else if(dt->first.compare("amingConfigsKeepAliveCnt") == 0){
							
							group.keepalive.keepcnt = (int) ::atoi(dt->second[0].c_str());
						
						} else if(dt->first.compare("amingConfigsKeepAliveIdle") == 0){
							
							group.keepalive.keepidle = (int) ::atoi(dt->second[0].c_str());
						
						} else if(dt->first.compare("amingConfigsKeepAliveIntvl") == 0){
							
							group.keepalive.keepintvl = (int) ::atoi(dt->second[0].c_str());
						
						} else if(dt->first.compare("amingConfigsConnectsSize") == 0){
							
							group.connects.size = Anyks::getBytes(dt->second[0]);
						
						} else if(dt->first.compare("amingConfigsConnectsConnect") == 0){
							
							group.connects.connect = (u_int) ::atoi(dt->second[0].c_str());
						
						} else if(dt->first.compare("amingConfigsProxySubnet") == 0){
							
							group.proxy.subnet = getBoolean(dt->second[0]);
						
						} else if(dt->first.compare("amingConfigsProxyReverse") == 0){
							
							group.proxy.reverse = getBoolean(dt->second[0]);
						
						} else if(dt->first.compare("amingConfigsProxyForward") == 0){
							
							group.proxy.forward = getBoolean(dt->second[0]);
						
						} else if(dt->first.compare("amingConfigsProxyTransfer") == 0){
							
							group.proxy.transfer = getBoolean(dt->second[0]);
						
						} else if(dt->first.compare("amingConfigsProxyPipelining") == 0){
							
							group.proxy.pipelining = getBoolean(dt->second[0]);
						
						} else if(dt->first.compare("amingConfigsGzipRegex") == 0){
							
							group.gzip.regex = dt->second[0];
						
						} else if(dt->first.compare("amingConfigsGzipVary") == 0){
							
							group.gzip.vary = getBoolean(dt->second[0]);
						
						} else if(dt->first.compare("amingConfigsGzipLength") == 0){
							
							group.gzip.length = (int) ::atoi(dt->second[0].c_str());
						
						} else if(dt->first.compare("amingConfigsGzipChunk") == 0){
							
							group.gzip.chunk = Anyks::getBytes(dt->second[0]);
						
						} else if(dt->first.compare("amingConfigsGzipVhttp") == 0){
							
							group.gzip.vhttp = dt->second;
						
						} else if(dt->first.compare("amingConfigsGzipTypes") == 0){
							
							group.gzip.types = dt->second;
						
						} else if(dt->first.compare("amingConfigsGzipProxied") == 0){
							
							group.gzip.proxied = dt->second;
						
						} else if(dt->first.compare("amingConfigsGzipLevel") == 0){
							
							u_int level = OPT_NULL;
							
							const string gzipLevel = dt->second[0];
							
							if(!gzipLevel.empty()){
								
								if(gzipLevel.compare("default") == 0)		level = Z_DEFAULT_COMPRESSION;
								else if(gzipLevel.compare("best") == 0)		level = Z_BEST_COMPRESSION;
								else if(gzipLevel.compare("speed") == 0)	level = Z_BEST_SPEED;
								else if(gzipLevel.compare("no") == 0)		level = Z_NO_COMPRESSION;
							}
							if(level != OPT_NULL) group.gzip.level = level;
						}
					}
				}
			}
		}
	}
}
 
void AUsers::Groups::setDataGroupFromFile(AUsers::DataGroup &group, INI * ini){
	
	bool rmINI = false;
	
	if(!ini){
		
		const string filename = Anyks::addToPath(this->config->proxy.dir, "groups.ini");
		
		if(!filename.empty() && Anyks::isFileExist(filename.c_str())){
			
			ini = new INI(filename);
			
			rmINI = true;
		
		} else return;
	}
	
	group.idnt = Anyks::split(ini->getString("identificators", group.name), "|");
	
	if(ini->checkParam(group.name + "_proxy", "connect")){
		
		const bool check = ini->getBoolean(group.name + "_proxy", "connect");
		
		setProxyOptions(OPT_CONNECT, group.options, check);
	}
	
	if(ini->checkParam(group.name + "_proxy", "upgrade")){
		
		const bool check = ini->getBoolean(group.name + "_proxy", "upgrade");
		
		setProxyOptions(OPT_UPGRADE, group.options, check);
	}
	
	if(ini->checkParam(group.name + "_proxy", "agent")){
		
		const bool check = ini->getBoolean(group.name + "_proxy", "agent");
		
		setProxyOptions(OPT_AGENT, group.options, check);
	}
	
	if(ini->checkParam(group.name + "_proxy", "deblock")){
		
		const bool check = ini->getBoolean(group.name + "_proxy", "deblock");
		
		setProxyOptions(OPT_DEBLOCK, group.options, check);
	}
	
	if(ini->checkParam(group.name + "_gzip", "transfer")){
		
		const bool check = ini->getBoolean(group.name + "_gzip", "transfer");
		
		setProxyOptions(OPT_GZIP, group.options, check);
	}
	
	if(ini->checkParam(group.name + "_gzip", "response")){
		
		const bool check = ini->getBoolean(group.name + "_gzip", "response");
		
		setProxyOptions(OPT_PGZIP, group.options, check);
	}
	
	if(ini->checkParam(group.name + "_proxy", "skill")){
		
		const bool check = (ini->getString(group.name + "_proxy", "skill", "dumb").compare("smart") == 0);
		
		setProxyOptions(OPT_SMART, group.options, check);
	}
	
	if(ini->checkParam(group.name + "_keepalive", "enabled")){
		
		const bool check = ini->getBoolean(group.name + "_keepalive", "enabled");
		
		setProxyOptions(OPT_KEEPALIVE, group.options, check);
	}
	
	if(ini->checkParam(group.name + "_ipv4", "external")){
		
		group.ipv4.ip = Anyks::split(ini->getString(group.name + "_ipv4", "external"), "|");
	}
	
	if(ini->checkParam(group.name + "_ipv6", "external")){
		
		group.ipv6.ip = Anyks::split(ini->getString(group.name + "_ipv6", "external"), "|");
	}
	
	if(ini->checkParam(group.name + "_ipv4", "resolver")){
		
		group.ipv4.resolver = Anyks::split(ini->getString(group.name + "_ipv4", "resolver"), "|");
	}
	
	if(ini->checkParam(group.name + "_ipv6", "resolver")){
		
		group.ipv6.resolver = Anyks::split(ini->getString(group.name + "_ipv6", "resolver"), "|");
	}
	
	if(ini->checkParam(group.name + "_timeouts", "read")){
		
		group.timeouts.read = (size_t) Anyks::getSeconds(ini->getString(group.name + "_timeouts", "read"));
	}
	
	if(ini->checkParam(group.name + "_timeouts", "write")){
		
		group.timeouts.write = (size_t) Anyks::getSeconds(ini->getString(group.name + "_timeouts", "write"));
	}
	
	if(ini->checkParam(group.name + "_timeouts", "upgrade")){
		
		group.timeouts.upgrade = (size_t) Anyks::getSeconds(ini->getString(group.name + "_timeouts", "upgrade"));
	}
	
	if(ini->checkParam(group.name + "_speed", "input")){
		
		group.buffers.read = Anyks::getSizeBuffer(ini->getString(group.name + "_speed", "input"));
	}
	
	if(ini->checkParam(group.name + "_speed", "output")){
		
		group.buffers.write = Anyks::getSizeBuffer(ini->getString(group.name + "_speed", "output"));
	}
	
	if(ini->checkParam(group.name + "_keepalive", "keepcnt")){
		
		group.keepalive.keepcnt = (int) ini->getNumber(group.name + "_keepalive", "keepcnt");
	}
	
	if(ini->checkParam(group.name + "_keepalive", "keepidle")){
		
		group.keepalive.keepidle = (int) ini->getNumber(group.name + "_keepalive", "keepidle");
	}
	
	if(ini->checkParam(group.name + "_keepalive", "keepintvl")){
		
		group.keepalive.keepintvl = (int) ini->getNumber(group.name + "_keepalive", "keepintvl");
	}
	
	if(ini->checkParam(group.name + "_connects", "size")){
		
		group.connects.size = Anyks::getBytes(ini->getString(group.name + "_connects", "size"));
	}
	
	if(ini->checkParam(group.name + "_connects", "connect")){
		
		group.connects.connect = (u_int) ini->getUNumber(group.name + "_connects", "connect");
	}
	
	if(ini->checkParam(group.name + "_proxy", "subnet")){
		
		group.proxy.subnet = ini->getBoolean(group.name + "_proxy", "subnet");
	}
	
	if(ini->checkParam(group.name + "_proxy", "reverse")){
		
		group.proxy.reverse = ini->getBoolean(group.name + "_proxy", "reverse");
	}
	
	if(ini->checkParam(group.name + "_proxy", "forward")){
		
		group.proxy.forward = ini->getBoolean(group.name + "_proxy", "forward");
	}
	
	if(ini->checkParam(group.name + "_proxy", "transfer")){
		
		group.proxy.transfer = ini->getBoolean(group.name + "_proxy", "transfer");
	}
	
	if(ini->checkParam(group.name + "_proxy", "pipelining")){
		
		group.proxy.pipelining = ini->getBoolean(group.name + "_proxy", "pipelining");
	}
	
	if(ini->checkParam(group.name + "_gzip", "regex")){
		
		group.gzip.regex = ini->getString(group.name + "_gzip", "regex");
	}
	
	if(ini->checkParam(group.name + "_gzip", "vary")){
		
		group.gzip.vary = ini->getBoolean(group.name + "_gzip", "vary");
	}
	
	if(ini->checkParam(group.name + "_gzip", "length")){
		
		group.gzip.length = ini->getNumber(group.name + "_gzip", "length");
	}
	
	if(ini->checkParam(group.name + "_gzip", "chunk")){
		
		group.gzip.chunk = Anyks::getBytes(ini->getString(group.name + "_gzip", "chunk"));
	}
	
	if(ini->checkParam(group.name + "_gzip", "vhttp")){
		
		group.gzip.vhttp = Anyks::split(ini->getString(group.name + "_gzip", "vhttp"), "|");
	}
	
	if(ini->checkParam(group.name + "_gzip", "types")){
		
		group.gzip.types = Anyks::split(ini->getString(group.name + "_gzip", "types"), "|");
	}
	
	if(ini->checkParam(group.name + "_gzip", "proxied")){
		
		group.gzip.proxied = Anyks::split(ini->getString(group.name + "_gzip", "proxied"), "|");
	}
	
	if(ini->checkParam(group.name + "_gzip", "level")){
		
		u_int level = OPT_NULL;
		
		const string gzipLevel = ini->getString(group.name + "_gzip", "level");
		
		if(!gzipLevel.empty()){
			
			if(gzipLevel.compare("default") == 0)		level = Z_DEFAULT_COMPRESSION;
			else if(gzipLevel.compare("best") == 0)		level = Z_BEST_COMPRESSION;
			else if(gzipLevel.compare("speed") == 0)	level = Z_BEST_SPEED;
			else if(gzipLevel.compare("no") == 0)		level = Z_NO_COMPRESSION;
		}
		if(level != OPT_NULL) group.gzip.level = level;
	}
	
	if(rmINI) delete ini;
}
 
void AUsers::Groups::setDataGroup(AUsers::DataGroup &group, INI * ini){
	
	switch(this->typeConfigs){
		
		case 0: setDataGroupFromFile(group, ini); break;
		
		case 1: setDataGroupFromLdap(group); break;
	}
}
 
const AUsers::DataGroup AUsers::Groups::createDefaultData(const gid_t id, const string name){
	
	DataGroup group;
	
	if(id && !name.empty()){
		
		string groupName = name;
		
		group.id		= id;
		group.name		= Anyks::toCase(groupName);
		group.auth		= this->config->auth.auth;
		group.options	= this->config->options;
		group.ipv4		= {this->config->ipv4.external, this->config->ipv4.resolver};
		group.ipv6		= {this->config->ipv6.external, this->config->ipv6.resolver};
		
		group.gzip = {
			this->config->gzip.vary,
			this->config->gzip.level,
			this->config->gzip.length,
			this->config->gzip.chunk,
			this->config->gzip.regex,
			this->config->gzip.vhttp,
			this->config->gzip.proxied,
			this->config->gzip.types
		};
		
		group.proxy = {
			this->config->proxy.reverse,
			this->config->proxy.transfer,
			this->config->proxy.forward,
			this->config->proxy.subnet,
			this->config->proxy.pipelining
		};
		
		group.connects = {
			this->config->connects.size,
			this->config->connects.connect
		};
		
		group.timeouts = {
			this->config->timeouts.read,
			this->config->timeouts.write,
			this->config->timeouts.upgrade
		};
		
		group.buffers = {
			this->config->buffers.read,
			this->config->buffers.write
		};
		
		group.keepalive = {
			this->config->keepalive.keepcnt,
			this->config->keepalive.keepidle,
			this->config->keepalive.keepintvl
		};
	}
	
	return group;
}
 
const bool AUsers::Groups::readGroupsFromLdap(){
	
	bool result = false;
	
	ALDAP ldap(this->config, this->log);
	
	auto users = ldap.data(this->ldap.dnUser, "gidNumber,uidNumber", this->ldap.scopeUser, this->ldap.filterUser);
	
	auto groups = ldap.data(this->ldap.dnGroup, "cn,description,gidNumber,Password,memberUid", this->ldap.scopeGroup, this->ldap.filterGroup);
	
	if(!groups.empty()){
		
		for(auto it = groups.cbegin(); it != groups.cend(); ++it){
			
			gid_t gid;
			
			string name;
			
			string description;
			
			string password;
			
			vector <uid_t> users;
			
			for(auto dt = it->vals.cbegin(); dt != it->vals.cend(); ++dt){
				
				if(!dt->second.empty()){
					
					if(dt->first.compare("cn") == 0) name = dt->second[0];
					
					else if(dt->first.compare("gidNumber") == 0) gid = ::atoi(dt->second[0].c_str());
					
					else if(dt->first.compare("description") == 0) description = dt->second[0];
					
					else if(dt->first.compare("Password") == 0) password = dt->second[0];
					
					else if(dt->first.compare("memberUid") == 0){
						
						for(auto ut = dt->second.cbegin(); ut != dt->second.cend(); ++ut){
							
							uid_t uid = getUidByUserName(* ut);
							
							if(uid > -1) users.push_back(uid);
						}
					}
				}
			}
			
			DataGroup group = createDefaultData(gid, name);
			
			group.type = 2;
			
			group.users = users;
			
			group.pass = password;
			
			group.desc = description;
			
			setDataGroup(group);
			
			this->data.insert(pair <gid_t, DataGroup>(group.id, group));
		}
		
		if(!users.empty()){
			
			for(auto it = users.cbegin(); it != users.cend(); ++it){
				
				gid_t gid;
				
				uid_t uid;
				
				for(auto dt = it->vals.cbegin(); dt != it->vals.cend(); ++dt){
					
					if(!dt->second.empty()){
						
						if(dt->first.compare("gidNumber") == 0){
							
							gid = ::atoi(dt->second[0].c_str());
						} else if(dt->first.compare("uidNumber") == 0) {
							
							uid = ::atoi(dt->second[0].c_str());
						}
					}
				}
				
				if(this->data.count(gid)){
					
					bool userExist = false;
					
					vector <uid_t> * users = &this->data.find(gid)->second.users;
					
					for(auto it = users->cbegin(); it != users->cend(); ++it){
						
						if(uid == *it){
							
							userExist = true;
							
							break;
						}
					}
					
					if(!userExist) users->push_back(uid);
				}
			}
		}
		
		result = true;
	}
	
	return result;
}
 
const bool AUsers::Groups::readGroupsFromPam(){
	
	bool result = false;
	
	struct passwd * pw = nullptr;
	
	smatch match;
	
	regex e("\\/(?:sh|bash)$", regex::ECMAScript | regex::icase);
	
	while((pw = getpwent()) != nullptr){
		
		if(pw->pw_uid > 0){
			
			string shell = pw->pw_shell;
			
			regex_search(shell, match, e);
			
			if(!match.empty()){
				
				int maxGroupsUser = this->maxPamGroupsUser;
				
				int * userGroups = new int[(const int) maxGroupsUser];
				
				struct group * gr = nullptr;
				
				if(getgrouplist(pw->pw_name, pw->pw_gid, userGroups, &maxGroupsUser) == -1){
					
					this->log->write(LOG_ERROR, 0, "groups from user = %s [%s] not found", pw->pw_name, pw->pw_gecos);
				
				} else {
					
					for(int i = 0; i < maxGroupsUser; i++){
						
						gr = getgrgid(userGroups[i]);
						
						if(gr != nullptr){
							
							if(this->data.count(gr->gr_gid)){
								
								bool userExist = false;
								
								vector <uid_t> * users = &this->data.find(gr->gr_gid)->second.users;
								
								for(auto it = users->cbegin(); it != users->cend(); ++it){
									
									if(pw->pw_uid == *it){
										
										userExist = true;
										
										break;
									}
								}
								
								if(!userExist) users->push_back(pw->pw_uid);
							
							} else {
								
								DataGroup group = createDefaultData(gr->gr_gid, gr->gr_name);
								
								group.type = 1;
								
								group.pass = gr->gr_passwd;
								
								group.users.push_back(pw->pw_uid);
								
								setDataGroup(group);
								
								this->data.insert(pair <gid_t, DataGroup>(group.id, group));
							}
						
						} else this->log->write(LOG_ERROR, 0, "group [%i] from user = %s [%s] not found", userGroups[i], pw->pw_name, pw->pw_gecos);
					}
					
					result = true;
				}
				
				delete [] userGroups;
			}
		}
	}
	
	return result;
}
 
const bool AUsers::Groups::readGroupsFromFile(){
	
	bool result = false;
	
	const string filename = Anyks::addToPath(this->config->proxy.dir, "groups.ini");
	
	if(!filename.empty() && Anyks::isFileExist(filename.c_str())){
		
		INI ini(filename);
		
		if(!ini.isError()){
			
			string filepass;
			
			auto users = ini.getParamsInSection("users");
			
			auto groups = ini.getParamsInSection("groups");
			
			auto passwords = ini.getParamsInSection("passwords");
			
			auto descriptions = ini.getParamsInSection("descriptions");
			
			if(!groups.empty()){
				
				for(auto it = groups.cbegin(); it != groups.cend(); ++it){
					
					if(Anyks::isNumber(it->key) || Anyks::isNumber(it->value)){
						
						const gid_t gid = (Anyks::isNumber(it->key) ? ::atoi(it->key.c_str()) : ::atoi(it->value.c_str()));
						
						const string name = (Anyks::isNumber(it->key) ? it->value : it->key);
						
						DataGroup group = createDefaultData(gid, name);
						
						group.type = 0;
						
						if(!users.empty()){
							
							for(auto ut = users.cbegin(); ut != users.cend(); ++ut){
								
								if((Anyks::isNumber(ut->value)
								&& (gid_t(::atoi(ut->value.c_str())) == group.id))
								|| (Anyks::toCase(ut->value).compare(group.name) == 0)){
									
									uid_t uid = -1;
									
									if(Anyks::isNumber(ut->key)) uid = ::atoi(ut->key.c_str());
									
									else uid = getUidByUserName(ut->key);
									
									if(uid > -1) group.users.push_back(uid);
								}
							}
						}
						
						if(!passwords.empty()){
							
							for(auto gp = passwords.cbegin(); gp != passwords.cend(); ++gp){
								
								const string key = Anyks::toCase(gp->key);
								
								if(key.compare("all") != 0){
									
									if((Anyks::isNumber(key)
									&& (gid_t(::atoi(key.c_str())) == group.id))
									|| (key.compare(group.name) == 0)) group.pass = getPasswordFromFile(gp->value, this->log, group.id, group.name);
								
								} else if(filepass.empty()) filepass = gp->value;
							}
						}
						
						if(!descriptions.empty()){
							
							for(auto gd = descriptions.cbegin(); gd != descriptions.cend(); ++gd){
								
								if((Anyks::isNumber(gd->key)
								&& (gid_t(::atoi(gd->key.c_str())) == group.id))
								|| (Anyks::toCase(gd->key).compare(group.name) == 0)) group.desc = gd->value;
							}
						}
						
						setDataGroup(group, &ini);
						
						this->data.insert(pair <gid_t, DataGroup>(group.id, group));
						
						result = true;
					}
				}
				
				if(!filepass.empty()) getPasswordsFromFile(filepass, this->log, this, AMING_GROUP);
			}
		}
	}
	
	return result;
}
 
const bool AUsers::Groups::update(){
	
	bool result = false;
	
	time_t curUpdate = time(nullptr);
	
	if((this->lastUpdate + this->maxUpdate) < curUpdate){
		
		this->lastUpdate = curUpdate;
		
		this->data.clear();
		
		switch(this->typeSearch){
			
			case 0: readGroupsFromFile(); break;
			
			case 1: readGroupsFromPam(); break;
			
			case 2: readGroupsFromLdap(); break;
			
			case 3: {
				readGroupsFromFile();
				readGroupsFromPam();
			} break;
			
			case 4: {
				readGroupsFromFile();
				readGroupsFromLdap();
			} break;
			
			case 5: {
				readGroupsFromPam();
				readGroupsFromLdap();
			} break;
			
			case 6: {
				readGroupsFromFile();
				readGroupsFromPam();
				readGroupsFromLdap();
			} break;
		}
		
		result = true;
	}
	
	return result;
}
 
const vector <const AUsers::DataGroup *> AUsers::Groups::getAllGroups(){
	
	vector <const DataGroup *> result;
	
	if(!this->data.empty()){
		
		for(auto it = this->data.cbegin(); it != this->data.cend(); ++it){
			
			result.push_back(&(it->second));
		}
	}
	
	return result;
}
 
const AUsers::DataGroup * AUsers::Groups::getDataById(const gid_t gid){
	
	if(gid && !this->data.empty()){
		
		if(this->data.count(gid)){
			
			return &(this->data.find(gid)->second);
		
		} else if(update()) {
			
			return getDataById(gid);
		}
	}
	
	return nullptr;
}
 
const AUsers::DataGroup * AUsers::Groups::getDataByName(const string groupName){
	
	if(!groupName.empty() && !this->data.empty()){
		
		string name = Anyks::toCase(groupName);
		
		for(auto it = this->data.cbegin(); it != this->data.cend(); ++it){
			
			if(it->second.name.compare(name) == 0){
				
				return &(it->second);
			}
		}
		
		if(update()) return getDataByName(groupName);
	}
	
	return nullptr;
}
 
const vector <gid_t> AUsers::Groups::getGroupIdByUser(const uid_t uid){
	
	vector <gid_t> result;
	
	if(uid){
		
		for(auto it = this->data.cbegin(); it != this->data.cend(); ++it){
			
			auto users = it->second.users;
			
			for(auto ut = users.cbegin(); ut != users.cend(); ++ut){
				
				if(uid == *ut){
					
					result.push_back(it->first);
					
					break;
				}
			}
		}
	}
	
	return result;
}
 
const vector <gid_t> AUsers::Groups::getGroupIdByUser(const string userName){
	
	vector <gid_t> result;
	
	if(!userName.empty()){
		
		const uid_t uid = getUidByUserName(userName);
		
		if(uid > -1) result = getGroupIdByUser(uid);
	}
	
	return result;
}
 
const vector <string> AUsers::Groups::getGroupNameByUser(const uid_t uid){
	
	vector <string> result;
	
	if(uid){
		
		for(auto it = this->data.cbegin(); it != this->data.cend(); ++it){
			
			auto users = it->second.users;
			
			for(auto ut = users.cbegin(); ut != users.cend(); ++ut){
				
				if(uid == *ut){
					
					result.push_back(it->second.name);
					
					break;
				}
			}
		}
	}
	
	return result;
}
 
const vector <string> AUsers::Groups::getGroupNameByUser(const string userName){
	
	vector <string> result;
	
	if(!userName.empty()){
		
		const uid_t uid = getUidByUserName(userName);
		
		if(uid > -1) result = getGroupNameByUser(uid);
	}
	
	return result;
}
 
const bool AUsers::Groups::checkUser(const gid_t gid, const uid_t uid){
	
	bool result = false;
	
	if(gid && uid && this->data.count(gid)){
		
		auto users = this->data.find(gid)->second.users;
		
		for(auto it = users.cbegin(); it != users.cend(); ++it){
			
			if(uid == *it){
				
				result = true;
				
				break;
			}
		}
	
	} else if(gid && uid && update()) {
		
		result = checkUser(gid, uid);
	}
	
	return result;
}
 
const bool AUsers::Groups::checkUser(const gid_t gid, const string userName){
	
	bool result = false;
	
	if(gid && !userName.empty()){
		
		const uid_t uid = getUidByUserName(userName);
		
		if(uid > -1) result = checkUser(gid, uid);
	}
	
	return result;
}
 
const bool AUsers::Groups::checkUser(const string groupName, const uid_t uid){
	
	bool result = false;
	
	if(uid && !groupName.empty()){
		
		const gid_t gid = getIdByName(groupName);
		
		result = checkUser(gid, uid);
	}
	
	return result;
}
 
const bool AUsers::Groups::checkUser(const string groupName, const string userName){
	
	bool result = false;
	
	if(!groupName.empty() && !userName.empty()){
		
		const gid_t gid = getIdByName(groupName);
		
		const uid_t uid = getUidByUserName(userName);
		
		if((gid > -1) && (uid > -1)) result = checkUser(gid, uid);
	}
	
	return result;
}
 
const bool AUsers::Groups::checkGroupById(const gid_t gid){
	
	if(gid && !this->data.empty()){
		
		if(this->data.count(gid)){
			
			return true;
		
		} else if(update()) {
			
			return checkGroupById(gid);
		}
	}
	
	return false;
}
 
const bool AUsers::Groups::checkGroupByName(const string groupName){
	
	return (getIdByName(groupName) > -1 ? true : false);
}
 
const uid_t AUsers::Groups::getUidByUserName(const string userName){
	
	uid_t result = -1;
	
	if(!userName.empty() && (this->users != nullptr)){
		
		result = (reinterpret_cast <Users *> (this->users))->getIdByName(userName);
	}
	
	return result;
}
 
const gid_t AUsers::Groups::getIdByName(const string groupName){
	
	if(!groupName.empty() && !this->data.empty()){
		
		string name = Anyks::toCase(groupName);
		
		for(auto it = this->data.cbegin(); it != this->data.cend(); ++it){
			
			if(it->second.name.compare(name) == 0){
				
				return it->first;
			}
		}
		
		if(update()) return getIdByName(groupName);
	}
	
	return -1;
}
 
const string AUsers::Groups::getUserNameByUid(const uid_t uid){
	
	string result;
	
	if((uid > -1) && (this->users != nullptr)){
		
		result = (reinterpret_cast <Users *> (this->users))->getNameById(uid);
	}
	
	return result;
}
 
const string AUsers::Groups::getNameById(const gid_t gid){
	
	if(gid && !this->data.empty()){
		
		if(this->data.count(gid)){
			
			return this->data.find(gid)->second.name;
		
		} else if(update()) {
			
			return getNameById(gid);
		}
	}
	
	return string();
}
 
const vector <string> AUsers::Groups::getNameUsers(const gid_t gid){
	
	vector <string> result;
	
	if((gid > -1) && (users != nullptr) && this->data.count(gid)){
		
		auto _users = this->data.find(gid)->second.users;
		
		for(auto it = _users.cbegin(); it != _users.cend(); ++it){
			
			const string userName = getUserNameByUid(* it);
			
			if(!userName.empty()) result.push_back(userName);
		}
	
	} else if(gid && update()) {
		
		result = getNameUsers(gid);
	}
	
	return result;
}
 
const vector <string> AUsers::Groups::getNameUsers(const string groupName){
	
	vector <string> result;
	
	if((users != nullptr) && !groupName.empty()){
		
		const gid_t gid = getIdByName(groupName);
		
		if(gid > -1) result = getNameUsers(gid);
	}
	
	return result;
}
 
const vector <uid_t> AUsers::Groups::getIdAllUsers(){
	
	vector <uid_t> result;
	
	auto groups = getAllGroups();
	
	if(!groups.empty()){
		
		for(auto it = groups.cbegin(); it != groups.cend(); ++it){
			
			copy((* it)->users.cbegin(), (* it)->users.cend(), back_inserter(result));
		}
		
		sort(result.begin(), result.end());
		
		result.resize(unique(result.begin(), result.end()) - result.begin());
	}
	
	return result;
}
 
const vector <uid_t> AUsers::Groups::getIdUsers(const gid_t gid){
	
	vector <uid_t> result;
	
	if(gid && this->data.count(gid)){
		
		result = this->data.find(gid)->second.users;
	
	} else if(gid && update()) {
		
		result = getIdUsers(gid);
	}
	
	return result;
}
 
const vector <uid_t> AUsers::Groups::getIdUsers(const string groupName){
	
	vector <uid_t> result;
	
	if(!groupName.empty()){
		
		const gid_t gid = getIdByName(groupName);
		
		if(gid > -1) result = getIdUsers(gid);
	}
	
	return result;
}
 
const bool AUsers::Groups::addUser(const gid_t gid, const uid_t uid){
	
	bool result = false;
	
	if(gid && uid && this->data.count(gid)){
		
		this->data.find(gid)->second.users.push_back(uid);
		
		result = true;
	
	} else if(gid && uid && update()) {
		
		result = addUser(gid, uid);
	}
	
	return result;
}
 
const bool AUsers::Groups::addUser(const gid_t gid, const string userName){
	
	bool result = false;
	
	if(gid && !userName.empty()){
		
		const uid_t uid = getUidByUserName(userName);
		
		if(uid > -1) result = addUser(gid, uid);
	}
	
	return result;
}
 
const bool AUsers::Groups::addUser(const string groupName, const uid_t uid){
	
	bool result = false;
	
	if(uid && !groupName.empty()){
		
		const gid_t gid = getIdByName(groupName);
		
		result = addUser(gid, uid);
	}
	
	return result;
}
 
const bool AUsers::Groups::addUser(const string groupName, const string userName){
	
	bool result = false;
	
	if(!groupName.empty() && !userName.empty()){
		
		const gid_t gid = getIdByName(groupName);
		
		const uid_t uid = getUidByUserName(userName);
		
		if((gid > -1) && (uid > -1)) result = addUser(gid, uid);
	}
	
	return result;
}
 
const bool AUsers::Groups::addGroup(const gid_t gid, const string name){
	
	bool result = false;
	
	if((gid > -1) && !name.empty()){
		
		DataGroup group = createDefaultData(gid, name);
		
		this->data.insert(pair <gid_t, DataGroup>(group.id, group));
		
		result = true;
	}
	
	return result;
}
 
void AUsers::Groups::setPassword(const gid_t gid, const string password){
	
	if((gid > -1) && !password.empty() && !this->data.empty()){
		
		if(this->data.count(gid)){
			
			(this->data.find(gid)->second).pass = password;
		
		} else if(update()) {
			
			setPassword(gid, password);
		}
	}
}
 
void AUsers::Groups::setUsers(void * users){
	
	this->users = users;
	
	update();
}
 
AUsers::Groups::Groups(Config * config, LogApp * log){
	
	if(config){
		
		this->log = log;
		
		this->config = config;
		
		this->typeSearch = 6;
		
		this->typeConfigs = 1;
		
		this->maxUpdate = 600;
		
		this->maxPamGroupsUser = 100;
		
		this->ldap = {
			"ou=groups,dc=agro24,dc=dev",
			"ou=users,dc=agro24,dc=dev",
			"ou=configs,ou=aming,dc=agro24,dc=dev",
			"one",
			"one",
			"one",
			"(objectClass=posixGroup)",
			"(&(!(agro24CoJpDismissed=TRUE))(objectClass=inetOrgPerson))",
			"(&(amingConfigsGroupId=%g)(amingConfigsType=groups)(objectClass=amingConfigs))"
		};
	}
}
