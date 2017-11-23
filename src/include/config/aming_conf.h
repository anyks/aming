/* SERVER PROXY AMING */
/*
*  author:     Yuriy Lobarev
*  phone:      +7(910)983-95-90
*  telegram:   @forman
*  email:      info@anyks.com
*  date:       11/23/2017 17:50:05
*  copyright:  © 2017 anyks.com
*/
 
 

#ifndef _CONFIG_AMING_
#define _CONFIG_AMING_

#include <map>
#include <string>
#include <vector>
#include <iostream>
#include <zlib.h>
#include <stdlib.h>
#include <sys/s/t/a/t/./h/>/
/#/i/n/c/l/u/d/e/ /"/i/n/i
#include "defaults/defaults.h"
#include "general/general.h"


using namespace std;

 
class Config {
	private:
		 
		struct Gzip {
			bool vary;					
			int level;					
			long length;				
			size_t chunk;				
			string regex;				
			vector <string> vhttp;		
			vector <string> proxied;	
			vector <string> types;		
		};
		 
		struct Timeouts {
			size_t read;	
			size_t write;	
			size_t upgrade;	
		};
		 
		struct BufferSize {
			long read;		
			long write;		
		};
		 
		struct IP {
			string internal;			
			vector <string> external;	
			vector <string> resolver;	
		};
		 
		struct Keepalive {
			int keepcnt;	
			int keepidle;	
			int keepintvl;	
		};
		 
		struct Logs {
			bool files;		
			bool console;	
			bool data;		
			bool enabled;	
			size_t size;	
			string dir;		
		};
		 
		struct Authorization {
			u_short auth;		
			u_short services;	
			u_int gmaxpam;		
			bool enabled;		
		};
		 
		struct Firewall {
			u_int maxtryauth;		
			bool blockauth;			
			bool blockloop;			
			bool blacklist;			
			bool whitelist;			
			bool bandlimin;			
			string timeblockloop;	
		};
		 
		struct Os {
			u_int ncpu;		
			string cpu;		
			string name;	
		};
		 
		struct Cache {
			bool dns;		
			bool dat;		
			time_t dttl;	
			string dir;		
		};
		 
		struct Connects {
			u_short key;	
			u_int fds;		
			u_int connect;	
			int total;		
			size_t size;	
		};
		 
		struct Ldap {
			private:
				 
				struct Dn {
					string groups;		
					string users;		
					string configs;		
					string headers;		
				};
				 
				struct Scope {
					string groups;	
					string users;	
				};
				 
				struct Filter {
					string groups;	
					string users;	
				};
				 
				struct Keys {
					private:
						 
						struct Users {
							string uid;			
							string login;		
							string firstname;	
							string lastname;	
							string secondname;	
							string desc;		
							string pass;		
						};
						 
						struct Groups {
							string gid;		
							string login;	
							string desc;	
							string pass;	
							string member;	
						};
					public:
						Users users;
						Groups groups;
				};
			public:
				u_int version;				
				string binddn;				
				string bindpw;				
				Dn dn;						
				Scope scope;				
				Filter filter;				
				Keys keys;					
				vector <string> servers;	
		};
		 
		struct Proxy {
			u_short type;				
			u_short configs;			
			u_int intIPv;				
			u_int extIPv;				
			u_int port;					
			u_int workers;				
			time_t conftime;			
			bool debug;					
			bool daemon;				
			bool reverse;				
			bool transfer;				
			bool forward;				
			bool ipv6only;				
			bool subnet;				
			bool optimos;				
			bool pipelining;			
			string user;				
			string group;				
			string name;				
			string piddir;				
			string dir;					
			vector <string> redirect;	
			map <u_int, string> atemp;	
		};
		
		string filename;
	public:
		
		Os os;					
		IP ipv6;				
		IP ipv4;				
		Ldap ldap;				
		Logs logs;				
		Gzip gzip;				
		Cache cache;			
		Proxy proxy;			
		Connects connects;		
		Firewall firewall;		
		Timeouts timeouts;		
		BufferSize buffers;		
		Authorization auth;		
		Keepalive keepalive;	
		
		u_short options;
		 
		Config(const string filename = "");
};

#endif 
