/* SERVER PROXY AMING */
/*
*  author:     Yuriy Lobarev
*  phone:      +7(910)983-95-90
*  telegram:   @forman
*  email:      info@anyks.com
*  date:       10/27/2017 18:42:35
*  copyright:  © 2017 anyks.com
*/
 
 

#include "ccache.h"


using namespace std;

 
const size_t CCache::DataDNS::size(){
	
	if(this->raw.empty()) data();
	
	return this->raw.size();
}
 
const u_char * CCache::DataDNS::data(){
	
	if(this->ttl || !this->ipv4.empty() || !this->ipv6.empty()){
		
		Map sizes = {sizeof(this->ttl), this->ipv4.size(), this->ipv6.size()};
		
		const size_t size = sizeof(sizes);
		
		const u_char * map = reinterpret_cast <const u_char *> (&sizes);
		
		copy(map, map + size, back_inserter(this->raw));
		
		const u_char * ttl = reinterpret_cast <const u_char *> (&this->ttl);
		
		copy(ttl, ttl + sizes.ttl, back_inserter(this->raw));
		
		copy(this->ipv4.begin(), this->ipv4.end(), back_inserter(this->raw));
		
		copy(this->ipv6.begin(), this->ipv6.end(), back_inserter(this->raw));
	}
	
	return this->raw.data();
}
 
void CCache::DataDNS::set(const u_char * data, size_t size){
	
	if(size){
		
		size_t size_map = sizeof(Map);
		
		if(size_map < size){
			
			size_t size_it = size_map;
			
			for(size_t i = 0, j = 0; i < size_map; i += sizeof(size_t), j++){
				
				size_t size_data;
				
				memcpy(&size_data, data + i, sizeof(size_t));
				
				if(size_data && ((size_data + size_it) <= size)){
					
					switch(j){
						
						case 0: Anyks::(data, size_data, size_it, &this->ttl); break;
						
						case 1: Anyks::cpydata(data, size_data, size_it, this->ipv4); break;
						
						case 2: Anyks::cpydata(data, size_data, size_it, this->ipv6); break;
					}
				}
			}
		}
	}
}
 
CCache::DataDNS::~DataDNS(){
	
	this->raw.clear();
	
	vector <u_char> ().swap(this->raw);
}
 
const size_t CCache::DataCache::size(){
	
	if(this->raw.empty()) data();
	
	return this->raw.size();
}
 
const u_char * CCache::DataCache::data(){
	
	if(!this->http.empty()){
		
		Map sizes = {
			sizeof(this->ipv),
			sizeof(this->age),
			sizeof(this->date),
			sizeof(this->expires),
			sizeof(this->modified),
			sizeof(this->valid),
			this->etag.size(),
			this->http.size()
		};
		
		const size_t size = sizeof(sizes);
		
		const u_char * map = reinterpret_cast <const u_char *> (&sizes);
		
		copy(map, map + size, back_inserter(this->raw));
		
		const u_char * ipv = reinterpret_cast <const u_char *> (&this->ipv);
		
		copy(ipv, ipv + sizes.ipv, back_inserter(this->raw));
		
		const u_char * age = reinterpret_cast <const u_char *> (&this->age);
		
		copy(age, age + sizes.age, back_inserter(this->raw));
		
		const u_char * date = reinterpret_cast <const u_char *> (&this->date);
		
		copy(date, date + sizes.date, back_inserter(this->raw));
		
		const u_char * expires = reinterpret_cast <const u_char *> (&this->expires);
		
		copy(expires, expires + sizes.expires, back_inserter(this->raw));
		
		const u_char * modified = reinterpret_cast <const u_char *> (&this->modified);
		
		copy(modified, modified + sizes.modified, back_inserter(this->raw));
		
		const u_char * valid = reinterpret_cast <const u_char *> (&this->valid);
		
		copy(valid, valid + sizes.valid, back_inserter(this->raw));
		
		copy(this->etag.begin(), this->etag.end(), back_inserter(this->raw));
		
		const u_char * cache = this->http.data();
		
		copy(cache, cache + sizes.cache, back_inserter(this->raw));
	}
	
	return this->raw.data();
}
 
