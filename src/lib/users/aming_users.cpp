/* SERVER PROXY AMING */
/*
*  author:     Yuriy Lobarev
*  phone:      +7(910)983-95-90
*  telegram:   @forman
*  email:      info@anyks.com
*  date:       10/29/2017 17:06:00
*  copyright:  © 2017 anyks.com
*/
 
 

#include "ausers/ausers.h"


using namespace std;

 
void AUsers::Users::setProxyOptions(const u_short option, u_short &proxyOptions, const bool flag){
	
	u_short options = proxyOptions;
	
	if(flag) options = options | option;
	
	else {
		
		options = options ^ option;
		
		if(options > proxyOptions) options = proxyOptions;
	}
	
	proxyOptions = options;
}
 
void AUsers::Users::setDataUserFromLdap(AUsers::DataUser &user){
	
	const char * key = "%u";
	
	size_t pos = this->ldap.filterConfig.find(key);
	
	if(pos != string::npos){
		
		ALDAP ldap(this->config, this->log);
		
		string filter = this->ldap.filterConfig;
		
		filter = filter.replace(pos, strlen(key), to_string(user.id));
		
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
		
		auto users = ldap.data(dn, params, this->ldap.scopeConfig, filter);
		
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
						
						if(dt->first.compare("amingConfigsIdnt") == 0) user.idnt = dt->second;
						
						else if(dt->first.compare("amingConfigsIpExternal4") == 0) user.ipv4.ip = dt->second;
						
						else if(dt->first.compare("amingConfigsIpExternal6") == 0) user.ipv6.ip = dt->second;
						
						else if(dt->first.compare("amingConfigsIpResolver4") == 0) user.ipv4.resolver = dt->second;
						
						else if(dt->first.compare("amingConfigsIpResolver6") == 0) user.ipv6.resolver = dt->second;
						
						else if(dt->first.compare("amingConfigsAuth") == 0) user.auth = Anyks::toCase(dt->second[0]);
						
						else if(dt->first.compare("amingConfigsProxyConnect") == 0){
							
							setProxyOptions(OPT_CONNECT, user.options, getBoolean(dt->second[0]));
						
						} else if(dt->first.compare("amingConfigsProxyUpgrade") == 0){
							
							setProxyOptions(OPT_UPGRADE, user.options, getBoolean(dt->second[0]));
						
						} else if(dt->first.compare("amingConfigsProxyAgent") == 0){
							
							setProxyOptions(OPT_AGENT, user.options, getBoolean(dt->second[0]));
						
						} else if(dt->first.compare("amingConfigsProxyDeblock") == 0){
							
							setProxyOptions(OPT_DEBLOCK, user.options, getBoolean(dt->second[0]));
						
						} else if(dt->first.compare("amingConfigsGzipTransfer") == 0){
							
							setProxyOptions(OPT_GZIP, user.options, getBoolean(dt->second[0]));
						
						} else if(dt->first.compare("amingConfigsGzipResponse") == 0){
							
							setProxyOptions(OPT_PGZIP, user.options, getBoolean(dt->second[0]));
						
						} else if(dt->first.compare("amingConfigsKeepAliveEnabled") == 0){
							
							setProxyOptions(OPT_KEEPALIVE, user.options, getBoolean(dt->second[0]));
						
						} else if(dt->first.compare("amingConfigsProxySkill") == 0){
							
							const bool check = (dt->second[0].compare("smart") == 0);
							
							setProxyOptions(OPT_SMART, user.options, check);
						
						} else if(dt->first.compare("amingConfigsTimeoutsRead") == 0){
							
							user.timeouts.read = (size_t) Anyks::getSeconds(dt->second[0]);
						
						} else if(dt->first.compare("amingConfigsTimeoutsWrite") == 0){
							
							user.timeouts.write = (size_t) Anyks::getSeconds(dt->second[0]);
						
						} else if(dt->first.compare("amingConfigsTimeoutsUpgrade") == 0){
							
							user.timeouts.upgrade = (size_t) Anyks::getSeconds(dt->second[0]);
						
						} else if(dt->first.compare("amingConfigsSpeedInput") == 0){
							
							user.buffers.read = Anyks::getSizeBuffer(dt->second[0]);
						
						} else if(dt->first.compare("amingConfigsSpeedOutput") == 0){
							
							user.buffers.write = Anyks::getSizeBuffer(dt->second[0]);
						
						} else if(dt->first.compare("amingConfigsKeepAliveCnt") == 0){
							
							user.keepalive.keepcnt = (int) ::atoi(dt->second[0].c_str());
						
						} else if(dt->first.compare("amingConfigsKeepAliveIdle") == 0){
							
							user.keepalive.keepidle = (int) ::atoi(dt->second[0].c_str());
						
						} else if(dt->first.compare("amingConfigsKeepAliveIntvl") == 0){
							
							user.keepalive.keepintvl = (int) ::atoi(dt->second[0].c_str());
						
						} else if(dt->first.compare("amingConfigsConnectsSize") == 0){
							
							user.connects.size = Anyks::getBytes(dt->second[0]);
						
						} else if(dt->first.compare("amingConfigsConnectsConnect") == 0){
							
							user.connects.connect = (u_int) ::atoi(dt->second[0].c_str());
						
						} else if(dt->first.compare("amingConfigsProxySubnet") == 0){
							
							user.proxy.subnet = getBoolean(dt->second[0]);
						
						} else if(dt->first.compare("amingConfigsProxyReverse") == 0){
							
							user.proxy.reverse = getBoolean(dt->second[0]);
						
						} else if(dt->first.compare("amingConfigsProxyForward") == 0){
							
							user.proxy.forward = getBoolean(dt->second[0]);
						
						} else if(dt->first.compare("amingConfigsProxyTransfer") == 0){
							
							user.proxy.transfer = getBoolean(dt->second[0]);
						
						} else if(dt->first.compare("amingConfigsProxyPipelining") == 0){
							
							user.proxy.pipelining = getBoolean(dt->second[0]);
						
						} else if(dt->first.compare("amingConfigsGzipRegex") == 0){
							
							user.gzip.regex = dt->second[0];
						
						} else if(dt->first.compare("amingConfigsGzipVary") == 0){
							
							user.gzip.vary = getBoolean(dt->second[0]);
						
						} else if(dt->first.compare("amingConfigsGzipLength") == 0){
							
							user.gzip.length = (int) ::atoi(dt->second[0].c_str());
						
						} else if(dt->first.compare("amingConfigsGzipChunk") == 0){
							
							user.gzip.chunk = Anyks::getBytes(dt->second[0]);
						
						} else if(dt->first.compare("amingConfigsGzipVhttp") == 0){
							
							user.gzip.vhttp = dt->second;
						
						} else if(dt->first.compare("amingConfigsGzipTypes") == 0){
							
							user.gzip.types = dt->second;
						
						} else if(dt->first.compare("amingConfigsGzipProxied") == 0){
							
							user.gzip.proxied = dt->second;
						
						} else if(dt->first.compare("amingConfigsGzipLevel") == 0){
							
							u_int level = OPT_NULL;
							
							const string gzipLevel = dt->second[0];
							
							if(!gzipLevel.empty()){
								
								if(gzipLevel.compare("default") == 0)		level = Z_DEFAULT_COMPRESSION;
								else if(gzipLevel.compare("best") == 0)		level = Z_BEST_COMPRESSION;
								else if(gzipLevel.compare("speed") == 0)	level = Z_BEST_SPEED;
								else if(gzipLevel.compare("no") == 0)		level = Z_NO_COMPRESSION;
							}
							if(level != OPT_NULL) user.gzip.level = level;
						}
					}
				}
			}
		}
	}
}
 
