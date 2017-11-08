/* SERVER PROXY AMING */
/*
*  author:     Yuriy Lobarev
*  phone:      +7(910)983-95-90
*  telegram:   @forman
*  email:      info@anyks.com
*  date:       11/08/2017 16:52:48
*  copyright:  © 2017 anyks.com
*/
 
 

#include "users/files.h"


using namespace std;

 
const AParams::Params Ufiles::createDefaultParams(const uid_t uid){
	
	AParams::Params result;
	
	if(uid > 0){
		
		auto groups = this->getGroupIdByUser(uid, AUSERS_TYPE_FILE);
		
		if(!groups.empty()){
			
			gid_t gid = 1000000;
			
			for(auto it = groups.cbegin(); it != groups.cend(); ++it){
				
				if((* it) < gid) gid = (* it);
			}
			
			auto * params = this->getParamsByGid(gid, AUSERS_TYPE_FILE);
			
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
					params->proxy.pipelining
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
	}
	
	return result;
}
 
const AParams::Params Ufiles::setParams(const uid_t uid, const string name){
	
	AParams::Params params = createDefaultParams(uid);
	
	if((uid > 0) && !name.empty()){
		
		const string filename = Anyks::addToPath(this->config->proxy.dir, "users.ini");
		
		if(!filename.empty() && Anyks::isFileExist(filename.c_str())){
			
			INI * ini = new INI(filename);
			
			params.idnt = Anyks::split(ini->getString("identificators", name), "|");
			
			const string nGzip		= (name + "_gzip");
			const string nIPv4		= (name + "_ipv4");
			const string nIPv6		= (name + "_ipv6");
			const string nProxy		= (name + "_proxy");
			const string nSpeed		= (name + "_speed");
			const string nTimeouts	= (name + "_timeouts");
			const string nConnects	= (name + "_connects");
			const string nKeepAlive	= (name + "_keepalive");
			
			const string uGzip		= Anyks::strFormat("%u_%s", uid, "_gzip");
			const string uIPv4		= Anyks::strFormat("%u_%s", uid, "_ipv4");
			const string uIPv6		= Anyks::strFormat("%u_%s", uid, "_ipv6");
			const string uProxy		= Anyks::strFormat("%u_%s", uid, "_proxy");
			const string uSpeed		= Anyks::strFormat("%u_%s", uid, "_speed");
			const string uTimeouts	= Anyks::strFormat("%u_%s", uid, "_timeouts");
			const string uConnects	= Anyks::strFormat("%u_%s", uid, "_connects");
			const string uKeepAlive	= Anyks::strFormat("%u_%s", uid, "_keepalive");
			 
			auto setList = [ini, this](const string name, const string uname, const string param, vector <string> &list){
				
				vector <string> data;
				
				if(ini->checkParam(name, param)){
					
					data = Anyks::split(ini->getString(name, param), "|");
				
				} else if(ini->checkParam(uname, param)){
					
					data = Anyks::split(ini->getString(uname, param), "|");
				}
				
				list.assign(data.cbegin(), data.cend());
			};
			 
			auto setSeconds = [ini, this](const string name, const string uname, const string param, size_t &seconds){
				
				if(ini->checkParam(name, param)){
					
					seconds = (size_t) Anyks::getSeconds(ini->getString(name, param));
				
				} else if(ini->checkParam(uname, param)){
					
					seconds = (size_t) Anyks::getSeconds(ini->getString(uname, param));
				}
			};
			 
			auto setBuffers = [ini, this](const string name, const string uname, const string param, long &buffer){
				
				if(ini->checkParam(name, param)){
					
					buffer = Anyks::getSizeBuffer(ini->getString(name, param));
				
				} else if(ini->checkParam(uname, param)){
					
					buffer = Anyks::getSizeBuffer(ini->getString(uname, param));
				}
			};
			 
			auto setBytes = [ini, this](const string name, const string uname, const string param, size_t &value){
				
				if(ini->checkParam(name, param)){
					
					value = Anyks::getBytes(ini->getString(name, param));
				
				} else if(ini->checkParam(uname, param)){
					
					value = Anyks::getBytes(ini->getString(uname, param));
				}
			};
			 
			auto setNumbers = [ini, this](const string name, const string uname, const string param, int &number){
				
				if(ini->checkParam(name, param)){
					
					number = (int) ini->getNumber(name, param);
				
				} else if(ini->checkParam(uname, param)){
					
					number = (int) ini->getNumber(uname, param);
				}
			};
			 
			auto setUNumbers = [ini, this](const string name, const string uname, const string param, u_int &number){
				
				if(ini->checkParam(name, param)){
					
					number = (u_int) ini->getUNumber(name, param);
				
				} else if(ini->checkParam(uname, param)){
					
					number = (u_int) ini->getUNumber(uname, param);
				}
			};
			 
			auto setLNumbers = [ini, this](const string name, const string uname, const string param, long &number){
				
				if(ini->checkParam(name, param)){
					
					number = ini->getNumber(name, param);
				
				} else if(ini->checkParam(uname, param)){
					
					number = ini->getNumber(uname, param);
				}
			};
			 
			auto setBool = [ini, this](const string name, const string uname, const string param, bool &value){
				
				if(ini->checkParam(name, param)){
					
					value = ini->getBoolean(name, param);
				
				} else if(ini->checkParam(uname, param)){
					
					value = ini->getBoolean(uname, param);
				}
			};
			 
			auto setString = [ini, this](const string name, const string uname, const string param, string &str){
				
				if(ini->checkParam(name, param)){
					
					str = ini->getString(name, param);
				
				} else if(ini->checkParam(uname, param)){
					
					str = ini->getString(uname, param);
				}
			};
			 
			auto setOptions = [&params, ini, this](const string name, const string uname, const string param, const u_short flag){
				
				short result = -1;
				
				if(ini->checkParam(name, param)){
					
					result = (ini->getBoolean(name, param) ? 1 : 0);
				
				} else if(ini->checkParam(uname, param)){
					
					result = (ini->getBoolean(uname, param) ? 1 : 0);
				}
				
				if(result > -1) Anyks::setOptions(flag, params.options, bool(result));
			};
			 
			auto setOptionsByParam = [&params, ini, this](const string name, const string uname, const string param, const string defstr, const string findstr, const u_short flag){
				
				short result = -1;
				
				if(ini->checkParam(name, param)){
					
					result = (ini->getString(name, param, defstr).compare(findstr) == 0 ? 1 : 0);
				
				} else if(ini->checkParam(uname, param)){
					
					result = (ini->getString(uname, param, defstr).compare(findstr) == 0 ? 1 : 0);
				}
				
				if(result > -1) Anyks::setOptions(flag, params.options, bool(result));
			};
			 
			auto setLevel = [ini, this](const string name, const string uname, int &level){
				
				string gzipLevel;
				
				if(ini->checkParam(name, "level")){
					
					gzipLevel = ini->getString(name, "level");
				
				} else if(ini->checkParam(uname, "level")){
					
					gzipLevel = ini->getString(uname, "level");
				}
				
				if(!gzipLevel.empty()){
					
					if(gzipLevel.compare("default") == 0)		level = Z_DEFAULT_COMPRESSION;
					else if(gzipLevel.compare("best") == 0)		level = Z_BEST_COMPRESSION;
					else if(gzipLevel.compare("speed") == 0)	level = Z_BEST_SPEED;
					else if(gzipLevel.compare("no") == 0)		level = Z_NO_COMPRESSION;
				}
			};
			
			setOptions(nProxy, uProxy, "connect", OPT_CONNECT);
			setOptions(nProxy, uProxy, "upgrade", OPT_UPGRADE);
			setOptions(nProxy, uProxy, "agent", OPT_AGENT);
			setOptions(nProxy, uProxy, "deblock", OPT_DEBLOCK);
			setOptions(nGzip, uGzip, "transfer", OPT_GZIP);
			setOptions(nGzip, uGzip, "response", OPT_PGZIP);
			setOptions(nKeepAlive, uKeepAlive, "enabled", OPT_KEEPALIVE);
			setOptionsByParam(nProxy, uProxy, "skill", "dumb", "smart", OPT_SMART);
			setList(nIPv4, uIPv4, "external", params.ipv4.ip);
			setList(nIPv6, uIPv6, "external", params.ipv6.ip);
			setList(nIPv4, uIPv4, "resolver", params.ipv4.resolver);
			setList(nIPv6, uIPv6, "resolver", params.ipv6.resolver);
			setList(nGzip, uGzip, "vhttp", params.gzip.vhttp);
			setList(nGzip, uGzip, "types", params.gzip.types);
			setList(nGzip, uGzip, "proxied", params.gzip.proxied);
			setLNumbers(nGzip, uGzip, "length", params.gzip.length);
			setNumbers(nKeepAlive, uKeepAlive, "keepcnt", params.keepalive.keepcnt);
			setNumbers(nKeepAlive, uKeepAlive, "keepidle", params.keepalive.keepidle);
			setNumbers(nKeepAlive, uKeepAlive, "keepintvl", params.keepalive.keepintvl);
			setUNumbers(nConnects, uConnects, "connect", params.connects.connect);
			setSeconds(nTimeouts, uTimeouts, "read", params.timeouts.read);
			setSeconds(nTimeouts, uTimeouts, "write", params.timeouts.write);
			setSeconds(nTimeouts, uTimeouts, "upgrade", params.timeouts.upgrade);
			setBuffers(nSpeed, uSpeed, "input", params.buffers.read);
			setBuffers(nSpeed, uSpeed, "output", params.buffers.write);
			setBool(nProxy, uProxy, "subnet", params.proxy.subnet);
			setBool(nProxy, uProxy, "reverse", params.proxy.reverse);
			setBool(nProxy, uProxy, "forward", params.proxy.forward);
			setBool(nProxy, uProxy, "transfer", params.proxy.transfer);
			setBool(nProxy, uProxy, "pipelining", params.proxy.pipelining);
			setBool(nGzip, uGzip, "vary", params.gzip.vary);
			setBytes(nGzip, uGzip, "chunk", params.gzip.chunk);
			setBytes(nConnects, uConnects, "size", params.connects.size);
			setString(nGzip, uGzip, "regex", params.gzip.regex);
			setLevel(nGzip, uGzip, params.gzip.level);
			
			delete ini;
		}
	}
	
	return params;
}
 
const vector <AParams::User> Ufiles::readUsers(){
	
	vector <AParams::User> result = {{0, "", "", ""}};
	
	time_t curUpdate = time(nullptr);
	
	if((this->lastUpdate + this->config->auth.update) < curUpdate){
		
		this->lastUpdate = curUpdate;
		
		this->users.clear();
		
		const string filename = Anyks::addToPath(this->config->proxy.dir, "users.ini");
		
		if(!filename.empty() && Anyks::isFileExist(filename.c_str())){
			
			INI ini(filename);
			
			if(!ini.isError()){
				
				string filepass;
				
				auto users = ini.getParamsInSection("users");
				
				auto passwords = ini.getParamsInSection("passwords");
				
				auto descriptions = ini.getParamsInSection("descriptions");
				
				if(!users.empty()){
					
					for(auto it = users.cbegin(); it != users.cend(); ++it){
						
						if(Anyks::isNumber(it->key) || Anyks::isNumber(it->value)){
							
							string pass, desc;
							
							const uid_t uid = (Anyks::isNumber(it->key) ? ::atoi(it->key.c_str()) : ::atoi(it->value.c_str()));
							
							const string name = (Anyks::isNumber(it->key) ? it->value : it->key);
							
							if(!passwords.empty()){
								
								for(auto it = passwords.cbegin(); it != passwords.cend(); ++it){
									
									const string key = Anyks::toCase(it->key);
									
									if(key.compare("all") != 0){
										
										if((Anyks::isNumber(key)
										&& (uid_t(::atoi(key.c_str())) == uid))
										|| (key.compare(name) == 0)) pass = this->getPassword(it->value, this->log, uid, name);
									
									} else if(filepass.empty()) filepass = it->value;
								}
							}
							
							if(!descriptions.empty()){
								
								for(auto it = descriptions.cbegin(); it != descriptions.cend(); ++it){
									
									if((Anyks::isNumber(it->key)
									&& (uid_t(::atoi(it->key.c_str())) == uid))
									|| (Anyks::toCase(it->key).compare(name) == 0)) desc = it->value;
								}
								
								if(desc.empty()) desc = name;
							}
							
							this->users.insert(pair <uid_t, AParams::UserData> (uid, {uid, AUSERS_TYPE_FILE, name, desc, pass}));
							
							result.push_back({uid, name, desc, pass});
						}
					}
					
					if(!filepass.empty()) this->getPasswords(filepass, this->log, this, AMING_USER);
				}
			}
		}
	
	} else if(!this->users.empty()) {
		
		for(auto it = this->users.cbegin(); it != this->users.cend(); ++it){
			
			result.push_back({it->second.uid, it->second.name, it->second.desc, it->second.pass});
		}
	}
	
	return result;
}
 
const vector <AParams::UserData> Ufiles::getAllUsers(){
	
	vector <AParams::UserData> result;
	
	readUsers();
	
	if(!this->users.empty()){
		
		for(auto it = this->users.cbegin(); it != this->users.cend(); ++it){
			
			result.push_back(it->second);
		}
	}
	
	return result;
}
 
const AParams::UserData Ufiles::getDataById(const uid_t uid){
	
	AParams::UserData result;
	
	if((uid > 0) && (this->users.count(uid) > 0)){
		
		result = this->users.find(uid)->second;
	
	} else if((uid > 0) && !(readUsers()).empty()) {
		
		if(this->users.count(uid) > 0) result = this->users.find(uid)->second;
	}
	
	return result;
}
 
const bool Ufiles::checkUserById(const uid_t uid){
	
	return !(getNameById(uid)).empty();
}
 
const bool Ufiles::checkUserByName(const string userName){
	
	return (getIdByName(userName) > 0 ? true : false);
}
 
const uid_t Ufiles::getIdByName(const string userName){
	
	uid_t result = 0;
	
	if(!userName.empty() && !this->users.empty()){
		
		readUsers();
		
		string name = Anyks::toCase(userName);
		
		for(auto it = this->users.cbegin(); it != this->users.cend(); ++it){
			
			if(it->second.name.compare(name) == 0){
				
				result = it->second.uid;
				
				break;
			}
		}
	}
	
	return result;
}
 
const string Ufiles::getNameById(const uid_t uid){
	
	string result;
	
	if((uid > 0) && !this->users.empty()){
		
		if(this->users.count(uid) > 0){
			
			result = this->users.find(uid)->second.name;
		
		} else if((uid > 0) && !(readUsers()).empty()) {
			
			if(this->users.count(uid) > 0) result = this->users.find(uid)->second.name;
		}
	}
	
	return result;
}
 
void Ufiles::setPassword(const uid_t uid, const string password){
	
	if((uid > 0) && !password.empty() && !this->users.empty()){
		
		if(this->users.count(uid) > 0){
			
			(this->users.find(uid)->second).pass = password;
		
		} else if((uid > 0) && !(readUsers()).empty()) {
			
			if(this->users.count(uid) > 0) (this->users.find(uid)->second).pass = password;
		}
	}
}
 
void Ufiles::setGroupsMethods(function <string (gid_t, u_short)> method1, function <gid_t (string, u_short)> method2){
	
	this->getGroupNameByGid = method1;
	
	this->getGidByGroupName = method2;
}
 
void Ufiles::setPasswordsMethod(function <void (const string, LogApp * log, void * object, const u_short)> method){
	
	this->getPasswords = method;
}
 
void Ufiles::setPasswordMethod(function <const string (const string, LogApp * log, const uid_t, const string)> method){
	
	this->getPassword = method;
}
 
void Ufiles::setParamsMethod(function <const AParams::Params * (const gid_t gid, const u_short type)> method){
	
	this->getParamsByGid = method;
}
 
void Ufiles::setGidsMethod(function <const vector <gid_t> (const uid_t uid, const u_short type)> method){
	
	this->getGroupIdByUser = method;
}
 
Ufiles::Ufiles(Config * config, LogApp * log){
	
	if(config != nullptr){
		
		this->log = log;
		
		this->config = config;
	}
}
