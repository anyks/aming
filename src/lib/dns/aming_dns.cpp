/* SERVER PROXY AMING */
/*
*  author:     Yuriy Lobarev
*  phone:      +7(910)983-95-90
*  telegram:   @forman
*  email:      info@anyks.com
*  date:       10/27/2017 18:42:35
*  copyright:  © 2017 anyks.com
*/
 
 

#include "dns/dns.h"


using namespace std;

 
void DNSResolver::callback(int errcode, struct evutil_addrinfo * addr, void * ctx){
	
	DomainData * domainData = reinterpret_cast <DomainData *> (ctx);
	
	if(domainData){
		
		string ip;
		
		vector <string> ips;
		
		if(errcode){
			
			if(domainData->log) domainData->log->write(LOG_ERROR, 0, "%s %s", domainData->domain.c_str(), evutil_gai_strerror(errcode));
		} else {
			
			struct evutil_addrinfo * ai;
			
			for(ai = addr; ai; ai = ai->ai_next){
				
				char buf[128];
				
				const char * ip = nullptr;
				
				if((ai->ai_family == domainData->family) || (domainData->family == AF_UNSPEC)){
					
					switch(ai->ai_family){
						
						case AF_INET: {
							
							struct sockaddr_in * sin = (struct sockaddr_in *) ai->ai_addr;
							
							ip = evutil_inet_ntop(ai->ai_family, &sin->sin_addr, buf, 128);
						} break;
						
						case AF_INET6: {
							
							struct sockaddr_in6 * sin6 = (struct sockaddr_in6 *) ai->ai_addr;
							
							ip = evutil_inet_ntop(ai->ai_family, &sin6->sin6_addr, buf, 128);
						} break;
					}
					
					ips.push_back(ip);
				}
			}
			
			evutil_freeaddrinfo(addr);
		}
		
		if(!ips.empty()){
			
			if(ips.size() > 1){
				
				srand(time(0));
				
				ip = ips[rand() % ips.size()];
			
			} else ip = ips[0];
		}
		
		domainData->cache->setDomain(domainData->domain, ip);
		
		domainData->fn(ip, domainData->ctx);
		
		delete domainData;
	}
}
 
void DNSResolver::resolve(const string domain, const int family, handler fn, void * ctx){
	
	if(!domain.empty()){
		
		smatch match;
		
		regex e("^\\[?(\\d{1,3}(?:\\.\\d{1,3}){3}|[A-Fa-f\\d\\:]{2,39})\\]?$", regex::ECMAScript | regex::icase);
		
		regex_search(domain, match, e);
		
		if(match.empty()){
			
			string ip = this->cache->getDomain(domain);
			
			if(!ip.empty()) fn(ip, ctx);
			
			else {
				
				struct evutil_addrinfo hints;
				
				memset(&hints, 0, sizeof(hints));
				
				hints.ai_family = AF_UNSPEC;
				
				hints.ai_flags = EVUTIL_AI_CANONNAME;
				
				hints.ai_socktype = SOCK_STREAM;
				
				hints.ai_protocol = IPPROTO_TCP;
				
				DomainData * domainData = new DomainData;
				
				domainData->fn = fn;
				
				domainData->ctx = ctx;
				
				domainData->domain = domain;
				
				domainData->log = this->log;
				
				domainData->cache = this->cache;
				
				domainData->family = family;
				
				struct evdns_getaddrinfo_request * req = evdns_getaddrinfo(this->dnsbase, domain.c_str(), nullptr, &hints, &DNSResolver::callback, domainData);
				
				if((req == nullptr) && this->log) this->log->write(LOG_ERROR, 0, "request for %s returned immediately", domain.c_str());
			}
		
		} else fn(match[1].str(), ctx);
	}
	
	return;
}
 
void DNSResolver::createDNSBase(){
	
	if(this->base){
		
		if(this->dnsbase) evdns_base_free(this->dnsbase, 0);
		
		this->dnsbase = evdns_base_new(this->base, 0);
		
		if(!this->dnsbase && this->log){
			
			this->log->write(LOG_ERROR, 0, "dns base does not created!");
		}
	}
	
	setNameServers(this->servers);
}
 
void DNSResolver::setLog(LogApp * log){
	
	if(log) this->log = log;
}
 
void DNSResolver::setBase(struct event_base * base){
	
	this->base = base;
	
	createDNSBase();
}
 
void DNSResolver::setNameServer(const string server){
	
	if(!server.empty() && this->dnsbase){
		
		if(evdns_base_nameserver_ip_add(this->dnsbase, server.c_str()) != 0){
			
			if(this->log) this->log->write(LOG_ERROR, 0, "name server [%s] does not add!", server.c_str());
		}
	}
}
 
void DNSResolver::setNameServers(vector <string> servers){
	
	if(!servers.empty()){
		
		this->servers = servers;
		
		for(u_int i = 0; i < this->servers.size(); i++){
			
			setNameServer(this->servers[i]);
		}
	}
}
 
DNSResolver::DNSResolver(LogApp * log, Cache * cache, struct event_base * base, vector <string> servers){
	
	this->log = log;
	
	this->cache = cache;
	
	this->base = base;
	
	this->servers = servers;
	
	createDNSBase();
}
 
DNSResolver::~DNSResolver(){
	
	this->mtx.lock();
	
	if(this->dnsbase){
		
		evdns_base_free(this->dnsbase, 0);
		
		this->dnsbase = nullptr;
	}
	
	this->mtx.unlock();
}