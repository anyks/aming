/* SERVER PROXY AMING */
/*
*  author:     Yuriy Lobarev
*  phone:      +7(910)983-95-90
*  telegram:   @forman
*  email:      info@anyks.com
*  date:       11/23/2017 17:50:05
*  copyright:  © 2017 anyks.com
*/
 
 

#include "headers/headers.h"


using namespace std;

 
template <typename T>
const vector <T> Anyks::concatVectors(const vector <T> &v1, const vector <T> &v2){
	
	vector <T> result;
	
	if(!v1.empty() && !v2.empty()){
		
		result.assign(v1.cbegin(), v1.cend());
		
		for(auto it = v2.cbegin(); it != v2.cend(); ++it){
			
			if(find(v1.begin(), v1.end(), * it) == v1.end()){
				
				result.push_back(* it);
			}
		}
	
	} else if(!v1.empty()) result.assign(v1.cbegin(), v1.cend());
	else if(!v2.empty()) result.assign(v2.cbegin(), v2.cend());
	
	return result;
};
 
const Headers::IsNot Headers::isNot(const string str){
	
	const bool result = (str[0] == '!');
	
	return {result, (result ? str.substr(1, str.length() - 1) : str)};
}
 
void Headers::modifyHeaders(const bool action, const vector <string> headers, HttpData &http){
	
	if(!headers.empty()){
		
		smatch match;
		
		for(auto it = headers.cbegin(); it != headers.cend(); ++it){
			
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
 
void Headers::modifyHeaders(const bool action, const vector <string> headers, string &data, HttpData &http){
	
	if(!headers.empty() && !data.empty()){
		
		smatch match;
		
		for(auto it = headers.cbegin(); it != headers.cend(); ++it){
			
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
 
void Headers::addParams(const map <bool, map <bool, unordered_map <string, Headers::Rules>>> * params){
	
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
											
											realRules->groups = Anyks::concatVectors <gid_t> (realRules->groups, rules->groups);
											
											realRules->broups = Anyks::concatVectors <gid_t> (realRules->broups, rules->broups);
											
											realRules->users = Anyks::concatVectors <uid_t> (realRules->users, rules->users);
											
											realRules->bsers = Anyks::concatVectors <uid_t> (realRules->bsers, rules->bsers);
											
											realRules->paths = Anyks::concatVectors <string> (realRules->paths, rules->paths);
											
											realRules->headers = Anyks::concatVectors <string> (realRules->headers, rules->headers);
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
 
void Headers::createRulesList(void * ctx){
	
	Headers::Params * params = reinterpret_cast <Headers::Params *> (ctx);
	
	if(params != nullptr){
		 
		auto createNode = [this](vector <string> list){
			
			vector <Node> nodes;
			
			for(auto it = list.cbegin(); it != list.cend(); ++it){
				
				string str = * it;
				
				Node node;
				
				if(str.compare("*") != 0){
					
					auto check = isNot(str);
					
					node.type = Anyks::getTypeAmingByString(check.str);
					
					if((node.type == AMING_DOMAIN)
					|| (node.type == AMING_MAC)
					|| (node.type == AMING_IPV4)
					|| (node.type == AMING_IPV6)
					|| (node.type == AMING_NETWORK)){
						
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
		
		vector <uid_t> users, bsers;
		
		vector <gid_t> groups, broups;
		
		for(auto it = params->groups.cbegin(); it != params->groups.cend(); ++it){
			
			string group = * it;
			
			if(group.compare("*") != 0){
				
				gid_t gid = 0;
				
				auto check = isNot(group);
				
				if(Anyks::isNumber(check.str)) gid = ::atoi(check.str.c_str());
				
				else gid = this->ausers->getGidByName(check.str);
				
				if(this->ausers->checkGroupById(gid)){
					
					if(!check.inv) groups.push_back(gid);
					
					else broups.push_back(gid);
				}
			
			} else break;
		}
		
		for(auto it = params->users.cbegin(); it != params->users.cend(); ++it){
			
			string user = * it;
			
			if(user.compare("*") != 0){
				
				uid_t uid = 0;
				
				auto check = isNot(user);
				
				if(Anyks::isNumber(check.str)) uid = ::atoi(check.str.c_str());
				
				else uid = this->ausers->getUidByName(check.str);
				
				if(this->ausers->checkUserById(uid)){
					
					if(!check.inv) users.push_back(uid);
					
					else bsers.push_back(uid);
				}
			
			} else break;
		}
		
		const Rules rules = {
			
			params->query,
			
			params->userAgent,
			
			users, bsers,
			
			groups, broups,
			
			createNode(params->clients),
			
			createNode(params->servers),
			
			params->paths,
			
			params->headers
		};
		
		unordered_map <string, Rules> methods;
		
		vector <string> nameMethods = Anyks::split(PROXY_HTTP_METHODS, "|");
		
		for(auto it = nameMethods.cbegin(); it != nameMethods.cend(); ++it){
			
			methods.insert(pair <string, Rules> (Anyks::toCase(* it), {}));
		}
		
		map <bool, unordered_map <string, Rules>> traffics = {{true, {}}, {false, {}}};
		
		map <bool, map <bool, unordered_map <string, Rules>>> actions = {{true, {}}, {false, {}}};
		
		for(auto it = params->methods.cbegin(); it != params->methods.cend(); ++it){
			
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
		
		for(auto it = params->traffic.cbegin(); it != params->traffic.cend(); ++it){
			
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
		
		for(auto it = params->actions.cbegin(); it != params->actions.cend(); ++it){
			
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
	}
}
 
void Headers::read(const u_short type){
	
	time_t curUpdate = time(nullptr);
	
	if(((this->lastUpdate + this->config->proxy.conftime) < curUpdate) || (type != AMING_NULL)){
		
		this->lastUpdate = curUpdate;
		
		if(type == AMING_NULL) this->rules.clear();
		
		const u_short service = (type != AMING_NULL ? type : this->config->proxy.configs);
		
		switch(service){
			
			case AUSERS_TYPE_FILE: {
				
				HFiles files(this->config, this->log, &this->names);
				
				auto params = files.getParams();
				
				for(auto it = params.cbegin(); it != params.cend(); ++it){
					
					auto param = (* it);
					
					createRulesList(&param);
				}
			} break;
			
			case AUSERS_TYPE_LDAP: {
				
				HLdap ldap(this->config, this->log, &this->names);
				
				auto params = ldap.getParams();
				
				for(auto it = params.cbegin(); it != params.cend(); ++it){
					
					auto param = (* it);
					
					createRulesList(&param);
				}
			} break;
			
			case AUSERS_TYPE_FILE_LDAP: {
				
				read(AUSERS_TYPE_FILE);
				
				read(AUSERS_TYPE_LDAP);
			} break;
		}
	}
}
 
const vector <string> Headers::findHeaders(Headers::Client * request, const string method, const Rules * rules){
	
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
				
				string str = reg;
				
				const string broken = "\\_";
				
				const size_t pos = str.find(broken);
				
				if(pos != string::npos) str = str.replace(pos, broken.length(), "");
				
				regex e(str, regex::ECMAScript | regex::icase);
				
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
		
		const bool groupFound = (checkGroups(&request->groups, &rules->groups) && (!rules->broups.empty() ? !checkGroups(&request->groups, &rules->broups) : true));
		
		const bool userFound = (checkUsers(request->user, &rules->users) && (!rules->bsers.empty() ? !checkUsers(request->user, &rules->bsers) : true));
		
		const bool clientFound = (checkCIP || checkCMac);
		
		const bool serverFound = (checkSIP || checkSDM);
		
		if(clientFound && serverFound && queryFound && userAgentFound && pathFound && groupFound && userFound){
			
			for(auto it = rules->headers.cbegin(); it != rules->headers.cend(); ++it){
				
				if(find(result.begin(), result.end(), * it) == result.end()){
					
					result.push_back(* it);
				}
			}
		}
	}
	
	return result;
}
 
const vector <string> Headers::get(Headers::Client * request){
	
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
 
void Headers::clear(){
	
	this->rules.clear();
	this->names.clear();
}
 
void Headers::rm(const bool action, const bool traffic){
	
	if(this->rules.count(action) > 0){
		
		auto traffics = this->rules.find(action)->second;
		
		if(!traffics.empty() && (traffics.count(traffic) > 0)) traffics.find(traffic)->second.clear();
	}
}
 
void Headers::add(const bool action, const bool traffic, const string method, void * ctx){
	
	if(ctx != nullptr){
		
		bool exist = false;
		
		Rules * rules = reinterpret_cast <Rules *> (ctx);
		
		if(this->rules.count(action) > 0) exist = true;
		
		if(!exist){
			
			bool exist = true;
			
			unordered_map <string, Rules> methods;
			
			vector <string> nameMethods = Anyks::split(PROXY_HTTP_METHODS, "|");
			
			for(auto it = nameMethods.cbegin(); it != nameMethods.cend(); ++it){
				
				methods.insert(pair <string, Rules> (Anyks::toCase(* it), {}));
			}
			
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
 
void Headers::addName(const string name){
	
	if(!name.empty()) this->names.push_front(name);
}
 
void Headers::setOptions(const u_short options){
	
	if(options != AMING_NULL) this->options = options;
}
 
void Headers::modify(AParams::Client client, HttpData &http){
	
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
		
		Client requestAdd	= {actionAdd, traffic, 0, {}, client.ip, client.mac, client.sip, host, agent, path, query, method};
		Client requestRm	= {actionRm, traffic, 0, {}, client.ip, client.mac, client.sip, host, agent, path, query, method};
		 
		auto modifyHeadersForUser = [&requestAdd, &requestRm, &actionAdd, &actionRm, this](const AParams::AUser * client, HttpData &http){
			
			if(client != nullptr){
				
				requestAdd.user	= client->user.uid;
				requestRm.user	= client->user.uid;
				
				for(auto it = client->groups.cbegin(); it != client->groups.cend(); ++it){
					
					requestAdd.groups.push_back(it->gid);
					requestRm.groups.push_back(it->gid);
				}
				
				auto headersAdd = get(&requestAdd);
				auto headersRm = get(&requestRm);
				
				modifyHeaders(actionAdd, headersAdd, http);
				
				modifyHeaders(actionRm, headersRm, http);
			}
		};
		
		if(client.auser != nullptr){
			
			modifyHeadersForUser(client.auser, http);
		
		} else {
			
			auto user = this->ausers->searchUser(client.ip, client.mac);
			
			if(user.auth) modifyHeadersForUser(&user, http);
			
			else modifyHeadersForUser(nullptr, http);
		}
	}
}
 
void Headers::modify(AParams::Client client, string &data){
	
	if(!client.ip.empty() && !client.mac.empty() && !client.sip.empty()){
		
		HttpData http;
		
		const u_short options = ((client.auser != nullptr) && (this->ausers != nullptr) ? this->ausers->getOptionsByUid(client.auser->user.uid) : this->options);
		
		if(http.parse(data.c_str(), data.size(), options, this->config, this->log)){
			
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
			
			Client requestAdd	= {actionAdd, traffic, 0, {}, client.ip, client.mac, client.sip, host, agent, path, query, method};
			Client requestRm	= {actionRm, traffic, 0, {}, client.ip, client.mac, client.sip, host, agent, path, query, method};
			 
			auto modifyHeadersForUser = [&requestAdd, &requestRm, &actionAdd, &actionRm, &data, this](const AParams::AUser * client, HttpData &http){
				
				if(client != nullptr){
					
					requestAdd.user	= client->user.uid;
					requestRm.user	= client->user.uid;
					
					for(auto it = client->groups.cbegin(); it != client->groups.cend(); ++it){
						
						requestAdd.groups.push_back(it->gid);
						requestRm.groups.push_back(it->gid);
					}
					
					auto headersAdd = get(&requestAdd);
					auto headersRm = get(&requestRm);
					
					modifyHeaders(actionAdd, headersAdd, data, http);
					
					modifyHeaders(actionRm, headersRm, data, http);
					
					data = http.modifyHeaderString(data);
				}
			};
			
			if(client.auser != nullptr){
				
				modifyHeadersForUser(client.auser, http);
			
			} else {
				
				auto user = this->ausers->searchUser(client.ip, client.mac);
				
				if(user.auth) modifyHeadersForUser(&user, http);
				
				else modifyHeadersForUser(nullptr, http);
			}
		}
	}
}
 
Headers::Headers(Config * config, LogApp * log, AUsers * ausers){
	
	if((config != nullptr) && (ausers != nullptr)){
		
		clear();
		
		this->log = log;
		
		this->config = config;
		
		this->ausers = ausers;
		
		this->options = this->config->options;
		
		this->names.push_front(this->config->proxy.name);
		
		read();
	}
}