void AUsers::Users::setDataUserFromFile(AUsers::DataUser &user, INI * ini){
	
	bool rmINI = false;
	
	if(!ini){
		
		const string filename = Anyks::addToPath(this->config->proxy.dir, "users.ini");
		
		if(!filename.empty() && Anyks::isFileExist(filename.c_str())){
			
			ini = new INI(filename);
			
			rmINI = true;
		
		} else return;
	}
	
	user.idnt = Anyks::split(ini->getString("identificators", user.name), "|");
	
	if(ini->checkParam(user.name + "_proxy", "connect")){
		
		const bool check = ini->getBoolean(user.name + "_proxy", "connect");
		
		setProxyOptions(OPT_CONNECT, user.options, check);
	}
	
	if(ini->checkParam(user.name + "_proxy", "upgrade")){
		
		const bool check = ini->getBoolean(user.name + "_proxy", "upgrade");
		
		setProxyOptions(OPT_UPGRADE, user.options, check);
	}
	
	if(ini->checkParam(user.name + "_proxy", "agent")){
		
		const bool check = ini->getBoolean(user.name + "_proxy", "agent");
		
		setProxyOptions(OPT_AGENT, user.options, check);
	}
	
	if(ini->checkParam(user.name + "_proxy", "deblock")){
		
		const bool check = ini->getBoolean(user.name + "_proxy", "deblock");
		
		setProxyOptions(OPT_DEBLOCK, user.options, check);
	}
	
	if(ini->checkParam(user.name + "_gzip", "transfer")){
		
		const bool check = ini->getBoolean(user.name + "_gzip", "transfer");
		
		setProxyOptions(OPT_GZIP, user.options, check);
	}
	
	if(ini->checkParam(user.name + "_gzip", "response")){
		
		const bool check = ini->getBoolean(user.name + "_gzip", "response");
		
		setProxyOptions(OPT_PGZIP, user.options, check);
	}
	
	if(ini->checkParam(user.name + "_proxy", "skill")){
		
		const bool check = (ini->getString(user.name + "_proxy", "skill", "dumb").compare("smart") == 0);
		
		setProxyOptions(OPT_SMART, user.options, check);
	}
	
	if(ini->checkParam(user.name + "_keepalive", "enabled")){
		
		const bool check = ini->getBoolean(user.name + "_keepalive", "enabled");
		
		setProxyOptions(OPT_KEEPALIVE, user.options, check);
	}
	
	if(ini->checkParam(user.name + "_ipv4", "external")){
		
		user.ipv4.ip = Anyks::split(ini->getString(user.name + "_ipv4", "external"), "|");
	}
	
	if(ini->checkParam(user.name + "_ipv6", "external")){
		
		user.ipv6.ip = Anyks::split(ini->getString(user.name + "_ipv6", "external"), "|");
	}
	
	if(ini->checkParam(user.name + "_ipv4", "resolver")){
		
		user.ipv4.resolver = Anyks::split(ini->getString(user.name + "_ipv4", "resolver"), "|");
	}
	
	if(ini->checkParam(user.name + "_ipv6", "resolver")){
		
		user.ipv6.resolver = Anyks::split(ini->getString(user.name + "_ipv6", "resolver"), "|");
	}
	
	if(ini->checkParam(user.name + "_timeouts", "read")){
		
		user.timeouts.read = (size_t) Anyks::getSeconds(ini->getString(user.name + "_timeouts", "read"));
	}
	
	if(ini->checkParam(user.name + "_timeouts", "write")){
		
		user.timeouts.write = (size_t) Anyks::getSeconds(ini->getString(user.name + "_timeouts", "write"));
	}
	
	if(ini->checkParam(user.name + "_timeouts", "upgrade")){
		
		user.timeouts.upgrade = (size_t) Anyks::getSeconds(ini->getString(user.name + "_timeouts", "upgrade"));
	}
	
	if(ini->checkParam(user.name + "_speed", "input")){
		
		user.buffers.read = Anyks::getSizeBuffer(ini->getString(user.name + "_speed", "input"));
	}
	
	if(ini->checkParam(user.name + "_speed", "output")){
		
		user.buffers.write = Anyks::getSizeBuffer(ini->getString(user.name + "_speed", "output"));
	}
	
	if(ini->checkParam(user.name + "_keepalive", "keepcnt")){
		
		user.keepalive.keepcnt = (int) ini->getNumber(user.name + "_keepalive", "keepcnt");
	}
	
	if(ini->checkParam(user.name + "_keepalive", "keepidle")){
		
		user.keepalive.keepidle = (int) ini->getNumber(user.name + "_keepalive", "keepidle");
	}
	
	if(ini->checkParam(user.name + "_keepalive", "keepintvl")){
		
		user.keepalive.keepintvl = (int) ini->getNumber(user.name + "_keepalive", "keepintvl");
	}
	
	if(ini->checkParam(user.name + "_connects", "size")){
		
		user.connects.size = Anyks::getBytes(ini->getString(user.name + "_connects", "size"));
	}
	
	if(ini->checkParam(user.name + "_connects", "connect")){
		
		user.connects.connect = (u_int) ini->getUNumber(user.name + "_connects", "connect");
	}
	
	if(ini->checkParam(user.name + "_proxy", "subnet")){
		
		user.proxy.subnet = ini->getBoolean(user.name + "_proxy", "subnet");
	}
	
	if(ini->checkParam(user.name + "_proxy", "reverse")){
		
		user.proxy.reverse = ini->getBoolean(user.name + "_proxy", "reverse");
	}
	
	if(ini->checkParam(user.name + "_proxy", "forward")){
		
		user.proxy.forward = ini->getBoolean(user.name + "_proxy", "forward");
	}
	
	if(ini->checkParam(user.name + "_proxy", "transfer")){
		
		user.proxy.transfer = ini->getBoolean(user.name + "_proxy", "transfer");
	}
	
	if(ini->checkParam(user.name + "_proxy", "pipelining")){
		
		user.proxy.pipelining = ini->getBoolean(user.name + "_proxy", "pipelining");
	}
	
	if(ini->checkParam(user.name + "_gzip", "regex")){
		
		user.gzip.regex = ini->getString(user.name + "_gzip", "regex");
	}
	
	if(ini->checkParam(user.name + "_gzip", "vary")){
		
		user.gzip.vary = ini->getBoolean(user.name + "_gzip", "vary");
	}
	
	if(ini->checkParam(user.name + "_gzip", "length")){
		
		user.gzip.length = ini->getNumber(user.name + "_gzip", "length");
	}
	
	if(ini->checkParam(user.name + "_gzip", "chunk")){
		
		user.gzip.chunk = Anyks::getBytes(ini->getString(user.name + "_gzip", "chunk"));
	}
	
	if(ini->checkParam(user.name + "_gzip", "vhttp")){
		
		user.gzip.vhttp = Anyks::split(ini->getString(user.name + "_gzip", "vhttp"), "|");
	}
	
	if(ini->checkParam(user.name + "_gzip", "types")){
		
		user.gzip.types = Anyks::split(ini->getString(user.name + "_gzip", "types"), "|");
	}
	
	if(ini->checkParam(user.name + "_gzip", "proxied")){
		
		user.gzip.proxied = Anyks::split(ini->getString(user.name + "_gzip", "proxied"), "|");
	}
	
	if(ini->checkParam(user.name + "_gzip", "level")){
		
		u_int level = OPT_NULL;
		
		const string gzipLevel = ini->getString(user.name + "_gzip", "level");
		
		if(!gzipLevel.empty()){
			
			if(gzipLevel.compare("default") == 0)		level = Z_DEFAULT_COMPRESSION;
			else if(gzipLevel.compare("best") == 0)		level = Z_BEST_COMPRESSION;
			else if(gzipLevel.compare("speed") == 0)	level = Z_BEST_SPEED;
			else if(gzipLevel.compare("no") == 0)		level = Z_NO_COMPRESSION;
		}
		if(level != OPT_NULL) user.gzip.level = level;
	}
	
	if(rmINI) delete ini;
}
 
 void AUsers::Users::setDataUser(AUsers::DataUser &user, INI * ini){
	
	switch(this->typeConfigs){
		
		case 0: setDataUserFromFile(user, ini); break;
		
		case 1: setDataUserFromLdap(user); break;
	}
}
 
 const AUsers::DataUser AUsers::Users::createDefaultData(const uid_t id, const string name){
	
	DataUser user;
	
	if(id && !name.empty() && (this->groups != nullptr)){
		
		Groups * groups = reinterpret_cast <Groups *> (this->groups);
		
		auto gids = groups->getGroupIdByUser(id);
		
		if(!gids.empty()){
			
			string userName = name;
			
			user.id		= id;
			user.name	= Anyks::toCase(userName);
			
			gid_t gid = 1000000;
			
			for(auto gt = gids.cbegin(); gt != gids.cend(); ++gt){
				
				if((* gt) < gid){
					
					gid = (* gt);
					
					auto group = groups->getDataById(gid);
					
					user.auth 		= group->auth;
					user.options	= group->options;
					user.ipv4		= {group->ipv4.ip, group->ipv4.resolver};
					user.ipv6		= {group->ipv6.ip, group->ipv6.resolver};
					
					user.gzip = {
						group->gzip.vary,
						group->gzip.level,
						group->gzip.length,
						group->gzip.chunk,
						group->gzip.regex,
						group->gzip.vhttp,
						group->gzip.proxied,
						group->gzip.types
					};
					
					user.proxy = {
						group->proxy.reverse,
						group->proxy.transfer,
						group->proxy.forward,
						group->proxy.subnet,
						group->proxy.pipelining
					};
					
					user.connects = {
						group->connects.size,
						group->connects.connect
					};
					
					user.timeouts = {
						group->timeouts.read,
						group->timeouts.write,
						group->timeouts.upgrade
					};
					
					user.buffers = {
						group->buffers.read,
						group->buffers.write
					};
					
					user.keepalive = {
						group->keepalive.keepcnt,
						group->keepalive.keepidle,
						group->keepalive.keepintvl
					};
				}
			}
		}
	}
	
	return user;
}
 