void CCache::DataCache::set(const u_char * data, size_t size){
	
	if(size){
		
		size_t size_map = sizeof(Map);
		
		if(size_map < size){
			
			size_t size_it = size_map;
			
			for(size_t i = 0, j = 0; i < size_map; i += sizeof(size_t), j++){
				
				size_t size_data;
				
				memcpy(&size_data, data + i, sizeof(size_t));
				
				if(size_data && ((size_data + size_it) <= size)){
					
					switch(j){
						
						case 0: Anyks::(data, size_data, size_it, &this->ipv); break;
						
						case 1: Anyks::cpydata(data, size_data, size_it, &this->age); break;
						
						case 2: Anyks::cpydata(data, size_data, size_it, &this->date); break;
						
						case 3: Anyks::cpydata(data, size_data, size_it, &this->expires); break;
						
						case 4: Anyks::cpydata(data, size_data, size_it, &this->modified); break;
						
						case 5: Anyks::cpydata(data, size_data, size_it, &this->valid); break;
						
						case 6: Anyks::cpydata(data, size_data, size_it, this->etag); break;
						
						case 7: {
							
							u_char * buffer = new u_char [size_data];
							
							memcpy(buffer, data + size_it, size_data);
							
							this->http.assign(buffer, buffer + size_data);
							
							size_it += size_data;
							
							delete [] buffer;
						} break;
					}
				}
			}
		}
	}
}
 
CCache::DataCache::~DataCache(){
	
	this->raw.clear();
	
	vector <u_char> ().swap(this->raw);
}
 
void CCache::readDomain(const string filename, DataDNS * data){
	
	if(data){
		
		if(!filename.empty() && Anyks::isFileExist(filename.c_str())){
			
			ifstream file(filename.c_str(), ios::binary);
			
			if(file.is_open()){
				
				file.seekg(0, file.end);
				
				const long size = file.tellg();
				
				file.seekg(0, file.beg);
				
				if(size > 0){
					
					u_char * buffer = new u_char [size];
					
					while(file.good()) file.read((char *) buffer + file.tellg(), 60);
					
					data->set(buffer, size);
					
					delete [] buffer;
				}
				
				file.close();
			
			} else printf("Cannot read dns cache file %s\r\n", filename.c_str());
		}
	}
}
 
void CCache::readCache(const string filename, DataCache * data){
	
	if(data){
		
		if(!filename.empty() && Anyks::isFileExist(filename.c_str())){
			
			ifstream file(filename.c_str(), ios::binary);
			
			if(file.is_open()){
				
				file.seekg(0, file.end);
				
				const size_t size = file.tellg();
				
				file.seekg(0, file.beg);
				
				u_char * buffer = new u_char [size];
				
				while(file.good()) file.read((char *) buffer + file.tellg(), 60);
				
				data->set(buffer, size);
				
				delete [] buffer;
				
				file.close();
			
			} else printf("Cannot read cache file %s\r\n", filename.c_str());
		}
	}
}
 
void CCache::checkDomains(const string filename, void * ctx){
	
	CCache * ccache = reinterpret_cast <CCache *> (ctx);
	
	if(!filename.empty() && ccache){
		
		smatch match;
		
		regex e("[\\s\\S]+\\/data$", regex::ECMAScript | regex::icase);
		
		regex_search(filename, match, e);
		
		if(!match.empty()){
			
			DataDNS data;
			
			ccache->readDomain(filename, &data);
			
			time_t seconds = time(nullptr);
			
			if((data.ttl + ccache->config->cache.dttl) <= seconds){
				 
				
				regex e("^([\\s\\S]+)\\/data$", regex::ECMAScript | regex::icase);
				
				regex_search(filename, match, e);
				
				if(!match.empty()){
					
					const string rmDomain = match[1].str();
					
					printf("Domain was deleted: %s/\r\n", rmDomain.c_str());
					
					Anyks::rmDir(rmDomain.c_str());
				}
			}
		}
	}
}
 
