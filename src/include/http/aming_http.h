/* SERVER PROXY AMING */
/*
*  author:     Yuriy Lobarev
*  phone:      +7(910)983-95-90
*  telegram:   @forman
*  email:      info@anyks.com
*  date:       10/29/2017 16:59:43
*  copyright:  © 2017 anyks.com
*/
 
 

#ifndef _HTTP_PARSER_AMING_
#define _HTTP_PARSER_AMING_

#include <regex>
#include <string>
#include <vector>
#include <cstring>
#include <sstream>
#include <algorithm>
#include <unordered_map>
#include <zlib.h>
#include <stdlib.h>
#include <sys/t/y/p/e/s/./h/>/
/#/i/n/c/l/u/d/e/ /"/b/a/s/e/6/4
#include "config/conf.h"
#include "general/general.h"


#define MOD_GZIP_ZLIB_WINDOWSIZE	15
#define MOD_GZIP_ZLIB_CFACTOR		9
#define MOD_GZIP_ZLIB_BSIZE			8096
#define MOD_GZIP_ZLIB_CHUNK			1024


#define PARSER_OPTIONS_DEFAULT OPT_AGENT | OPT_GZIP | OPT_KEEPALIVE | OPT_LOG


using namespace std;

 
class HttpData {
	private:
		 
		class HttpHeaders {
			private:
				 
				struct Header {
					string head;	
					string value;	
				};
				
				bool end = false;
				
				vector <u_char> raw;
				
				vector <Header> headers;
			public:
				 
				const string getHeaders();
				 
				Header getHeader(const string key);
				 
				const u_char * data();
				 
				void set(const u_char * data, size_t size);
				 
				void clear();
				 
				void remove(const string key);
				 
				void append(const string key, const string val);
				 
				void setEnd();
				 
				const bool create(const char * buffer);
				 
				const bool isEnd();
				 
				const bool empty();
				 
				const size_t size();
				 
				const size_t getHeadersSize();
				 
				vector <Header>::const_iterator cbegin() const noexcept;
				 
				vector <Header>::const_iterator cend() const noexcept;
				 
				~HttpHeaders();
		};
		 
		class HttpBody {
			private:
				 
				struct Chunk {
					
					string hsize = "0";
					
					string content;
					 
					
					 
					void init(const char * data, const size_t size);
					 
					Chunk(const char * data = nullptr, const size_t size = 0);
				};
				
				u_int levelGzip;
				
				size_t chunkSize = 0;
				
				bool end = false;
				
				bool intGzip = false;
				
				bool extGzip = false;
				
				string body;
				
				string rody;
				
				vector <u_char> raw;
				
				vector <Chunk> chunks;
				 
				const string compress_gzip(const string &str);
				 
				const string decompress_gzip(const string &str);
				 
				const string compressData(const char * buffer, const size_t size);
				 
				void createChunk(const char * buffer, const size_t size);
			public:
				 
				void clear();
				 
				void setChunkSize(const size_t size);
				 
				void setLevelGzip(const u_int levelGzip);
				 
				void setEnd();
				 
				void set(const u_char * data, size_t size);
				 
				const bool isEnd();
				 
				const bool isIntCompress();
				 
				const bool isExtCompress();
				 
				const u_int getLevelGzip();
				 
				const size_t getChunkSize();
				 
				const size_t getBodySize(const bool chunked = false);
				 
				const size_t size();
				 
				const size_t addData(const char * buffer, const size_t size, const size_t length = 0, const bool strict = false);
				 
				const u_char * data();
				 
				const string getBody(const bool chunked = false);
				 
				const string getRawBody();
				 
				vector <Chunk> getChunks();
				 
				HttpBody(const size_t chunkSize = 1024, const u_int levelGzip = Z_DEFAULT_COMPRESSION, const bool intGzip = false, const bool extGzip = false);
				 
				~HttpBody();
		};
	private:
		 
		struct Dump {
			size_t status;		
			size_t options;		
			size_t http;		
			size_t auth;		
			size_t path;		
			size_t host;		
			size_t port;		
			size_t login;		
			size_t method;		
			size_t appName;		
			size_t version;		
			size_t protocol;	
			size_t password;	
			size_t appVersion;	
			size_t headers;		
			size_t body;		
		};
		 
		struct Http {
			string text;	
			string headers;	
			string body;	
		};
		 
		struct Connect {
			string host;		
			string port;		
			string path;		
			string protocol;	
		};
		 
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
		
