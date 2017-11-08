/* SERVER PROXY AMING */
/*
*  author:     Yuriy Lobarev
*  phone:      +7(910)983-95-90
*  telegram:   @forman
*  email:      info@anyks.com
*  date:       11/08/2017 16:52:48
*  copyright:  © 2017 anyks.com
*/
 
 

#ifndef _CACHE_AMING_
#define _CACHE_AMING_

#include <regex>
#include <string>
#include <vector>
#include <fstream>
#include <iostream>
#include <ctime>
#include <stdlib.h>
#include "config/conf.h"
#include "nwk/nwk.h"
#include "log/log.h"
#include "http/http.h"
#include "general/general.h"


using namespace std;

 
class Cache {
	private:
		 
		class DataDNS {
			private:
				 
				struct Map {
					size_t ttl;
					size_t ipv4;
					size_t ipv6;
				};
				
				vector <u_char> raw;
			public:
				
				time_t ttl = 0;
				
				string ipv4;
				
				string ipv6;
				 
				const size_t size();
				 
				const u_char * data();
				 
				void set(const u_char * data, size_t size);
				 
				~DataDNS();
		};
		 
		class DataCache {
			private:
				 
				struct Map {
					size_t ipv;
					size_t age;
					size_t date;
					size_t expires;
					size_t modified;
					size_t valid;
					size_t etag;
					size_t cache;
				};
				
				vector <u_char> raw;
			public:
				u_int ipv;				
				time_t age;				
				time_t date;			
				time_t expires;			
				time_t modified;		
				bool valid;				
				string etag;			
				vector <u_char> http;	
				 
				const size_t size();
				 
				const u_char * data();
				 
				void set(const u_char * data, size_t size);
				 
				~DataCache();
		};
		 
		struct ResultData {
			
			bool valid;
			
			u_int ipv;
			
			time_t age;
			
			string etag;
			
			string modified;
			
			vector <u_char> http;
		};
		
		LogApp * log = nullptr;
		
		Config * config = nullptr;
		 
		const string getPathDomain(const string domain);
		 
		void readDomain(const string domain, DataDNS * data);
		 
		void readCache(HttpData &http, DataCache * data);
		 
		void writeDomain(const string domain, DataDNS data);
		 
		void writeCache(HttpData &http, DataCache data);
		 
		const bool isDomain(const string domain);
		 
		const bool isIpV4(const string ip);
		 
		const bool isIpV6(const string ip);
		 
		const bool checkEnabledCache(HttpData &http);
	public:
		 
		const string getDomain(const string domain);
		 
		ResultData getCache(HttpData &http);
		 
		void setDomain(const string domain, const string ip);
		 
		void rmDomain(const string domain);
		 
		void rmAllDomains();
		 
		void setCache(HttpData &http);
		 
		void rmCache(HttpData &http);
		 
		void rmAllCache();
		 
		Cache(Config * config = nullptr, LogApp * log = nullptr);
};

#endif 