void CCache::checkCache(const string filename, void * ctx){
	
	CCache * ccache = reinterpret_cast <CCache *> (ctx);
	
	if(!filename.empty() && ccache){
		
		smatch match;
		
		regex e("[\\s\\S]+\\/[A-Fa-f\\d]{32}$", regex::ECMAScript | regex::icase);
		
		regex_search(filename, match, e);
		
		if(!match.empty()){
			
			DataCache cache;
			
			ccache->readCache(filename, &cache);
			
			bool check = false;
			
			if(!cache.http.empty()){
				
				time_t date = time(nullptr);
				
				if(cache.expires){
					
					if(cache.expires < date) check = false;
					else check = true;
				}
				
				if(cache.age){
					
					time_t mdate = (cache.modified ? cache.modified : cache.date);
					
					if((mdate + cache.age) < date) check = false;
					else check = true;
				}
				 
			}
			
			if(!check){
				 
				
				regex e("^([\\s\\S]+)\\/\\d+\\/[\\s\\S]+\\/[A-Fa-f\\d]{32}$", regex::ECMAScript | regex::icase);
				
				regex_search(filename, match, e);
				
				if(!match.empty()){
					
					const string rmDomain = match[1].str();
					
					printf("Cache was deleted: %s/\r\n", rmDomain.c_str());
					
					Anyks::rmDir(rmDomain.c_str());
				}
			}
		}
	}
}
 
const short CCache::infoPatch(const string pathName){
	
	int result = -1;
	
	if(!pathName.empty()){
		
		struct stat entryInfo;
		
		if(lstat(pathName.c_str(), &entryInfo) == 0){
			
			if(S_ISDIR(entryInfo.st_mode)){
				
				printf("Catalog name: %s/\r\n", pathName.c_str());
				
				return 1;
			
			} else if(S_ISREG(entryInfo.st_mode)) {
				
				printf("\t%s has %lld bytes\r\n", pathName.c_str(), (long long) entryInfo.st_size);
				
				return 0;
			
			} else if(S_ISLNK(entryInfo.st_mode)) {
				
				char targetName[PATH_MAX + 1];
				
				if(readlink(pathName.c_str(), targetName, PATH_MAX) != -1){
					
					printf("\t%s -> %s\r\n", pathName.c_str(), targetName);
					
					return infoPatch(targetName);
				
				} else printf("\t%s -> (invalid symbolic link!)\r\n", pathName.c_str());
			}
		
		} else printf("Error statting %s: %s\r\n", pathName.c_str(), strerror(errno));
	}
	
	return result;
}
 
const u_long CCache::processDirectory(const string curDir, handler fn){
	
	struct dirent entry;
	struct dirent * entryPtr = nullptr;
	
	u_long count = 0;
	
	char pathName[PATH_MAX + 1];
	
	DIR * dir = opendir(curDir.c_str());
	
	if(dir == nullptr){
		
		printf("Error opening %s: %s\r\n", curDir.c_str(), strerror(errno));
		
		return 0;
	}
	
	int retval = readdir_r(dir, &entry, &entryPtr);
	
	while(entryPtr != nullptr){
		
		if((strncmp(entry.d_name, ".", PATH_MAX) == 0)
		|| (strncmp(entry.d_name, "..", PATH_MAX) == 0)){
			 
			retval = readdir_r(dir, &entry, &entryPtr);
			continue;
		}
		
		(void) strncpy(pathName, curDir.c_str(), PATH_MAX);
		
		(void) strncat(pathName, "/", PATH_MAX);
		
		(void) strncat(pathName, entry.d_name, PATH_MAX);
		
		int info = infoPatch(pathName);
		
		if(info > -1){
			
			count++;
			
			switch(info){
				
				case 0: fn(pathName, this);							break;
				
				case 1: count += processDirectory(pathName, fn);	break;
			}
		}
		
		retval = readdir_r(dir, &entry, &entryPtr);
	}
	
	(void) closedir(dir);
	
	return count;
}
 
CCache::CCache(const string config){
	
	this->config = new Config(config);
	
	const string dns = Anyks::addToPath(this->config->cache.dir, "dns");
	
	const string cache = Anyks::addToPath(this->config->cache.dir, "cache");
	
	processDirectory(dns, &CCache::checkDomains);
	
	processDirectory(cache, &CCache::checkCache);
}
 
CCache::~CCache(){
	
	if(this->config) delete this->config;
}
 
int main(int argc, char * argv[]){
	
	
	setlocale(LC_ALL, "en_US.UTF-8");
	
	string configfile;
	
	string param = (argc >= 2 ? argv[1] : "");
	
	if(param.compare("-c") == 0)
		
		configfile = (argc >= 3 ? argv[2] : "");
	
	else if(param.find("--config=") != string::npos)
		
		configfile = param.replace(0, 9, "");
	
	CCache * app = new CCache(configfile);
	
	if(app) delete app;
	
	return 0;
}