		bool intGzip;	
		bool extGzip;	
		
		u_int status;
		
		u_short options;
		
		vector <u_char> raw;
		
		string http;		
		string auth;		
		string path;		
		string host;		
		string port;		
		string login;		
		string method;		
		string version;		
		string appName;		
		string protocol;	
		string password;	
		string appVersion;	
		
		HttpBody body;
		
		HttpHeaders headers;
		
		Gzip * gzipParams = nullptr;
		
		unordered_map <u_short, Http> response;
		 
		void genDataConnect();
		 
		void createRequest(const u_short code);
		 
		const string createHeadResponse();
		 
		const string createHeadRequest();
		 
		Connect getConnection(const string str);
	public:
		 
		const bool isIntGzip();
		 
		const bool isExtGzip();
		 
		const bool isUpgrade();
		 
		const bool isConnect();
		 
		const bool isClose();
		 
		const bool isHttps();
		 
		const bool isAlive();
		 
		const bool isEmpty();
		 
		const bool isEndHeaders();
		 
		const bool isEndBody();
		 
		const bool compressIsAllowed(const string userAgent = "");
		 
		const bool rmHeaderInString(const string header, string &headers);
		 
		const bool addHeaderToString(const string header, const string value, string &headers);
		 
		const bool setRedirect(HttpData &response);
		 
		const size_t parse(const char * buffer, const size_t size, const string name = APP_NAME, const u_short options = PARSER_OPTIONS_DEFAULT);
		 
		const size_t getBodySize();
		 
		const size_t getRawBodySize();
		 
		const size_t setEntitybody(const char * buffer, const size_t size);
		 
		const size_t size();
		 
		const u_int getPort();
		 
		const u_int getStatus();
		 
		const float getVersion();
		 
		const string getHttp();
		 
		const string getMethod();
		 
		const string getHost();
		 
		const string getPath();
		 
		const string getProtocol();
		 
		const string getAuth();
		 
		const string getLogin();
		 
		const string getPassword();
		 
		const string getUseragent();
		 
		const string getHeader(const string key);
		 
		const string getResponseHeaders();
		 
		const string getRequestHeaders();
		 
		const string modifyHeaderString(const string str);
		 
		const string getBody(const bool chunked = false);
		 
		const string getResponseData(const bool chunked = false);
		 
		const string getRequestData();
		 
		const string getRawResponseData();
		 
		const string getRawRequestData();
		 
		const u_char * data();
		 
		void set(const u_char * data, size_t size);
		 
		void clear();
		 
		void initBody();
		 
		void rmHeader(const string key);
		 
		void unsetGzip();
		 
		void setGzip(const bool intGzip = true, const bool extGzip = false);
		 
		void setBodyEnd();
		 
		void setHeader(const string key, const string value);
		 
		void setData(const char * buffer, const size_t size);
		 
		void setGzipParams(void * params = nullptr);
		 
		void setOptions(const u_short options);
		 
		void setMethod(const string str);
		 
		void setHost(const string str);
		 
		void setPort(const u_int number);
		 
		void setStatus(const u_int number);
		 
		void setPath(const string str);
		 
		void setProtocol(const string str);
		 
		void setVersion(const float number);
		 
		void setAuth(const string str);
		 
		void setUseragent(const string str);
		 
		void setClose();
		 
		void addHeader(const char * buffer = nullptr);
		 
		void largeRequest();
		 
		void brokenRequest();
		 
		void faultConnect();
		 
		void pageNotFound();
		 
		void faultAuth();
		 
		void requiredAuth();
		 
		void authSuccess();
		 
		void create(const string name, const u_short options);
		 
		HttpData(const string name = APP_NAME, const u_short options = PARSER_OPTIONS_DEFAULT);
		 
		~HttpData();
};
 
class Http {
	private:
		 
		struct HttpEnd {
			u_short	type = 0;
			size_t	begin = 0, end = 0;
		};
		
		u_short options;
		
		string name, version;
	public:
		
		vector <HttpData> httpData;
		 
		const bool isHttp(const string buffer);
		 
		const size_t parse(const char * buffer, const size_t size);
		 
		void modify(vector <char> &data);
		 
		void clear();
		 
		void create(const string name, const u_short options);
		 
		Http(const string name = APP_NAME, const u_short options = PARSER_OPTIONS_DEFAULT);
		 
		~Http();
};

#endif 
