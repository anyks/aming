/* SERVER PROXY AMING */
/*
*  author:     Yuriy Lobarev
*  phone:      +7(910)983-95-90
*  telegram:   @forman
*  email:      info@anyks.com
*  date:       10/27/2017 18:42:35
*  copyright:  © 2017 anyks.com
*/
 
 

#include "headers2/headers.h"


using namespace std;

 
struct {
	template <typename T>
	void operator()(T &list1, T list2) const {
		
		for(auto it = list2.cbegin(); it != list2.cend(); ++it){
			
			if(find(list1.begin(), list1.end(), * it) == list1.end()){
				
				list1.push_back(* it);
			}
		}
	};
} copyListParams;
 
const Headers2::IsNot Headers2::isNot(const string str){
	
	bool result = str[0] == '!';
	
	return {result, (result ? str.substr(1, str.length() - 1) : str)};
}
 
void Headers2::modifyHeaders(const bool action, const vector <string> headers, HttpData &http){
	
	if(!headers.empty()){
		
		for(auto it = headers.cbegin(); it != headers.cend(); ++it){
			
			smatch match;
			
			if(action){
				
				regex e("^([\\w\\-]+)\\s*\\:\\s*([^\\r\\n\\t\\s]+)$", regex::ECMAScript | regex::icase);
				
				regex_search(* it, match, e);
				
				if(!match.empty()) http.setHeader(match[1].str(), match[2].str());
			
			} else {
				
				regex e("^([\\w\\-]+)\\s*\\:?", regex::ECMAScript | regex::icase);
				
				regex_search(* it, match, e);
				
				if(!match.empty()) http.rmHeader(match[1].str());
			}
		}
	}
}
 
void Headers2::modifyHeaders(const bool action, const vector <string> headers, string &data, HttpData &http){
	
	if(!headers.empty() && !data.empty()){
		
		for(auto it = headers.cbegin(); it != headers.cend(); ++it){
			
			smatch match;
			
			if(action){
				
				regex e("^([\\w\\-]+)\\s*\\:\\s*([^\\r\\n\\t\\s]+)$", regex::ECMAScript | regex::icase);
				
				regex_search(* it, match, e);
				
				if(!match.empty()) http.addHeaderToString(match[1].str(), match[2].str(), data);
			
			} else {
				
				regex e("^([\\w\\-]+)\\s*\\:?", regex::ECMAScript | regex::icase);
				
				regex_search(* it, match, e);
				
				if(!match.empty()) http.rmHeaderInString(match[1].str(), data);
			}
		}
	}
}
 
void Headers2::addParams(const map <bool, map <bool, unordered_map <string, Headers2::Rules>>> * params){
	
	if((params != nullptr) && !params->empty()){
		 
		auto copyListNodes = [](vector <Node> &node1, vector <Node> node2){
			
			for(auto it1 = node2.cbegin(); it1 != node2.cend(); ++it1){
				
				bool result = false;
				
				for(auto it2 = node1.cbegin(); it2 != node1.cend(); ++it2){
					
					if((it1->type != it2->type)
					|| (it1->data.compare(it2->data) != 0)) result = true;
				}
				
				if(!result) node1.push_back(* it1);
			}
		};
		
		for(auto it = params->cbegin(); it != params->cend(); ++it){
			
			const bool action = it->first;
			
			const auto * traffics = &it->second;
			
			if(this->rules.count(action) > 0){
				
				auto * realTraffics = &this->rules.find(action)->second;
				
				if(!traffics->empty()){
					
					for(auto it = traffics->cbegin(); it != traffics->cend(); ++it){
						
						const bool traffic = it->first;
						
						const auto * methods = &it->second;
						
						if(realTraffics->count(traffic) > 0){
							
							auto * realMethods = &realTraffics->find(traffic)->second;
							
							if(!methods->empty()){
								
								for(auto it = methods->cbegin(); it != methods->cend(); ++it){
									
									const string method = it->first;
									
									const auto * rules = &it->second;
									
									if(realMethods->count(method) > 0){
										
										auto * realRules = &realMethods->find(method)->second;
										
										if(rules != nullptr){
											
											realRules->query = rules->query;
											
											realRules->userAgent = rules->userAgent;
											
											copyListNodes(realRules->clients, rules->clients);
											
											copyListNodes(realRules->servers, rules->servers);
											
											copyListParams(realRules->groups, rules->groups);
											
											copyListParams(realRules->users, rules->users);
											
											copyListParams(realRules->paths, rules->paths);
											
											copyListParams(realRules->headers, rules->headers);
										}
									
									} else realMethods->emplace(method, * rules);
								}
							}
						
						} else realTraffics->emplace(traffic, * methods);
					}
				}
			
			} else this->rules.emplace(action, * traffics);
		}
	}
}
 