const bool AUsers::Users::readUsersFromLdap(){
	
	bool result = false;
	
	ALDAP ldap(this->config, this->log);
	
	auto users = ldap.data(this->ldap.dnUser, "uid,sn,cn,givenName,initials,userPassword,uidNumber", this->ldap.scopeUser, this->ldap.filterUser);
	
	if(!users.empty()){
		
		for(auto it = users.cbegin(); it != users.cend(); ++it){
			
			uid_t uid;
			
			string name;
			
			string firstName;
			
			string lastName;
			
			string secondName;
			
			string description;
			
			string password;
			
			for(auto dt = it->vals.cbegin(); dt != it->vals.cend(); ++dt){
				
				if(!dt->second.empty()){
					
					if(dt->first.compare("uid") == 0) name = dt->second[0];
					
					else if(dt->first.compare("uidNumber") == 0) uid = ::atoi(dt->second[0].c_str());
					
					else if(dt->first.compare("cn") == 0) description = dt->second[0];
					
					else if(dt->first.compare("userPassword") == 0) password = dt->second[0];
					
					else if(dt->first.compare("givenName") == 0) firstName = dt->second[0];
					
					else if(dt->first.compare("sn") == 0) lastName = dt->second[0];
					
					else if(dt->first.compare("initials") == 0) secondName = dt->second[0];
				}
			}
			
			DataUser user = createDefaultData(uid, name);
			
			user.type = 2;
			
			user.pass = password;
			
			user.desc = (!firstName.empty() ? (!lastName.empty() ? (!secondName.empty() ? firstName + string(" ") + lastName + string(" ") + secondName : firstName + string(" ") + lastName) : firstName) : !description.empty() ? description : name);
			
			setDataUser(user);
			
			this->data.insert(pair <uid_t, DataUser>(user.id, user));
		}
		
		result = true;
	}
	
	return result;
}
 
