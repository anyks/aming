/* SERVER PROXY AMING */
/*
*  author:     Yuriy Lobarev
*  phone:      +7(910)983-95-90
*  telegram:   @forman
*  email:      info@anyks.com
*  date:       11/23/2017 17:50:05
*  copyright:  © 2017 anyks.com
*/
 
 

#include "groups/files.h"


using namespace std;

 
const AParams::Params Gfiles::setParams(const gid_t gid, const string name){
	
	AParams::Params params = createDefaultParams(gid);
	
	if((gid > 0) && !name.empty()){
		
		const string filename = Anyks::addToPath(this->config->proxy.dir, "groups.ini");
		
		if(!filename.empty() && Anyks::isFileExist(filename.c_str())){
			
			INI ini = INI(filename);
			
			params.idnt = Anyks::split(ini.getString("identificators", name), "|");
			
			const string nGzip		= (name + "_gzip");
			const string nIPv4		= (name + "_ipv4");
			const string nIPv6		= (name + "_ipv6");
			const string nProxy		= (name + "_proxy");
			const string nSpeed		= (name + "_speed");
			const string nTimeouts	= (name + "_timeouts");
			const string nConnects	= (name + "_connects");
			const string nKeepAlive	= (name + "_keepalive");
			
			const string gGzip		= Anyks::strFormat("%u_%s", gid, "_gzip");
			const string gIPv4		= Anyks::strFormat("%u_%s", gid, "_ipv4");
			const string gIPv6		= Anyks::strFormat("%u_%s", gid, "_ipv6");
			const string gProxy		= Anyks::strFormat("%u_%s", gid, "_proxy");
			const string gSpeed		= Anyks::strFormat("%u_%s", gid, "_speed");
			const string gTimeouts	= Anyks::strFormat("%u_%s", gid, "_timeouts");
			const string gConnects	= Anyks::strFormat("%u_%s", gid, "_connects");
			const string gKeepAlive	= Anyks::strFormat("%u_%s", gid, "_keepalive");
			 
			auto setList = [&ini, this](const string name, const string gname, const string param, vector <string> &list){
				
				vector <string> data;
				
				if(ini.checkParam(name, param)){
					
					data = Anyks::split(ini.getString(name, param), "|");
				
				} else if(ini.checkParam(gname, param)){
					
					data = Anyks::split(ini.getString(gname, param), "|");
				}
				
				list.assign(data.cbegin(), data.cend());
			};
			 
			auto setSeconds = [&ini, this](const string name, const string gname, const string param, size_t &seconds){
				
				if(ini.checkParam(name, param)){
					
					seconds = (size_t) Anyks::getSeconds(ini.getString(name, param));
				
				} else if(ini.checkParam(gname, param)){
					
					seconds = (size_t) Anyks::getSeconds(ini.getString(gname, param));
				}
			};
			 
			auto setBuffers = [&ini, this](const string name, const string gname, const string param, long &buffer){
				
				if(ini.checkParam(name, param)){
					
					buffer = Anyks::getSizeBuffer(ini.getString(name, param));
				
				} else if(ini.checkParam(gname, param)){
					
					buffer = Anyks::getSizeBuffer(ini.getString(gname, param));
				}
			};
			 
			auto setBytes = [&ini, this](const string name, const string gname, const string param, size_t &value){
				
				if(ini.checkParam(name, param)){
					
					value = Anyks::getBytes(ini.getString(name, param));
				
				} else if(ini.checkParam(gname, param)){
					
					value = Anyks::getBytes(ini.getString(gname, param));
				}
			};
			 
			auto setNumbers = [&ini, this](const string name, const string gname, const string param, int &number){
				
				if(ini.checkParam(name, param)){
					
					number = (int) ini.getNumber(name, param);
				
				} else if(ini.checkParam(gname, param)){
					
					number = (int) ini.getNumber(gname, param);
				}
			};
			 
			auto setUNumbers = [&ini, this](const string name, const string gname, const string param, u_int &number){
				
				if(ini.checkParam(name, param)){
					
					number = (u_int) ini.getUNumber(name, param);
				
				} else if(ini.checkParam(gname, param)){
					
					number = (u_int) ini.getUNumber(gname, param);
				}
			};
			 
			auto setLNumbers = [&ini, this](const string name, const string gname, const string param, long &number){
				
				if(ini.checkParam(name, param)){
					
					number = ini.getNumber(name, param);
				
				} else if(ini.checkParam(gname, param)){
					
					number = ini.getNumber(gname, param);
				}
			};
			 
			auto setBool = [&ini, this](const string name, const string gname, const string param, bool &value){
				
				if(ini.checkParam(name, param)){
					
					value = ini.getBoolean(name, param);
				
				} else if(ini.checkParam(gname, param)){
					
					value = ini.getBoolean(gname, param);
				}
			};
			 
			auto setString = [&ini, this](const string name, const string gname, const string param, string &str){
				
				if(ini.checkParam(name, param)){
					
					str = ini.getString(name, param);
				
				} else if(ini.checkParam(gname, param)){
					
					str = ini.getString(gname, param);
				}
			};
			 
			auto setOptions = [&params, &ini, this](const string name, const string gname, const string param, const u_short flag){
				
				short result = -1;
				
				if(ini.checkParam(name, param)){
					
					result = (ini.getBoolean(name, param) ? 1 : 0);
				
				} else if(ini.checkParam(gname, param)){
					
					result = (ini.getBoolean(gname, param) ? 1 : 0);
				}
				
				if(result > -1) Anyks::setOptions(flag, params.options, bool(result));
			};
			 
			auto setOptionsByParam = [&params, &ini, this](const string name, const string gname, const string param, const string defstr, const string findstr, const u_short flag){
				
				short result = -1;
				
				if(ini.checkParam(name, param)){
					
					result = (ini.getString(name, param, defstr).compare(findstr) == 0 ? 1 : 0);
				
				} else if(ini.checkParam(gname, param)){
					
					result = (ini.getString(gname, param, defstr).compare(findstr) == 0 ? 1 : 0);
				}
				
				if(result > -1) Anyks::setOptions(flag, params.options, bool(result));
			};
			 
			auto setLevel = [&ini, this](const string name, const string gname, int &level){
				
				string gzipLevel;
				
				if(ini.checkParam(name, "level")){
					
					gzipLevel = ini.getString(name, "level");
				
				} else if(ini.checkParam(gname, "level")){
					
					gzipLevel = ini.getString(gname, "level");
				}
				
				if(!gzipLevel.empty()){
					
					if(gzipLevel.compare("default") == 0)		level = Z_DEFAULT_COMPRESSION;
					else if(gzipLevel.compare("best") == 0)		level = Z_BEST_COMPRESSION;
					else if(gzipLevel.compare("speed") == 0)	level = Z_BEST_SPEED;
					else if(gzipLevel.compare("no") == 0)		level = Z_NO_COMPRESSION;
				}
			};
			
			setOptions(nProxy, gProxy, "connect", OPT_CONNECT);
			setOptions(nProxy, gProxy, "upgrade", OPT_UPGRADE);
			setOptions(nProxy, gProxy, "agent", OPT_AGENT);
			setOptions(nProxy, gProxy, "deblock", OPT_DEBLOCK);
			setOptions(nGzip, gGzip, "transfer", OPT_GZIP);
			setOptions(nGzip, gGzip, "response", OPT_PGZIP);
			setOptions(nKeepAlive, gKeepAlive, "enabled", OPT_KEEPALIVE);
			setOptionsByParam(nProxy, gProxy, "skill", "dumb", "smart", OPT_SMART);
			setList(nIPv4, gIPv4, "external", params.ipv4.ip);
			setList(nIPv6, gIPv6, "external", params.ipv6.ip);
			setList(nIPv4, gIPv4, "resolver", params.ipv4.resolver);
			setList(nIPv6, gIPv6, "resolver", params.ipv6.resolver);
			setList(nGzip, gGzip, "vhttp", params.gzip.vhttp);
			setList(nGzip, gGzip, "types", params.gzip.types);
			setList(nGzip, gGzip, "proxied", params.gzip.proxied);
			setList(nProxy, gProxy, "redirect", params.proxy.redirect);
			setLNumbers(nGzip, gGzip, "length", params.gzip.length);
			setNumbers(nKeepAlive, gKeepAlive, "keepcnt", params.keepalive.keepcnt);
			setNumbers(nKeepAlive, gKeepAlive, "keepidle", params.keepalive.keepidle);
			setNumbers(nKeepAlive, gKeepAlive, "keepintvl", params.keepalive.keepintvl);
			setUNumbers(nConnects, gConnects, "connect", params.connects.connect);
			setSeconds(nTimeouts, gTimeouts, "read", params.timeouts.read);
			setSeconds(nTimeouts, gTimeouts, "write", params.timeouts.write);
			setSeconds(nTimeouts, gTimeouts, "upgrade", params.timeouts.upgrade);
			setBuffers(nSpeed, gSpeed, "input", params.buffers.read);
			setBuffers(nSpeed, gSpeed, "output", params.buffers.write);
			setBool(nProxy, gProxy, "subnet", params.proxy.subnet);
			setBool(nProxy, gProxy, "reverse", params.proxy.reverse);
			setBool(nProxy, gProxy, "forward", params.proxy.forward);
			setBool(nProxy, gProxy, "transfer", params.proxy.transfer);
			setBool(nProxy, gProxy, "pipelining", params.proxy.pipelining);
			setBool(nGzip, gGzip, "vary", params.gzip.vary);
			setBytes(nGzip, gGzip, "chunk", params.gzip.chunk);
			setBytes(nConnects, gConnects, "size", params.connects.size);
			setString(nGzip, gGzip, "regex", params.gzip.regex);
			setLevel(nGzip, gGzip, params.gzip.level);
		}
	}
	
	return params;
}
 
