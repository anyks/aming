/* SERVER PROXY AMING */
/*
*  author:     Yuriy Lobarev
*  phone:      +7(910)983-95-90
*  telegram:   @forman
*  email:      info@anyks.com
*  date:       10/29/2017 16:59:43
*  copyright:  © 2017 anyks.com
*/
 
 

#include "config/conf.h"


using namespace std;

 
Config::Config(const string filename){
	
	this->filename = filename;
	
	if(this->filename.empty() || !Anyks::isFileExist(this->filename.c_str())){
		
		this->filename = (string(CONFIG_DIR) + string("/") + string(PROXY_NAME) + string("/config.ini"));
	}
	
	INI ini(this->filename);
	
	if(ini.isError()){
		
		printf("Can't load config file %s\n", this->filename.c_str());
		
		this->options = (OPT_CONNECT | OPT_AGENT | OPT_GZIP | OPT_KEEPALIVE | OPT_LOG | OPT_PGZIP);
		
		string type = PROXY_TYPE;
		
		u_int connect_key = (string(CONNECTS_KEY).compare("mac") == 0 ? 1 : 0);
		
		u_int proxy_type, proxy_port;
		
		auto ipVx = Anyks::split(PROXY_IPV, "->");
		
		const u_int proxy_intIPv = ::atoi(ipVx[0].c_str());
		
		const u_int proxy_extIPv = ::atoi(ipVx[1].c_str());
		
		if(type.compare("http") == 0) proxy_type = 1;
		else if(type.compare("socks5") == 0) proxy_type = 2;
		else if(type.compare("redirect") == 0) proxy_type = 3;
		else proxy_type = 1;
		
		switch(proxy_type){
			
			case 1: proxy_port = PROXY_HTTP_PORT;		break;
			
			case 2: proxy_port = PROXY_SOCKS5_PORT;		break;
			
			case 3: proxy_port = PROXY_REDIRECT_PORT;	break;
		}
		
		this->proxy = {
			
			proxy_intIPv,
			
			proxy_extIPv,
			
			proxy_type,
			
			proxy_port,
			
			MAX_WORKERS,
			
			PROXY_DEBUG,
			
			PROXY_DAEMON,
			
			PROXY_REVERSE,
			
			PROXY_TRANSFER,
			
			PROXY_FORWARD,
			
			PROXY_IPV6ONLY,
			
			PROXY_SUBNET,
			
			PROXY_OPTIMOS,
			
			PROXY_PIPELINING,
			
			PROXY_USER,
			
			PROXY_GROUP,
			
			PROXY_NAME,
			
			PID_DIR,
			
			CONFIG_DIR
		};
		
		this->cache = {
			
			CACHE_DNS,
			
			CACHE_RESPONSE,
			
			(time_t) Anyks::getSeconds(CACHE_DTTL),
			
			CACHE_DIR
		};
		
		this->gzip = {
			
			GZIP_VARY,
			
			Z_DEFAULT_COMPRESSION,
			
			GZIP_LENGTH,
			
			Anyks::getBytes(GZIP_CHUNK),
			
			GZIP_REGEX,
			
			GZIP_VHTTP,
			
			GZIP_PROXIED,
			
			GZIP_TYPES
		};
		
		this->firewall = {
			
			FIREWALL_MAX_TRYAUTH,
			
			FIREWALL_AUTH,
			
			FIREWALL_LOOP,
			
			FIREWALL_BLACK_LIST,
			
			FIREWALL_WHITE_LIST,
			
			FIREWALL_BANDLIMIN,
			
			FIREWALL_TIME_LOOP
		};
		
		this->auth = {
			
			AUTH_CLIENT_TYPE,
			
			AUTH_OS_USERS,
			
			AUTH_FILE_USERS,
			
			AUTH_ENABLED
		};
		
		this->logs = {
			
			LOGS_FILES,
			
			LOGS_CONSOLE,
			
			LOGS_DATA,
			
			LOGS_ENABLED,
			
			Anyks::getBytes(LOGS_SIZE),
			
			LOGS_DIR
		};
		
		this->connects = {
			
			connect_key,
			
			CONNECTS_FDS,
			
			CONNECTS_CONNECT,
			
			CONNECTS_TOTAL,
			
			Anyks::getBytes(CONNECTS_SIZE)
		};
		
		this->ipv4 = {
			
			IPV4_INTERNAL,
			
			IPV4_EXTERNAL,
			
			IPV4_RESOLVER
		};
		
		this->ipv6 = {
			
			IPV6_INTERNAL,
			
			IPV6_EXTERNAL,
			
			IPV6_RESOLVER
		};
		
		this->ldap = {
			
			LDAP_ENABLED,
			
			LDAP_VER,
			
			LDAP_SCOPE,
			
			LDAP_SERVER,
			
			LDAP_USERDN,
			
			LDAP_FILTER,
			
			LDAP_BINDDN,
			
			LDAP_BINDPW
		};
		
		this->timeouts = {
			
			(size_t) Anyks::getSeconds(TIMEOUTS_READ),
			
			(size_t) Anyks::getSeconds(TIMEOUTS_WRITE),
			
			(size_t) Anyks::getSeconds(TIMEOUTS_UPGRADE)
		};
		
		this->buffers = {
			
			Anyks::getSizeBuffer(BUFFER_READ_SIZE),
			
			Anyks::getSizeBuffer(BUFFER_WRITE_SIZE)
		};
		
		this->keepalive = {
			
			KEEPALIVE_CNT,
			
			KEEPALIVE_IDLE,
			
			KEEPALIVE_INTVL
		};
	
	} else {
		
		this->options = (ini.getBoolean("proxy", "connect", true) ? OPT_CONNECT : OPT_NULL);
		
		this->options = (this->options | (ini.getBoolean("proxy", "upgrade", false) ? OPT_UPGRADE : OPT_NULL));
		
		this->options = (this->options | (ini.getBoolean("proxy", "agent", true) ? OPT_AGENT : OPT_NULL));
		
		this->options = (this->options | (ini.getBoolean("proxy", "deblock", false) ? OPT_DEBLOCK : OPT_NULL));
		
		this->options = (this->options | (ini.getBoolean("gzip", "transfer", true) ? OPT_GZIP : OPT_NULL));
		
		this->options = (this->options | (ini.getBoolean("gzip", "response", true) ? OPT_PGZIP : OPT_NULL));
		
		this->options = (this->options | ((ini.getString("proxy", "skill", "dumb").compare("smart") == 0) ? OPT_SMART : OPT_NULL));
		
		this->options = (this->options | (ini.getBoolean("keepalive", "enabled", true) ? OPT_KEEPALIVE : OPT_NULL));
		
		this->options = (this->options | (ini.getBoolean("logs", "enabled", true) ? OPT_LOG : OPT_NULL));
		
		int glevel = Z_DEFAULT_COMPRESSION;
		
		const string gzipLevel = ini.getString("gzip", "level");
		
		if(!gzipLevel.empty()){
			
			if(gzipLevel.compare("default") == 0)		glevel = Z_DEFAULT_COMPRESSION;
			else if(gzipLevel.compare("best") == 0)		glevel = Z_BEST_COMPRESSION;
			else if(gzipLevel.compare("speed") == 0)	glevel = Z_BEST_SPEED;
			else if(gzipLevel.compare("no") == 0)		glevel = Z_NO_COMPRESSION;
		}
		
		u_int connect_key = (Anyks::toCase(ini.getString("connects", "key", CONNECTS_KEY)).compare("mac") == 0 ? 1 : 0);
		
		u_int proxy_type, proxy_port;
		
		const string type = ini.getString("proxy", "type", PROXY_TYPE);
		
		if(type.compare("http") == 0) proxy_type = 1;
		else if(type.compare("socks5") == 0) proxy_type = 2;
		else if(type.compare("redirect") == 0) proxy_type = 3;
		else proxy_type = 1;
		
		switch(proxy_type){
			
			case 1: proxy_port = PROXY_HTTP_PORT;		break;
			
			case 2: proxy_port = PROXY_SOCKS5_PORT;		break;
			
			case 3: proxy_port = PROXY_REDIRECT_PORT;	break;
		}
		
		bool proxy_subnet = ini.getBoolean("proxy", "subnet", PROXY_SUBNET);
		
		auto ipVx = Anyks::split(ini.getString("proxy", "ipv", PROXY_IPV), "->");
		
		auto externalIPv4 = Anyks::split(ini.getString("ipv4", "external"), "|");
		auto externalIPv6 = Anyks::split(ini.getString("ipv6", "external"), "|");
		
		auto resolver4 = Anyks::split(ini.getString("ipv4", "resolver"), "|");
		auto resolver6 = Anyks::split(ini.getString("ipv6", "resolver"), "|");
		
		auto gproxied = Anyks::split(ini.getString("gzip", "proxied"), "|");
		
		auto gvhttp = Anyks::split(ini.getString("gzip", "vhttp"), "|");
		
		auto gtypes = Anyks::split(ini.getString("gzip", "types"), "|");
		
		u_int proxy_intIPv = ::atoi(ipVx[0].c_str());
		u_int proxy_extIPv = ::atoi(ipVx[1].c_str());
		
		if(externalIPv4.empty()) externalIPv4 = IPV4_EXTERNAL;
		if(externalIPv6.empty()) externalIPv6 = IPV6_EXTERNAL;
		
		if(resolver4.empty()) resolver4 = IPV4_RESOLVER;
		if(resolver6.empty()) resolver6 = IPV6_RESOLVER;
		
		if(gproxied.empty()) gproxied = GZIP_PROXIED;
		
		if(gvhttp.empty()) gvhttp = GZIP_VHTTP;
		
		if(gtypes.empty()) gtypes = GZIP_TYPES;
		
		proxy_intIPv = ((proxy_intIPv < 4) || (proxy_intIPv > 6) ? 4 : proxy_intIPv);
		proxy_extIPv = ((proxy_extIPv < 4) || (proxy_extIPv > 6) ? 4 : proxy_extIPv);
		
		if((proxy_intIPv != 6) && (proxy_extIPv != 6)) proxy_subnet = false;
		
		this->proxy = {
			
			proxy_intIPv,
			
			proxy_extIPv,
			
			proxy_type,
			
			(u_int) ini.getUNumber("proxy", "port", proxy_port),
			
			(u_int) ini.getUNumber("proxy", "workers", MAX_WORKERS),
			
			ini.getBoolean("proxy", "debug", PROXY_DEBUG),
			
			ini.getBoolean("proxy", "daemon", PROXY_DAEMON),
			
			ini.getBoolean("proxy", "reverse", PROXY_REVERSE),
			
			ini.getBoolean("proxy", "transfer", PROXY_TRANSFER),
			
			ini.getBoolean("proxy", "forward", PROXY_FORWARD),
			
			ini.getBoolean("proxy", "ipv6only", PROXY_IPV6ONLY),
			
			proxy_subnet,
			
			ini.getBoolean("proxy", "optimos", PROXY_OPTIMOS),
			
			ini.getBoolean("proxy", "pipelining", PROXY_PIPELINING),
			
			ini.getString("proxy", "user", PROXY_USER),
			
			ini.getString("proxy", "group", PROXY_GROUP),
			
			ini.getString("proxy", "name", PROXY_NAME),
			
			ini.getString("proxy", "piddir", PID_DIR),
			
			ini.getString("proxy", "dir", CONFIG_DIR)
		};
		
		this->gzip = {
			
			ini.getBoolean("gzip", "vary", GZIP_VARY),
			
			glevel,
			
			ini.getNumber("gzip", "length", GZIP_LENGTH),
			
			Anyks::getBytes(ini.getString("gzip", "chunk", GZIP_CHUNK)),
			
			ini.getString("gzip", "regex", GZIP_REGEX),
			
			gvhttp,
			
			gproxied,
			
			gtypes
		};
		
		this->cache = {
			
			ini.getBoolean("cache", "dns", CACHE_DNS),
			
			ini.getBoolean("cache", "dat", CACHE_RESPONSE),
			
			(time_t) Anyks::getSeconds(ini.getString("cache", "dttl", CACHE_DTTL)),
			
			ini.getString("cache", "dir", CACHE_DIR)
		};
		
		this->firewall = {
			
			(u_int) ini.getUNumber("firewall", "maxtryauth", FIREWALL_MAX_TRYAUTH),
			
			ini.getBoolean("firewall", "blockauth", FIREWALL_AUTH),
			
			ini.getBoolean("firewall", "blockloop", FIREWALL_LOOP),
			
			ini.getBoolean("firewall", "blacklist", FIREWALL_BLACK_LIST),
			
			ini.getBoolean("firewall", "whitelist", FIREWALL_WHITE_LIST),
			
			ini.getBoolean("firewall", "bandlimin", FIREWALL_BANDLIMIN),
			
			ini.getString("firewall", "timeblockloop", FIREWALL_TIME_LOOP)
		};
		
		this->auth = {
			
			Anyks::toCase(ini.getString("authorization", "auth", AUTH_CLIENT_TYPE)),
			
			ini.getBoolean("authorization", "osusers", AUTH_OS_USERS),
			
			ini.getBoolean("authorization", "listusers", AUTH_FILE_USERS),
			
			ini.getBoolean("authorization", "enabled", AUTH_ENABLED)
		};
		
		this->logs = {
			
			ini.getBoolean("logs", "files", LOGS_FILES),
			
			ini.getBoolean("logs", "console", LOGS_CONSOLE),
			
			ini.getBoolean("logs", "data", LOGS_DATA),
			
			ini.getBoolean("logs", "enabled", LOGS_ENABLED),
			
			Anyks::getBytes(ini.getString("logs", "size", LOGS_SIZE)),
			
			ini.getString("logs", "dir", LOGS_DIR)
		};
		
		this->connects = {
			
			connect_key,
			
			(u_int) ini.getUNumber("connects", "fds", CONNECTS_FDS),
			
			(u_int) ini.getUNumber("connects", "connect", CONNECTS_CONNECT),
			
			(int) ini.getNumber("connects", "total", CONNECTS_TOTAL),
			
			Anyks::getBytes(ini.getString("connects", "size", CONNECTS_SIZE))
		};
		
		this->ipv4 = {
			
			ini.getString("ipv4", "internal", IPV4_INTERNAL),
			
			externalIPv4,
			
			resolver4
		};
		
		this->ipv6 = {
			
			ini.getString("ipv6", "internal", IPV6_INTERNAL),
			
			externalIPv6,
			
			resolver6
		};
		
		this->ldap = {
			
			ini.getBoolean("authorization", "ldap", LDAP_ENABLED),
			
			(u_int) ini.getUNumber("ldap", "version", LDAP_VER),
			
			ini.getString("ldap", "scope", LDAP_SCOPE),
			
			ini.getString("ldap", "server", LDAP_SERVER),
			
			ini.getString("ldap", "userdn", LDAP_USERDN),
			
			ini.getString("ldap", "filter", LDAP_FILTER),
			
			ini.getString("ldap", "binddn", LDAP_BINDDN),
			
			ini.getString("ldap", "bindpw", LDAP_BINDPW)
		};
		
		this->timeouts = {
			
			(size_t) Anyks::getSeconds(ini.getString("timeouts", "read", TIMEOUTS_READ)),
			
			(size_t) Anyks::getSeconds(ini.getString("timeouts", "write", TIMEOUTS_WRITE)),
			
			(size_t) Anyks::getSeconds(ini.getString("timeouts", "upgrade", TIMEOUTS_UPGRADE))
		};
		
		this->buffers = {
			
			Anyks::getSizeBuffer(ini.getString("speed", "input", BUFFER_READ_SIZE)),
			
			Anyks::getSizeBuffer(ini.getString("speed", "output", BUFFER_WRITE_SIZE))
		};
		
		this->keepalive = {
			
			(int) ini.getNumber("keepalive", "keepcnt", KEEPALIVE_CNT),
			
			(int) ini.getNumber("keepalive", "keepidle", KEEPALIVE_IDLE),
			
			(int) ini.getNumber("keepalive", "keepintvl", KEEPALIVE_INTVL)
		};
	}
}