const bool AUsers::Users::readUsersFromPam(){
	
	bool result = false;
	
	struct passwd * pw = nullptr;
	
	smatch match;
	
	regex e("\\/(?:sh|bash)$", regex::ECMAScript | regex::icase);
	
	while((pw = getpwent()) != nullptr){
		
		if(pw->pw_uid > 0){
			
			string shell = pw->pw_shell;
			
			regex_search(shell, match, e);
			
			if(!match.empty()){
				
				DataUser user = createDefaultData(pw->pw_uid, pw->pw_name);
				
				user.type = 1;
				
				user.pass = pw->pw_passwd;
				
				user.desc = pw->pw_name;
				
				setDataUser(user);
				
				this->data.insert(pair <uid_t, DataUser>(user.id, user));
				
				result = true;
			}
		}
	}
	
	return result;
}
 
const bool AUsers::Users::readUsersFromFile(){
	
	bool result = false;
	
	const string filename = Anyks::addToPath(this->config->proxy.dir, "users.ini");
	
	if(!filename.empty() && Anyks::isFileExist(filename.c_str())){
		
		INI ini(filename);
		
		if(!ini.isError()){
			
			string filepass;
			
			auto users = ini.getParamsInSection("users");
			
			auto passwords = ini.getParamsInSection("passwords");
			
			auto descriptions = ini.getParamsInSection("descriptions");
			
			if(!users.empty()){
				
				for(auto ut = users.cbegin(); ut != users.cend(); ++ut){
					
					if(Anyks::isNumber(ut->key) || Anyks::isNumber(ut->value)){
						
						const uid_t uid = (Anyks::isNumber(ut->key) ? ::atoi(ut->key.c_str()) : ::atoi(ut->value.c_str()));
						
						const string name = (Anyks::isNumber(ut->key) ? ut->value : ut->key);
						
						DataUser user = createDefaultData(uid, name);
						
						user.type = 0;
						
						if(!passwords.empty()){
							
							for(auto up = passwords.cbegin(); up != passwords.cend(); ++up){
								
								const string key = Anyks::toCase(up->key);
								
								if(key.compare("all") != 0){
									
									if((Anyks::isNumber(key)
									&& (uid_t(::atoi(key.c_str())) == user.id))
									|| (key.compare(user.name) == 0)) user.pass = getPasswordFromFile(up->value, this->log, user.id, user.name);
								
								} else if(filepass.empty()) filepass = up->value;
							}
						}
						
						if(!descriptions.empty()){
							
							for(auto ud = descriptions.cbegin(); ud != descriptions.cend(); ++ud){
								
								if((Anyks::isNumber(ud->key)
								&& (uid_t(::atoi(ud->key.c_str())) == user.id))
								|| (Anyks::toCase(ud->key).compare(user.name) == 0)) user.desc = ud->value;
							}
						}
						
						setDataUser(user, &ini);
						
						this->data.insert(pair <uid_t, DataUser>(user.id, user));
						
						result = true;
					}
				}
				
				if(!filepass.empty()) getPasswordsFromFile(filepass, this->log, this, AMING_USER);
			}
		}
	}
	
	return result;
}
 
