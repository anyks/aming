/* SERVER PROXY AMING */
/*
*  author:     Yuriy Lobarev
*  phone:      +7(910)983-95-90
*  telegram:   @forman
*  email:      info@anyks.com
*  date:       11/08/2017 16:52:48
*  copyright:  © 2017 anyks.com
*/
 
 

#include "groups/ldap.h"


using namespace std;

 
const AParams::Params Gldap::setParams(const gid_t gid, const string name){
	
	AParams::Params params = createDefaultParams(gid);
	
	if((gid > 0) && !name.empty()){
		
		ALDAP ldap(this->config, this->log);
		
		const string dn = Anyks::strFormat("ac=%s,%s", name.c_str(), this->config->ldap.dn.configs.c_str());
		
		const string filter = Anyks::strFormat("(&%s(amingConfigsType=groups)(%s=%u))", this->config->ldap.filter.configs.c_str(), "amingConfigsGroupId", gid);
		
		const string keys =	"amingConfigsConnectsConnect,amingConfigsConnectsSize,amingConfigsGzipChunk,"
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
		
		auto groups = ldap.data(dn, keys, this->config->ldap.scope.configs, filter);
		
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
						
						if(dt->first.compare("amingConfigsIdnt") == 0) params.idnt = dt->second;
						
						else if(dt->first.compare("amingConfigsIpExternal4") == 0) params.ipv4.ip = dt->second;
						
						else if(dt->first.compare("amingConfigsIpExternal6") == 0) params.ipv6.ip = dt->second;
						
						else if(dt->first.compare("amingConfigsIpResolver4") == 0) params.ipv4.resolver = dt->second;
						
						else if(dt->first.compare("amingConfigsIpResolver6") == 0) params.ipv6.resolver = dt->second;
						
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
 
const AParams::Params Gldap::createDefaultParams(const gid_t gid){
	
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
			this->config->proxy.pipelining
		
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
 
const vector <AParams::Group> Gldap::readGroups(){
	
	vector <AParams::Group> result = {{0, "", "", ""}};
	
	ALDAP ldap(this->config, this->log);
	
	const string keys = Anyks::strFormat(
		"%s,%s,%s,%s",
		this->config->ldap.keys.groups.login.c_str(),
		this->config->ldap.keys.groups.desc.c_str(),
		this->config->ldap.keys.groups.gid.c_str(),
		this->config->ldap.keys.groups.pass.c_str()
	);
	
	auto groups = ldap.data(this->config->ldap.dn.groups, keys, this->config->ldap.scope.groups, this->config->ldap.filter.groups);
	
	if(!groups.empty()){
		
		for(auto it = groups.cbegin(); it != groups.cend(); ++it){
			
			gid_t gid = 0;
			
			string name, desc, pass;
			
			for(auto dt = it->vals.cbegin(); dt != it->vals.cend(); ++dt){
				
				if(!dt->second.empty()){
					
					if(dt->first.compare(this->config->ldap.keys.groups.gid) == 0) gid = ::atoi(dt->second[0].c_str());
					
					else if(dt->first.compare(this->config->ldap.keys.groups.login) == 0) name = Anyks::toCase(dt->second[0]);
					
					else if(dt->first.compare(this->config->ldap.keys.groups.desc) == 0) desc = dt->second[0];
					
					else if(dt->first.compare(this->config->ldap.keys.groups.pass) == 0) pass = dt->second[0];
				}
			}
			
			if((gid > 0) && !name.empty()) result.push_back({gid, name, desc, pass});
		}
	}
	
	return result;
}
 
const vector <AParams::GroupData> Gldap::getAllGroups(){
	
	vector <AParams::GroupData> result;
	
	ALDAP ldap(this->config, this->log);
	
	string keys = Anyks::strFormat(
		"%s,%s,%s,%s,%s",
		this->config->ldap.keys.groups.login.c_str(),
		this->config->ldap.keys.groups.desc.c_str(),
		this->config->ldap.keys.groups.gid.c_str(),
		this->config->ldap.keys.groups.pass.c_str(),
		this->config->ldap.keys.groups.member.c_str()
	);
	
	auto groups = ldap.data(this->config->ldap.dn.groups, keys, this->config->ldap.scope.groups, this->config->ldap.filter.groups);
	
	if(!groups.empty()){
		
		for(auto it = groups.cbegin(); it != groups.cend(); ++it){
			
			gid_t _gid = 0;
			
			vector <uid_t> _users;
			
			string _name, _desc, _pass;
			
			for(auto dt = it->vals.cbegin(); dt != it->vals.cend(); ++dt){
				
				if(!dt->second.empty()){
					
					if(dt->first.compare(this->config->ldap.keys.groups.login) == 0) _name = Anyks::toCase(dt->second[0]);
					
					else if(dt->first.compare(this->config->ldap.keys.groups.gid) == 0) _gid = ::atoi(dt->second[0].c_str());
					
					else if(dt->first.compare(this->config->ldap.keys.groups.desc) == 0) _desc = dt->second[0];
					
					else if(dt->first.compare(this->config->ldap.keys.groups.pass) == 0) _pass = dt->second[0];
					
					else if(dt->first.compare(this->config->ldap.keys.groups.member) == 0){
						
						for(auto ut = dt->second.cbegin(); ut != dt->second.cend(); ++ut){
							
							uid_t _uid = (Anyks::isNumber(* ut) ? ::atoi(ut->c_str()) : this->getUidByUserName(* ut, AUSERS_TYPE_LDAP));
							
							if(_uid > 0) _users.push_back(_uid);
						}
					}
				}
			}
			
			if(_gid > 0){
				
				result.push_back({
					_gid,
					AUSERS_TYPE_LDAP,
					_name,
					_desc,
					_pass,
					_users
				});
			}
		}
	}
	
	keys = Anyks::strFormat("%s,%s", this->config->ldap.keys.groups.gid.c_str(), this->config->ldap.keys.users.uid.c_str());
	
	auto users = ldap.data(this->config->ldap.dn.users, keys, this->config->ldap.scope.users, this->config->ldap.filter.users);
	
	if(!users.empty()){
		
		for(auto it = users.cbegin(); it != users.cend(); ++it){
			
			gid_t _gid = 0;
			
			vector <uid_t> _users;
			
			for(auto dt = it->vals.cbegin(); dt != it->vals.cend(); ++dt){
				
				if(!dt->second.empty()){
					
					if(dt->first.compare(this->config->ldap.keys.groups.gid) == 0) _gid = ::atoi(dt->second[0].c_str());
					
					else if(dt->first.compare(this->config->ldap.keys.users.uid) == 0) {
						
						uid_t _uid = ::atoi(dt->second[0].c_str());
						
						if(_uid > 0) _users.push_back(_uid);
					}
				}
			}
			
			for(auto group = result.cbegin(); group != result.cend(); ++group){
				
				if(group->gid == _gid){
					
					AParams::GroupData _group = {
						group->gid,
						group->type,
						group->name,
						group->desc,
						group->pass,
						Anyks::concatVectors <uid_t> (group->users, _users)
					};
					
					result.emplace(group, _group);
					
					break;
				}
			}
		}
	}
	
	return result;
}
 
const AParams::GroupData Gldap::getDataById(const gid_t gid){
	
	AParams::GroupData result;
	
	if(gid > 0){
		
		ALDAP ldap(this->config, this->log);
		
		const string filter = Anyks::strFormat("(&%s(%s=%u))", this->config->ldap.filter.groups.c_str(), this->config->ldap.keys.groups.gid.c_str(), gid);
		
		const string keys = Anyks::strFormat(
			"%s,%s,%s,%s",
			this->config->ldap.keys.groups.login.c_str(),
			this->config->ldap.keys.groups.desc.c_str(),
			this->config->ldap.keys.groups.pass.c_str(),
			this->config->ldap.keys.groups.member.c_str()
		);
		
		auto groups = ldap.data(this->config->ldap.dn.groups, keys, this->config->ldap.scope.groups, filter);
		
		if(!groups.empty()){
			
			for(auto it = groups.cbegin(); it != groups.cend(); ++it){
				
				vector <uid_t> users;
				
				string name, desc, pass;
				
				for(auto dt = it->vals.cbegin(); dt != it->vals.cend(); ++dt){
					
					if(!dt->second.empty()){
						
						if(dt->first.compare(this->config->ldap.keys.groups.login) == 0) name = dt->second[0];
						
						else if(dt->first.compare(this->config->ldap.keys.groups.desc) == 0) desc = dt->second[0];
						
						else if(dt->first.compare(this->config->ldap.keys.groups.pass) == 0) pass = dt->second[0];
						
						else if(dt->first.compare(this->config->ldap.keys.groups.member) == 0){
							
							for(auto ut = dt->second.cbegin(); ut != dt->second.cend(); ++ut){
								
								uid_t uid = (Anyks::isNumber(* ut) ? ::atoi(ut->c_str()) : this->getUidByUserName(* ut, AUSERS_TYPE_LDAP));
								
								if(uid > 0) users.push_back(uid);
							}
						}
					}
				}
				
				result = {gid, AUSERS_TYPE_LDAP, name, desc, pass, users};
				
				break;
			}
		}
	}
	
	return result;
}
 
const vector <gid_t> Gldap::getGroupIdByUser(const uid_t uid){
	
	vector <gid_t> result;
	
	if(uid > 0){
		
		string userName;
		
		ALDAP ldap(this->config, this->log);
		
		const string keys = Anyks::strFormat("%s,%s", this->config->ldap.keys.users.login.c_str(), this->config->ldap.keys.groups.gid.c_str());
		
		const string filter = Anyks::strFormat("(&%s(%s=%u))", this->config->ldap.filter.users.c_str(), this->config->ldap.keys.users.uid.c_str(), uid);
		
		auto users = ldap.data(this->config->ldap.dn.users, keys, this->config->ldap.scope.users, filter);
		
		if(!users.empty()){
			
			for(auto it = users.cbegin(); it != users.cend(); ++it){
				
				for(auto dt = it->vals.cbegin(); dt != it->vals.cend(); ++dt){
					
					if(!dt->second.empty()){
						
						if(dt->first.compare(this->config->ldap.keys.users.login) == 0) userName = dt->second[0];
						
						else if(dt->first.compare(this->config->ldap.keys.groups.gid) == 0){
							
							const gid_t gid = ::atoi(dt->second[0].c_str());
							
							if(gid > 0) result.push_back(gid);
						}
					}
				}
			}
			
			if(!userName.empty()){
				
				const string filter = Anyks::strFormat(
					"(&%s(|(%s=%s)(%s=%u)))",
					this->config->ldap.filter.groups.c_str(),
					this->config->ldap.keys.groups.member.c_str(),
					userName.c_str(),
					this->config->ldap.keys.groups.member.c_str(),
					uid
				);
				
				auto groups = ldap.data(this->config->ldap.dn.groups, this->config->ldap.keys.groups.gid, this->config->ldap.scope.groups, filter);
				
				if(!groups.empty()){
					
					for(auto it = groups.cbegin(); it != groups.cend(); ++it){
						
						for(auto dt = it->vals.cbegin(); dt != it->vals.cend(); ++dt){
							
							if(!dt->second.empty() && (dt->first.compare(this->config->ldap.keys.groups.gid) == 0)){
								
								const gid_t gid = ::atoi(dt->second[0].c_str());
								
								if(gid > 0){
									
									if(find(result.begin(), result.end(), gid) == result.end()){
										
										result.push_back(gid);
									}
								}
							}
						}
					}
				}
			}
		}
	}
	
	return result;
}
 
const vector <string> Gldap::getGroupNameByUser(const uid_t uid){
	
	vector <string> result;
	
	if(uid > 0){
		
		string userName;
		
		ALDAP ldap(this->config, this->log);
		
		const string keys = Anyks::strFormat("%s,%s", this->config->ldap.keys.users.login.c_str(), this->config->ldap.keys.groups.gid.c_str());
		
		const string filter = Anyks::strFormat("(&%s(%s=%u))", this->config->ldap.filter.users.c_str(), this->config->ldap.keys.users.uid.c_str(), uid);
		
		auto users = ldap.data(this->config->ldap.dn.users, keys, this->config->ldap.scope.users, filter);
		
		if(!users.empty()){
			
			for(auto it = users.cbegin(); it != users.cend(); ++it){
				
				for(auto dt = it->vals.cbegin(); dt != it->vals.cend(); ++dt){
					
					if(!dt->second.empty()){
						
						if(dt->first.compare(this->config->ldap.keys.users.login) == 0) userName = dt->second[0];
						
						else if(dt->first.compare(this->config->ldap.keys.groups.gid) == 0){
							
							const gid_t gid = ::atoi(dt->second[0].c_str());
							
							if(gid > 0){
								
								const string name = getNameById(gid);
								
								if(!name.empty()) result.push_back(name);
							}
						}
					}
				}
			}
			
			if(!userName.empty()){
				
				const string filter = Anyks::strFormat(
					"(&%s(|(%s=%s)(%s=%u)))",
					this->config->ldap.filter.groups.c_str(),
					this->config->ldap.keys.groups.member.c_str(),
					userName.c_str(),
					this->config->ldap.keys.groups.member.c_str(),
					uid
				);
				
				auto groups = ldap.data(this->config->ldap.dn.groups, this->config->ldap.keys.groups.login, this->config->ldap.scope.groups, filter);
				
				if(!groups.empty()){
					
					for(auto it = groups.cbegin(); it != groups.cend(); ++it){
						
						for(auto dt = it->vals.cbegin(); dt != it->vals.cend(); ++dt){
							
							if(!dt->second.empty() && (dt->first.compare(this->config->ldap.keys.groups.login) == 0)){
								
								if(find(result.begin(), result.end(), dt->second[0]) == result.end()){
									
									result.push_back(dt->second[0]);
								}
							}
						}
					}
				}
			}
		}
	}
	
	return result;
}
 
const bool Gldap::checkUser(const gid_t gid, const uid_t uid){
	
	bool result = false;
	
	if((gid > 0) && (uid > 0)){
		
		ALDAP ldap(this->config, this->log);
		
		string filter = Anyks::strFormat(
			"(&%s(%s=%u)(%s=%u))",
			this->config->ldap.filter.users.c_str(),
			this->config->ldap.keys.groups.gid.c_str(),
			gid,
			this->config->ldap.keys.users.uid.c_str(),
			uid
		);
		
		auto users = ldap.data(this->config->ldap.dn.users, this->config->ldap.keys.users.uid, this->config->ldap.scope.users, filter);
		
		if(!users.empty()){
			
			for(auto it = users.cbegin(); it != users.cend(); ++it){
				
				for(auto dt = it->vals.cbegin(); dt != it->vals.cend(); ++dt){
					
					if(!dt->second.empty() && (dt->first.compare(this->config->ldap.keys.users.uid) == 0)) return true;
				}
			}
		}
		
		filter = Anyks::strFormat("(&%s(%s=%u))", this->config->ldap.filter.groups.c_str(), this->config->ldap.keys.groups.gid.c_str(), gid);
		
		auto groups = ldap.data(this->config->ldap.dn.groups, this->config->ldap.keys.groups.member, this->config->ldap.scope.groups, filter);
		
		if(!groups.empty()){
			
			for(auto it = groups.cbegin(); it != groups.cend(); ++it){
				
				for(auto dt = it->vals.cbegin(); dt != it->vals.cend(); ++dt){
					
					if(!dt->second.empty()){
						
						if(dt->first.compare(this->config->ldap.keys.groups.member) == 0){
							
							for(auto ut = dt->second.cbegin(); ut != dt->second.cend(); ++ut){
								
								uid_t _uid = (Anyks::isNumber(* ut) ? ::atoi(ut->c_str()) : this->getUidByUserName(* ut, AUSERS_TYPE_LDAP));
								
								if((_uid > 0) && (uid == _uid)) return true;
							}
						}
					}
				}
			}
		}
	}
	
	return result;
}
 
const bool Gldap::checkUser(const string groupName, const uid_t uid){
	
	bool result = false;
	
	if((uid > 0) && !groupName.empty()){
		
		const gid_t gid = getIdByName(groupName);
		
		if(gid > 0) result = checkUser(gid, uid);
	}
	
	return result;
}
 
const bool Gldap::checkGroupById(const gid_t gid){
	
	return !(getNameById(gid)).empty();
}
 
const bool Gldap::checkGroupByName(const string groupName){
	
	return (getIdByName(groupName) > 0 ? true : false);
}
 
const gid_t Gldap::getIdByName(const string groupName){
	
	gid_t result = 0;
	
	if(!groupName.empty()){
		
		ALDAP ldap(this->config, this->log);
		
		const string dn = Anyks::strFormat(
			"%s=%s,%s",
			this->config->ldap.keys.groups.login.c_str(),
			groupName.c_str(),
			this->config->ldap.dn.groups.c_str()
		);
		
		auto groups = ldap.data(dn, this->config->ldap.keys.groups.gid, this->config->ldap.scope.groups, this->config->ldap.filter.groups);
		
		if(!groups.empty()){
			
			for(auto it = groups.cbegin(); it != groups.cend(); ++it){
				
				for(auto dt = it->vals.cbegin(); dt != it->vals.cend(); ++dt){
					
					if(!dt->second.empty() && (dt->first.compare(this->config->ldap.keys.groups.gid) == 0)){
						
						return ::atoi(dt->second[0].c_str());
					}
				}
			}
		}
	}
	
	return result;
}
 
const string Gldap::getNameById(const gid_t gid){
	
	string result;
	
	if(gid > 0){
		
		ALDAP ldap(this->config, this->log);
		
		const string filter = Anyks::strFormat("(&%s(%s=%u))", this->config->ldap.filter.groups.c_str(), this->config->ldap.keys.groups.gid.c_str(), gid);
		
		auto groups = ldap.data(this->config->ldap.dn.groups, this->config->ldap.keys.groups.login, this->config->ldap.scope.groups, filter);
		
		if(!groups.empty()){
			
			for(auto it = groups.cbegin(); it != groups.cend(); ++it){
				
				for(auto dt = it->vals.cbegin(); dt != it->vals.cend(); ++dt){
					
					if(!dt->second.empty() && (dt->first.compare(this->config->ldap.keys.groups.login) == 0)) return dt->second[0];
				}
			}
		}
	}
	
	return result;
}
 
const vector <string> Gldap::getNameUsers(const gid_t gid){
	
	vector <string> result;
	
	if(gid > 0){
		
		ALDAP ldap(this->config, this->log);
		
		string filter = Anyks::strFormat("(&%s(%s=%u))", this->config->ldap.filter.groups.c_str(), this->config->ldap.keys.groups.gid.c_str(), gid);
		
		auto groups = ldap.data(this->config->ldap.dn.groups, this->config->ldap.keys.groups.member, this->config->ldap.scope.groups, filter);
		
		if(!groups.empty()){
			
			for(auto it = groups.cbegin(); it != groups.cend(); ++it){
				
				for(auto dt = it->vals.cbegin(); dt != it->vals.cend(); ++dt){
					
					if(!dt->second.empty()){
						
						if(dt->first.compare(this->config->ldap.keys.groups.member) == 0){
							
							for(auto ut = dt->second.cbegin(); ut != dt->second.cend(); ++ut){
								
								const string name = (!Anyks::isNumber(* ut) ? * ut : this->getUserNameByUid(::atoi(ut->c_str()), AUSERS_TYPE_LDAP));
								
								if(!name.empty()) result.push_back(name);
							}
						}
					}
				}
			}
		}
		
		filter = Anyks::strFormat("(&%s(%s=%u))", this->config->ldap.filter.users.c_str(), this->config->ldap.keys.groups.gid.c_str(), gid);
		
		auto users = ldap.data(this->config->ldap.dn.users, this->config->ldap.keys.users.login, this->config->ldap.scope.users, filter);
		
		if(!users.empty()){
			
			vector <string> list;
			
			for(auto it = users.cbegin(); it != users.cend(); ++it){
				
				for(auto dt = it->vals.cbegin(); dt != it->vals.cend(); ++dt){
					
					if(!dt->second.empty()){
						
						if(dt->first.compare(this->config->ldap.keys.users.login) == 0){
							
							const string name = dt->second[0];
							
							if(!name.empty()) list.push_back(name);
						}
					}
				}
			}
			
			result = Anyks::concatVectors <string> (result, list);
		}
	}
	
	return result;
}
 
const vector <uid_t> Gldap::getIdUsers(const gid_t gid){
	
	vector <uid_t> result;
	
	if(gid > 0){
		
		ALDAP ldap(this->config, this->log);
		
		string filter = Anyks::strFormat("(&%s(%s=%u))", this->config->ldap.filter.groups.c_str(), this->config->ldap.keys.groups.gid.c_str(), gid);
		
		auto groups = ldap.data(this->config->ldap.dn.groups, this->config->ldap.keys.groups.member, this->config->ldap.scope.groups, filter);
		
		if(!groups.empty()){
			
			for(auto it = groups.cbegin(); it != groups.cend(); ++it){
				
				for(auto dt = it->vals.cbegin(); dt != it->vals.cend(); ++dt){
					
					if(!dt->second.empty()){
						
						if(dt->first.compare(this->config->ldap.keys.groups.member) == 0){
							
							for(auto ut = dt->second.cbegin(); ut != dt->second.cend(); ++ut){
								
								uid_t uid = (Anyks::isNumber(* ut) ? ::atoi(ut->c_str()) : this->getUidByUserName(* ut, AUSERS_TYPE_LDAP));
								
								if(uid > 0) result.push_back(uid);
							}
						}
					}
				}
			}
		}
		
		filter = Anyks::strFormat("(&%s(%s=%u))", this->config->ldap.filter.users.c_str(), this->config->ldap.keys.groups.gid.c_str(), gid);
		
		auto users = ldap.data(this->config->ldap.dn.users, this->config->ldap.keys.users.uid, this->config->ldap.scope.users, filter);
		
		if(!users.empty()){
			
			vector <uid_t> list;
			
			for(auto it = users.cbegin(); it != users.cend(); ++it){
				
				for(auto dt = it->vals.cbegin(); dt != it->vals.cend(); ++dt){
					
					if(!dt->second.empty()){
						
						if(dt->first.compare(this->config->ldap.keys.users.uid) == 0){
							
							uid_t uid = ::atoi(dt->second[0].c_str());
							
							if(uid > 0) list.push_back(uid);
						}
					}
				}
			}
			
			result = Anyks::concatVectors <uid_t> (result, list);
		}
	}
	
	return result;
}
 
void Gldap::setUsersMethods(function <string (uid_t, u_short)> method1, function <uid_t (string, u_short)> method2){
	
	this->getUserNameByUid = method1;
	
	this->getUidByUserName = method2;
}
 
Gldap::Gldap(Config * config, LogApp * log){
	
	if(config != nullptr){
		
		this->log = log;
		
		this->config = config;
	}
}
