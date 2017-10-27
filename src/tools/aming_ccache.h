/* SERVER PROXY AMING */
/*
*  author:     Yuriy Lobarev
*  phone:      +7(910)983-95-90
*  telegram:   @forman
*  email:      info@anyks.com
*  date:       10/27/2017 18:42:35
*  copyright:  © 2017 anyks.com
*/
 
 

 
#ifndef _CACHE_CLEANER_AMING_
#define _CACHE_CLEANER_AMING_

#include <map>
#include <regex>
#include <vector>
#include <string>
#include <fstream>
#include <iostream>
#include <stdio.h>
#include <stdlib.h>
#include <errno.h>
#include <stdlib.h>
#include <limits.h>
#include <unistd.h>
#include <dirent.h>
#include <sys/s/t/a/t/./h/>/
/#/i/n/c/l/u/d/e/ /</s/y/s
#include "../include/config/conf.h"
#include "../include/general/general.h"


using namespace std;

 
class CCache {
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
		
		Config * config = nullptr;
		 
		typedef void (* handler) (const string filename, void * ctx);
		 
		void readDomain(const string filename, DataDNS * data);
		 
		void readCache(const string filename, DataCache * data);
		 
		static void checkDomains(const string filename, void * ctx);
		 
		static void checkCache(const string filename, void * ctx);
		 
		const short infoPatch(const string pathName);
		 
		const u_long processDirectory(const string curDir, handler fn);
	public:
		 
		CCache(const string config);
		 
		~CCache();
};

#endif 