const bool AUsers::Users::update(){
	
	bool result = false;
	
	time_t curUpdate = time(nullptr);
	
	if((this->lastUpdate + this->maxUpdate) < curUpdate){
		
		this->lastUpdate = curUpdate;
		
		this->data.clear();
		
		switch(this->typeSearch){
			
			case 0: readUsersFromFile(); break;
			
			case 1: readUsersFromPam(); break;
			
			case 2: readUsersFromLdap(); break;
			
			case 3: {
				readUsersFromFile();
				readUsersFromPam();
			} break;
			
			case 4: {
				readUsersFromFile();
				readUsersFromLdap();
			} break;
			
			case 5: {
				readUsersFromPam();
				readUsersFromLdap();
			} break;
			
			case 6: {
				readUsersFromFile();
				readUsersFromPam();
				readUsersFromLdap();
			} break;
		}
		
		result = true;
	}
	
	return result;
}
 
const vector <const AUsers::DataUser *> AUsers::Users::getAllUsers(){
	
	vector <const DataUser *> result;
	
	if(!this->data.empty()){
		
		for(auto it = this->data.cbegin(); it != this->data.cend(); ++it){
			
			result.push_back(&(it->second));
		}
	}
	
	return result;
}
 
const AUsers::DataUser * AUsers::Users::getUserByConnect(const string ip, const string mac){
	
	if(!ip.empty() || !mac.empty()){
		
		if(!this->data.empty()){
			
			string cmac = mac;
			
			const DataUser * user = nullptr;
			
			Network nwk;
			
			u_int nettype = (!ip.empty() ? nwk.checkNetworkByIp(ip) : 0);
			
			for(auto it = this->data.cbegin(); it != this->data.cend(); ++it){
				
				bool _ip = false;
				
				bool _mac = false;
				
				for(auto iit = it->second.idnt.cbegin(); iit != it->second.idnt.cend(); ++iit){
					
					const u_int idnt = Anyks::getTypeAmingByString(* iit);
					
					switch(idnt){
						case AMING_IPV4: {
							
							if((nettype == 4) && (ip.compare(* iit) == 0)) _ip = true;
						} break;
						case AMING_IPV6: {
							
							if((nettype == 6) && nwk.compareIP6(ip, * iit)) _ip = true;
						} break;
						case AMING_MAC: {
							
							if(!cmac.empty() && (Anyks::toCase(* iit).compare(Anyks::toCase(cmac)) == 0)) _mac = true;
						} break;
					}
				}
				
				if(_ip && _mac) return &(it->second);
				
				else if((_ip || _mac) && (user == nullptr)) user = &(it->second);
			}
			
			return user;
		}
	}
	
	return nullptr;
}
 
