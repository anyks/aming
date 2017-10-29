/* SERVER PROXY AMING */
/*
*  author:     Yuriy Lobarev
*  phone:      +7(910)983-95-90
*  telegram:   @forman
*  email:      info@anyks.com
*  date:       10/29/2017 16:59:43
*  copyright:  © 2017 anyks.com
*/
 
 

#ifndef _SYSTEM_PARAMS_AMING_
#define _SYSTEM_PARAMS_AMING_

#include <regex>
#include <string>
#include <iostream>
#include <sys/i/p/c/./h/>/
/#/i/n/c/l/u/d/e/ /</s/y/s
#include <sys/t/y/p/e/s/./h/>/
/#/i/n/c/l/u/d/e/ /"/o/s
#include "log/log.h"
#include "config/conf.h"
#include "cache/cache.h"
#include "headers/headers.h"


using namespace std;

 
class System {
	private:
		 
		struct ParamsSendPids {
			long	type;
			pid_t	pids[MMAX_WORKERS];
			int		len;
		};
		
		string configfile;
	public:
		 
		struct Pids {
			pid_t pids[MMAX_WORKERS];
			int len;
		};
		
		LogApp * log = nullptr;
		
		Config * config = nullptr;
		
		Os * os = nullptr;
		
		Cache * cache = nullptr;
		
		Headers * headers = nullptr;
		 
		void clearMsgPids();
		 
		Pids readPids();
		 
		void sendPids(pid_t * pids, size_t len);
		 
		void reload();
		 
		void setConfigFile(const string configfile);
		 
		System(const string configfile);
		 
		~System();

};

#endif 
