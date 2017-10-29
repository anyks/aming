/* SERVER PROXY AMING */
/*
*  author:     Yuriy Lobarev
*  phone:      +7(910)983-95-90
*  telegram:   @forman
*  email:      info@anyks.com
*  date:       10/29/2017 16:59:43
*  copyright:  © 2017 anyks.com
*/
 
 

#ifndef _DNS_RESOLVER_AMING_
#define _DNS_RESOLVER_AMING_

#include <mutex>
#include <regex>
#include <string>
#include <vector>
#include <random>
#include <ctime>
#include <stdio.h>
#include <stdlib.h>
#include <sys/s/o/c/k/e/t/./h/>/
/#/i/n/c/l/u/d/e/ /</n/e/t/i/n/e/t
#include <event2/d/n/s/./h/>/
/#/i/n/c/l/u/d/e/ /</e/v/e/n/t/2
#include <event2/e/v/e/n/t/./h/>/
/#/i/n/c/l/u/d/e/ /"/l/o/g
#include "cache/cache.h"


using namespace std;

 
class DNSResolver {
	private:
		 
		typedef void (* handler) (const string ip, void * ctx);
		
		struct DomainData {
			handler fn;		
			int family;		
			void * ctx;		
			string domain;	
			LogApp * log;	
			Cache * cache;	
		};
		
		mutex mtx;
		
		LogApp * log = nullptr;
		
		Cache * cache = nullptr;
		
		struct event_base * base = nullptr;
		
		struct evdns_base * dnsbase = nullptr;
		
		vector <string> servers;
		 
		void createDNSBase();
		 
		static void callback(int errcode, struct evutil_addrinfo * addr, void * ctx);
	public:
		 
		void resolve(const string domain, const int family, handler fn, void * ctx = nullptr);
		 
		void setLog(LogApp * log);
		 
		void setBase(struct event_base * base);
		 
		void setNameServer(const string server);
		 
		void setNameServers(vector <string> servers);
		 
		DNSResolver(LogApp * log = nullptr, Cache * cache = nullptr, struct event_base * base = nullptr, vector <string> servers = {});
		 
		~DNSResolver();
};

#endif 