const AUsers::DataUser * AUsers::Users::getDataById(const uid_t uid){
	
	if(uid && !this->data.empty()){
		
		if(this->data.count(uid)){
			
			return &(this->data.find(uid)->second);
		
		} else if(update()) {
			
			return getDataById(uid);
		}
	}
	
	return nullptr;
}
 
const AUsers::DataUser * AUsers::Users::getDataByName(const string userName){
	
	if(!userName.empty() && !this->data.empty()){
		
		string name = Anyks::toCase(userName);
		
		for(auto it = this->data.cbegin(); it != this->data.cend(); ++it){
			
			if(it->second.name.compare(name) == 0){
				
				return &(it->second);
			}
		}
		
		if(update()) return getDataByName(userName);
	}
	
	return nullptr;
}
 
const bool AUsers::Users::checkUserById(const uid_t uid){
	
	if(uid && !this->data.empty()){
		
		if(this->data.count(uid)){
			
			return true;
		
		} else if(update()) {
			
			return checkUserById(uid);
		}
	}
	
	return false;
}
 
const bool AUsers::Users::checkUserByName(const string userName){
	
	return (getIdByName(userName) > -1 ? true : false);
}
 
const uid_t AUsers::Users::getIdByName(const string userName){
	
	if(!userName.empty() && !this->data.empty()){
		
		string name = Anyks::toCase(userName);
		
		for(auto it = this->data.cbegin(); it != this->data.cend(); ++it){
			
			if(it->second.name.compare(name) == 0){
				
				return it->first;
			}
		}
		
		if(update()) return getIdByName(userName);
	}
	
	return -1;
}
 
