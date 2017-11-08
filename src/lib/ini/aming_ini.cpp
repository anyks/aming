/* SERVER PROXY AMING */
/*
*  author:     Yuriy Lobarev
*  phone:      +7(910)983-95-90
*  telegram:   @forman
*  email:      info@anyks.com
*  date:       11/08/2017 16:52:48
*  copyright:  © 2017 anyks.com
*/
 
 

#include "ini/ini.h"


using namespace std;

 
const INI::Params INI::getParams(const string str){
	
	Params params;
	
	if(!str.empty()){
		
		smatch match;
		
		regex e("^\\s*(\\w+)\\s*\\=\\s*([^\\r\\n\\;\\#]+)", regex::ECMAScript | regex::icase);
		
		regex_search(str, match, e);
		
		if(!match.empty()){
			
			params.key = match[1].str();
			
			params.value = match[2].str();
			
			params.value = Anyks::trim(params.value);
		}
	}
	
	return params;
}
 
const string INI::getSection(const string str){
	
	string result;
	
	if(!str.empty()){
		
		smatch match;
		
		regex e("^\\s*\\[(\\w+)\\]", regex::ECMAScript | regex::icase);
		
		regex_search(str, match, e);
		
		if(!match.empty()) result = match[1].str();
	}
	
	return result;
}
 
const string INI::get(const string section, const string key){
	
	string result;
	
	if(!section.empty() && !key.empty()){
		
		if(this->data.count(section)){
			
			auto params = this->data.find(section)->second;
			
			for(auto it = params.cbegin(); it != params.cend(); ++it){
				
				string key1 = key;
				
				string key2 = it->key;
				
				if(Anyks::toCase(key1).compare(Anyks::toCase(key2)) == 0) return it->value;
			}
		}
	}
	
	return result;
}
 
const vector <string> INI::getSections(){
	
	vector <string> result;
	
	if(!this->data.empty()){
		
		for(auto it = this->data.cbegin(); it != this->data.cend(); ++it){
			
			result.push_back(it->first);
		}
	}
	
	return result;
}
 
const vector <INI::Params> INI::getParamsInSection(const string section){
	
	vector <Params> result;
	
	if(!section.empty() && !this->data.empty()){
		
		for(auto it = this->data.cbegin(); it != this->data.cend(); ++it){
			
			string section1 = section;
			
			string section2 = it->first;
			
			if(Anyks::toCase(section1).compare(Anyks::toCase(section2)) == 0){
				
				auto params = it->second;
				
				for(auto it = params.cbegin(); it != params.cend(); ++it){
					
					result.push_back({it->key, it->value});
				}
				
				break;
			}
		}
	}
	
	return result;
}
 
const double INI::getFloat(const string section, const string key, const double defval){
	
	string data = get(section, key);
	
	if(!data.empty()){
		
		smatch match;
		
		regex e("^\\-?(?:\\d+(?:\\.|\\,)\\d+|\\d+)$", regex::ECMAScript | regex::icase);
		
		regex_search(data, match, e);
		
		if(!match.empty()) return ::atof(match[0].str().c_str());
		
		else {
			
			regex e("^(true|false)$", regex::ECMAScript | regex::icase);
			
			regex_search(data, match, e);
			
			if(!match.empty()){
				
				string value = match[1].str();
				
				if(Anyks::toCase(value).compare("true") == 0)
					return 1.0;
				else return 0.0;
			}
		}
	}
	
	return defval;
}
 
const int64_t INI::getNumber(const string section, const string key, const int64_t defval){
	
	string data = get(section, key);
	
	if(!data.empty()){
		
		smatch match;
		
		regex e("^\\-?\\d+$", regex::ECMAScript | regex::icase);
		
		regex_search(data, match, e);
		
		if(!match.empty()) return ::atoi(match[0].str().c_str());
		
		else {
			
			regex e("^(true|false)$", regex::ECMAScript | regex::icase);
			
			regex_search(data, match, e);
			
			if(!match.empty()){
				
				string value = match[1].str();
				
				if(Anyks::toCase(value).compare("true") == 0)
					return 1;
				else return 0;
			}
		}
	}
	
	return defval;
}
 
const size_t INI::getUNumber(const string section, const string key, const size_t defval){
	
	string data = get(section, key);
	
	if(!data.empty()){
		
		smatch match;
		
		regex e("^\\d+$", regex::ECMAScript | regex::icase);
		
		regex_search(data, match, e);
		
		if(!match.empty()) return ::atoi(match[0].str().c_str());
		
		else {
			
			regex e("^(true|false)$", regex::ECMAScript | regex::icase);
			
			regex_search(data, match, e);
			
			if(!match.empty()){
				
				string value = match[1].str();
				
				if(Anyks::toCase(value).compare("true") == 0)
					return 1;
				else return 0;
			}
		}
	}
	
	return defval;
}
 
