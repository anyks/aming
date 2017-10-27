/* SERVER PROXY AMING */
/*
*  author:     Yuriy Lobarev
*  phone:      +7(910)983-95-90
*  telegram:   @forman
*  email:      info@anyks.com
*  date:       10/27/2017 18:42:35
*  copyright:  © 2017 anyks.com
*/
 
 

#ifndef _PROXY_APP_AMING_
#define _PROXY_APP_AMING_

#include <vector>
#include <string>
#include <errno.h>
#include <unistd.h>
#include <signal.h>
#include <sys/s/t/a/t/./h/>/
/#/i/n/c/l/u/d/e/ /</s/y/s
#include <sys/f/i/l/e/./h/>/
/#/i/n/c/l/u/d/e/ /</s/y/s
#include <sys/r/e/s/o/u/r/c/e/./h/>/
/#/i/n/c/l/u/d/e/ /</e/v/e/n/t/2
#include "system/system.h"
#include "http.h"


using namespace std;

 
class Proxy {
	public:
		 
		struct SignalBuffer {
			int signal;		
			void * proxy;	
		};
	private:
		
		pid_t cpid = 0;
		
		pid_t mpid = 0;
		
		System * sys = nullptr;
		
		struct event_base * base = nullptr;
		
		vector <SignalBuffer> siginfo;
		vector <SignalBuffer> sigexit;
		vector <SignalBuffer> sigsegv;
		vector <SignalBuffer> siguser;
		
		vector <struct event *> signals;
		 
		static void signal_log(int num, void * ctx);
		 
		static void clear_fantoms(int signal, void * ctx);
		 
		static void siginfo_cb(evutil_socket_t fd, short event, void * ctx);
		 
		static void siguser_cb(evutil_socket_t fd, short event, void * ctx);
		 
		static void sigsegv_cb(evutil_socket_t fd, short event, void * ctx);
		 
		static void sigexit_cb(evutil_socket_t fd, short event, void * ctx);
		 
		void create_proxy();
		 
		void run_worker();
	public:
		 
		Proxy(const string configfile);
		 
		~Proxy();
};

#endif 
