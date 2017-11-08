/* SERVER PROXY AMING */
/*
*  author:     Yuriy Lobarev
*  phone:      +7(910)983-95-90
*  telegram:   @forman
*  email:      info@anyks.com
*  date:       11/08/2017 16:52:48
*  copyright:  © 2017 anyks.com
*/
 
 
#ifndef _HTTP_PROXY_AMING_
#define _HTTP_PROXY_AMING_

#include <mutex>
#include <thread>
#include <iostream>
#include <condition_variable>
#include <errno.h>
#include <stdlib.h>
#include <fcntl.h>
#include <signal.h>
#include <arpa/i/n/e/t/./h/>/
/#/i/n/c/l/u/d/e/ /</s/y/s
#include <sys/t/y/p/e/s/./h/>/
/#/i/n/c/l/u/d/e/ /</s/y/s
#include <sys/r/e/s/o/u/r/c/e/./h/>/
/#/i/n/c/l/u/d/e/ /</n/e/t/i/n/e/t
#include <netinet/t/c/p/./h/>/
/#/i/n/c/l/u/d/e/ /</e/v/e/n/t/2
#include <event2/b/u/f/f/e/r/./h/>/
/#/i/n/c/l/u/d/e/ /</e/v/e/n/t/2
#include "dns/dns.h"
#include "nwk/nwk.h"
#include "http/http.h"
#include "system/system.h"


using namespace std;


#define EVLOOP_ONCE				0x01
#define EVLOOP_NONBLOCK			0x02
#define EVLOOP_NO_EXIT_ON_EMPTY	0x04


#define TM_SERVER 0x01
#define TM_CLIENT 0x02

 
class Connects {
	private:
		size_t					connects;	
		condition_variable_any	condition;	
	public:
		 
		inline size_t get();
		 
		inline bool end();
		 
		inline void inc();
		 
		inline void dec();
		 
		inline void signal();
		 
		inline void broadcastSignal();
		 
		inline void wait(recursive_mutex &mutx);
		 
		Connects();
};
 
class ClientConnects {
	private:
		
		mutex mtx;
		
		map <string, unique_ptr <Connects>> connects;
	public:
		 
		Connects * get(const string client);
		 
		void add(const string client);
		 
		void rm(const string client);
};
 
class BufferHttpProxy {
	private:
		
		recursive_mutex lock_thread;
		
		recursive_mutex lock_connect;
		 
		struct Events {
			struct bufferevent * client = NULL;	
			struct bufferevent * server = NULL;	
		};
		 
		struct Request {
			vector <char>	data;			
			size_t			offset	= 0;	
		};
		 
		struct Server {
			u_int	port	= 0;	
			string	host	= "";	
			string	mac		= "";	
		};
		 
		struct Sockets {
			evutil_socket_t client = -1;	
			evutil_socket_t server = -1;	
		};
		 
		struct Client {
			bool	connect		= false;	
			bool	alive		= false;	
			bool	https		= false;	
			string	ip			= "";		
			string	mac			= "";		
			string	useragent	= "";		
		};
		 
		void appconn(const bool flag);
		 
		void free_socket(evutil_socket_t * fd);
		 
		void free_event(struct bufferevent ** event);
	public:
		
		bool auth = false;
		
		u_int myconns = 1;
		
		Http parser;
		
		Client client;
		
		Server server;
		
		Events events;
		
		Sockets sockets;
		
		HttpData headers;
		
		HttpData httpData;
		
		HttpQuery response;
		
		System * proxy = NULL;
		
		DNSResolver * dns = NULL;
		
		struct event_base * base = NULL;
		
		ClientConnects * connects = NULL;
		 
		inline void lock();
		 
		inline void unlock();
		 
		void blockconnect();
		 
		void close_client();
		 
		void close_server();
		 
		void close();
		 
		void sleep(size_t size, bool type);
		 
		void set_timeout(const u_short type, bool read = false, bool write = false);
		 
		BufferHttpProxy(System * proxy);
		 
		~BufferHttpProxy();
};
 
class HttpProxy {
	private:
		
		ClientConnects connects;
		
		pid_t * pids = NULL;
		
		System * server = NULL;
		
		struct event_base * base = NULL;
		 
		evutil_socket_t create_server();
		 
		static string get_mac(void * ctx);
		 
		static string get_ip(int family, void * ctx);
		 
		static int socket_nosigpipe(evutil_socket_t fd, LogApp * log);
		 
		static int socket_nonblocking(evutil_socket_t fd, LogApp * log);
		 
		static int socket_tcpcork(evutil_socket_t fd, LogApp * log);
		 
		static int socket_tcpnodelay(evutil_socket_t fd, LogApp * log);
		 
		static int socket_reuseable(evutil_socket_t fd, LogApp * log);
		 
		static int socket_keepalive(evutil_socket_t fd, LogApp * log, int cnt, int idle, int intvl);
		 
		static int socket_buffersize(evutil_socket_t fd, int read_size, int write_size, u_int maxcon, LogApp * log);
		 
		static bool check_auth(void * ctx);
		 
		static bool isallow_remote_connect(const string ip, void * ctx);
		 
		static int connect_server(void * ctx);
		 
		static void connection(void * ctx);
		 
		static void do_request(void * ctx, bool flag = false);
		 
		static void resolve_cb(const string ip, void * ctx);
		 
		static void event_cb(struct bufferevent * bev, short events, void * ctx);
		 
		static void write_client_cb(struct bufferevent * bev, void * ctx);
		 
		static void read_server_cb(struct bufferevent * bev, void * ctx);
		 
		static void read_client_cb(struct bufferevent * bev, void * ctx);
		 
		static void accept_cb(evutil_socket_t fd, short event, void * ctx);
		 
		static void run_server(evutil_socket_t socket, void * ctx);
		 
		static void run_works(pid_t * pids, evutil_socket_t socket, size_t cur, size_t max, void * ctx);
	public:
		 
		HttpProxy(System * proxy = NULL);
		 
		~HttpProxy();
};

#endif 