const AParams::Params Gfiles::createDefaultParams(const gid_t gid){
	
	return {
		
		gid,
		
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
}
 
const vector <AParams::Group> Gfiles::readGroups(){
	
	vector <AParams::Group> result = {{0, "", "", ""}};
	
	time_t curUpdate = time(nullptr);
	
	if((this->lastUpdate + this->config->proxy.conftime) < curUpdate){
		
		this->lastUpdate = curUpdate;
		
		this->groups.clear();
		
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
							
							vector <uid_t> uids;
							
							string pass, desc;
							
							const gid_t gid = (Anyks::isNumber(it->key) ? ::atoi(it->key.c_str()) : ::atoi(it->value.c_str()));
							
							const string name = (Anyks::isNumber(it->key) ? it->value : it->key);
							
							if((gid > 0) && !name.empty()){
								
								if(!users.empty()){
									
									for(auto it = users.cbegin(); it != users.cend(); ++it){
										
										if((Anyks::isNumber(it->value)
										&& (gid_t(::atoi(it->value.c_str())) == gid))
										|| (Anyks::toCase(it->value).compare(name) == 0)){
											
											uid_t uid = 0;
											
											if(Anyks::isNumber(it->key)) uid = ::atoi(it->key.c_str());
											
											else uid = this->getUidByUserName(it->key, AUSERS_TYPE_FILE);
											
											if(uid > 0) uids.push_back(uid);
										}
									}
								}
								
								if(!passwords.empty()){
									
									for(auto it = passwords.cbegin(); it != passwords.cend(); ++it){
										
										const string key = Anyks::toCase(it->key);
										
										if(key.compare("all") != 0){
											
											if((Anyks::isNumber(key)
											&& (gid_t(::atoi(key.c_str())) == gid))
											|| (key.compare(name) == 0)) pass = this->getPassword(it->value, this->log, gid, name);
										
										} else if(filepass.empty()) filepass = it->value;
									}
								}
								
								if(!descriptions.empty()){
									
									for(auto it = descriptions.cbegin(); it != descriptions.cend(); ++it){
										
										if((Anyks::isNumber(it->key)
										&& (gid_t(::atoi(it->key.c_str())) == gid))
										|| (Anyks::toCase(it->key).compare(name) == 0)) desc = it->value;
									}
									
									if(desc.empty()) desc = name;
								}
								
								this->groups.insert(pair <gid_t, AParams::GroupData> (gid, {gid, AUSERS_TYPE_FILE, name, desc, pass, uids}));
								
								result.push_back({gid, name, desc, pass});
							}
						}
					}
					
					if(!filepass.empty()) this->getPasswords(filepass, this->log, this, AMING_GROUP);
				}
			}
		}
	
	} else if(!this->groups.empty()) {
		
		for(auto it = this->groups.cbegin(); it != this->groups.cend(); ++it){
			
			result.push_back({it->second.gid, it->second.name, it->second.desc, it->second.pass});
		}
	}
	
	return result;
}
 