void Headers2::createRulesList(const Headers2::Params params){
	
	auto dataGroups = this->ausers->getAllGroups();
	
	auto dataUsers = this->ausers->getAllUsers();
	
	if(!dataGroups.empty() && !dataUsers.empty()){
		 
		auto createNode = [](vector <string> list){
			
			vector <Node> nodes;
			
			for(auto it = list.cbegin(); it != list.cend(); ++it){
				
				string str = * it;
				
				Node node;
				
				if(str.compare("*") != 0){
					
					node.type = Anyks::getTypeAmingByString(str);
					
					if(node.type != AMING_NULL){
						
						node.data = str;
						
						nodes.push_back(node);
					}
				
				} else {
					
					node.type = AMING_NULL;
					
					node.data = str;
					
					nodes.push_back(node);
				}
			}
			
			return nodes;
		};
		
		vector <gid_t> groups;
		
		vector <uid_t> users;
		
		for(auto it = params.groups.cbegin(); it != params.groups.cend(); ++it){
			
			string group = * it;
			
			if(group.compare("*") != 0){
				
				auto check = isNot(group);
				
				if(!check.inv){
					
					gid_t gid = -1;
					
					if(Anyks::isNumber(check.str)) gid = ::atoi(check.str.c_str());
					
					else gid = this->ausers->getGidByName(check.str);
					
					if(this->ausers->checkGroupById(gid)) groups.push_back(gid);
				
				} else {
					
					const bool numberFlag = Anyks::isNumber(check.str);
					
					for(auto it = dataGroups.cbegin(); it != dataGroups.cend(); ++it){
						
						if((numberFlag && ((* it)->id != gid_t(::atoi(check.str.c_str()))))
						|| (!numberFlag && (this->ausers->getGroupNameByGid((* it)->id).compare(check.str) != 0))) groups.push_back((* it)->id);
					}
					
					break;
				}
			
			} else {
				
				for(auto it = dataGroups.cbegin(); it != dataGroups.cend(); ++it){
					
					groups.push_back((* it)->id);
				}
				
				break;
			}
		}
		
		for(auto it = params.users.cbegin(); it != params.users.cend(); ++it){
			
			string user = * it;
			
			if(user.compare("*") != 0){
				
				auto check = isNot(user);
				
				if(!check.inv){
					
					uid_t uid = -1;
					
					if(Anyks::isNumber(check.str)) uid = ::atoi(check.str.c_str());
					
					else uid = this->ausers->getUidByName(check.str);
					
					if(this->ausers->checkUserById(uid)) users.push_back(uid);
				
				} else {
					
					const bool numberFlag = Anyks::isNumber(check.str);
					
					for(auto it = dataUsers.cbegin(); it != dataUsers.cend(); ++it){
						
						if((numberFlag && ((* it)->id != uid_t(::atoi(check.str.c_str()))))
						|| (!numberFlag && (this->ausers->getUserNameByUid((* it)->id).compare(check.str) != 0))) users.push_back((* it)->id);
					}
					
					break;
				}
			
			} else {
				
				for(auto it = dataUsers.cbegin(); it != dataUsers.cend(); ++it){
					
					users.push_back((* it)->id);
				}
				
				break;
			}
		}
		
		const Rules rules = {
			
			params.query,
			
			params.userAgent,
			
			users, groups,
			
			createNode(params.clients),
			
			createNode(params.servers),
			
			params.paths,
			
			params.headers
		};
		
		unordered_map <string, Rules> methods = {
			{"get", {}},
			{"put", {}},
			{"head", {}},
			{"post", {}},
			{"patch", {}},
			{"trace", {}},
			{"delete", {}},
			{"connect", {}},
			{"options", {}}
		};
		
		map <bool, unordered_map <string, Rules>> traffics = {{true, {}}, {false, {}}};
		
		map <bool, map <bool, unordered_map <string, Rules>>> actions = {{true, {}}, {false, {}}};
		
		for(auto it = params.methods.cbegin(); it != params.methods.cend(); ++it){
			
			string method = * it;
			
			method = Anyks::toCase(method);
			
			if(method.compare("*") != 0){
				
				auto check = isNot(method);
				
				if(!check.inv) methods.at(check.str) = rules;
				
				else {
					
					for(auto mt = methods.cbegin(); mt != methods.cend(); ++mt){
						
						if(mt->first.compare(check.str) != 0) methods.at(mt->first) = rules;
					}
					
					break;
				}
			
			} else {
				
				for(auto mt = methods.cbegin(); mt != methods.cend(); ++mt){
					
					methods.at(mt->first) = rules;
				}
				
				break;
			}
		}
		
		for(auto it = params.traffic.cbegin(); it != params.traffic.cend(); ++it){
			
			string traffic = * it;
			
			traffic = Anyks::toCase(traffic);
			
			if(traffic.compare("*") != 0){
				
				auto check = isNot(traffic);
				
				if(!check.inv){
					
					if(check.str.compare("in") == 0) traffics.at(true) = methods;
					
					if(check.str.compare("out") == 0) traffics.at(false) = methods;
				
				} else {
					
					if(check.str.compare("in") == 0) traffics.at(false) = methods;
					
					if(check.str.compare("out") == 0) traffics.at(true) = methods;
				}
			
			} else {
				
				traffics.at(true)	= methods;
				traffics.at(false)	= methods;
				
				break;
			}
		}
		
		for(auto it = params.actions.cbegin(); it != params.actions.cend(); ++it){
			
			string action = * it;
			
			action = Anyks::toCase(action);
			
			if(action.compare("*") != 0){
				
				auto check = isNot(action);
				
				if(!check.inv){
					
					if(check.str.compare("add") == 0) actions.at(true) = traffics;
					
					if(check.str.compare("rm") == 0) actions.at(false) = traffics;
				
				} else {
					
					if(check.str.compare("add") == 0) actions.at(false) = traffics;
					
					if(check.str.compare("rm") == 0) actions.at(true) = traffics;
				}
			
			} else {
				
				actions.at(true)	= traffics;
				actions.at(false)	= traffics;
				
				break;
			}
		}
		
		addParams(&actions);
	
	} else if(this->log) this->log->write(LOG_ERROR, 0, "groups not found for headers rules");
}
 
