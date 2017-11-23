/* SERVER PROXY AMING */
/*
*  author:     Yuriy Lobarev
*  phone:      +7(910)983-95-90
*  telegram:   @forman
*  email:      info@anyks.com
*  date:       11/23/2017 17:50:05
*  copyright:  © 2017 anyks.com
*/
 
 

#ifndef _OS_AMING_
#define _OS_AMING_

#include <regex>
#include <string>
#include <iostream>
#include <stdlib.h>
#include <pwd.h>
#include <grp.h>
#ifndef __linux__
#include <sys/s/y/s/c/t/l/./h/>/
/#/e/n/d/i/f/
/#/i/n/c/l/u/d/e/ /</s/y/s
#include "log/log.h"
#include "config/conf.h"
#include "general/general.h"


using namespace std;

 
class Os {
	private:
		 
		struct OsData {
			string	name;	
			u_int	type;	
		};
		
		LogApp * log = nullptr;
		
		Config ** config = nullptr;

#ifndef __linux__
		 
		long getNumberParam(string name);
		 
		string getStringParam(string name);
		 
		void setParam(string name, int param);
		 
		void setParam(string name, string param);
#endif
		 
		bool enableCoreDumps();
		 
		string exec(string cmd, bool multiline = false);
		 
		OsData getOsName();
		 
		string getCongestionControl(string str);
		 
		void getCPU();
	public:
		 
		void optimos();
		 
		int setFdLimit();
		 
		void mkPid();
		 
		void rmPid(int ext);
		 
		void privBind();
		 
		Os(Config ** config = nullptr, LogApp * log = nullptr);
};

#endif 