const vector <AParams::GroupData> Gfiles::getAllGroups(){
	
	vector <AParams::GroupData> result;
	
	readGroups();
	
	if(!this->groups.empty()){
		
		for(auto it = this->groups.cbegin(); it != this->groups.cend(); ++it){
			
			result.push_back(it->second);
		}
	}
	
	return result;
}
 
const AParams::GroupData Gfiles::getDataById(const gid_t gid){
	
	AParams::GroupData result;
	
	if(gid > 0){
		
		if(!this->groups.empty() && (this->groups.count(gid) > 0)){
			
			result = this->groups.find(gid)->second;
		
		} else if(!(readGroups()).empty()) {
			
			if(this->groups.count(gid) > 0) result = this->groups.find(gid)->second;
		}
	}
	
	return result;
}
 
const vector <gid_t> Gfiles::getGroupIdByUser(const uid_t uid){
	
	vector <gid_t> result;
	
	if(uid > 0){
		
		auto groups = getAllGroups();
		
		if(!groups.empty()){
			
			for(auto it = groups.cbegin(); it != groups.cend(); ++it){
				
				auto users = it->users;
				
				if(find(users.begin(), users.end(), uid) != users.end()){
					
					result.push_back(it->gid);
				}
			}
		}
	}
	
	return result;
}
 
