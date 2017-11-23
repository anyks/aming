/* SERVER PROXY AMING */
/*
*  author:     Yuriy Lobarev
*  phone:      +7(910)983-95-90
*  telegram:   @forman
*  email:      info@anyks.com
*  date:       11/23/2017 17:50:05
*  copyright:  © 2017 anyks.com
*/
 
 

#include "log/log.h"


using namespace std;

 
void LogApp::write_data_to_file(const string id, const string data, void * ctx){
	
	LogApp * log = reinterpret_cast <LogApp *> (ctx);
	
	if(log && log->dataEnabled){
		
		string path = Anyks::addToPath((* log->config)->logs.dir, (* log->config)->proxy.name);
		
		path = Anyks::addToPath(path, "data");
		
		if(!Anyks::makePath(path.c_str(), (* log->config)->proxy.user, (* log->config)->proxy.group)){
			
			perror("Unable to create directory for log files");
			
			exit(EXIT_FAILURE);
		}
		
		char datefile[80], date[80];
		
		time_t seconds = time(nullptr);
		
		struct tm * timeinfo = localtime(&seconds);
		
		string fileformat = "_%m-%d-%Y";
		
		string dateformat = "%m/%d/%Y %H:%M:%S";
		
		int filelen = strftime(datefile, sizeof(datefile), fileformat.c_str(), timeinfo);
		
		int datelen = strftime(date, sizeof(date), dateformat.c_str(), timeinfo);
		
		datefile[filelen] = '\0';
		
		date[datelen] = '\0';
		
		string zdate(date, datelen);
		
		zdate = string("Log date: ") + date;
		
		string filename = string(id + datefile) + ".log.gz";
		
		gzFile gz = gzopen(string(path + string("/") + filename).c_str(), "ab9f");
		
		gzwrite(gz, "*************** START ***************\r\n\r\n", 41);
		
		gzwrite(gz, zdate.c_str(), zdate.size());
		
		gzwrite(gz, "\r\n\r\n", 4);
		
		gzwrite(gz, data.data(), data.size());
		
		gzwrite(gz, "\r\n\r\n---------------- END ----------------\r\n\r\n", 45);
		
		gzclose(gz);
	}
}
 
void LogApp::write_to_file(u_short type, const char * message, void * ctx){
	
	LogApp * log = reinterpret_cast <LogApp *> (ctx);
	
	if(log){
		
		string path = Anyks::addToPath((* log->config)->logs.dir, (* log->config)->proxy.name);
		
		if(!Anyks::makePath(path.c_str(), (* log->config)->proxy.user, (* log->config)->proxy.group)){
			
			perror("Unable to create directory for log files");
			
			exit(EXIT_FAILURE);
		}
		
		string filename;
		
		switch(type){
			
			case 1: filename = (path + string("/") + "error.log"); break;
			
			case 2: filename = (path + string("/") + "access.log"); break;
			
			case 3: filename = (path + string("/") + "warning.log"); break;
			
			case 4: filename = (path + string("/") + "message.log"); break;
		}
		
		FILE * file = nullptr;
		
		if(Anyks::isFileExist(filename.c_str())){
			
			Anyks::setOwner(filename.c_str(), (* log->config)->proxy.user, (* log->config)->proxy.group);
			
			file = fopen(filename.c_str(), "rb");
			
			if(file){
				
				fseek(file, 0, SEEK_END);
				
				size_t size = ftell(file);
				
				fclose(file);
				
				if(size >= log->size){
					
					char datefile[80];
					
					time_t seconds = time(nullptr);
					
					struct tm * timeinfo = localtime(&seconds);
					
					string dateformat = "_%m-%d-%Y_%H-%M-%S";
					
					int datelen = strftime(datefile, sizeof(datefile), dateformat.c_str(), timeinfo);
					
					datefile[datelen] = '\0';
					
					ifstream logfile(filename.c_str());
					
					if(logfile.is_open()){
						
						string filedata;
						
						string gzlogfile = filename;
						
						gzlogfile.replace(gzlogfile.length() - 4, 4, string(datefile) + ".log.gz");
						
						gzFile gz = gzopen(gzlogfile.c_str(), "wb9h");
						
						while(logfile.good()){
							
							getline(logfile, filedata);
							
							filedata += "\r\n";
							
							gzwrite(gz, filedata.c_str(), filedata.size());
						}
						
						gzclose(gz);
						
						logfile.close();
					}
					
					remove(filename.c_str());
				}
			
			} else write_to_console(LOG_ERROR, (string("cannot read log file, ") + filename).c_str(), log);
		}
		
		file = fopen(filename.c_str(), "a");
		
		if(file){
			
			fprintf(file, "%s\n", message);
			
			fclose(file);
		}
	}
}
 