void Headers2::readFromLDAP(){
	
	const string name = getName();
	
	if(!name.empty() && checkAvailable(name)){
		
		ALDAP ldap(this->config, this->log);
		
		const string dn = (string("ah=") + name + string(",") + this->ldap.dn);
		
		const string lParams =	"amingHeadersUser,amingHeadersGroup,amingHeadersAgent,"
								"amingHeadersAction,amingHeadersTraffic,amingHeadersMethod,"
								"amingHeadersPath,amingHeadersQuery,amingHeadersHeaders,"
								"amingHeadersClientDomain,amingHeadersClientMac,"
								"amingHeadersClientIp4,amingHeadersClientIp6,"
								"amingHeadersClientNetwork4,amingHeadersClientNetwork6,"
								"amingHeadersServerDomain,amingHeadersServerMac,"
								"amingHeadersServerIp4,amingHeadersServerIp6,"
								"amingHeadersServerNetwork4,amingHeadersServerNetwork6";
		
		auto data = ldap.data(dn, lParams, this->ldap.scope, this->ldap.filter);
		
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
				
				createRulesList(params);
			}
		}
	
	} else if(this->log) this->log->write(LOG_ERROR, 0, "headers params is not found");
}
 
void Headers2::readFromFile(){
	
	const string name = getName();
	
	if(!name.empty() && checkAvailable(name)){
		
		const string dir = this->config->proxy.dir;
		
		const string filename = Anyks::addToPath(dir, name + ".headers");
		
		if(!filename.empty()
		
		&& Anyks::isDirExist(dir.c_str())
		
		&& Anyks::isFileExist(filename.c_str())){
			
			Anyks::setOwner(filename.c_str(), this->config->proxy.user, this->config->proxy.group);
			
			string filedata;
			
			ifstream file(filename.c_str());
			
			if(file.is_open()){
				
				while(file.good()){
					
					getline(file, filedata);
					
					size_t pos = filedata.find("#");
					
					if(pos != string::npos) filedata = filedata.replace(pos, filedata.length() - pos, "");
					
					if(!filedata.empty()){
						
						smatch match;
						
						regex e(
							
							"((?:ADD|RM|\\*)(?:\\s*\\|\\s*(?:ADD|RM|\\*))*)(?:\\s+|\\t+)"
							
							"((?:IN|OUT|\\*)(?:\\s*\\|\\s*(?:IN|OUT|\\*))*)(?:\\s+|\\t+)"
							
							"((?:\\!?[\\w\\.\\-\\@\\:\\!\\/]+|\\*)(?:\\s*\\|\\s*(?:\\!?[\\w\\.\\-\\@\\:\\!\\/]+|\\*))*)(?:\\s+|\\t+)"
							
							"((?:\\!?[\\w\\.\\-\\@\\:\\!\\/]+|\\*)(?:\\s*\\|\\s*(?:\\!?[\\w\\.\\-\\@\\:\\!\\/]+|\\*))*)(?:\\s+|\\t+)"
							
							"(\\!?(?:" PROXY_HTTP_METHODS "|\\*)(?:\\s*\\|\\s*\\!?(?:" PROXY_HTTP_METHODS "|\\*))*)(?:\\s+|\\t+)"
							
							"((?:\\!?\\/[\\w\\-\\_]*(?:\\/[\\w\\-\\_]*)*|\\*)(?:\\s*\\|\\s*(?:\\!?\\/[\\w\\-\\_]*(?:\\/[\\w\\-\\_]*)*|\\*))*)(?:\\s+|\\t+)"
							
							"([^\\s\\r\\n\\t]+|\\*)(?:\\s+|\\t+)"
							
							"([^\\s\\r\\n\\t]+|\\*)(?:\\s+|\\t+)"
							
							"(\\!?[\\w]{1,30}(?:\\s*\\|\\s*(?:\\!?[\\w]{1,30}|\\*))*)(?:\\s+|\\t+)"
							
							"(\\!?[\\w]{1,30}(?:\\s*\\|\\s*(?:\\!?[\\w]{1,30}|\\*))*)(?:\\s+|\\t+)"
							
							"([^\\r\\n\\t]+)$",
							regex::ECMAScript | regex::icase
						);
						
						regex_search(filedata, match, e);
						
						if(!match.empty()){
							
							const Params params = {
								
								match[7].str(),
								
								match[8].str(),
								
								Anyks::split(match[1].str(), "|"),
								
								Anyks::split(match[2].str(), "|"),
								
								Anyks::split(match[3].str(), "|"),
								
								Anyks::split(match[4].str(), "|"),
								
								Anyks::split(match[5].str(), "|"),
								
								Anyks::split(match[6].str(), "|"),
								
								Anyks::split(match[9].str(), "|"),
								
								Anyks::split(match[10].str(), "|"),
								
								Anyks::split(match[11].str(), "|")
							};
							
							createRulesList(params);
						
						} else if(this->log) this->log->write(LOG_ERROR, 0, "Invalid syntax for header file (%s)", filename.c_str());
					}
				}
				
				file.close();
			
			} else if(this->log) this->log->write(LOG_ERROR, 0, "headers file (%s) is cannot open", filename.c_str());
		
		} else if(!filename.empty() && this->log){
			
			this->log->write(LOG_WARNING, 0, "headers file (%s) is not found", filename.c_str());
		}
	
	} else if(this->log) this->log->write(LOG_ERROR, 0, "headers params is not found");
}
 
