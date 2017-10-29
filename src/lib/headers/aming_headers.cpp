/* SERVER PROXY AMING */
/*
*  author:     Yuriy Lobarev
*  phone:      +7(910)983-95-90
*  telegram:   @forman
*  email:      info@anyks.com
*  date:       10/29/2017 17:06:00
*  copyright:  © 2017 anyks.com
*/
 
 

#include "headers/headers.h"


using namespace std;

 
vector <Headers::Params> Headers::get(const string client, bool addGeneral){
	
	vector <Params> rules;
	
	if(!client.empty()){
		
		Network nwk;
		
		auto check = isNot(client);
		
		string userId = Anyks::toCase(check.str);
		
		u_short utype = checkTypeId(userId);
		
		
		if(utype == 1)		userId	= nwk.setLowIp(userId);		
		else if(utype == 2)	userId	= nwk.setLowIp6(userId);	
		
		if(!check.inv && (this->rules.count(userId) > 0)){
			
			auto user = this->rules[userId];
			
			rules.assign(user.begin(), user.end());
		
		} else if(check.inv) {
			
			for(auto it = this->rules.begin(); it != this->rules.end(); ++it){
				
				if(it->first.compare("*") != 0) copy(it->second.begin(), it->second.end(), back_inserter(rules));
			}
		}
		
		if(addGeneral && (this->rules.count("*") > 0)){
			
			auto general = this->rules["*"];
			
			copy(general.begin(), general.end(), back_inserter(rules));
		}
	}
	
	return rules;
}
 
void Headers::add(const string client, Headers::Params params){
	
	if(!client.empty()){
		
		Network nwk;
		
		string userId = Anyks::toCase(client);
		
		u_short utype = checkTypeId(userId);
		
		
		if(utype == 1)		userId	= nwk.setLowIp(userId);		
		else if(utype == 2)	userId	= nwk.setLowIp6(userId);	
		
		if(this->rules.count(userId) > 0){
			
			this->rules[userId].push_back(params);
		
		} else {
			
			vector <Params> rules = {params};
			
			this->rules.insert(pair <const string, vector <Params>> (userId, rules));
		}
	}
}
 
void Headers::rm(const string client){
	
	if(!client.empty()){
		
		Network nwk;
		
		string userId = Anyks::toCase(client);
		
		u_short utype = checkTypeId(userId);
		
		
		if(utype == 1)		userId	= nwk.setLowIp(userId);		
		else if(utype == 2)	userId	= nwk.setLowIp6(userId);	
		
		if(this->rules.count(userId) > 0) this->rules.erase(userId);
	}
}
 
void Headers::read(){
	
	if(this->config){
		
		const string name = getName();
		
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
							
							"(ADD|RM|\\*)(?:\\s+|\\t+)"
							
							"(IN|OUT|\\*)(?:\\s+|\\t+)"
							
							"([\\w\\.\\-\\@\\:\\!]+|\\*)(?:\\s+|\\t+)"
							
							"([A-Za-z\\s\\|\\!]+|\\*)(?:\\s+|\\t+)"
							
							"([^\\s\\r\\n\\t]+)(?:\\s+|\\t+)"
							
							"([\\w\\.\\-\\@\\:\\!]+|\\*)(?:\\s+|\\t+)"
							
							"([^\\s\\r\\n\\t]+|\\*)(?:\\s+|\\t+)"
							
							"([^\\r\\n\\t]+)",
							regex::ECMAScript | regex::icase
						);
						
						regex_search(filedata, match, e);
						
						if(!match.empty()){
							
							Network nwk;
							
							string userId = Anyks::toCase(match[3].str());
							
							string serverId = Anyks::toCase(match[6].str());
							
							u_short utype = checkTypeId(userId);
							u_short stype = checkTypeId(serverId);
							
							
							if(utype == 1)		userId		= nwk.setLowIp(userId);		
							else if(utype == 2)	userId		= nwk.setLowIp6(userId);	
							
							if(stype == 1)		serverId	= nwk.setLowIp(serverId);	
							else if(stype == 2)	serverId	= nwk.setLowIp6(serverId);	
							
							Params params = {
								
								utype,
								
								stype,
								
								Anyks::toCase(match[1].str()),
								
								Anyks::toCase(match[2].str()),
								
								Anyks::toCase(serverId),
								
								match[5].str(),
								
								match[7].str(),
								
								Anyks::split(Anyks::toCase(match[4].str()), "|"),
								
								Anyks::split(match[8].str(), "|")
							};
							
							add(userId, params);
						}
					}
				}
				
				file.close();
			}
		
		} else if(!filename.empty() && this->log){
			
			this->log->write(LOG_WARNING, 0, "headers file (%s) is not found", filename.c_str());
		}
	}
}
 
