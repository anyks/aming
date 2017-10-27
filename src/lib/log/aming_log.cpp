/* SERVER PROXY AMING */
/*
*  author:     Yuriy Lobarev
*  phone:      +7(910)983-95-90
*  telegram:   @forman
*  email:      info@anyks.com
*  date:       10/27/2017 18:42:35
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
	
	if(this->enabled){
		
		const char * _gzipt = (OPT_GZIP & (* this->config)->options ? "yes" : "no");
		const char * _gzipr = (OPT_PGZIP & (* this->config)->options ? "yes" : "no");
		const char * _keepalive = (OPT_KEEPALIVE & (* this->config)->options ? "yes" : "no");
		const char * _connect = (OPT_CONNECT & (* this->config)->options ? "yes" : "no");
		const char * _hideagent = (OPT_AGENT & (* this->config)->options ? "yes" : "no");
		const char * _deblock = (OPT_DEBLOCK & (* this->config)->options ? "yes" : "no");
		const char * _debug = ((* this->config)->proxy.debug ? "yes" : "no");
		const char * _daemon = ((* this->config)->proxy.daemon ? "yes" : "no");
		const char * _reverse = ((* this->config)->proxy.reverse ? "yes" : "no");
		const char * _forward = ((* this->config)->proxy.forward ? "yes" : "no");
		const char * _transfer = ((* this->config)->proxy.transfer ? "yes" : "no");
		const char * _optimos = ((* this->config)->proxy.optimos ? "yes" : "no");
		const char * _bandlimin = ((* this->config)->firewall.bandlimin ? "yes" : "no");
		const char * _cache = ((* this->config)->cache.dat ? "yes" : "no");
		const char * _totalcon = ((* this->config)->connects.total > 0 ? to_string((* this->config)->connects.total).c_str() : "auto");
		
		string proxyname = APP_NAME, proxyver = APP_VERSION, proxytype, proxyskill;
		
		string internal, external;
		
		string	author = APP_AUTHOR,
				site = APP_SITE,
				email = APP_EMAIL,
				support = APP_SUPPORT,
				copyright = APP_COPYRIGHT;
		
		switch((* this->config)->proxy.type){
			case 1: proxytype = "http";		break;
			case 2: proxytype = "socks5";	break;
			case 3: proxytype = "redirect";	break;
		}
		
		if(OPT_SMART & (* this->config)->options)
			proxyskill = "smart";
		else proxyskill = "dumb";
		
		switch((* this->config)->proxy.intIPv){
			
			case 4: {
				internal = (* this->config)->ipv4.internal;
				external = (* this->config)->ipv4.external[0];
			} break;
			
			case 6: {
				internal = (* this->config)->ipv6.internal;
				external = (* this->config)->ipv6.external[0];
			} break;
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
		
		transform(proxyname.begin(), proxyname.end(), proxyname.begin(), ::toupper);
		transform(copyright.begin(), copyright.end(), copyright.begin(), ::toupper);
		
		char buffer[1024 * 16];
		
		const char * format = "\n*\n"
		"*   WELCOME TO %s PROXY\n*\n*   Parameters proxy:\n"
		"*   name:                 %s\n*   version:              %s\n"
		"*   user:                 %s\n*   group:                %s\n"
		"*   daemon:               %s\n*   debug:                %s\n"
		"*   gzip transfer:        %s\n*   gzip response:        %s\n"
		"*   all connects:         %s\n*   max connect:          %i\n"
		"*   max sockets:          %i\n*   cache:                %s\n"
		"*   hide agent:           %s\n*   bandlimin:            %s\n"
		"*   deblock:              %s\n*   optimos:              %s\n"
		"*   keep-alive:           %s\n*   reverse:              %s\n"
		"*   forward:              %s\n*   transfer:             %s\n"
		"*   connect:              %s\n*   type:                 %s\n"
		"*   internet protocol:    IPv%i -> IPv%i\n*   skill:                %s\n"
		"*   internal ip:          %s\n*   external ip:          %s\n"
		"*   port:                 %i\n*   cpu cores:            %i\n"
		"*   cpu name:             %s\n*   operating system:     %s\n"
		"*   date start proxy:     %s\n*\n*   Contact Developer:\n"
		"*   copyright:            %s\n*   site:                 %s\n"
		"*   e-mail:               %s\n*   support:              %s\n"
		"*   author:               @%s\n*\n";
		
		len = sprintf(
			buffer, format, proxyname.c_str(),
			(* this->config)->proxy.name.c_str(),
			proxyver.c_str(),
			(* this->config)->proxy.user.c_str(),
			(* this->config)->proxy.group.c_str(),
			_daemon, _debug, _gzipt, _gzipr,
			_totalcon, (* this->config)->connects.connect,
			(* this->config)->connects.fds, _cache,
			_hideagent, _bandlimin, _deblock,
			_optimos, _keepalive, _reverse,
			_forward, _transfer, _connect,
			proxytype.c_str(),
			(* this->config)->proxy.intIPv,
			(* this->config)->proxy.extIPv,
			proxyskill.c_str(), internal.c_str(),
			external.c_str(), (* this->config)->proxy.port,
			(* this->config)->os.ncpu, (* this->config)->os.cpu.c_str(),
			(* this->config)->os.name.c_str(), date, copyright.c_str(),
			site.c_str(), email.c_str(), support.c_str(), author.c_str()
		);
		
		printf("%s\n", buffer);
		
		write_to_file(LOG_MESSAGE, buffer, this);
	}
}
 
LogApp::LogApp(Config ** config, u_short type){
	
	if(* config){
		
		this->config = config;
		
		this->type = type;
		
		this->enabled = (* this->config)->logs.enabled;
		
		this->dataEnabled = (* this->config)->logs.data;
		
		this->filesEnabled = (* this->config)->logs.files;
		
		this->consoleEnabled = (* this->config)->logs.console;
		
		this->size = ((* this->config)->logs.size <= 104857600 ? (* this->config)->logs.size : 104857600);
	}
}