const bool Headers2::checkAvailable(const string name){
	
	bool result = false;
	
	if(!name.empty()){
		
		const string dir = this->config->proxy.dir;
		
		const string filename = Anyks::addToPath(dir, name + ".headers");
		
		if(Anyks::isDirExist(dir.c_str()) && Anyks::isFileExist(filename.c_str())) result = true;
	}
	
	return result;
}
 
const string Headers2::getName(){
	
	string result;
	
	for(auto it = this->names.cbegin(); it != this->names.cend(); ++it){
		
		if(checkAvailable(* it)){
			
			result = * it;
			
			break;
		}
	}
	
	return result;
}
 
const vector <string> Headers2::findHeaders(Headers2::Client * request, const string method, const Rules * rules){
	
	vector <string> result;
	
	if((request != nullptr) && (rules != nullptr)){
		 
		auto checkData = [this](const string data, const vector <Node> * nodes){
			
			bool result = false;
			
			if(!data.empty() && !nodes->empty()){
				
				const string tmpData = Anyks::toCase(data);
				
				const u_short type = Anyks::getTypeAmingByString(tmpData);
				
				for(auto it = nodes->cbegin(); it != nodes->cend(); ++it){
					
					if(it->data.compare("*") != 0){
						
						if(type == it->type){
							
							auto check = isNot(Anyks::toCase(it->data));
							
							switch(type){
								
								case AMING_IPV4: {
									
									if(!check.inv){
										
										result = (check.str.compare(tmpData) == 0);
										
										if(result) goto stop;
									
									} else {
										
										result = (check.str.compare(tmpData) != 0);
										
										if(!result) goto stop;
									}
								} break;
								
								case AMING_IPV6: {
									
									Network nwk;
									
									if(!check.inv){
										
										result = nwk.compareIP6(check.str, tmpData);
										
										if(result) goto stop;
									
									} else {
										
										result = !nwk.compareIP6(check.str, tmpData);
										
										if(!result) goto stop;
									}
								} break;
								
								case AMING_MAC: {
									
									if(!check.inv){
										
										result = (check.str.compare(tmpData) == 0);
										
										if(result) goto stop;
									
									} else {
										
										result = (check.str.compare(tmpData) != 0);
										
										if(!result) goto stop;
									}
								} break;
								
								case AMING_DOMAIN: {
									
									if(!check.inv){
										
										result = Anyks::checkDomainByMask(tmpData, check.str);
										
										if(result) goto stop;
									
									} else {
										
										result = !Anyks::checkDomainByMask(tmpData, check.str);
										
										if(!result) goto stop;
									}
								} break;
							}
						
						} else if((it->type == AMING_NETWORK) && ((type == AMING_IPV4) || (type == AMING_IPV6))){
							
							Network nwk;
							
							auto check = isNot(Anyks::toCase(it->data));
							
							const string ip = nwk.getIPByNetwork(check.str);
							
							if(type == Anyks::getTypeAmingByString(ip)){
								
								if(!check.inv){
									
									result = (type == AMING_IPV4 ? nwk.checkIPByNetwork(tmpData, check.str) : nwk.checkIPByNetwork6(tmpData, check.str));
									
									if(result) goto stop;
								
								} else {
									
									result = (type == AMING_IPV4 ? !nwk.checkIPByNetwork(tmpData, check.str) : !nwk.checkIPByNetwork6(tmpData, check.str));
									
									if(!result) goto stop;
								}
							}
						}
					
					} else {
						
						result = true;
						
						break;
					}
				}
			}
			
			stop:
			
			return result;
		};
		 
		auto checkPath = [this](const string data, const vector <string> * list){
			
			bool result = false;
			
			if(!data.empty() && !list->empty()){
				
				const string tmp = Anyks::toCase(data);
				
				for(auto it = list->cbegin(); it != list->cend(); ++it){
					
					if(it->compare("*") != 0){
						
						auto check = isNot(Anyks::toCase(* it));
						
						const string path = Anyks::getPathByString(check.str);
						
						if(!check.inv){
							
							result = (path.compare(tmp) == 0);
							
							if(result) break;
						
						} else {
							
							result = (path.compare(tmp) != 0);
							
							if(!result) break;
						}
					
					} else {
						
						result = true;
						
						break;
					}
				}
			}
			
			return result;
		};
		 
		auto checkRegexp = [](const string data, const string reg){
			
			bool result = false;
			
			if(!reg.empty() && (reg.compare("*") != 0)){
				
				regex e(reg, regex::ECMAScript | regex::icase);
				
				result = regex_match(data, e);
			
			} else result = true;
			
			return result;
		};
		 
		auto checkGroups = [](const vector <gid_t> * list1, const vector <gid_t> * list2){
			
			bool result = false;
			
			if(list2->empty()) result = true;
			
			else {
				
				for(auto it = list1->cbegin(); it != list1->cend(); ++it){
					
					if(find(list2->begin(), list2->end(), * it) != list2->end()){
						
						result = true;
						
						break;
					}
				}
			}
			
			return result;
		};
		 
		auto checkUsers = [](const uid_t uid, const vector <uid_t> * list){
			
			bool result = false;
			
			if(list->empty()) result = true;
			
			else result = (find(list->begin(), list->end(), uid) != list->end());
			
			return result;
		};
		
		const bool checkCIP = checkData(request->ip, &rules->clients);
		
		const bool checkCMac = (!checkCIP ? checkData(request->mac, &rules->clients) : checkCIP);
		
		const bool checkSIP = checkData(request->sip, &rules->servers);
		
		const bool checkSDM = (!checkSIP ? checkData(request->domain, &rules->servers) : checkSIP);
		
		const bool pathFound = checkPath(request->path, &rules->paths);
		
		const bool queryFound = checkRegexp(request->query, rules->query);
		
		const bool userAgentFound = checkRegexp(request->agent, rules->userAgent);
		
		const bool groupFound = checkGroups(&request->groups, &rules->groups);
		
		const bool userFound = checkUsers(request->user, &rules->users);
		
		const bool clientFound = (checkCIP || checkCMac);
		
		const bool serverFound = (checkSIP || checkSDM);
		
		if(clientFound && serverFound && queryFound && userAgentFound && pathFound && groupFound && userFound){
			
			for(auto it = rules->headers.cbegin(); it != rules->headers.cend(); ++it){
				
				const string str = * find(result.begin(), result.end(), * it);
				
				if(str.empty()) result.push_back(* it);
			}
		}
	}
	
	return result;
}
 
