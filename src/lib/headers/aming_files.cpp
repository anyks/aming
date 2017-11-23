/* SERVER PROXY AMING */
/*
*  author:     Yuriy Lobarev
*  phone:      +7(910)983-95-90
*  telegram:   @forman
*  email:      info@anyks.com
*  date:       11/23/2017 17:50:05
*  copyright:  © 2017 anyks.com
*/
 
 

#include "headers/files.h"

 
const string HFiles::getName(){
	
	string result;
	
	if(!this->names->empty()){
		
		for(auto it = this->names->cbegin(); it != this->names->cend(); ++it){
			
			if(isAvailable(* it)){
				
				result = * it;
				
				break;
			}
		}
	}
	
	return result;
}
 
const bool HFiles::isAvailable(const string name){
	
	bool result = false;
	
	if(!name.empty()){
		
		const string dir = this->config->proxy.dir;
		
		const string filename = Anyks::addToPath(dir, name + ".headers");
		
		result = (Anyks::isDirExist(dir.c_str()) && Anyks::isFileExist(filename.c_str()));
	}
	
	return result;
}
 
const vector <HFiles::Params> HFiles::getParams(){
	
	vector <Params> result;
	
	const string name = getName();
	
	if(!name.empty() && isAvailable(name)){
		
		const string dir = this->config->proxy.dir;
		
		const string filename = Anyks::addToPath(dir, name + ".headers");
		
		if(!filename.empty()
		
		&& Anyks::isDirExist(dir.c_str())
		
		&& Anyks::isFileExist(filename.c_str())){
			
			Anyks::setOwner(filename.c_str(), this->config->proxy.user, this->config->proxy.group);
			
			ifstream file(filename.c_str());
			
			if(file.is_open()){
				
				string filedata;
				
				while(file.good()){
					
					getline(file, filedata);
					
					filedata = Anyks::trim(filedata);
					
					const size_t pos = filedata.find("#");
					
					if(pos != string::npos) filedata = filedata.replace(pos, filedata.length() - pos, "");
					
					if(!filedata.empty()){
						
						smatch match;
						
						regex e(
							
							"(\\!?(?:ADD|RM|\\*)(?:\\s*\\|\\s*\\!?(?:ADD|RM|\\*))*)(?:\\s+|\\t+)"
							
							"(\\!?(?:IN|OUT|\\*)(?:\\s*\\|\\s*\\!?(?:IN|OUT|\\*))*)(?:\\s+|\\t+)"
							
							"((?:\\!?(?:[\\w\\.\\-\\:\\*]+|\\*))(?:\\s*\\|\\s*(?:\\!?(?:[\\w\\.\\-\\:\\*]+|\\*)))*)(?:\\s+|\\t+)"
							
							"((?:\\!?(?:[\\w\\.\\-\\:\\*]+|\\*))(?:\\s*\\|\\s*(?:\\!?(?:[\\w\\.\\-\\:\\*]+|\\*)))*)(?:\\s+|\\t+)"
							
							"(\\!?(?:" PROXY_HTTP_METHODS "|\\*)(?:\\s*\\|\\s*\\!?(?:" PROXY_HTTP_METHODS "|\\*))*)(?:\\s+|\\t+)"
							
							"(\\!?(?:\\/[^\\s\\r\\n\\t\\/]*(?:\\/[^\\s\\r\\n\\t\\/]*)*|\\*)(?:\\s*\\|\\s*(?:\\/[^\\s\\r\\n\\t\\/]*(?:\\/[^\\s\\r\\n\\t\\/]*)*|\\*))*)(?:\\s+|\\t+)"
							
							"([^\\s\\r\\n\\t]+|\\*)(?:\\s+|\\t+)"
							
							"([^\\s\\r\\n\\t]+|\\*)(?:\\s+|\\t+)"
							
							"(\\!?[\\w]{1,30}(?:\\s*\\|\\s*(?:\\!?[\\w]{1,30}|\\*))*|\\*)(?:\\s+|\\t+)"
							
							"(\\!?[\\w]{1,30}(?:\\s*\\|\\s*(?:\\!?[\\w]{1,30}|\\*))*|\\*)(?:\\s+|\\t+)"
							
							"([^\\r\\n\\t]+)$",
							regex::ECMAScript | regex::icase
						);
						
						regex_search(filedata, match, e);
						
						if(!match.empty()){
							
							result.push_back({
								
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
							});
						
						} else if(this->log != nullptr) this->log->write(LOG_ERROR, 0, "Invalid syntax for header file (%s) [%s]", filename.c_str(), filedata.c_str());
					}
				}
				
				file.close();
			
			} else if(this->log != nullptr) this->log->write(LOG_ERROR, 0, "headers file (%s) is cannot open", filename.c_str());
		
		} else if(!filename.empty() && (this->log != nullptr)){
			
			this->log->write(LOG_WARNING, 0, "headers file (%s) is not found", filename.c_str());
		}
	
	} else if(this->log != nullptr) this->log->write(LOG_ERROR, 0, "headers params is not found");
	
	return result;
}
 
HFiles::HFiles(Config * config, LogApp * log, deque <string> * names){
	
	if((config != nullptr) && (names != nullptr)){
		
		this->log = log;
		
		this->config = config;
		
		this->names = names;
	}
}
