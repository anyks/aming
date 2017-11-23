/* SERVER PROXY AMING */
/*
*  author:     Yuriy Lobarev
*  phone:      +7(910)983-95-90
*  telegram:   @forman
*  email:      info@anyks.com
*  date:       11/23/2017 17:50:05
*  copyright:  © 2017 anyks.com
*/
 
 

#include "acodes/acodes.h"

 
void ACodes::read(){
	 
	auto addToTemplate = [this](const u_short code, const u_short type, const string data){
		
		this->templates.insert(pair <u_short, Template> (code, {code, type, data}));
	};
	
	if(!this->config->proxy.atemp.empty()){
		
		for(auto it = this->config->proxy.atemp.cbegin(); it != this->config->proxy.atemp.cend(); ++it){
			
			if(!it->second.empty()){
				
				const u_short type = Anyks::getTypeAmingByString(it->second);
				
				switch(type){
					
					case AMING_ADDRESS: {
						
						if(Anyks::isFileExist(it->second.c_str())){
							
							Anyks::setOwner(it->second.c_str(), this->config->proxy.user, this->config->proxy.group);
							
							ifstream file(it->second.c_str());
							
							if(file.is_open()){
								
								string data;
								
								string filedata;
								
								while(file.good()){
									
									getline(file, filedata);
									
									data.append(filedata);
								}
								
								file.close();
								
								addToTemplate(it->first, type, data);
							
							} else {
								
								addToTemplate(it->first, AMING_NULL, this->deftemps.find(it->first)->second);
								
								if(this->log != nullptr) this->log->write(LOG_WARNING, 0, "template acode file (%s) is cannot open", it->second.c_str());
							}
						
						} else {
							
							addToTemplate(it->first, AMING_NULL, this->deftemps.find(it->first)->second);
							
							if(this->log != nullptr) this->log->write(LOG_WARNING, 0, "template acode file (%s) is not found", it->second.c_str());
						}
					} break;
					
					case AMING_HTTP_ADDRESS: addToTemplate(it->first, type, it->second); break;
					
					default: {
						
						addToTemplate(it->first, AMING_NULL, this->deftemps.find(it->first)->second);
						
						if(this->log != nullptr) this->log->write(LOG_ERROR, 0, "template acode file (%s) is broken", it->second.c_str());
					}
				}
			
			} else addToTemplate(it->first, AMING_NULL, this->deftemps.find(it->first)->second);
		}
	
	} else {
		
		for(auto it = this->deftemps.cbegin(); it != this->deftemps.cend(); ++it){
			
			addToTemplate(it->first, AMING_NULL, it->second);
		}
	}
}
 
void ACodes::clear(){
	
	this->templates.clear();
}
 
const ACodes::Template ACodes::get(u_short code){
	
	Template result;
	
	if(code > 0){
		
		if(this->templates.count(code) > 0) result = this->templates.find(code)->second;
	}
	
	return result;
}
 
ACodes::ACodes(Config * config, LogApp * log){
	
	if(config != nullptr){
		
		clear();
		
		this->log = log;
		
		this->config = config;
		
		read();
	}
}