const vector <string> Headers2::get(Headers2::Client * request){
	
	vector <string> result;
	
	if((request != nullptr) && !request->ip.empty() && !request->mac.empty() && !request->sip.empty() && !this->rules.empty()){
		
		if(this->rules.count(request->action) > 0){
			
			auto methods = this->rules.find(request->action)->second.find(request->traffic)->second;
			
			const string tmpMethod = Anyks::toCase(request->method);
			
			if(tmpMethod.compare("*") == 0){
				
				for(auto it = methods.cbegin(); it != methods.cend(); ++it){
					
					result = findHeaders(request, it->first, &it->second);
				}
			
			} else if(methods.count(tmpMethod) > 0){
				
				auto rules = methods.find(tmpMethod)->second;
				
				result = findHeaders(request, tmpMethod, &rules);
			}
		}
	}
	
	return result;
}
 
void Headers2::add(const bool action, const bool traffic, const string method, void * ctx){
	
	if(ctx != nullptr){
		
		Rules * rules = reinterpret_cast <Rules *> (ctx);
		
		bool exist = false;
		
		if(this->rules.count(action) > 0) exist = true;
		
		if(!exist){
			
			bool exist = true;
			
			unordered_map <string, Rules> methods = {
				{"get", {}},
				{"put", {}},
				{"head", {}},
				{"post", {}},
				{"patch", {}},
				{"trace", {}},
				{"delete", {}},
				{"connect", {}},
				{"options", {}}
			};
			
			map <bool, unordered_map <string, Rules>> traffics = {{true, {}}, {false, {}}};
			
			const string tmpMethod = Anyks::toCase(method);
			
			if(tmpMethod.compare("*") == 0){
				
				for(auto it = methods.cbegin(); it != methods.cend(); ++it){
					
					methods.at(it->first) = * rules;
				}
			
			} else if(methods.count(tmpMethod) > 0) methods.at(tmpMethod) = * rules;
			
			else exist = false;
			
			if(exist){
				
				traffics.at(traffic) = methods;
				
				this->rules.emplace(action, traffics);
			}
		
		} else {
			
			const string tmpMethod = Anyks::toCase(method);
			
			auto * methods = &this->rules.find(action)->second.find(traffic)->second;
			
			if(tmpMethod.compare("*") == 0){
				
				for(auto it = methods->cbegin(); it != methods->cend(); ++it){
					
					methods->at(it->first) = * rules;
				}
			
			} else if(methods->count(tmpMethod) > 0) methods->at(tmpMethod) = * rules;
		}
	}
}
 
