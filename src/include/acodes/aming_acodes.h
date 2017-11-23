/* SERVER PROXY AMING */
/*
*  author:     Yuriy Lobarev
*  phone:      +7(910)983-95-90
*  telegram:   @forman
*  email:      info@anyks.com
*  date:       11/23/2017 17:50:05
*  copyright:  © 2017 anyks.com
*/
 
 

#ifndef _HTTP_ACODES_AMING_
#define _HTTP_ACODES_AMING_

#include <map>
#include <regex>
#include <string>
#include <fstream>
#include <iostream>
#include <stdlib.h>
#include "log/log.h"
#include "config/conf.h"
#include "general/general.h"


using namespace std;

 
class ACodes {
	private:
		 
		struct Template {
			u_short code;	
			u_short type;	
			string data;	
		};
		
		LogApp * log = nullptr;
		Config * config = nullptr;
		
		map <u_short, Template> templates;
		
		const map <u_short, string> deftemps = {
			{100, ""},
			{200, "OK"},
			{400, "<html><head><title>400 Bad Request</title></head>\r\n<body><h2>400 Bad Request</h2></body></html>"},
			{403, "<html><head><title>403 Access Denied</title></head>\r\n<body><h2>403 Access Denied</h2><h3>Access control list denies you to access this resource</body></html>"},
			{404, "<html><head><title>404 Not Found</title></head>\r\n<body><h2>404 Not Found</h2><h3>Page not found</body></html>"},
			{407, "<html><head><title>407 Proxy Authentication Required</title></head>\r\n<body><h2>407 Proxy Authentication Required</h2>\r\n<h3>Access to requested resource disallowed by administrator or you need valid username/password to use this resource</h3>\r\n</body></html>"},
			{500, "<html><head><title>500 Internal Error</title></head>\r\n<body><h2>500 Internal Error</h2><h3>Internal proxy error during processing your request</h3></body></html>"},
			{502, "<html><head><title>502 Bad Gateway</title></head>\r\n<body><h2>502 Bad Gateway</h2><h3>Failed to connect server</h3></body></html>"},
			{503, "<html><head><title>503 Service Unavailable</title></head>\r\n<body><h2>503 Service Unavailable</h2><h3>Recursion detected</h3></body></html>"},
			{509, "<html><head><title>509 Bandwidth Limit Exceeded</title></head>\r\n<body><h2>509 Bandwidth Limit Exceeded</h2><h3>Data or File too large size</h3></body></html>"},
			{510, "<html><head><title>510 Not Extended</title></head>\r\n<body><h2>510 Not Extended</h2><h3>Required action is not extended by proxy server</h3></body></html>"}
		};
		 
		void read();
	public:
		 
		void clear();
		 
		const Template get(u_short code);
		 
		ACodes(Config * config = nullptr, LogApp * log = nullptr);
};

#endif 