void LogApp::write_to_console(u_short type, const char * message, void * ctx, u_int sec){
	
	string str;
	
	switch(type){
		
		case 1: str = "\x1B[31m\x1B[1mError\x1B[0m "; break;
		
		case 2: str = "\x1B[32m\x1B[1mAccess\x1B[0m "; break;
		
		case 3: str = "\x1B[33m\x1B[1mWarning\x1B[0m "; break;
		
		case 4: str = "\x1B[34m\x1B[1mMessage\x1B[0m "; break;
	}
	
	str += message;
	
	if(sec) this_thread::sleep_for(chrono::milliseconds(sec));
	
	printf("\r\n%s\r\n\r\n%s\r\n\r\n%s\r\n", "*************** START ***************", str.c_str(), "---------------- END ----------------");
}
 
void LogApp::enable(){
	
	this->enabled = true;
}
 
void LogApp::disable(){
	
	this->enabled = false;
}
 
void LogApp::write_data(const string id, const string data){
	
	if(this->enabled && this->dataEnabled){
		
		async(launch::async, &LogApp::write_data_to_file, id, data, this);
	}
}
 
void LogApp::write(u_short type, u_int sec, const char * message, ...){
	
	if(this->enabled){
		
		char buffer[1024 * 16];
		
		time_t seconds = time(nullptr);
		
		struct tm * timeinfo = localtime(&seconds);
		
		char * uk = asctime(timeinfo);
		
		*(uk + 24) = 0;
		
		string str = ((* this->config)->proxy.name + string(" [") + string(uk) + string("]: "));
		
		int len = sprintf(buffer, "%s", str.c_str());
		
		va_list args;
		
		va_start(args, message);
		
		if((len = vsnprintf(buffer + len, sizeof(buffer), message, args)) > 0){
			
			buffer[strlen(buffer)] = '\0';
			
			if(this->filesEnabled && (this->type & TOLOG_FILES))		async(launch::async, &LogApp::write_to_file, type, buffer, this);
			if(this->consoleEnabled && (this->type & TOLOG_CONSOLE))	async(launch::async, &LogApp::write_to_console, type, buffer, this, sec);
		}
		
		va_end(args);
	}
}
 