void Headers2::rm(const gid_t gid, const uid_t uid){
	
	if(gid && uid && (this->rules.count(gid) > 0)){
		
		auto group = this->rules.find(gid)->second;
		
		if(group.count(uid) > 0) group.find(uid)->second.clear();
	}
}
 
void Headers2::read(){
	
	time_t curUpdate = time(nullptr);
	
	if((this->lastUpdate + this->maxUpdate) < curUpdate){
		
		this->lastUpdate = curUpdate;
		
		this->rules.clear();
		
		switch(this->typeSearch){
			
			case 0: readFromFile(); break;
			
			case 1: readFromLDAP(); break;
			
			case 2: {
				readFromFile();
				readFromLDAP();
			} break;
		}
	}
}
 
void Headers2::clear(){
	
	this->rules.clear();
	this->names.clear();
}
 
void Headers2::addName(const string name){
	
	if(!name.empty()) this->names.push_front(name);
}
 
void Headers2::setOptions(const u_short options){
	
	if(options != 0x00) this->options = options;
}
 
void Headers2::modify(AParams::Client client, HttpData &http){
	
	if(!client.ip.empty() && !client.mac.empty() && !client.sip.empty()){
		
		const string method = http.getMethod();
		
		const string agent = http.getUseragent();
		
		const string host = http.getHost();
		
		string path = Anyks::toCase(http.getPath());
		
		string query = path;
		
		path = Anyks::getPathByString(path);
		
		query = Anyks::getQueryByString(query);
		
		const bool traffic = (http.getStatus() != 0);
		
		const bool actionAdd = true;
		
		const bool actionRm = false;
		
		Client requestAdd	= {actionAdd, traffic, static_cast <uid_t> (-1), {}, client.ip, client.mac, client.sip, host, agent, path, query, method};
		Client requestRm	= {actionRm, traffic, static_cast <uid_t> (-1), {}, client.ip, client.mac, client.sip, host, agent, path, query, method};
		 
		auto modifyHeadersForUser = [&requestAdd, &requestRm, &actionAdd, &actionRm, this](const AParams::AUser * client, HttpData &http){
			
			if(client != nullptr){
				
				requestAdd.user	= client->user.uid;
				requestRm.user	= client->user.uid;
				
				for(auto it = client->groups.cbegin(); it != client->groups.cend(); ++it){
					
					requestAdd.groups.push_back(it->gid);
					requestRm.groups.push_back(it->gid);
				}
			}
			
			auto headersAdd = get(&requestAdd);
			auto headersRm = get(&requestRm);
			
			modifyHeaders(actionAdd, headersAdd, http);
			
			modifyHeaders(actionRm, headersRm, http);
		};
		
		if(client.user != nullptr){
			
			modifyHeadersForUser(client.user, http);
		
		} else {
			
			auto user = this->ausers->searchUser(client.ip, client.mac);
			
			if(user.auth) modifyHeadersForUser(&user, http);
			
			else modifyHeadersForUser(nullptr, http);
		}
	}
}
 
