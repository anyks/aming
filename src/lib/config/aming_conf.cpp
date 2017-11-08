/* SERVER PROXY AMING */
/*
*  author:     Yuriy Lobarev
*  phone:      +7(910)983-95-90
*  telegram:   @forman
*  email:      info@anyks.com
*  date:       11/08/2017 16:52:48
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
		
		auto ipVx = Anyks::split(PROXY_IPV, "->");
		
		const u_int proxyIntIPv = ::atoi(ipVx[0].c_str());
		
		const u_int proxyExtIPv = ::atoi(ipVx[1].c_str());
		
		this->proxy = {
			
			AMING_TYPE_HTTP,
			
			AMING_NULL,
			
			proxyIntIPv,
			
			proxyExtIPv,
			
			PROXY_HTTP_PORT,
			
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
			
			AMING_AUTH_BASIC,
			
			AUSERS_TYPE_PAM,
			
			AUTH_GROUP_MAX_PAM,
			
			(time_t) Anyks::getSeconds(AUTH_UPDATE),
			
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
			
			AMING_IP,
			
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
			
			LDAP_VER,
			
			AMING_EMPTY,
			
			AMING_EMPTY,
			
			{
				AMING_EMPTY,
				AMING_EMPTY,
				AMING_EMPTY,
				AMING_EMPTY
			
			},{
				LDAP_SCOPE,
				LDAP_SCOPE,
				LDAP_SCOPE,
				LDAP_SCOPE
			
			},{
				AMING_EMPTY,
				AMING_EMPTY,
				AMING_EMPTY,
				AMING_EMPTY
			
			},{
				{
					LDAP_USER_UID,
					LDAP_USER_LOGIN,
					LDAP_USER_FIRST_NAME,
					LDAP_USER_LAST_NAME,
					LDAP_USER_SECOND_NAME,
					LDAP_USER_DESCRIPTION,
					LDAP_USER_PASSWORD
				},{
					LDAP_GROUP_GID,
					LDAP_GROUP_LOGIN,
					LDAP_GROUP_DESCRIPTION,
					LDAP_GROUP_PASSWORD,
					LDAP_GROUP_MEMBER
				}
			},			
			
			AMING_CLEAR
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
		
		u_short configProxy = AMING_NULL;
		
		auto configs = Anyks::split(ini.getString("proxy", "configs"), "|");
		
		for(auto it = configs.cbegin(); it != configs.cend(); ++it){
			
			const string config = Anyks::toCase(* it);
			
			if(config.compare("files") == 0) configProxy = (configProxy | AUSERS_TYPE_FILE);
			
			else if(config.compare("ldap") == 0) configProxy = (configProxy | AUSERS_TYPE_LDAP);
		}
		
		u_short authServices = AUSERS_TYPE_PAM;
		
		auto services = Anyks::split(ini.getString("authorization", "services"), "|");
		
		for(auto it = services.cbegin(); it != services.cend(); ++it){
			
			const string service = Anyks::toCase(* it);
			
			if(service.compare("files") == 0) authServices = (authServices | AUSERS_TYPE_FILE);
			
			else if(service.compare("pam") == 0) authServices = (authServices | AUSERS_TYPE_PAM);
			
			else if(service.compare("ldap") == 0) authServices = (authServices | AUSERS_TYPE_LDAP);
		}
		
		u_short authType = AMING_AUTH_BASIC;
		
		const string auth = Anyks::toCase(ini.getString("authorization", "auth"));
		
		if(auth.compare("basic") == 0) authType = AMING_AUTH_BASIC;
		else if(auth.compare("bearer") == 0) authType = AMING_AUTH_BEARER;
		else if(auth.compare("digest") == 0) authType = AMING_AUTH_DIGEST;
		else if(auth.compare("hmac") == 0) authType = AMING_AUTH_HMAC;
		else if(auth.compare("hoba") == 0) authType = AMING_AUTH_HOBA;
		else if(auth.compare("mutual") == 0) authType = AMING_AUTH_MUTUAL;
		else if(auth.compare("aws4-hmac-sha256") == 0) authType = AMING_AUTH_AWS4HMACSHA256;
		
		int glevel = Z_DEFAULT_COMPRESSION;
		
		const string gzipLevel = ini.getString("gzip", "level");
		
		if(!gzipLevel.empty()){
			
			if(gzipLevel.compare("default") == 0)		glevel = Z_DEFAULT_COMPRESSION;
			else if(gzipLevel.compare("best") == 0)		glevel = Z_BEST_COMPRESSION;
			else if(gzipLevel.compare("speed") == 0)	glevel = Z_BEST_SPEED;
			else if(gzipLevel.compare("no") == 0)		glevel = Z_NO_COMPRESSION;
		}
		
		const u_short connectKey = (Anyks::toCase(ini.getString("connects", "key")).compare("mac") == 0 ? AMING_MAC : AMING_IP);
		
		u_short proxyType = AMING_TYPE_HTTP;
		
		u_int proxyPort = PROXY_HTTP_PORT;
		
		const string type = ini.getString("proxy", "type", PROXY_TYPE);
		
		if(type.compare("http") == 0) proxyType = AMING_TYPE_HTTP;
		else if(type.compare("socks5") == 0) proxyType = AMING_TYPE_SOCKS;
		else if(type.compare("redirect") == 0) proxyType = AMING_TYPE_REDIRECT;
		
		switch(proxyType){
			
			case 1: proxyPort = PROXY_HTTP_PORT;		break;
			
			case 2: proxyPort = PROXY_SOCKS5_PORT;		break;
			
			case 3: proxyPort = PROXY_REDIRECT_PORT;	break;
		}
		
		bool proxySubnet = ini.getBoolean("proxy", "subnet", PROXY_SUBNET);
		
		auto ipVx = Anyks::split(ini.getString("proxy", "ipv", PROXY_IPV), "->");
		
		auto externalIPv4 = Anyks::split(ini.getString("ipv4", "external"), "|");
		auto externalIPv6 = Anyks::split(ini.getString("ipv6", "external"), "|");
		
		auto resolver4 = Anyks::split(ini.getString("ipv4", "resolver"), "|");
		auto resolver6 = Anyks::split(ini.getString("ipv6", "resolver"), "|");
		
		auto gproxied = Anyks::split(ini.getString("gzip", "proxied"), "|");
		
		auto gvhttp = Anyks::split(ini.getString("gzip", "vhttp"), "|");
		
		auto gtypes = Anyks::split(ini.getString("gzip", "types"), "|");
		
		u_int proxyIntIPv = ::atoi(ipVx[0].c_str());
		u_int proxyExtIPv = ::atoi(ipVx[1].c_str());
		
		if(externalIPv4.empty()) externalIPv4 = IPV4_EXTERNAL;
		if(externalIPv6.empty()) externalIPv6 = IPV6_EXTERNAL;
		
		if(resolver4.empty()) resolver4 = IPV4_RESOLVER;
		if(resolver6.empty()) resolver6 = IPV6_RESOLVER;
		
		if(gproxied.empty()) gproxied = GZIP_PROXIED;
		
		if(gvhttp.empty()) gvhttp = GZIP_VHTTP;
		
		if(gtypes.empty()) gtypes = GZIP_TYPES;
		
		proxyIntIPv = ((proxyIntIPv < 4) || (proxyIntIPv > 6) ? 4 : proxyIntIPv);
		proxyExtIPv = ((proxyExtIPv < 4) || (proxyExtIPv > 6) ? 4 : proxyExtIPv);
		
		if((proxyIntIPv != 6) && (proxyExtIPv != 6)) proxySubnet = false;
		
		this->proxy = {
			
			proxyType,
			
			configProxy,
			
			proxyIntIPv,
			
			proxyExtIPv,
			
			(u_int) ini.getUNumber("proxy", "port", proxyPort),
			
			(u_int) ini.getUNumber("proxy", "workers", MAX_WORKERS),
			
			ini.getBoolean("proxy", "debug", PROXY_DEBUG),
			
			ini.getBoolean("proxy", "daemon", PROXY_DAEMON),
			
			ini.getBoolean("proxy", "reverse", PROXY_REVERSE),
			
			ini.getBoolean("proxy", "transfer", PROXY_TRANSFER),
			
			ini.getBoolean("proxy", "forward", PROXY_FORWARD),
			
			ini.getBoolean("proxy", "ipv6only", PROXY_IPV6ONLY),
			
			proxySubnet,
			
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
			
			authType,
			
			authServices,
			
			(u_int) ini.getUNumber("authorization", "gmaxpam", AUTH_GROUP_MAX_PAM),
			
			(time_t) Anyks::getSeconds(ini.getString("authorization", "update", AUTH_UPDATE)),
			
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
			
			connectKey,
			
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
			
			(u_int) ini.getUNumber("ldap", "version", LDAP_VER),
			
			ini.getString("ldap", "binddn"),
			
			ini.getString("ldap", "bindpw"),
			
			{
				ini.getString("ldap", "gdn"),
				ini.getString("ldap", "udn"),
				ini.getString("ldap", "cdn"),
				ini.getString("ldap", "hdn")
			
			},{
				ini.getString("ldap", "gscope", LDAP_SCOPE),
				ini.getString("ldap", "uscope", LDAP_SCOPE),
				ini.getString("ldap", "cscope", LDAP_SCOPE),
				ini.getString("ldap", "hscope", LDAP_SCOPE)
			
			},{
				ini.getString("ldap", "gfilter"),
				ini.getString("ldap", "ufilter"),
				ini.getString("ldap", "cfilter"),
				ini.getString("ldap", "hfilter")
			
			},{
				{
					ini.getString("ldap", "kuid", LDAP_USER_UID),
					ini.getString("ldap", "kulogin", LDAP_USER_LOGIN),
					ini.getString("ldap", "kufname", LDAP_USER_FIRST_NAME),
					ini.getString("ldap", "kulname", LDAP_USER_LAST_NAME),
					ini.getString("ldap", "kusname", LDAP_USER_SECOND_NAME),
					ini.getString("ldap", "kudesc", LDAP_USER_DESCRIPTION),
					ini.getString("ldap", "kupass", LDAP_USER_PASSWORD)
				},{
					ini.getString("ldap", "kgid", LDAP_GROUP_GID),
					ini.getString("ldap", "kglogin", LDAP_GROUP_LOGIN),
					ini.getString("ldap", "kgdesc", LDAP_GROUP_DESCRIPTION),
					ini.getString("ldap", "kgpass", LDAP_GROUP_PASSWORD),
					ini.getString("ldap", "kgmember", LDAP_GROUP_MEMBER)
				}
			},			
			
			Anyks::split(ini.getString("ldap", "server"), "|")
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