void Headers::modifyHeaders(const string server, vector <Headers::Params> rules, HttpData &http){
	
	if(!server.empty() && !rules.empty()){
		
		bool routeIn = false, routeAll = false, action = false;
		
		const string method = http.getMethod();
		
		const u_int status = http.getStatus();
		
		for(u_int i = 0; i < rules.size(); i++){
			
			bool result = false;
			
			if(rules[i].route.compare("*") == 0) routeAll = true;
			else routeAll = false;
			
			if(rules[i].route.compare("in") == 0)		routeIn = true;
			else if(rules[i].route.compare("out") == 0)	routeIn = false;
			
			if(routeAll || ((!status && !routeIn) || (status && routeIn))) result = true;
			
			if(result){
				
				for(u_int j = 0; j < rules[i].methods.size(); j++){
					
					auto check = isNot(rules[i].methods[j]);
					
					if(check.inv){
						
						if(check.str.compare(method) == 0){
							
							result = false;
							
							break;
						
						} else result = true;
					
					} else {
						
						if((check.str.compare("*") == 0)
						|| (check.str.compare(method) == 0)){
							
							result = true;
							
							break;
						
						} else result = false;
					}
				}
				
				if(result){
					
					if(rules[i].server.compare("*") != 0){
						
						auto check = isNot(rules[i].server);
						
						if(check.inv){
							
							switch(rules[i].stype){
								
								case 1:
								case 2: if(check.str.compare(server) == 0) result = false; break;
								
								case 4: if(Anyks::toCase(check.str).compare(Anyks::toCase(http.getHost())) == 0) result = false; break;
								
								default: result = false;
							}
						
						} else {
							
							switch(rules[i].stype){
								
								case 1:
								case 2: if(check.str.compare(server) != 0) result = false; break;
								
								case 4: if(Anyks::toCase(check.str).compare(Anyks::toCase(http.getHost())) != 0) result = false; break;
								
								default: result = false;
							}
						}
					}
					
					if(result){
						
						if(rules[i].path.compare("*") != 0){
							
							auto check = isNot(rules[i].path);
							
							if(check.inv){
								
								if(Anyks::toCase(check.str)
								.compare(Anyks::toCase(http.getPath())) == 0) result = false;
							
							} else {
								
								if(Anyks::toCase(check.str)
								.compare(Anyks::toCase(http.getPath())) != 0) result = false;
							}
						}
						
						if(result && (rules[i].regex.compare("*") != 0)){
							
							
							smatch match;
							
							regex e(rules[i].regex, regex::ECMAScript | regex::icase);
							
							const string userAgent = http.getUseragent();
							
							regex_search(userAgent, match, e);
							
							result = !match.empty();
						}
					}
				}
			}
			
			if(result){
				
				if(rules[i].action.compare("add") == 0)		action = true;
				else if(rules[i].action.compare("rm") == 0)	action = false;
				
				for(u_int j = 0; j < rules[i].headers.size(); j++){
					
					smatch match;
					
					const string header = rules[i].headers[j];
					
					if(action){
						
						regex e("^([\\w\\-]+)\\s*\\:\\s*([^\\r\\n\\t\\s]+)$", regex::ECMAScript | regex::icase);
						
						regex_search(header, match, e);
						
						if(!match.empty()) http.setHeader(match[1].str(), match[2].str());
					
					} else {
						
						regex e("^([\\w\\-]+)\\s*\\:?", regex::ECMAScript | regex::icase);
						
						regex_search(header, match, e);
						
						if(!match.empty()) http.rmHeader(match[1].str());
					}
				}
			}
		}
	}
}
 
