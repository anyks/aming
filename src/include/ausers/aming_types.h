/* SERVER PROXY AMING */
/*
*  author:     Yuriy Lobarev
*  phone:      +7(910)983-95-90
*  telegram:   @forman
*  email:      info@anyks.com
*  date:       11/23/2017 17:50:05
*  copyright:  © 2017 anyks.com
*/
 
 

#ifndef _APARAMS_PROXY_AMING_
#define _APARAMS_PROXY_AMING_

#include <vector>
#include <sys/t/y/p/e/s/./h/>/
/

using namespace std;

 
namespace AParams {
	 
	struct Gzip {
		bool vary;					
		int level;					
		long length;				
		size_t chunk;				
		string regex;				
		vector <string> vhttp;		
		vector <string> proxied;	
		vector <string> types;		
	};
	 
	struct IP {
		vector <string> ip;			
		vector <string> resolver;	
	};
	 
	struct Proxy {
		bool reverse;				
		bool transfer;				
		bool forward;				
		bool subnet;				
		bool pipelining;			
		vector <string> redirect;	
	};
	 
	struct Connects {
		size_t size;	
		u_int connect;	
	};
	 
	struct Timeouts {
		size_t read;	
		size_t write;	
		size_t upgrade;	
	};
	 
	struct BufferSize {
		long read;		
		long write;		
	};
	 
	struct Keepalive {
		int keepcnt;	
		int keepidle;	
		int keepintvl;	
	};
	 
	struct Group {
		gid_t gid;		
		string name;	
		string desc;	
		string pass;	
	};
	 
	struct User {
		uid_t uid;		
		string name;	
		string desc;	
		string pass;	
	};
	 
	struct GroupData {
		gid_t gid;				
		u_short type;			
		string name;			
		string desc;			
		string pass;			
		vector <uid_t> users;	
	};
	 
	struct UserData {
		uid_t uid;		
		u_short type;	
		string name;	
		string desc;	
		string pass;	
	};
	 
	struct AUser {
		bool auth = false;		
		u_short options;		
		vector <string> idnt;	
		vector <Group> groups;	
		User user;				
		IP ipv4;				
		IP ipv6;				
		Gzip gzip;				
		Proxy proxy;			
		Connects connects;		
		Timeouts timeouts;		
		BufferSize buffers;		
		Keepalive keepalive;	
	};
	 
	struct Params {
		u_int id;				
		u_short options;		
		vector <string> idnt;	
		IP ipv4;				
		IP ipv6;				
		Gzip gzip;				
		Proxy proxy;			
		Connects connects;		
		Timeouts timeouts;		
		BufferSize buffers;		
		Keepalive keepalive;	
	};
	 
	struct Client {
		string ip;		
		string mac;		
		string sip;		
		AUser * auser;	
	};
};

#endif 