const string AUsers::Users::getNameById(const uid_t uid){
	
	if(uid && !this->data.empty()){
		
		if(this->data.count(uid)){
			
			return this->data.find(uid)->second.name;
		
		} else if(update()) {
			
			return getNameById(uid);
		}
	}
	
	return string();
}
 
const vector <uid_t> AUsers::Users::getIdAllUsers(){
	
	vector <uid_t> result;
	
	auto users = getAllUsers();
	
	if(!users.empty()){
		
		for(auto it = users.cbegin(); it != users.cend(); ++it){
			
			result.push_back((* it)->id);
		}
		
		sort(result.begin(), result.end());
		
		result.resize(unique(result.begin(), result.end()) - result.begin());
	}
	
	return result;
}
 
void AUsers::Users::setPassword(const uid_t uid, const string password){
	
	if((uid > -1) && !password.empty() && !this->data.empty()){
		
		if(this->data.count(uid)){
			
			(this->data.find(uid)->second).pass = password;
		
		} else if(update()) {
			
			setPassword(uid, password);
		}
	}
}
 
void AUsers::Users::setGroups(void * groups){
	
	this->groups = groups;
	
	update();
}
 
AUsers::Users::Users(Config * config, LogApp * log){
	
	if(config){
		
		this->log = log;
		
		this->config = config;
		
		this->typeSearch = 0;
		
		this->typeConfigs = 0;
		
		this->maxUpdate = 600;
		
		this->ldap = {
			"ou=users,dc=agro24,dc=dev",
			"ou=configs,ou=aming,dc=agro24,dc=dev",
			"one",
			"one",
			"(&(!(agro24CoJpDismissed=TRUE))(objectClass=inetOrgPerson))",
			"(&(amingConfigsUserId=%u)(amingConfigsType=users)(objectClass=amingConfigs))"
		};
	}
}
