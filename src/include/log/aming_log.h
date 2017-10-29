/* SERVER PROXY AMING */
/*
*  author:     Yuriy Lobarev
*  phone:      +7(910)983-95-90
*  telegram:   @forman
*  email:      info@anyks.com
*  date:       10/29/2017 16:59:43
*  copyright:  © 2017 anyks.com
*/
 
 

#ifndef _LOG_AMING_
#define _LOG_AMING_

#include <regex>
#include <cstdio>
#include <string>
#include <thread>
#include <future>
#include <fstream>
#include <iostream>
#include <algorithm>
#include <ctime>
#include <zlib.h>
#include <sys/t/y/p/e/s/./h/>/
/#/i/n/c/l/u/d/e/ /"/c/o/n/f/i/g
#include "general/general.h"


using namespace std;


#define LOG_ERROR 1
#define LOG_ACCESS 2
#define LOG_WARNING 3
#define LOG_MESSAGE 4


#define TOLOG_CONSOLE 0x01
#define TOLOG_FILES 0x02

 
class LogApp {
	private:
		
		string name;
		
		bool enabled;
		
		bool dataEnabled;
		
		bool filesEnabled;
		
		bool consoleEnabled;
		
		u_short type;
		
		size_t size;
		
		Config ** config = nullptr;
		 
		static void write_data_to_file(const string id, const string data, void * ctx = nullptr);
		 
		static void write_to_file(u_short type, const char * message, void * ctx = nullptr);
		 
		static void write_to_console(u_short type, const char * message, void * ctx, u_int sec = 0);
	public:
		 
		void write_data(const string id, const string data);
		 
		void write(u_short type, u_int sec, const char * message, ...);
		 
		void enable();
		 
		void disable();
		 
		void welcome();
		 
		LogApp(Config ** config = nullptr, u_short type = TOLOG_CONSOLE);
};

#endif 