void Headers::modifyHeaders(const string server, vector <Headers::Params> rules, string &data, HttpData &http){
	
	if(!server.empty() && !rules.empty() && !data.empty()){
		
		bool routeIn = false, routeAll = false, action = false;
		
		const string method = http.getMethod();
		
		const u_int status = http.getStatus();
		
		for(u_int i = 0; i < rules.size(); i++){
			
			bool result = false;
			
			if(rules[i].route.compare("*") == 0) routeAll = true;
			else routeAll = false;
			
			if(rules[i].route.compare("in") == 0)		routeIn = true;
			else if(rules[i].route.compare("out") == 0)	routeIn = false;
			
			if(routeAll || ((!status && !routeIn) || (status && routeIn))) result = true;
			
			if(result){
				
				for(u_int j = 0; j < rules[i].methods.size(); j++){
					
					auto check = isNot(rules[i].methods[j]);
					
					if(check.inv){
						
						if(check.str.compare(method) == 0){
							
							result = false;
							
							break;
						
						} else result = true;
					
					} else {
						
						if((check.str.compare("*") == 0)
						|| (check.str.compare(method) == 0)){
							
							result = true;
							
							break;
						
						} else result = false;
					}
				}
				
				if(result){
					
					if(rules[i].server.compare("*") != 0){
						
						auto check = isNot(rules[i].server);
						
						if(check.inv){
							
							switch(rules[i].stype){
								
								case 1:
								case 2: if(check.str.compare(server) == 0) result = false; break;
								
								case 4: if(Anyks::toCase(check.str).compare(Anyks::toCase(http.getHost())) == 0) result = false; break;
								
								default: result = false;
							}
						
						} else {
							
							switch(rules[i].stype){
								
								case 1:
								case 2: if(check.str.compare(server) != 0) result = false; break;
								
								case 4: if(Anyks::toCase(check.str).compare(Anyks::toCase(http.getHost())) != 0) result = false; break;
								
								default: result = false;
							}
						}
					}
					
					if(result){
						
						if(rules[i].path.compare("*") != 0){
							
							auto check = isNot(rules[i].path);
							
							if(check.inv){
								
								if(Anyks::toCase(check.str)
								.compare(Anyks::toCase(http.getPath())) == 0) result = false;
							
							} else {
								
								if(Anyks::toCase(check.str)
								.compare(Anyks::toCase(http.getPath())) != 0) result = false;
							}
						}
						
						if(result && (rules[i].regex.compare("*") != 0)){
							
							
							smatch match;
							
							regex e(rules[i].regex, regex::ECMAScript | regex::icase);
							
							const string userAgent = http.getUseragent();
							
							regex_search(userAgent, match, e);
							
							result = !match.empty();
						}
					}
				}
			}
			
			if(result){
				
				if(rules[i].action.compare("add") == 0)		action = true;
				else if(rules[i].action.compare("rm") == 0)	action = false;
				
				for(u_int j = 0; j < rules[i].headers.size(); j++){
					
					smatch match;
					
					const string header = rules[i].headers[j];
					
					if(action){
						
						regex e("^([\\w\\-]+)\\s*\\:\\s*([^\\r\\n\\t\\s]+)$", regex::ECMAScript | regex::icase);
						
						regex_search(header, match, e);
						
						if(!match.empty()) http.addHeaderToString(match[1].str(), match[2].str(), data);
					
					} else {
						
						regex e("^([\\w\\-]+)\\s*\\:?", regex::ECMAScript | regex::icase);
						
						regex_search(header, match, e);
						
						if(!match.empty()) http.rmHeaderInString(match[1].str(), data);
					}
				}
			}
		}
	}
}
 
const Headers::IsNot Headers::isNot(const string str){
	
	bool result = str[0] == '!';
	
	return {result, (result ? str.substr(1, str.length() - 1) : str)};
}
 
const u_short Headers::checkTypeId(const string str){
	
	Network nwk;
	
	u_short type = 0;
	
	if(isIpV4(nwk.setLowIp(str))) type = 1;
	
	else if(isIpV6(nwk.setLowIp6(str))) type = 2;
	
	else if(isMac(str)) type = 3;
	
	else if(isDomain(str)) type = 4;
	
	return type;
}
 
const bool Headers::isIpV4(const string ip){
	
	smatch match;
	
	regex e("\\d{1,3}(?:\\.\\d{1,3}){3}", regex::ECMAScript | regex::icase);
	
	regex_search(ip, match, e);
	
	return !match.empty();
}
 