void Headers2::modify(AParams::Client client, string &data){
	
	if(!client.ip.empty() && !client.mac.empty() && !client.sip.empty()){
		
		HttpData http;
		
		const u_short options = ((client.user != nullptr) && (this->ausers != nullptr) ? this->ausers->getOptionsByUid(client.user->user.uid) : this->options);
		
		if(http.parse(data.c_str(), data.size(), this->config->proxy.name, options)){
			
			const string method = http.getMethod();
			
			const string agent = http.getUseragent();
			
			const string host = http.getHost();
			
			string path = Anyks::toCase(http.getPath());
			
			string query = path;
			
			path = Anyks::getPathByString(path);
			
			query = Anyks::getQueryByString(query);
			
			const bool traffic = (http.getStatus() != 0);
			
			const bool actionAdd = true;
			
			const bool actionRm = false;
			
			Client requestAdd	= {actionAdd, traffic, static_cast <uid_t> (-1), {}, client.ip, client.mac, client.sip, host, agent, path, query, method};
			Client requestRm	= {actionRm, traffic, static_cast <uid_t> (-1), {}, client.ip, client.mac, client.sip, host, agent, path, query, method};
			 
			auto modifyHeadersForUser = [&requestAdd, &requestRm, &actionAdd, &actionRm, &data, this](const AParams::AUser * client, HttpData &http){
				
				if(client != nullptr){
					
					requestAdd.user	= client->user.uid;
					requestRm.user	= client->user.uid;
					
					for(auto it = client->groups.cbegin(); it != client->groups.cend(); ++it){
						
						requestAdd.groups.push_back(it->gid);
						requestRm.groups.push_back(it->gid);
					}
				}
				
				auto headersAdd = get(&requestAdd);
				auto headersRm = get(&requestRm);
				
				modifyHeaders(actionAdd, headersAdd, data, http);
				
				modifyHeaders(actionRm, headersRm, data, http);
				
				data = http.modifyHeaderString(data);
			};
			
			if(client.user != nullptr){
				
				modifyHeadersForUser(client.user, http);
			
			} else {
				
				auto user = this->ausers->searchUser(client.ip, client.mac);
				
				if(user.auth) modifyHeadersForUser(&user, http);
				
				else modifyHeadersForUser(nullptr, http);
			}
		}
	}
}
 
Headers2::Headers2(Config * config, LogApp * log, AUsers * ausers){
	
	if(config && ausers){
		
		clear();
		
		this->log = log;
		
		this->config = config;
		
		this->options = this->config->options;
		
		this->ausers = ausers;
		
		this->typeSearch = 0;
		
		this->maxUpdate = 600;
		
		this->ldap = {
			"ou=headers,ou=aming,dc=agro24,dc=dev",
			"one",
			"(objectClass=amingHeaders)"
		};
		
		if(this->typeSearch == 0) this->names.push_front(this->config->proxy.name);
		
		read();
	}
}