const bool INI::getBoolean(const string section, const string key, const bool defval){
	
	string data = get(section, key);
	
	if(!data.empty()){
		
		smatch match;
		
		regex e("^\\-?\\d+$", regex::ECMAScript | regex::icase);
		
		regex_search(data, match, e);
		
		if(!match.empty()){
			
			int64_t value = ::atoi(match[0].str().c_str());
			
			if(value > 0) return true;
			else return false;
		
		} else {
			
			regex e("^(true|false)$", regex::ECMAScript | regex::icase);
			
			regex_search(data, match, e);
			
			if(!match.empty()){
				
				string value = match[1].str();
				
				if(Anyks::toCase(value).compare("true") == 0)
					return true;
				else return false;
			}
		}
	}
	
	return defval;
}
 
const string INI::getString(const string section, const string key, const string defval){
	
	string data = get(section, key);
	
	return (!data.empty() ? data : defval);
}
 
const bool INI::checkParam(const string section, const string key){
	
	bool result = false;
	
	if(!section.empty() && !key.empty() && this->data.count(section)){
		
		auto params = this->data.find(section)->second;
		
		for(auto it = params.cbegin(); it != params.cend(); ++it){
			
			string key1 = key;
			
			string key2 = it->key;
			
			if(Anyks::toCase(key1).compare(Anyks::toCase(key2)) == 0){
				
				result = true;
				
				break;
			}
		}
	}
	
	return result;
}
 
const bool INI::checkSection(const string name){
	
	bool result = false;
	
	if(!name.empty() && this->data.count(name)) result = true;
	
	return result;
}
 
const bool INI::addData(const string section, const string key, const string value){
	
	bool result = false;
	
	if(!section.empty() && !key.empty()){
		
		result = true;
		
		Params params = {key, value};
		
		if(this->data.count(section)){
			
			this->data.find(section)->second.push_back(params);
		
		} else {
			
			this->data.insert(pair <string, vector <Params>>(section, {params}));
		}
	}
	
	return result;
}
 
const bool INI::delData(const string section, const string key){
	
	bool result = false;
	
	if(!section.empty() && !key.empty()){
		
		if(this->data.count(section)){
			
			auto params = this->data.find(section)->second;
			
			for(auto it = params.cbegin(); it != params.cend(); ++it){
				
				string key1 = key;
				
				string key2 = it->key;
				
				if(Anyks::toCase(key1).compare(Anyks::toCase(key2)) == 0){
					
					result = true;
					
					params.erase(it);
					
					this->data.erase(section);
					
					this->data.insert(pair <string, vector <Params>>(section, params));
					
					break;
				}
			}
		}
	}
	
	return result;
}
 
const bool INI::addSection(const string name){
	
	bool result = false;
	
	if(!name.empty() && !this->data.count(name)){
		
		result = true;
		
		this->data.insert(pair <string, vector <Params>>(name, {{}}));
	}
	
	return result;
}
 
const bool INI::delSection(const string name){
	
	bool result = false;
	
	if(!name.empty() && this->data.count(name)){
		
		result = true;
		
		this->data.erase(name);
	}
	
	return result;
}
 
const bool INI::isError(){
	
	return this->error;
}
 
void INI::read(const string filename){
	
	if(!filename.empty() && Anyks::isFileExist(filename.c_str())){
		
		ifstream config(filename.c_str());
		
		if(config.is_open()){
			
			string filedata;
			
			while(config.good()){
				
				getline(config, filedata);
				
				const string section = getSection(filedata);
				
				const Params params = getParams(filedata);
				
				if(!section.empty()) this->section = section;
				
				else if(!params.key.empty()){
					
					if(!this->data.count(this->section))
						
						this->data.insert(pair <string, vector <Params>>(this->section, {params}));
					
					else this->data.find(this->section)->second.push_back(params);
				}
			}
			
			this->section.clear();
			
			config.close();
		}
	
	} else this->error = true;
}
 
void INI::write(const string filename){
	
	if(!filename.empty()){
		
		string result = "";
		
		for(auto it = this->data.cbegin(); it != this->data.cend(); ++it){
			
			auto params = it->second;
			
			result.append(string("\r\n[") + it->first + string("]\r\n"));
			
			for(auto it = params.cbegin(); it != params.cend(); ++it){
				
				result.append(it->key + string(" = ") + it->value + string("\r\n"));
			}
		}
		
		if(!result.empty()){
			
			ofstream config(filename.c_str(), ios::binary);
			
			if(config.is_open()){
				
				config.write((const char *) result.data(), result.size());
				
				config.close();
			}
		}
	}
}
 
INI::INI(const string filename){
	
	read(filename);
}
 
INI::~INI(){
	
	this->data.clear();
}