void LogApp::welcome(){
	
	auto * config = (* this->config);
	
	if(this->enabled && (config != nullptr)){
		
		string appname = APP_NAME, apptype, appskill;
		
		string internal, external;
		
		string copyright = APP_COPYRIGHT;
		
		string authtype = "none", authservices = "none", proxyconfigs = "none";
		
		if(OPT_SMART & config->options)
			appskill = "smart";
		else appskill = "dumb";
		
		switch(config->proxy.intIPv){
			
			case 4: {
				internal = config->ipv4.internal;
				external = config->ipv4.external[0];
			} break;
			
			case 6: {
				internal = config->ipv6.internal;
				external = config->ipv6.external[0];
			} break;
		}
		
		switch(config->proxy.type){
			case AMING_TYPE_HTTP:				apptype = "http";				break;
			case AMING_TYPE_SOCKS5:				apptype = "socks5";				break;
			case AMING_TYPE_HTTP_REDIRECT:		apptype = "http -> redirect";	break;
			case AMING_TYPE_SOCKS5_REDIRECT:	apptype = "socks5 -> redirect";	break;
		}
		
		switch(config->auth.auth){
			case AMING_AUTH_BASIC:			authtype = "basic";				break;
			case AMING_AUTH_BEARER:			authtype = "bearer";			break;
			case AMING_AUTH_DIGEST:			authtype = "digest";			break;
			case AMING_AUTH_HMAC:			authtype = "hmac";				break;
			case AMING_AUTH_HOBA:			authtype = "hoba";				break;
			case AMING_AUTH_MUTUAL:			authtype = "mutual";			break;
			case AMING_AUTH_AWS4HMACSHA256:	authtype = "aws4-hmac-sha256";	break;
		}
		
		switch(config->auth.services){
			case AUSERS_TYPE_FILE:			authservices = "files";					break;
			case AUSERS_TYPE_PAM:			authservices = "pam";					break;
			case AUSERS_TYPE_LDAP:			authservices = "ldap";					break;
			case AUSERS_TYPE_FILE_PAM:		authservices = "files | pam";			break;
			case AUSERS_TYPE_FILE_LDAP:		authservices = "files | ldap";			break;
			case AUSERS_TYPE_PAM_LDAP:		authservices = "pam | ldap";			break;
			case AUSERS_TYPE_FILE_PAM_LDAP:	authservices = "files | pam | ldap";	break;
		}
		
		switch(config->proxy.configs){
			case AUSERS_TYPE_FILE:		proxyconfigs = "files";			break;
			case AUSERS_TYPE_LDAP:		proxyconfigs = "ldap";			break;
			case AUSERS_TYPE_FILE_LDAP:	proxyconfigs = "files | ldap";	break;
		}
		
		char year[5], date[80];
		
		time_t seconds = time(nullptr);
		
		struct tm * timeinfo = localtime(&seconds);
		
		string yearformat = "%Y";
		
		string dateformat = "%m/%d/%Y %H:%M:%S";
		
		int len = strftime(year, sizeof(year), yearformat.c_str(), timeinfo);
		
		year[len] = '\0';
		
		len = strftime(date, sizeof(date), dateformat.c_str(), timeinfo);
		
		date[len] = '\0';
		
		string cpyear = (::atoi(APP_YEAR) < ::atoi(year) ? string(APP_YEAR) + string(" - ") + year : string("- ") + year);
		
		copyright = (copyright + string(" ") + cpyear);
		
		transform(appname.begin(), appname.end(), appname.begin(), ::toupper);
		transform(copyright.begin(), copyright.end(), copyright.begin(), ::toupper);
		
		const string total = (config->connects.total > 0 ? to_string(config->connects.total) : "auto");
		
		const char * _gzipt = (OPT_GZIP & config->options ? "yes" : "no");
		const char * _gzipr = (OPT_PGZIP & config->options ? "yes" : "no");
		const char * _keepalive = (OPT_KEEPALIVE & config->options ? "yes" : "no");
		const char * _connect = (OPT_CONNECT & config->options ? "yes" : "no");
		const char * _hideagent = (OPT_AGENT & config->options ? "yes" : "no");
		const char * _deblock = (OPT_DEBLOCK & config->options ? "yes" : "no");
		const char * _upgrade = (OPT_UPGRADE & config->options ? "yes" : "no");
		const char * _pipelining = (config->proxy.pipelining ? "yes" : "no");
		const char * _ipv6only = (config->proxy.ipv6only ? "yes" : "no");
		const char * _subnet = (config->proxy.subnet ? "yes" : "no");
		const char * _debug = (config->proxy.debug ? "yes" : "no");
		const char * _daemon = (config->proxy.daemon ? "yes" : "no");
		const char * _reverse = (config->proxy.reverse ? "yes" : "no");
		const char * _forward = (config->proxy.forward ? "yes" : "no");
		const char * _transfer = (config->proxy.transfer ? "yes" : "no");
		const char * _optimos = (config->proxy.optimos ? "yes" : "no");
		const char * _bandlimin = (config->firewall.bandlimin ? "yes" : "no");
		const char * _cache = (config->cache.dat ? "yes" : "no");
		const char * _auth = (config->auth.enabled ? "yes" : "no");
		const char * _authtype = authtype.c_str();
		const char * _authservices = authservices.c_str();
		const char * _proxyconfigs = proxyconfigs.c_str();
		const char * _totalcon =  total.c_str();
		const char * _appname = appname.c_str();
		const char * _appver = APP_VERSION;
		const char * _apptype = apptype.c_str();
		const char * _appskill = appskill.c_str();
		const char * _proxyname = config->proxy.name.c_str();
		const char * _proxyuser = config->proxy.user.c_str();
		const char * _proxygroup = config->proxy.group.c_str();
		const char * _proxyinternal = internal.c_str();
		const char * _proxyexternal = external.c_str();
		const char * _systemname = config->os.name.c_str();
		const char * _systemcpuname = config->os.cpu.c_str();
		const char * _infocopyright = copyright.c_str();
		const char * _infosite = APP_SITE;
		const char * _infoemail = APP_EMAIL;
		const char * _infosupport = APP_SUPPORT;
		const char * _infoauthor = APP_AUTHOR;
		const u_int _proxyconnect = config->connects.connect;
		const u_int _proxyintipv = config->proxy.intIPv;
		const u_int _proxyextipv = config->proxy.extIPv;
		const u_int _proxyport = config->proxy.port;
		const u_int _systemcpu = config->os.ncpu;
		
		char buffer[1024 * 16];
		
		const char * format = "\r\n*\r\n"
		"*   WELCOME TO %s PROXY\r\n*\r\n"
		"*   Information:\r\n"
		"*   name:                 %s\r\n"
		"*   version:              %s\r\n"
		"*   type:                 %s\r\n"
		"*   user:                 %s\r\n"
		"*   group:                %s\r\n"
		"*   skill:                %s\r\n"
		"*   all connects:         %s\r\n"
		"*   max connect:          %u\r\n"
		"*   internet protocol:    IPv%u -> IPv%u\r\n"
		"*   internal ip:          %s\r\n"
		"*   external ip:          %s\r\n"
		"*   port:                 %u\r\n"
		"*\r\n"
		"*   Parameters proxy:\r\n"
		"*   daemon:               %s\r\n"
		"*   debug:                %s\r\n"
		"*   cache:                %s\r\n"
		"*   hide agent:           %s\r\n"
		"*   bandlimin:            %s\r\n"
		"*   deblock:              %s\r\n"
		"*   optimos:              %s\r\n"
		"*   reverse:              %s\r\n"
		"*   forward:              %s\r\n"
		"*   transfer:             %s\r\n"
		"*   keep-alive:           %s\r\n"
		"*   pipelining:           %s\r\n"
		"*   connect:              %s\r\n"
		"*   upgrade:              %s\r\n"
		"*   subnet:               %s\r\n"
		"*   gzip transfer:        %s\r\n"
		"*   gzip response:        %s\r\n"
		"*   ipv6 only:            %s\r\n"
		"*   auth:                 %s\r\n"
		"*   auth type:            %s\r\n"
		"*   auth serivces:        %s\r\n"
		"*   configs:              %s\r\n"
		"*\r\n"
		"*   Server information:\r\n"
		"*   cpu cores:            %u\r\n"
		"*   cpu name:             %s\r\n"
		"*   operating system:     %s\r\n"
		"*   date start proxy:     %s\r\n"
		"*\r\n"
		"*   Contact Developer:\r\n"
		"*   copyright:            %s\r\n"
		"*   site:                 %s\r\n"
		"*   e-mail:               %s\r\n"
		"*   support:              %s\r\n"
		"*   author:               %s\r\n"
		"*\r\n";
		
		len = sprintf(
			buffer, format, _appname,
			_proxyname, _appver, _apptype,
			_proxyuser, _proxygroup, _appskill,
			_totalcon, _proxyconnect, _proxyintipv,
			_proxyextipv, _proxyinternal,
			_proxyexternal, _proxyport,
			_daemon, _debug, _cache, _hideagent,
			_bandlimin, _deblock, _optimos,
			_reverse, _forward, _transfer,
			_keepalive, _pipelining, _connect,
			_upgrade, _subnet, _gzipt, _gzipr,
			_ipv6only, _auth, _authtype,
			_authservices, _proxyconfigs, _systemcpu,
			_systemcpuname, _systemname, date,
			_infocopyright, _infosite, _infoemail,
			_infosupport, _infoauthor
		);
		
		printf("%s\r\n", buffer);
		
		write_to_file(LOG_MESSAGE, buffer, this);
	}
}
 
LogApp::LogApp(Config ** config, u_short type){
	
	if((config != nullptr) && ((* config) != nullptr)){
		
		this->config = config;
		
		this->type = type;
		
		this->enabled = (* this->config)->logs.enabled;
		
		this->dataEnabled = (* this->config)->logs.data;
		
		this->filesEnabled = (* this->config)->logs.files;
		
		this->consoleEnabled = (* this->config)->logs.console;
		
		this->size = ((* this->config)->logs.size <= 104857600 ? (* this->config)->logs.size : 104857600);
	}
}