const bool Headers::isIpV6(const string ip){
	
	smatch match;
	
	regex e("[A-Fa-f\\d]{4}(?:\\:[A-Fa-f\\d]{4}){7}", regex::ECMAScript | regex::icase);
	
	regex_search(ip, match, e);
	
	return !match.empty();
}
 
const bool Headers::isIp(const string ip){
	
	smatch match;
	
	regex e(
		"(?:\\d{1,3}(?:\\.\\d{1,3}){3}|"
		"[A-Fa-f\\d]{4}(?:\\:[A-Fa-f\\d]{4}){7})",
		regex::ECMAScript | regex::icase
	);
	
	regex_search(ip, match, e);
	
	return !match.empty();
}
 
const bool Headers::isMac(const string mac){
	
	smatch match;
	
	regex e("[A-Fa-f\\d]{2}(?:\\:[A-Fa-f\\d]{2}){5}", regex::ECMAScript | regex::icase);
	
	regex_search(mac, match, e);
	
	return !match.empty();
}
 
const bool Headers::isDomain(const string domain){
	
	smatch match;
	
	regex e("[\\w\\-\\.]+\\.[\\w\\-]+", regex::ECMAScript | regex::icase);
	
	regex_search(domain, match, e);
	
	return !match.empty();
}
 
const bool Headers::isLogin(const string login){
	
	return !Anyks::isAddress(login);
}
 
const bool Headers::checkAvailable(const string name){
	
	bool result = false;
	
	if(!name.empty()){
		
		const string dir = this->config->proxy.dir;
		
		const string filename = Anyks::addToPath(dir, name + ".headers");
		
		if(Anyks::isDirExist(dir.c_str()) && Anyks::isFileExist(filename.c_str())) result = true;
	}
	
	return result;
}
 
const string Headers::getName(){
	
	string result;
	
	for(auto it = this->names.cbegin(); it != this->names.cend(); ++it){
		
		if(checkAvailable(* it)){
			
			result = * it;
			
			break;
		}
	}
	
	return result;
}
 
void Headers::clear(){
	
	this->rules.clear();
	this->names.clear();
}
 
void Headers::addName(const string name){
	
	if(!name.empty()) this->names.push_front(name);
}
 
void Headers::setOptions(const u_short options){
	
	if(options != 0x00) this->options = options;
}
 
void Headers::modify(const string ip, const string mac, const string server, HttpData &http){
	
	if(!ip.empty() && !mac.empty() && !server.empty()){
		
		Network nwk;
		
		string serverId = Anyks::toCase(server);
		
		u_short stype = checkTypeId(serverId);
		
		
		if(stype == 1)		serverId	= nwk.setLowIp(serverId);	
		else if(stype == 2)	serverId	= nwk.setLowIp6(serverId);	
		
		auto rules1 = get(ip, false);
		
		auto rules2 = get(mac, false);
		
		auto rules3 = get("*", false);
		
		modifyHeaders(serverId, rules1, http);
		modifyHeaders(serverId, rules2, http);
		modifyHeaders(serverId, rules3, http);
	}
}
 
void Headers::modify(const string ip, const string mac, const string server, string &data){
	
	if(!ip.empty() && !mac.empty() && !server.empty() && !data.empty()){
		
		HttpData http;
		
		if(http.parse(data.c_str(), data.size(), this->config->proxy.name, this->options)){
			
			Network nwk;
			
			string serverId = Anyks::toCase(server);
			
			u_short stype = checkTypeId(serverId);
			
			
			if(stype == 1)		serverId	= nwk.setLowIp(serverId);	
			else if(stype == 2)	serverId	= nwk.setLowIp6(serverId);	
			
			auto rules1 = get(ip, false);
			
			auto rules2 = get(mac, false);
			
			auto rules3 = get("*", false);
			
			modifyHeaders(serverId, rules1, data, http);
			modifyHeaders(serverId, rules2, data, http);
			modifyHeaders(serverId, rules3, data, http);
			
			data = http.modifyHeaderString(data);
		}
	}
}
 
Headers::Headers(Config * config, LogApp * log){
	
	if(config){
		
		clear();
		
		this->log = log;
		
		this->config = config;
		
		this->options = this->config->options;
		
		this->names.push_front(this->config->proxy.name);
		
		read();
	}
}