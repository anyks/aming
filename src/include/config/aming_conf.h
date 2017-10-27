/* SERVER PROXY AMING */
/*
*  author:     Yuriy Lobarev
*  phone:      +7(910)983-95-90
*  telegram:   @forman
*  email:      info@anyks.com
*  date:       10/27/2017 18:42:35
*  copyright:  © 2017 anyks.com
*/
 
 

#ifndef _CONFIG_AMING_
#define _CONFIG_AMING_

#include <string>
#include <vector>
#include <iostream>
#include <zlib.h>
#include <stdlib.h>
#include <sys/s/t/a/t/./h/>/
/#/i/n/c/l/u/d/e/ /"/i/n/i
#include "general/general.h"


#define APP_NAME "AMING"
#define APP_VERSION "1.0"
#define APP_YEAR "2017"
#define APP_COPYRIGHT "ANYKS LLC"
#define APP_DOMAIN "anyks.com"
#define APP_SITE "http://anyks.com"
#define APP_EMAIL "info@anyks.com"
#define APP_SUPPORT "support@anyks.com"
#define APP_AUTHOR "Yuriy Lobarev"
#define APP_TELEGRAM "@forman"
#define APP_PHONE "+7(910)983-95-90"


#define OPT_NULL 0x00		
#define OPT_CONNECT 0x01	
#define OPT_UPGRADE 0x02	
#define OPT_AGENT 0x04		
#define OPT_GZIP 0x08		
#define OPT_SMART 0x10		
#define OPT_KEEPALIVE 0x20	
#define OPT_LOG 0x40		
#define OPT_PGZIP 0x80		
#define OPT_DEBLOCK 0x100	


#define PROXY_TYPE "http"
#define PROXY_USER "nobody"
#define PROXY_GROUP "nogroup"
#define PROXY_NAME "anyks"
#define PROXY_TRANSFER false
#define PROXY_REVERSE false
#define PROXY_FORWARD true
#define PROXY_OPTIMOS false
#define PROXY_DEBUG false
#define PROXY_DAEMON false
#define PROXY_IPV6ONLY true
#define PROXY_SUBNET false
#define PROXY_PIPELINING true
#define PROXY_IPV "4 -> 4"
#define PROXY_HTTP_PORT 8080
#define PROXY_SOCKS5_PORT 1080
#define PROXY_REDIRECT_PORT 1180


#define CACHE_DNS true
#define CACHE_RESPONSE false
#define CACHE_DTTL "1d"
#define CACHE_DIR "/var/cache"


#define PID_DIR "/var/run"


#ifdef __linux__
	#define CONFIG_DIR "/etc"
#else
	#define CONFIG_DIR "/usr/local/etc"
#endif


#define MAX_WORKERS 0

#define MMAX_WORKERS 300


#define CONNECTS_KEY "ip"
#define CONNECTS_TOTAL 100
#define CONNECTS_CONNECT 100
#define CONNECTS_FDS 1024 * 10
#define CONNECTS_SIZE "5MB"


#define IPV4_INTERNAL "127.0.0.1"
#define IPV4_EXTERNAL {"0.0.0.0"}
#define IPV4_RESOLVER {"8.8.8.8", "8.8.4.4"}


#define IPV6_INTERNAL "::1"
#define IPV6_EXTERNAL {"::0"}
#define IPV6_RESOLVER {"2001:4860:4860::8888", "2001:4860:4860::8844"}


#define LDAP_ENABLED false
#define LDAP_VER 2
#define LDAP_SCOPE "sub"
#define LDAP_SERVER ""
#define LDAP_USERDN "ou=users,dc=example,dc=com"
#define LDAP_FILTER "(uid=%v)"
#define LDAP_BINDDN "cn=manager,dc=example,dc=com"
#define LDAP_BINDPW "manager"


#define BUFFER_WRITE_SIZE "auto"
#define BUFFER_READ_SIZE "auto"


#define TIMEOUTS_READ "5s"
#define TIMEOUTS_WRITE "15s"
#define TIMEOUTS_UPGRADE "300s"


#define LOGS_ENABLED false
#define LOGS_FILES true
#define LOGS_CONSOLE true
#define LOGS_DATA false
#define LOGS_SIZE "1MB"
#define LOGS_DIR "/var/log"


#define KEEPALIVE_CNT 3
#define KEEPALIVE_IDLE 1
#define KEEPALIVE_INTVL 2


#define AUTH_CLIENT_TYPE "basic"
#define AUTH_OS_USERS true
#define AUTH_FILE_USERS false
#define AUTH_ENABLED false


#define GZIP_VARY false
#define GZIP_LENGTH 100
#define GZIP_CHUNK "4KB"
#define GZIP_REGEX "msie6"
#define GZIP_VHTTP {"1.0", "1.1"}
#define GZIP_PROXIED {"private", "no-cache", "no-store"}
#define GZIP_TYPES {"text/html", "text/css", "text/plain", "text/xml", "text/javascript", "text/csv"}


#define FIREWALL_MAX_TRYAUTH 10
#define FIREWALL_AUTH false
#define FIREWALL_LOOP true
#define FIREWALL_BLACK_LIST false
#define FIREWALL_WHITE_LIST false
#define FIREWALL_BANDLIMIN false
#define FIREWALL_TIME_LOOP "60s"


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
			string auth;	
			bool osusers;	
			bool listusers;	
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
			u_int key;		
			u_int fds;		
			u_int connect;	
			int total;		
			size_t size;	
		};
		 
		struct Ldap {
			bool enabled;	
			u_int version;	
			string scope;	
			string server;	
			string userdn;	
			string filter;	
			string binddn;	
			string bindpw;	
		};
		 
		struct Proxy {
			u_int intIPv;		
			u_int extIPv;		
			u_int type;			
			u_int port;			
			u_int workers;		
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
