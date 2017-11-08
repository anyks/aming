/* SERVER PROXY AMING */
/*
*  author:     Yuriy Lobarev
*  phone:      +7(910)983-95-90
*  telegram:   @forman
*  email:      info@anyks.com
*  date:       11/08/2017 16:52:48
*  copyright:  © 2017 anyks.com
*/
 
 

#include "cache/cache.h"


using namespace std;

 
const size_t Cache::DataDNS::size(){
	
	if(this->raw.empty()) data();
	
	return this->raw.size();
}
 
const u_char * Cache::DataDNS::data(){
	
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
 
void Cache::DataDNS::set(const u_char * data, size_t size){
	
	if(size){
		
		size_t size_map = sizeof(Map);
		
		if(size_map < size){
			
			size_t size_it = size_map;
			
			for(size_t i = 0, j = 0; i < size_map; i += sizeof(size_t), j++){
				
				size_t size_data;
				
				memcpy(&size_data, data + i, sizeof(size_t));
				
				if(size_data && ((size_data + size_it) <= size)){
					
					switch(j){
						
						case 0: Anyks::cpydata(data, size_data, size_it, &this->ttl); break;
						
						case 1: Anyks::cpydata(data, size_data, size_it, this->ipv4); break;
						
						case 2: Anyks::cpydata(data, size_data, size_it, this->ipv6); break;
					}
				}
			}
		}
	}
}
 
Cache::DataDNS::~DataDNS(){
	
	this->raw.clear();
	
	vector <u_char> ().swap(this->raw);
}
 
const size_t Cache::DataCache::size(){
	
	if(this->raw.empty()) data();
	
	return this->raw.size();
}
 
const u_char * Cache::DataCache::data(){
	
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
 
void Cache::DataCache::set(const u_char * data, size_t size){
	
	if(size){
		
		size_t size_map = sizeof(Map);
		
		if(size_map < size){
			
			size_t size_it = size_map;
			
			for(size_t i = 0, j = 0; i < size_map; i += sizeof(size_t), j++){
				
				size_t size_data;
				
				memcpy(&size_data, data + i, sizeof(size_t));
				
				if(size_data && ((size_data + size_it) <= size)){
					
					switch(j){
						
						case 0: Anyks::cpydata(data, size_data, size_it, &this->ipv); break;
						
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
 
Cache::DataCache::~DataCache(){
	
	this->raw.clear();
	
	vector <u_char> ().swap(this->raw);
}
 
const string Cache::getPathDomain(const string domain){
	
	string result;
	
	if(!domain.empty()){
		
		smatch match;
		
		regex e("[\\w\\-\\.]+\\.[\\w\\-]+", regex::ECMAScript | regex::icase);
		
		regex_search(domain, match, e);
		
		if(!match.empty()){
			
			regex e("\\.");
			
			result = match[0].str();
			
			result = regex_replace(result, e, "/");
			
			reverse(result.begin(), result.end());
		}
	}
	
	return result;
}
 
void Cache::readDomain(const string domain, DataDNS * data){
	
	if(this->config->cache.dns && data){
		
		string dir = this->config->cache.dir;
		
		dir = Anyks::addToPath(dir, "dns");
		
		dir = Anyks::addToPath(dir, getPathDomain(domain));
		
		const string filename = Anyks::addToPath(dir, "data");
		
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
			
			} else if(this->log) this->log->write(LOG_ERROR, 0, "cannot read dns cache file %s for domain %s", filename.c_str(), domain.c_str());
		}
	}
}
 
void Cache::readCache(HttpData &http, DataCache * data){
	
	if(this->config->cache.dat){
		
		string dir = this->config->cache.dir;
		
		dir = Anyks::addToPath(dir, "cache");
		
		dir = Anyks::addToPath(dir, getPathDomain(http.getHost()));
		
		dir = Anyks::addToPath(dir, to_string(http.getPort()));
		
		dir = Anyks::addToPath(dir, http.getMethod());
		
		dir = Anyks::addToPath(dir, (http.isAlive() ? "a" : "c"));
		
		const string filename = Anyks::addToPath(dir, Anyks::md5(http.getPath()));
		
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
			
			} else if(this->log) this->log->write(LOG_ERROR, 0, "cannot read cache file %s for domain %s", filename.c_str(), http.getHost().c_str());
		}
	}
}
 
void Cache::writeDomain(const string domain, DataDNS data){
	
	if(this->config->cache.dns){
		
		string dir = this->config->cache.dir;
		
		dir = Anyks::addToPath(dir, "dns");
		
		dir = Anyks::addToPath(dir, getPathDomain(domain));
		
		const string filename = Anyks::addToPath(dir, "data");
		
		if(!filename.empty()){
			
			if(!Anyks::makePath(dir.c_str(), this->config->proxy.user, this->config->proxy.group)){
				
				if(this->log) this->log->write(LOG_ERROR, 0, "unable to create directory for dns cache file %s for domain %s", dir.c_str(), domain.c_str());
				
				return;
			}
			
			ofstream file(filename.c_str(), ios::binary);
			
			if(file.is_open()){
				
				file.write((const char *) data.data(), data.size());
				
				file.close();
			
			} else if(this->log) this->log->write(LOG_ERROR, 0, "cannot write dns cache file %s for domain %s", filename.c_str(), domain.c_str());
		}
	}
}
 
void Cache::writeCache(HttpData &http, DataCache data){
	
	if(this->config->cache.dat){
		
		string dir = this->config->cache.dir;
		
		dir = Anyks::addToPath(dir, "cache");
		
		dir = Anyks::addToPath(dir, getPathDomain(http.getHost()));
		
		dir = Anyks::addToPath(dir, to_string(http.getPort()));
		
		dir = Anyks::addToPath(dir, http.getMethod());
		
		dir = Anyks::addToPath(dir, (http.isAlive() ? "a" : "c"));
		
		const string filename = Anyks::addToPath(dir, Anyks::md5(http.getPath()));
		
		if(!filename.empty()){
			
			if(!Anyks::makePath(dir.c_str(), this->config->proxy.user, this->config->proxy.group)){
				
				if(this->log) this->log->write(LOG_ERROR, 0, "unable to create directory for cache file %s for domain %s", dir.c_str(), http.getHost().c_str());
				
				return;
			}
			
			ofstream file(filename.c_str(), ios::binary);
			
			if(file.is_open()){
				
				file.write((const char *) data.data(), data.size());
				
				file.close();
			
			} else if(this->log) this->log->write(LOG_ERROR, 0, "cannot write cache file %s for domain %s", filename.c_str(), http.getHost().c_str());
		}
	}
}
 
const bool Cache::isDomain(const string domain){
	
	smatch match;
	
	regex e("[\\w\\-\\.]+\\.[\\w\\-]+", regex::ECMAScript | regex::icase);
	
	regex_search(domain, match, e);
	
	return !match.empty();
}
 
const bool Cache::isIpV4(const string ip){
	
	smatch match;
	
	regex e("\\d{1,3}(?:\\.\\d{1,3}){3}", regex::ECMAScript | regex::icase);
	
	regex_search(ip, match, e);
	
	return !match.empty();
}
 
const bool Cache::isIpV6(const string ip){
	
	smatch match;
	
	regex e("[A-Fa-f\\d]{4}(?:\\:[A-Fa-f\\d]{4}){7}", regex::ECMAScript | regex::icase);
	
	regex_search(ip, match, e);
	
	return !match.empty();
}
 
const bool Cache::checkEnabledCache(HttpData &http){
	
	bool result = true;
	
	if(this->config->cache.dat && http.isEndHeaders()){
		
		const u_int status = http.getStatus();
		
		if(((status > 199) && (status < 300))
		|| ((status > 399) && (status < 599))){
			
			time_t cdate = time(nullptr), ldate = 0, expires = 0;
			
			const string dt = http.getHeader("date");
			
			const string ag = http.getHeader("age");
			
			const string et = http.getHeader("etag");
			
			const string pr = http.getHeader("pragma");
			
			const string ex = http.getHeader("expires");
			
			const string cc = http.getHeader("cache-control");
			
			const string lm = http.getHeader("last-modified");
			
			if(!dt.empty()) cdate = Anyks::strToTime(dt.c_str());
			
			if(!lm.empty()) ldate = Anyks::strToTime(lm.c_str());
			
			if(!ex.empty()) expires = Anyks::strToTime(ex.c_str());
			
			if(!pr.empty() && (pr.find("no-cache") != string::npos)) result = false;
			
			if(expires && (cdate >= expires)) result = false;
			
			if(!cc.empty()){
				
				bool ccPrivate		= (cc.find("private") != string::npos);
				bool ccNoStore		= (cc.find("no-store") != string::npos);
				bool ccNoCache		= (cc.find("no-cache") != string::npos);
				bool ccRevalidate	= (cc.find("proxy-revalidate") != string::npos);
				bool ccNoTransform	= (cc.find("no-transform") != string::npos);
				
				
				
				if(ccNoStore || ccPrivate || ccNoTransform) result = false;
				
				else if((ccNoCache || ccRevalidate) && (et.empty() && lm.empty())) result = false;
				
				else if(!et.empty()) result = true;
				
				else {
					
					size_t age = 0;
					
					auto control = Anyks::split(cc, ",");
					
					for(auto it = control.begin(); it != control.end(); it++){
						
						const string cache = * it;
						
						bool ccMaxAge	= (cache.find("max-age") != string::npos);
						bool ccsMaxAge	= (cache.find("s-maxage") != string::npos);
						
						if(ccMaxAge || ccsMaxAge){
							
							if(!age || ccsMaxAge){
								
								const size_t pos = cache.find("=");
								
								if(pos != string::npos) age = ::atoi(cache.substr(pos + 1, cache.length() - (pos + 1)).c_str());
								
								if(!age || (ldate && ((ldate + age) < cdate))) result = false;
								
								else if(ldate && ((ldate + age) >= cdate)) result = true;
								
								if(age && (!ag.empty() && (::atoi(ag.c_str()) < age))) result = true;
								
								else if(age && !ag.empty()) result = false;
							}
						}
					}
				}
			
			} else if(!et.empty()) result = true;
		}
	}
	
	return result;
}
 
const string Cache::getDomain(const string domain){
	
	string result;
	
	if(this->config->cache.dns){
		
		if(!domain.empty() && isDomain(domain)){
			
			DataDNS data;
			
			readDomain(domain, &data);
			
			time_t seconds = time(nullptr);
			
			if((data.ttl + this->config->cache.dttl) > seconds){
				
				Network nwk;
				
				switch(this->config->proxy.extIPv){
					
					case 4: result = nwk.getLowIp(data.ipv4);	break;
					
					case 6: result = nwk.getLowIp6(data.ipv6);	break;
				}
			}
		}
	}
	
	return result;
}
 
Cache::ResultData Cache::getCache(HttpData &http){
	
	ResultData result;
	
	if(this->config->cache.dat && http.isEndHeaders()){
		
		const string inm = http.getHeader("if-none-match");
		
		const string ims = http.getHeader("if-modified-since");
		
		if(inm.empty() && ims.empty()){
			
			DataCache cache;
			
			readCache(http, &cache);
			
			if(!cache.http.empty()){
				
				bool check = false;
				
				time_t date = time(nullptr);
				
				if(cache.expires){
					
					if(cache.expires < date) check = false;
					else check = true;
				}
				
				if(cache.age){
					
					time_t mdate = (cache.modified ? cache.modified : cache.date);
					
					if((mdate + cache.age) < date) check = false;
					else check = true;
					
					result.age = (date - mdate);
				}
				
				if(check || !cache.etag.empty() || (cache.modified < date)){
					
					if(!cache.etag.empty()) result.etag = cache.etag;
					
					if(cache.modified) result.modified = Anyks::timeToStr(cache.modified);
					
					result.ipv = cache.ipv;
					
					result.valid = cache.valid;
					
					if(!check && !result.valid) result.valid = true;
					
					result.http.assign(cache.http.begin(), cache.http.end());
				
				} else rmCache(http);
			
			}
		}
	}
	
	return result;
}
 
void Cache::setDomain(const string domain, const string ip){
	
	if(this->config->cache.dns){
		
		if(!domain.empty() && !ip.empty() && isDomain(domain)){
			
			Network nwk;
			
			DataDNS data;
			
			readDomain(domain, &data);
			
			u_int nettype = nwk.checkNetworkByIp(ip);
			
			switch(nettype){
				
				case 4: {
					
					const string ipv4 = nwk.setLowIp(ip);
					
					if(isIpV4(ipv4)) data.ipv4 = ipv4;
				} break;
				
				case 6: {
					
					const string ipv6 = nwk.setLowIp6(ip);
					
					if(isIpV6(ipv6)) data.ipv6 = ipv6;
				} break;
			}
			
			data.ttl = time(nullptr);
			
			writeDomain(domain, data);
		}
	}
}
 
void Cache::rmDomain(const string domain){
	
	if(this->config->cache.dns){
		
		string dir = this->config->cache.dir;
		
		dir = Anyks::addToPath(dir, "dns");
		
		dir = Anyks::addToPath(dir, getPathDomain(domain));
		
		if(!dir.empty() && Anyks::isDirExist(dir.c_str())) Anyks::rmDir(dir.c_str());
	}
}
 
void Cache::rmAllDomains(){
	
	if(this->config->cache.dns){
		
		string dir = this->config->cache.dir;
		
		dir = Anyks::addToPath(dir, "dns");
		
		if(!dir.empty() && Anyks::isDirExist(dir.c_str())) Anyks::rmDir(dir.c_str());
	}
}
 
void Cache::setCache(HttpData &http){
	
	if(this->config->cache.dat && http.isEndHeaders()){
		
		const string method = http.getMethod();
		
		if(((method.compare("get") == 0)
		|| (method.compare("post") == 0))
		
		&& checkEnabledCache(http)){
			
			const string dt = http.getHeader("date");
			
			const string ag = http.getHeader("age");
			
			const string ex = http.getHeader("expires");
			
			const string cc = http.getHeader("cache-control");
			
			const string lm = http.getHeader("last-modified");
			
			bool valid = false;
			
			time_t age = 0, expires = 0, modified = 0, date = time(nullptr);
			
			if(!dt.empty()) date = Anyks::strToTime(dt.c_str());
			
			if(!lm.empty()) modified = Anyks::strToTime(lm.c_str());
			
			if(!ex.empty()) expires = Anyks::strToTime(ex.c_str());
			
			if(!cc.empty()){
				
				auto control = Anyks::split(cc, ",");
				
				for(auto it = control.begin(); it != control.end(); it++){
					
					const string cache = * it;
					
					if((cache.compare("no-cache") == 0)
					|| (cache.compare("proxy-revalidate") == 0)) valid = true;
					
					else if((cache.find("s-maxage") != string::npos)
					|| ((cache.find("max-age") != string::npos)
					&& (cc.find("s-maxage") == string::npos))){
						
						size_t pos = cache.find("=");
						
						if(pos != string::npos) age = ::atoi(cache.substr(pos + 1, cache.length() - (pos + 1)).c_str());
						
						if(age && !ag.empty()) age -= ::atoi(ag.c_str());
					}
				}
			}
			
			DataCache cache;
			
			const u_char * dump = http.data();
			
			string etag = http.getHeader("etag");
			
			size_t posEtag = etag.find("W/");
			
			if(posEtag != string::npos) etag = etag.substr(posEtag + 2, etag.length() - 2);
			
			cache.age		= age;
			cache.etag		= etag;
			cache.date		= date;
			cache.valid		= valid;
			cache.expires	= expires;
			cache.modified	= modified;
			cache.ipv		= this->config->proxy.extIPv;
			cache.http.assign(dump, dump + http.size());
			
			if(!cache.http.empty()) writeCache(http, cache);
		}
	}
}
 
void Cache::rmCache(HttpData &http){
	
	if(this->config->cache.dat && http.isEndHeaders()){
		
		string dir = this->config->cache.dir;
		
		dir = Anyks::addToPath(dir, "cache");
		
		dir = Anyks::addToPath(dir, getPathDomain(http.getHost()));
		
		if(!dir.empty() && Anyks::isDirExist(dir.c_str())) Anyks::rmDir(dir.c_str());
	}
}
 
void Cache::rmAllCache(){
	
	if(this->config->cache.dat){
		
		string dir = this->config->cache.dir;
		
		dir = Anyks::addToPath(dir, "cache");
		
		if(!dir.empty() && Anyks::isDirExist(dir.c_str())) Anyks::rmDir(dir.c_str());
	}
}
 
Cache::Cache(Config * config, LogApp * log){
	
	if(config){
		
		this->log = log;
		
		this->config = config;
	}
}