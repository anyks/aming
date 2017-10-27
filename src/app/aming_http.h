/* SERVER PROXY AMING */
/*
*  author:     Yuriy Lobarev
*  phone:      +7(910)983-95-90
*  telegram:   @forman
*  email:      info@anyks.com
*  date:       10/27/2017 18:42:35
*  copyright:  © 2017 anyks.com
*/
 
 

#ifndef _HTTP_PROXY_AMING_
#define _HTTP_PROXY_AMING_

#include <mutex>
#include <thread>
#include <random>
#include <string>
#include <iostream>
#include <unordered_map>
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
#include "ldap/ldap.h"
#include "http/http.h"
#include "system/system.h"
#include "general/general.h"
#include "ausers/ausers.h"


using namespace std;


#define EVLOOP_ONCE				0x01
#define EVLOOP_NONBLOCK			0x02
#define EVLOOP_NO_EXIT_ON_EMPTY	0x04


#define TM_SERVER 0x01
#define TM_CLIENT 0x02

 
class ConnectClients {
	public:
		 
		struct Freeze {
			
			mutex mtx;
			
			condition_variable cond;
		};
	private:
		 
		class Client {
			private:
				
				mutex mtx;
				
				string key;
				
				Freeze freeze;
				
				u_int max = 0;
				
				u_int connects = 0;
				 
				void rm();
			public:
				
				function <void (const string)> remove;
				 
				void add(void * ctx);
		};
		
		mutex mtx;
		
		unordered_map <string, unique_ptr <Client>> clients;
		 
		void rm(const string key);
	public:
		 
		void add(void * ctx);
};
 
class BufferHttpProxy {
	private:
		
		mutex mtx;
		
		time_t readTimeout = 0, writeTimeout = 0;
		 
		struct Events {
			struct bufferevent * client = nullptr;	
			struct bufferevent * server = nullptr;	
		};
		 
		struct Request {
			vector <char>	data;			
			size_t			offset	= 0;	
		};
		 
		struct Server {
			bool	upgrade	= false;	
			u_int	port	= 0;		
			string	ip		= "";		
			string	mac		= "";		
		};
		 
		struct Sockets {
			evutil_socket_t client = -1;	
			evutil_socket_t server = -1;	
		};
		 
		struct Client {
			bool	upgrade		= false;	
			bool	connect		= false;	
			bool	alive		= false;	
			bool	https		= false;	
			string	ip			= "";		
			string	mac			= "";		
			string	useragent	= "";		
			string	request		= "";		
		};
		 
		void free_socket(evutil_socket_t * fd);
		 
		void free_event(struct bufferevent ** event);
	public:
		
		void * frze = nullptr;
		
		bool auth = false;
		
		Http parser;
		
		Client client;
		
		Server server;
		
		Events events;
		
		Sockets sockets;
		
		HttpData httpResponse;
		
		HttpData httpRequest;
		
		AuthLDAP * ldap = nullptr;
		
		System * proxy = nullptr;
		
		DNSResolver * dns = nullptr;
		
		struct event_base * base = nullptr;
		
		vector <u_char> cache;
		
		function <void (void)> remove;
		
		function <bool (void)> isFull;
		
		function <u_int (void)> activeConnects;
		 
		void closeClient();
		 
		void closeServer();
		 
		void next();
		 
		void close();
		 
		void freeze();
		 
		void checkUpgrade();
		 
		void checkClose();
		 
		void sleep(const size_t size, const bool type);
		 
		void setTimeout(const u_short type, const bool read = false, const bool write = false);
		 
		void sendClient();
		 
		void sendServer();
		 
		BufferHttpProxy(System * proxy);
		 
		~BufferHttpProxy();
};
 
class HttpProxy {
	private:
		
		mutex mtx;
		
		ConnectClients clients;
		
		pid_t * pids = nullptr;
		
		AuthLDAP * ldap = nullptr;
		
		System * server = nullptr;
		
		struct event_base * base = nullptr;
		 
		void create_client(const string ip, const string mac, const evutil_socket_t fd);
		 
		const evutil_socket_t create_server();
		 
		static const string get_mac(void * ctx);
		 
		static const string get_ip(const int family, void * ctx);
		 
		static const int nosigpipe(LogApp * log);
		 
		static const int socket_nosigpipe(const evutil_socket_t fd, LogApp * log);
		 
		static const int socket_nonblocking(const evutil_socket_t fd, LogApp * log);
		 
		static const int socket_tcpcork(const evutil_socket_t fd, LogApp * log);
		 
		static const int socket_tcpnodelay(const evutil_socket_t fd, LogApp * log);
		 
		static const int socket_reuseable(const evutil_socket_t fd, LogApp * log);
		 
		static const int socket_ipv6only(const evutil_socket_t fd, const bool mode, LogApp * log);
		 
		static const int socket_keepalive(const evutil_socket_t fd, LogApp * log, const int cnt, const int idle, const int intvl);
		 
		static const int socket_buffersize(const evutil_socket_t fd, const int read_size, const int write_size, const u_int maxcon, LogApp * log);
		 
		static const bool check_auth(void * ctx);
		 
		static const bool isallow_remote_connect(const string ip, void * ctx);
		 
		static const int connect_server(void * ctx);
		 
		static void send_http_data(void * ctx);
		 
		static void resolve_cb(const string ip, void * ctx);
		 
		static void event_cb(struct bufferevent * bev, const short events, void * ctx);
		 
		static void write_client_cb(struct bufferevent * bev, void * ctx);
		 
		static void read_server_cb(struct bufferevent * bev, void * ctx);
		 
		static void read_client_cb(struct bufferevent * bev, void * ctx);
		 
		static void accept_cb(const evutil_socket_t fd, const short event, void * ctx);
		 
		static void run_server(const evutil_socket_t fd, void * ctx);
		 
		static void run_works(pid_t * pids, const evutil_socket_t socket, const size_t cur, const size_t max, void * ctx);
	public:
		 
		static void do_request(void * ctx);
		 
		static void connection(void * ctx);
		 
		HttpProxy(System * proxy = nullptr);
		 
		~HttpProxy();
};

#endif 