const vector <string> Gfiles::getGroupNameByUser(const uid_t uid){
	
	vector <string> result;
	
	if(uid > 0){
		
		auto groups = getAllGroups();
		
		if(!groups.empty()){
			
			for(auto it = groups.cbegin(); it != groups.cend(); ++it){
				
				auto users = it->users;
				
				if(find(users.begin(), users.end(), uid) != users.end()){
					
					result.push_back(it->name);
				}
			}
		}
	}
	
	return result;
}
 
const bool Gfiles::checkUser(const gid_t gid, const uid_t uid){
	
	bool result = false;
	
	if((gid > 0) && (uid > 0)){
		
		auto users = getIdUsers(gid);
		
		if(find(users.begin(), users.end(), uid) != users.end()) result = true;
	}
	
	return result;
}
 
const bool Gfiles::checkUser(const string groupName, const uid_t uid){
	
	bool result = false;
	
	if((uid > 0) && !groupName.empty()){
		
		const gid_t gid = getIdByName(groupName);
		
		if(gid > 0) result = checkUser(gid, uid);
	}
	
	return result;
}
 
const bool Gfiles::checkGroupById(const gid_t gid){
	
	return !(getNameById(gid)).empty();
}
 
const bool Gfiles::checkGroupByName(const string groupName){
	
	return (getIdByName(groupName) > 0 ? true : false);
}
 
const gid_t Gfiles::getIdByName(const string groupName){
	
	gid_t result = 0;
	
	if(!groupName.empty()){
		
		readGroups();
		
		if(!this->groups.empty()){
			
			string name = Anyks::toCase(groupName);
			
			for(auto it = this->groups.cbegin(); it != this->groups.cend(); ++it){
				
				if(it->second.name.compare(name) == 0){
					
					result = it->second.gid;
					
					break;
				}
			}
		}
	}
	
	return result;
}
 
const string Gfiles::getNameById(const gid_t gid){
	
	string result;
	
	if(gid > 0){
		
		if(!this->groups.empty() && (this->groups.count(gid) > 0)){
			
			result = this->groups.find(gid)->second.name;
		
		} else if(!(readGroups()).empty()) {
			
			if(this->groups.count(gid) > 0) result = this->groups.find(gid)->second.name;
		}
	}
	
	return result;
}
 
const vector <string> Gfiles::getNameUsers(const gid_t gid){
	
	vector <string> result;
	
	if(gid > 0){
		
		auto users = getIdUsers(gid);
		
		if(!users.empty()){
			
			for(auto it = users.cbegin(); it != users.cend(); ++it){
				
				const string userName = this->getUserNameByUid(* it, AUSERS_TYPE_FILE);
				
				if(!userName.empty()) result.push_back(userName);
			}
		}
	}
	
	return result;
}
 
const vector <uid_t> Gfiles::getIdUsers(const gid_t gid){
	
	vector <uid_t> result;
	
	if(gid > 0){
		
		if(!this->groups.empty() && (this->groups.count(gid) > 0)){
			
			result = this->groups.find(gid)->second.users;
		
		} else if(!(readGroups()).empty()) {
			
			if(this->groups.count(gid) > 0) result = this->groups.find(gid)->second.users;
		}
	}
	
	return result;
}
 
void Gfiles::setPassword(const gid_t gid, const string password){
	
	if((gid > 0) && !password.empty()){
		
		if(!this->groups.empty() && (this->groups.count(gid) > 0)){
			
			(this->groups.find(gid)->second).pass = password;
		
		} else if(!(readGroups()).empty()) {
			
			if(this->groups.count(gid) > 0) (this->groups.find(gid)->second).pass = password;
		}
	}
}
 
void Gfiles::setUsersMethods(function <string (uid_t, u_short)> method1, function <uid_t (string, u_short)> method2){
	
	this->getUserNameByUid = method1;
	
	this->getUidByUserName = method2;
}
 
void Gfiles::setPasswordsMethod(function <void (const string, LogApp * log, void * object, const u_short)> method){
	
	this->getPasswords = method;
}
 
void Gfiles::setPasswordMethod(function <const string (const string, LogApp * log, const uid_t, const string)> method){
	
	this->getPassword = method;
}
 
Gfiles::Gfiles(Config * config, LogApp * log){
	
	if(config != nullptr){
		
		this->log = log;
		
		this->config = config;
	}
}
