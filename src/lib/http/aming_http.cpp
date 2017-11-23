/* SERVER PROXY AMING */
/*
*  author:     Yuriy Lobarev
*  phone:      +7(910)983-95-90
*  telegram:   @forman
*  email:      info@anyks.com
*  date:       11/23/2017 17:50:05
*  copyright:  © 2017 anyks.com
*/
 
 

#include "http/http.h"


using namespace std;

 
const string HttpData::HttpHeaders::getHeaders(){
	
	string headers;
	
	for(auto it = this->headers.begin(); it != this->headers.end(); it++){
		
		headers.append(it->head + string(": ") + it->value + string("\r\n"));
	}
	
	headers.append("\r\n");
	
	return headers;
}
 
HttpData::HttpHeaders::Header HttpData::HttpHeaders::getHeader(const string key){
	
	string ckey = key;
	
	ckey = Anyks::toCase(Anyks::trim(ckey));
	
	for(auto it = this->headers.begin(); it != this->headers.end(); it++){
		
		if(Anyks::toCase(it->head).compare(ckey) == 0) return (* it);
	}
	
	return {"", ""};
}
 
const u_char * HttpData::HttpHeaders::data(){
	
	if(isEnd()){
		
		string headers;
		
		const size_t size = getHeadersSize();
		
		for(size_t i = 0; i < size; i++){
			
			if((Anyks::toCase(this->headers[i].head).compare("age") != 0)
			&& (Anyks::toCase(this->headers[i].head).compare("content-length") != 0)
			&& (Anyks::toCase(this->headers[i].head).compare("transfer-encoding") != 0)
			&& !((Anyks::toCase(this->headers[i].head).compare("content-encoding") == 0)
			&& (Anyks::toCase(this->headers[i].value).find("gzip") != string::npos))){
				
				string header = this->headers[i].head;
				
				string value = this->headers[i].value;
				
				if(Anyks::toCase(header).compare("etag") == 0){
					
					size_t pos = value.find("W/");
					
					if(pos != string::npos) value = value.substr(pos + 2, value.length() - 2);
				}
				
				headers.append(header + string("<-|params|->") + value);
				
				if(i < (size - 1)) headers.append("<-|heads|->");
			}
		}
		
		this->raw.assign(headers.begin(), headers.end());
	}
	
	return this->raw.data();
}
 
void HttpData::HttpHeaders::set(const u_char * data, size_t size){
	
	string headers;
	
	if(size) headers.assign(reinterpret_cast <const char *> (data), size);
	
	if(!headers.empty()){
		
		clear();
		
		vector <string> data;
		
		Anyks::split(headers, "<-|heads|->", data);
		
		if(!data.empty()){
			
			for(auto it = data.begin(); it != data.end(); it++){
				
				vector <string> header;
				
				Anyks::split(* it, "<-|params|->", header);
				
				if(!header.empty()) append(* header.begin(), * (header.end() - 1));
			}
			
			setEnd();
		}
	}
}
 
void HttpData::HttpHeaders::clear(){
	
	this->end = false;
	
	this->raw.clear();
	
	this->headers.clear();
}
 
void HttpData::HttpHeaders::remove(const string key){
	
	string ckey = key;
	
	ckey = Anyks::toCase(Anyks::trim(ckey));
	
	for(auto it = this->headers.begin(); it != this->headers.end(); it++){
		
		if(Anyks::toCase(it->head).compare(ckey) == 0){
			
			this->headers.erase(it);
			
			break;
		}
	}
}
 
void HttpData::HttpHeaders::append(const string key, const string val){
	
	string ckey = key;
	string cval = val;
	
	ckey = Anyks::trim(ckey);
	cval = Anyks::trim(cval);
	
	string rkey = Anyks::toCase(ckey);
	
	if((rkey.compare("cookie") != 0)
	&& (rkey.compare("set-cookie") != 0)) remove(rkey);
	
	this->headers.push_back({ckey, cval});
}
 
void HttpData::HttpHeaders::setEnd(){
	
	this->end = true;
}
 
const bool HttpData::HttpHeaders::create(const char * buffer){
	
	clear();
	
	string str = buffer;
	
	smatch match;
	
	regex e(
		"^(?:(?:(?:(?:" PROXY_HTTP_METHODS ")"
		"\\s+[^\\r\\n\\s]+\\s+[A-Za-z]+\\/[\\d\\.]+\\r\\n)|(?:[A-Za-z]+"
		"\\/[\\d\\.]+\\s+\\d+(?:\\s+[^\\r\\n]+)?\\r\\n))((?:[\\w\\-]+\\s*\\:"
		"\\s*[^\\r\\n]+\\r\\n)+)\\r\\n)",
		regex::ECMAScript | regex::icase
	);
	
	regex_search(str, match, e);
	
	if(!match.empty()){
		
		str = match[1].str();
		
		vector <string> strings;
		
		Anyks::split(str, "\r\n", strings);
		
		if(!strings.empty()){
			
			for(auto it = strings.begin(); it != strings.end(); it++){
				
				smatch match;
				
				regex e("\\b([\\w\\-]+)\\s*\\:\\s*([\\s\\S]+)", regex::ECMAScript | regex::icase);
				
				regex_search(* it, match, e);
				
				if(!match.empty()) this->append(match[1].str(), match[2].str());
			}
			
			setEnd();
			
			return getHeadersSize();
		}
	}
	
	return false;
}
 
const bool HttpData::HttpHeaders::isEnd(){
	
	return this->end;
}
 
const bool HttpData::HttpHeaders::empty(){
	
	return this->headers.empty();
}
 
const size_t HttpData::HttpHeaders::size(){
	
	if(this->raw.empty()) data();
	
	return this->raw.size();
}
 
const size_t HttpData::HttpHeaders::getHeadersSize(){
	
	return this->headers.size();
}
 
vector <HttpData::HttpHeaders::Header>::const_iterator HttpData::HttpHeaders::cbegin() const noexcept {
	
	return this->headers.cbegin();
}
 
vector <HttpData::HttpHeaders::Header>::const_iterator HttpData::HttpHeaders::cend() const noexcept {
	
	return this->headers.cend();
}
 
HttpData::HttpHeaders::~HttpHeaders(){
	
	clear();
	
	vector <u_char> ().swap(this->raw);
	
	vector <Header> ().swap(this->headers);
}
 
 
 
void HttpData::HttpBody::Chunk::init(const char * data, const size_t size){
	
	if(data && size){
		
		this->content.assign(data, data + size);
		
		stringstream stream;
		
		stream << std::hex << this->content.size();
		
		string hsize(stream.str());
		
		this->hsize = hsize;
	}
}
 
HttpData::HttpBody::Chunk::Chunk(const char * data, const size_t size){
	
	init(data, size);
}
 
const size_t HttpData::HttpBody::getBodySize(const bool chunked){
	
	size_t size = 0;
	
	if(chunked){
		
		for(auto it = this->chunks.begin(); it != this->chunks.end(); it++) size += it->content.size();
	
	} else size = this->body.size();
	
	return size;
}
 
const size_t HttpData::HttpBody::size(){
	
	if(this->raw.empty()) data();
	
	return this->raw.size();
}
 
const string HttpData::HttpBody::compress_gzip(const string &str){
	
	z_stream zs;
	
	int ret = 0;
	
	string outstring;
	
	memset(&zs, 0, sizeof(zs));
	
	if(deflateInit2(&zs, this->levelGzip, Z_DEFLATED, MOD_GZIP_ZLIB_WINDOWSIZE + 16, MOD_GZIP_ZLIB_CFACTOR, Z_DEFAULT_STRATEGY) == Z_OK){
		
		zs.next_in = (Bytef *) str.data();
		
		zs.avail_in = str.size();
		
		char * zbuff = new char[(const size_t) zs.avail_in];
		
		do {
			
			zs.next_out = reinterpret_cast<Bytef *> (zbuff);
			
			zs.avail_out = sizeof(zbuff);
			
			ret = deflate(&zs, Z_FINISH);
			
			if(outstring.size() < zs.total_out)
				
				outstring.append(zbuff, zs.total_out - outstring.size());
		} while(ret == Z_OK);
		
		delete [] zbuff;
	}
	
	deflateEnd(&zs);
	
	if(ret != Z_STREAM_END) outstring.clear();
	
	return outstring;
}
 
const string HttpData::HttpBody::decompress_gzip(const string &str){
	
	z_stream zs;
	
	int ret = 0;
	
	string outstring;
	
	memset(&zs, 0, sizeof(zs));
	
	if(inflateInit2(&zs, MOD_GZIP_ZLIB_WINDOWSIZE + 16) == Z_OK){
		
		zs.next_in = (Bytef *) str.data();
		
		zs.avail_in = str.size();
		
		const size_t osize = (zs.avail_in * 10);
		
		do {
			
			char * zbuff = new char[osize];
			
			zs.next_out = reinterpret_cast<Bytef *> (zbuff);
			
			zs.avail_out = osize;
			
			ret = inflate(&zs, 0);
			
			if(outstring.size() < zs.total_out)
				
				outstring.append(zbuff, zs.total_out - outstring.size());
			
			delete [] zbuff;
		} while(ret == Z_OK);
	}
	
	inflateEnd(&zs);
	
	if(ret != Z_STREAM_END) outstring.clear();
	
	return outstring;
}
 
const string HttpData::HttpBody::compressData(const char * buffer, const size_t size){
	
	vector <char> data;
	
	z_stream zs;
	
	memset(&zs, 0, sizeof(z_stream));
	 
	
	if(deflateInit(&zs, this->levelGzip) == Z_OK){
		
		size_t size_out = size;
		
		char * zbuff = new char [(const size_t) size_out];
		
		zs.next_in = reinterpret_cast <Bytef *> (const_cast <char *> (buffer));
		
		zs.avail_in = static_cast <uInt> (size);
		
		zs.next_out = reinterpret_cast <Bytef *> (zbuff);
		
		zs.avail_out = static_cast <uInt> (size_out);
		
		int checksum = crc32(0, zs.next_in, zs.avail_in);
		
		int const result = deflate(&zs, Z_FINISH);
		
		size_out = zs.total_out;
		
		deflateEnd(&zs);
		
		if(result == Z_STREAM_END){
			
			const short prefix = 2;
			
			const short suffix = 4;
			
			 
			const char gzipheader[] = {'\037', '\213', 0x08, 0x01, 0x00, 0x00, 0x00, 0x00, 0x02, 0x03};
			
			size_out -= prefix + suffix;
			
			const char * cchecksum = reinterpret_cast <char *> (&checksum);
			
			copy(gzipheader, gzipheader + sizeof(gzipheader), back_inserter(data));
			
			copy(zbuff + prefix, zbuff + prefix + size_out, back_inserter(data));
			
			copy(cchecksum, cchecksum + sizeof(checksum), back_inserter(data));
		}
		
		delete [] zbuff;
	}
	
	return string(data.data(), data.size());
}
 
void HttpData::HttpBody::createChunk(const char * buffer, const size_t size){
	
	size_t copySize = size;
	
	size_t used = 0;
	
	while(copySize){
		
		size_t size = (copySize < this->chunkSize ? copySize : this->chunkSize);
		
		vector <char> data(buffer + used, buffer + used + size);
		
		Chunk chunk(data.data(), data.size());
		
		this->chunks.push_back(chunk);
		
		copySize -= size;
		
		used += size;
	}
}
 
void HttpData::HttpBody::clear(){
	
	this->end = false;
	
	this->intGzip = false;
	this->extGzip = false;
	
	this->chunks.clear();
	
	this->body.clear();
	this->rody.clear();
	
	this->raw.clear();
}
 
void HttpData::HttpBody::set(const u_char * data, size_t size){
	
	if(size){
		
		size_t size_data;
		
		memcpy(&size_data, data, sizeof(size_t));
		
		bool intGzip = this->intGzip;
		
		if(size_data){
			
			clear();
			
			this->intGzip = intGzip;
			
			char * buffer = new char [size_data];
			
			memcpy(buffer, data + sizeof(size_data), size_data);
			
			addData(buffer, size_data, size_data, true);
			
			delete [] buffer;
		}
	}
}
 
void HttpData::HttpBody::setChunkSize(const size_t size){
	
	this->chunkSize = size;
}
 
void HttpData::HttpBody::setLevelGzip(const u_int levelGzip){
	
	this->levelGzip = levelGzip;
}
 
void HttpData::HttpBody::setEnd(){
	
	this->end = true;
	
	if(!this->extGzip) this->rody = this->body;
	
	else this->rody = decompress_gzip(this->body);
	
	if(!this->intGzip) createChunk(this->body.data(), this->body.size());
	
	else if(!this->extGzip && this->intGzip){
		
		string data = compress_gzip(this->body);
		
		if(!data.empty()) this->body = data;
		
		else this->intGzip = false;
		
		createChunk(this->body.data(), this->body.size());
	}
}
 
const bool HttpData::HttpBody::isEnd(){
	
	return this->end;
}
 
const bool HttpData::HttpBody::isIntCompress(){
	
	return this->intGzip;
}
 
const bool HttpData::HttpBody::isExtCompress(){
	
	return this->extGzip;
}
 
const u_int HttpData::HttpBody::getLevelGzip(){
	
	return this->levelGzip;
}
 
const size_t HttpData::HttpBody::getChunkSize(){
	
	return this->chunkSize;
}
 
const size_t HttpData::HttpBody::addData(const char * buffer, const size_t size, const size_t length, const bool strict){
	
	size_t readbytes = 0;
	
	if(!this->end && size && length){
		
		switch(length){
			
			case 1: {
				
				readbytes = size;
				
				this->body.append(buffer, readbytes);
			} break;
			
			case 2: {
				
				size_t pos = string(buffer, size).find("0\r\n\r\n");
				
				if(pos != string::npos){
					
					readbytes = (pos + 7);
					
					this->end = true;
				
				} else readbytes = size;
				
				this->body.append(buffer, readbytes);
				
				if(this->end){
					
					string body;
					
					size_t offset = 0;
					
					size_t len = this->body.size();
					
					const char * bodyBuffer = this->body.data();
					
					while(true){
						
						string data = string(bodyBuffer + offset, len - offset);
						
						size_t pos = data.find("\r\n");
						
						if(pos != string::npos){
							
							size_t ichunk = 0;
							
							string hsize(data, 0, pos);
							
							if(!hsize.empty()){
								
								if(hsize.compare("0") == 0) break;
								
								stringstream stream;
								
								stream << hsize;
								
								stream >> std::hex >> ichunk;
								
								offset += hsize.length() + 2;
								
								body.append(data, pos + 2, ichunk);
								
								if(!this->intGzip){
									
									Chunk chunk(bodyBuffer + offset, ichunk);
									
									this->chunks.push_back(chunk);
								}
								
								offset += ichunk;
								
								if(offset > len) break;
							
							} else offset += 2;
						
						} else break;
					}
					
					if(!this->extGzip) this->rody = body;
					
					else this->rody = decompress_gzip(body);
					
					if(!this->extGzip && this->intGzip){
						
						string data = compress_gzip(body);
						
						if(!data.empty()) this->body = data;
						
						else {
							
							this->body = body;
							
							this->intGzip = false;
						}
						
						createChunk(this->body.data(), this->body.size());
					
					} else this->body = body;
				}
			} break;
			
			default: {
				
				
				if(strict && (size < length)) return 0;
				
				size_t csize = this->body.size();
				
				size_t copySize = (size > length ? length : size);
				
				if((copySize + csize) > length) copySize = length - csize;
				
				readbytes = copySize;
				
				this->body.append(buffer, readbytes);
				
				if(length == this->body.size()) this->end = true;
				
				if(this->end){
					
					if(!this->extGzip) this->rody = this->body;
					
					else this->rody = decompress_gzip(this->body);
					
					if(!this->intGzip) createChunk(this->body.data(), length);
					
					else if(!this->extGzip && this->intGzip){
						
						string data = compress_gzip(this->body);
						
						if(!data.empty()) this->body = data;
						
						else this->intGzip = false;
						
						createChunk(this->body.data(), this->body.size());
					}
				}
			}
		}
	}
	
	return readbytes;
}
 
const u_char * HttpData::HttpBody::data(){
	
	if(isEnd()){
		
		const size_t size = this->rody.size();
		
		const u_char * map = reinterpret_cast <const u_char *> (&size);
		
		copy(map, map + sizeof(size), back_inserter(this->raw));
		
		copy(this->rody.begin(), this->rody.end(), back_inserter(this->raw));
	}
	
	return this->raw.data();
}
 
const string HttpData::HttpBody::getBody(const bool chunked){
	
	string result;
	
	if(chunked){
		
		for(auto it = this->chunks.begin(); it != this->chunks.end(); it++){
			
			string chunksize = (it->hsize + "\r\n");
			
			result.append(chunksize);
			
			result.append(it->content);
			
			result.append("\r\n");
		}
		
		result.append("0\r\n\r\n");
	
	} else result = this->body;
	
	return result;
}
 
const string HttpData::HttpBody::getRawBody(){
	
	return this->rody;
}
 
vector <HttpData::HttpBody::Chunk> HttpData::HttpBody::getChunks(){
	
	return this->chunks;
}
 
HttpData::HttpBody::HttpBody(const size_t chunkSize, const u_int levelGzip, const bool intGzip, const bool extGzip){
	
	clear();
	
	this->chunkSize = chunkSize;
	
	this->levelGzip = levelGzip;
	
	this->intGzip = intGzip;
	this->extGzip = extGzip;
}
 
HttpData::HttpBody::~HttpBody(){
	
	clear();
	
	vector <u_char> ().swap(this->raw);
	
	vector <Chunk> ().swap(this->chunks);
}
 
void HttpData::genDataConnect(){
	
	smatch match;
	
	regex e("\\b([A-Za-z]+)\\s+([\\s\\S]+)\\s+([A-Za-z]+)\\/([\\d\\.]+)", regex::ECMAScript | regex::icase);
	
	regex_search(this->http, match, e);
	
	if(!match.empty()){
		
		this->method = Anyks::toCase(match[1].str());
		
		this->path = match[2].str();
		
		this->protocol = Anyks::toCase(match[3].str());
		
		this->version = match[4].str();
		
		string host = getHeader("host");
		
		string auth = getHeader("proxy-authorization");
		
		string proxy_connection = getHeader("proxy-connection");
		
		if((this->options & OPT_SMART) && !proxy_connection.empty())
			
			setHeader("Connection", proxy_connection);
		
		if(!host.empty()){
			
			Connect gcon = getConnection(this->path);
			
			Connect scon = getConnection(host);
			
			if((scon.port.compare(gcon.port) != 0)
			&& (gcon.port.compare("80") == 0)){
				
				this->protocol = scon.protocol;
				
				this->port = scon.port;
			
			} else if(Anyks::checkPort(gcon.port)) {
				
				this->protocol = gcon.protocol;
				
				this->port = gcon.port;
			
			} else {
				
				this->protocol = "http";
				
				this->port = "80";
			}
			
			string fulladdr1 = scon.protocol + string("://") + scon.host;
			string fulladdr2 = fulladdr1 + "/";
			string fulladdr3 = fulladdr1 + string(":") + scon.port;
			string fulladdr4 = fulladdr3 + "/";
			
			if(((this->protocol.compare("http") == 0)
			|| (this->protocol.compare("https") == 0))
			&& ((Anyks::toCase(this->method).compare("connect") != 0)
			&& ((Anyks::toCase(this->path).compare(Anyks::toCase(host)) == 0)
			|| (Anyks::toCase(this->path).compare(Anyks::toCase(fulladdr1)) == 0)
			|| (Anyks::toCase(this->path).compare(Anyks::toCase(fulladdr2)) == 0)
			|| (Anyks::toCase(this->path).compare(Anyks::toCase(fulladdr3)) == 0)
			|| (Anyks::toCase(this->path).compare(Anyks::toCase(fulladdr4)) == 0)))) this->path = "/";
			
			else this->path = gcon.path;
			
			this->host = scon.host;
		}
		
		if(!auth.empty()){
			
			regex e("\\b([A-Za-z]+)\\s+([\\s\\S]+)", regex::ECMAScript | regex::icase);
			
			regex_search(auth, match, e);
			
			if(!match.empty()){
				
				const string auth = Anyks::toCase(match[1].str());
				
				if(auth.compare("basic") == 0) this->auth = AMING_AUTH_BASIC;
				else if(auth.compare("bearer") == 0) this->auth = AMING_AUTH_BEARER;
				else if(auth.compare("digest") == 0) this->auth = AMING_AUTH_DIGEST;
				else if(auth.compare("hmac") == 0) this->auth = AMING_AUTH_HMAC;
				else if(auth.compare("hoba") == 0) this->auth = AMING_AUTH_HOBA;
				else if(auth.compare("mutual") == 0) this->auth = AMING_AUTH_MUTUAL;
				else if(auth.compare("aws4-hmac-sha256") == 0) this->auth = AMING_AUTH_AWS4HMACSHA256;
				
				if(this->auth == AMING_AUTH_BASIC){
					
					Base64 base64;
					
					const string dauth = base64.decode(match[2].str());
					
					regex e("\\b([\\s\\S]+)\\:([\\s\\S]+)", regex::ECMAScript | regex::icase);
					
					regex_search(dauth, match, e);
					
					if(!match.empty()){
						
						this->login = match[1].str();
						
						this->password = match[2].str();
					}
				
				} else this->login = match[2].str();
			}
		}
	}
}
 
void HttpData::createRequest(const u_short code){
	
	if(this->response.count(code) > 0){
		
		setStatus(code);
		
		this->body.clear();
		
		this->version = "1.1";
		
		const auto temp = this->response[code];
		
		this->http = Anyks::strFormat("HTTP/%s %u %s", this->version.c_str(), temp.code, temp.text.c_str());
		
		string headers = (this->http + "\r\n");
		
		headers.append(temp.headers);
		
		this->headers.create(headers.c_str());
		
		if(!temp.body.empty()){
			
			this->body.addData(temp.body.c_str(), temp.body.size(), temp.body.size());
		}
	}
}
 
const string HttpData::createHeadResponse(){
	
	bool smart = (this->options & OPT_SMART);
	
	bool agent = (this->options & OPT_AGENT);
	
	bool via = false;
	
	string response = (this->http + string("\r\n"));
	
	string connection = getHeader("connection");
	
	
	
	for(auto it = this->headers.cbegin(); it != this->headers.cend(); ++it){
		
		string head = it->head;
		
		string value = it->value;
		
		if(agent && (Anyks::toCase(head).compare("via") == 0)){
			
			value += (string(", ") + this->version + string(" ")
			+ this->appName + string(" (") + string(APP_NAME)
			+ string("/") + this->appVersion + string(")"));
			
			via = true;
		}
		
		if((Anyks::toCase(head).compare("connection") != 0)
		&& (Anyks::toCase(head).compare("proxy-authorization") != 0)
		&& (!smart || (smart && (Anyks::toCase(head).compare("proxy-connection") != 0)))){
			
			response.append(head + string(": ") + value + string("\r\n"));
		}
	}
	
	if(agent){
		
		response.append(string("Proxy-Agent: ") + this->appName + string(" (")
		+ string(APP_NAME) + string("/") + this->appVersion + string(")\r\n"));
		
		if(!via) response.append(string("Via: ") + this->version + string(" ")
		+ this->appName + string(" (") + string(APP_NAME)
		+ string("/") + this->appVersion + string(")\r\n"));
	}
	
	const string proxy_connection = getHeader("proxy-connection");
	
	if(!smart && proxy_connection.empty()) response.append(string("Proxy-Connection: ") + connection + string("\r\n"));
	
	else if(smart && !proxy_connection.empty()) connection = proxy_connection;
	
	if(!connection.empty()) response.append(string("Connection: ") + connection + string("\r\n"));
	
	response.append(string("\r\n"));
	
	return response;
}
 
const string HttpData::createHeadRequest(){
	
	bool smart = (this->options & OPT_SMART);
	
	bool gzip = (this->options & OPT_GZIP);
	
	bool agent = (this->options & OPT_AGENT);
	
	bool keepalive = (this->options & OPT_KEEPALIVE);
	
	bool deblock = (this->options & OPT_DEBLOCK);
	
	bool via = false;
	
	string request = (
		Anyks::toCase(this->method, true)
		+ string(" ") + this->path
		+ string(" ") + string("HTTP/")
		+ this->version + string("\r\n")
	);
	
	if(deblock){
		
		request.append(
			string("Host: ") + this->host
			+ string(":") + this->port + string("\r\n")
		);
	
	} else request.append(string("Host: ") + this->host + string("\r\n"));
	
	string useragent = getHeader("user-agent");
	
	if(!useragent.empty()) request.append(string("User-Agent: ") + useragent + string("\r\n"));
	
	for(auto it = this->headers.cbegin(); it != this->headers.cend(); ++it){
		
		string head = it->head;
		
		string value = it->value;
		
		if(agent && (Anyks::toCase(head).compare("via") == 0)){
			
			value += (string(", ") + this->version + string(" ")
			+ this->appName + string(" (") + string(APP_NAME)
			+ string("/") + this->appVersion + string(")"));
			
			via = true;
		}
		
		if((Anyks::toCase(head).compare("host") != 0)
		&& (Anyks::toCase(head).compare("user-agent") != 0)
		&& (Anyks::toCase(head).compare("connection") != 0)
		&& (Anyks::toCase(head).compare("proxy-authorization") != 0)
		&& (gzip || (!gzip && (Anyks::toCase(head).compare("accept-encoding") != 0)))
		&& (!smart || (smart && (Anyks::toCase(head).compare("proxy-connection") != 0)))){
			
			request.append(head + string(": ") + value + string("\r\n"));
		}
	}
	
	if(agent){
		
		request.append(string("Proxy-Agent: ") + this->appName + string(" (")
		+ string(APP_NAME) + string("/") + this->appVersion + string(")\r\n"));
		
		if(!via) request.append(string("Via: ") + this->version + string(" ")
		+ this->appName + string(" (") + string(APP_NAME)
		+ string("/") + this->appVersion + string(")\r\n"));
	}
	
	if(!keepalive) setHeader("Connection", "close");
	
	string connection = getHeader("connection");
	
	if(!connection.empty()){
		
		request.append(string("Connection: ") + connection + string("\r\n"));
		
		if(!smart){
			
			string pc = getHeader("proxy-connection");
			
			if(pc.empty()) request.append(string("Proxy-Connection: ") + connection + string("\r\n"));
		}
	}
	
	request.append(string("\r\n"));
	
	return request;
}
 
const string HttpData::modifyHeaderString(const string str){
	
	string headers = str;
	
	if(!headers.empty()){
		
		string connection;
		
		bool smart = (this->options & OPT_SMART);
		
		bool agent = (this->options & OPT_AGENT);
		
		if(agent){
			
			smatch match;
			
			regex e("\\r\\nVia\\s*\\:\\s*([^\\r\\n]+)\\r\\n", regex::ECMAScript | regex::icase);
			
			regex_search(headers, match, e);
			
			string via, proxy;
			
			if(!match.empty()){
				
				via = match[1].str();
				
				via.append(string(", ") + this->version + string(" ")
				+ this->appName + string(" (") + string(APP_NAME)
				+ string("/") + this->appVersion + string(")"));
			
			} else {
				
				via.append(this->version + string(" ")
				+ this->appName + string(" (") + string(APP_NAME)
				+ string("/") + this->appVersion + string(")"));
			}
			
			proxy.append(this->appName + string(" (")
			+ string(APP_NAME) + string("/") + this->appVersion + string(")"));
			
			addHeaderToString("Via", via, headers);
			
			addHeaderToString("Proxy-Agent", proxy, headers);
		}
		
		smatch match;
		
		regex e("\\r\\nConnection\\s*\\:\\s*([^\\r\\n]+)\\r\\n", regex::ECMAScript | regex::icase);
		
		regex_search(headers, match, e);
		
		if(!match.empty()) connection = match[1].str();
		
		if(!smart) addHeaderToString("Proxy-Connection", connection, headers);
		
		else {
			
			regex e("\\r\\nProxy-Connection\\s*\\:\\s*([^\\r\\n]+)\\r\\n", regex::ECMAScript | regex::icase);
			
			regex_search(headers, match, e);
			
			if(!match.empty()) connection = match[1].str();
		}
		
		if(!connection.empty()) addHeaderToString("Connection", connection, headers);
	}
	
	return headers;
}
 
HttpData::Connect HttpData::getConnection(const string str){
	
	Connect data;
	
	smatch match;
	
	string query = str;
	
	regex ep("\\b([A-Za-z]+):\\/{2}", regex::ECMAScript | regex::icase);
	
	regex_search(query, match, ep);
	
	if(!match.empty()) data.protocol = Anyks::toCase(match[1].str());
	
	regex eh(
		
		"([\\w\\.\\-]+\\.[\\w\\-]+|"
		
		"\\d{1,3}(?:\\.\\d{1,3}){3}|"
		
		"\\[[A-Fa-f\\d\\:]{2,39}\\])(?:\\:(\\d+))?",
		regex::ECMAScript | regex::icase
	);
	
	regex_search(str, match, eh);
	
	if(!match.empty()){
		
		data.host = Anyks::toCase(match[1].str());
		
		if(match.size() == 3) data.port = match[2].str();
	}
	
	if(data.port.empty() && ((data.protocol.compare("https") == 0)
	|| (this->method.compare("connect") == 0))) data.port = "443";
	
	else if(data.port.empty()) data.port = "80";
	
	if(data.protocol.empty()){
		
		if(data.port.compare("443") == 0) data.protocol = "https";
		
		else data.protocol = "http";
	}
	
	regex ed("^(?:http[s]?\\:\\/\\/)?(?:[\\w\\-\\.]+\\.[\\w\\-]+)(?:\\:\\d+)?(\\/)", regex::ECMAScript | regex::icase);
	
	data.path = regex_replace(str, ed, "$1");
	
	return data;
}
 
const bool HttpData::isIntGzip(){
	
	return this->intGzip;
}
 
const bool HttpData::isExtGzip(){
	
	return this->extGzip;
}
 
const bool HttpData::isUpgrade(){
	
	return (!getHeader("upgrade").empty()
	&& (Anyks::toCase(getHeader("connection")).find("upgrade") != string::npos));
}
 
const bool HttpData::isRedirect(){
	
	const u_int status = getStatus();
	
	return (((status > 299) && (status < 400)) && !getLocation().empty());
}
 
const bool HttpData::isConnect(){
	
	return (this->method.compare("connect") == 0);
}
 
const bool HttpData::isClose(){
	
	return (Anyks::toCase(getHeader("connection")).find("close") != string::npos);
}
 
const bool HttpData::isHttps(){
	
	return (this->protocol.compare("https") == 0);
}
 
const bool HttpData::isAlive(){
	
	string connection = Anyks::toCase(getHeader("connection"));
	
	if(getVersion() > 1){
		
		return (connection.find("close") == string::npos);
	
	} else return (connection.find("keep-alive") != string::npos);
}
 
const bool HttpData::isEmpty(){
	
	return this->headers.empty();
}
 
const bool HttpData::isEndHeaders(){
	
	return this->headers.isEnd();
}
 
const bool HttpData::isEndBody(){
	
	return this->body.isEnd();
}
 
const bool HttpData::compressIsAllowed(const string userAgent){
	
	bool gzip = false;
	
	if(this->gzipParams
	&& (this->options & OPT_PGZIP)
	&& isEndHeaders()
	&& getHeader("content-encoding").empty()){
		
		const string method = getMethod();
		
		if((method.compare("get") != 0)
		&& (method.compare("post") != 0)) return gzip;
		
		if(!isIntGzip()){
			
			const u_int status = getStatus();
			
			if(((status > 99) && (status < 200))
			|| ((status > 203) && (status < 400))) return gzip;
			
			const string clength = getHeader("content-length");
			
			const string chunked = getHeader("transfer-encoding");
			
			size_t contentLength = (!clength.empty() ? ::atoi(clength.c_str()) : 0);
			
			if(!contentLength && (!chunked.empty()
			&& (chunked.find("chunked") != string::npos))) contentLength = 2;
			
			else if(!contentLength && isClose()) contentLength = 1;
			
			if(!contentLength) return gzip;
			
			smatch match;
			
			gzip = true;
			
			string via = getHeader("via");
			
			string cmime = getHeader("content-type");
			
			regex e(this->gzipParams->regex, regex::ECMAScript | regex::icase);
			
			for(auto it = this->gzipParams->vhttp.begin(); it != this->gzipParams->vhttp.end(); it++){
				
				if(getVersion() == float(::atof(it->c_str()))){
					
					gzip = true;
					
					break;
				
				} else gzip = false;
			}
			
			if(gzip){
				
				for(auto it = this->gzipParams->types.begin(); it != this->gzipParams->types.end(); it++){
					
					if((it->compare("*") == 0) || (cmime.find(* it) != string::npos)){
						
						gzip = true;
						
						break;
					
					} else gzip = false;
				}
				
				if(gzip && !via.empty()){
					
					
					gzip = false;
					
					for(auto it = this->gzipParams->proxied.begin(); it != this->gzipParams->proxied.end(); it++){
						
						const string param = * it;
						
						if(param.compare("off") == 0){
							
							gzip = false;
							
							break;
						
						} else if(param.compare("any") == 0) {
							
							gzip = true;
							
							break;
						
						} else if(param.compare("expired") == 0) {
							
							if(!getHeader("expires").empty()) gzip = true;
							
							else {
								
								gzip = false;
								
								break;
							}
						
						} else if(param.compare("no-cache") == 0) {
							
							string cc = getHeader("cache-control");
							
							if(!cc.empty() && (Anyks::toCase(cc).find(param) != string::npos)) gzip = true;
							
							else {
								
								gzip = false;
								
								break;
							}
						
						} else if(param.compare("no-store") == 0) {
							
							string cc = getHeader("cache-control");
							
							if(!cc.empty() && (Anyks::toCase(cc).find(param) != string::npos)) gzip = true;
							
							else {
								
								gzip = false;
								
								break;
							}
						
						} else if(param.compare("private") == 0) {
							
							string cc = getHeader("cache-control");
							
							if(!cc.empty() && (Anyks::toCase(cc).find(param) != string::npos)) gzip = true;
							
							else {
								
								gzip = false;
								
								break;
							}
						
						} else if(param.compare("no_last_modified") == 0) {
							
							if(getHeader("last-modified").empty()) gzip = true;
							
							else {
								
								gzip = false;
								
								break;
							}
						
						} else if(param.compare("no_etag") == 0) {
							
							if(getHeader("etag").empty()) gzip = true;
							
							else {
								
								gzip = false;
								
								break;
							}
						
						} else if(param.compare("auth") == 0) {
							
							if(!getHeader("authorization").empty()) gzip = true;
							
							else {
								
								gzip = false;
								
								break;
							}
						}
					}
				}
				
				if(gzip && (contentLength > 2)){
					
					if(this->gzipParams->length >= contentLength) gzip = false;
				}
				
				if(gzip && !userAgent.empty()){
					
					regex_search(userAgent, match, e);
					
					if(!match.empty()) gzip = false;
				}
				
				if(gzip && this->gzipParams->vary){
					
					string vary = getHeader("vary");
					
					if(!vary.empty() && (Anyks::toCase(vary)
					.find("accept-encoding") != string::npos)) rmHeader("vary");
				}
			}
		}
	}
	
	return gzip;
}
 
const bool HttpData::rmHeaderInString(const string header, string &headers){
	
	bool result = false;
	
	if(!headers.empty() && !header.empty()){
		
		smatch match;
		
		regex e(
			"(?:(?:(?:" PROXY_HTTP_METHODS ")"
			"\\s+[^\\r\\n\\s]+\\s+[A-Za-z]+\\/[\\d\\.]+\\r\\n)|(?:[A-Za-z]+"
			"\\/[\\d\\.]+\\s+\\d+(?:\\s+[^\\r\\n]+)?\\r\\n))(?:[\\w\\-]+\\s*\\:"
			"\\s*[^\\r\\n]+\\r\\n)+\\r\\n",
			regex::ECMAScript | regex::icase
		);
		
		regex_search(headers, match, e);
		
		if(!match.empty()){
			
			string headers1 = match[0].str(), headers2 = match[0].str();
			
			regex he("[^\\w\\-]+", regex::ECMAScript | regex::icase);
			
			const string head = regex_replace(header, he, "");
			
			regex re(head + string("\\s*\\:\\s*[^\\r\\n]+\\r\\n"), regex::ECMAScript | regex::icase);
			
			headers2 = regex_replace(headers2, re, "");
			
			size_t pos = headers.find(headers1);
			
			if(pos != string::npos) headers.replace(pos, headers1.length(), headers2);
			
			result = (headers1.compare(headers2) != 0);
		}
	}
	
	return result;
}
 
const bool HttpData::addHeaderToString(const string header, const string value, string &headers){
	
	bool result = false;
	
	if(!headers.empty() && !value.empty() && !header.empty()){
		
		smatch match;
		
		regex e(
			"(?:(?:(?:" PROXY_HTTP_METHODS ")"
			"\\s+[^\\r\\n\\s]+\\s+[A-Za-z]+\\/[\\d\\.]+\\r\\n)|(?:[A-Za-z]+"
			"\\/[\\d\\.]+\\s+\\d+(?:\\s+[^\\r\\n]+)?\\r\\n))(?:[\\w\\-]+\\s*\\:"
			"\\s*[^\\r\\n]+\\r\\n)+\\r\\n",
			regex::ECMAScript | regex::icase
		);
		
		regex_search(headers, match, e);
		
		if(!match.empty()){
			
			string headers1 = match[0].str(), headers2 = match[0].str();
			
			regex he("[^\\w\\-]+", regex::ECMAScript | regex::icase);
			
			regex ae("\\r\\n\\r\\n", regex::ECMAScript | regex::icase);
			
			const string head = regex_replace(header, he, "");
			
			regex re(head + string("\\s*\\:\\s*[^\\r\\n]+\\r\\n"), regex::ECMAScript | regex::icase);
			
			headers2 = regex_replace(headers2, re, "");
			
			headers2 = regex_replace(headers2, ae, (string("\r\n") + head + string(": ") + value + string("\r\n\r\n")));
			
			size_t pos = headers.find(headers1);
			
			if(pos != string::npos) headers.replace(pos, headers1.length(), headers2);
			
			result = (headers1.compare(headers2) != 0);
		}
	}
	
	return result;
}
 
const bool HttpData::setRedirect(HttpData &response){
	
	bool result = false;
	
	if(response.isEndHeaders()){
		
		const u_int status = response.getStatus();
		
		const string request = response.getLocation();
		
		if(((status > 299) && (status < 400)) && !request.empty()){
			
			Connect connect = getConnection(request);
			
			if(connect.protocol.compare("http") == 0){
				
				const string host = getHost();
				
				const u_int port = getPort();
				
				const u_int cport = ::atoi(connect.port.c_str());
				
				string referer = getProtocol();
				
				if((host.compare(connect.host) != 0)
				|| (port != cport)
				|| (referer.compare(connect.protocol) != 0)){
					
					referer.append("://");
					
					referer.append(host);
					
					if(port != 80) referer.append(string(":") + to_string(port));
					
					referer.append(getPath());
					
					setHeader("Referer", referer);
				
				} else setHeader("Referer", getPath());
				
				setPort(cport);
				
				setPath(connect.path);
				
				setProtocol(connect.protocol);
				
				if(connect.path.find(connect.host) == string::npos){
					
					setHost(connect.host);
				}
				
				result = true;
			}
		}
	}
	
	return result;
}
 
const size_t HttpData::parse(const char * buffer, const size_t size, const u_short options, Config * config, LogApp * log){
	
	size_t maxsize = 0;
	
	if(size && (config != nullptr)){
		
		smatch match;
		
		string str(buffer, size);
		
		regex e(
			"^([^\\r\\n\\s]*)((?:(?:(?:" PROXY_HTTP_METHODS ")"
			"\\s+[^\\r\\n\\s]+\\s+[A-Za-z]+\\/[\\d\\.]+\\r\\n)|(?:[A-Za-z]+"
			"\\/[\\d\\.]+\\s+\\d+(?:\\s+[^\\r\\n]+)?\\r\\n))(?:[\\w\\-]+\\s*\\:"
			"\\s*[^\\r\\n]+\\r\\n)+\\r\\n)",
			regex::ECMAScript | regex::icase
		);
		
		regex_search(str, match, e);
		
		if(!match.empty()){
			
			string badchars = match[1].str();
			
			maxsize += badchars.size();
			
			string headers = match[2].str();
			
			maxsize += headers.size();
			
			create(options, config, log);
			
			setData(headers.c_str(), headers.size());
			
			size_t sizeBody = setEntitybody(buffer + maxsize, size - maxsize);
			
			if(!sizeBody || isEndBody()) maxsize += sizeBody;
			
			else maxsize = 0;
		}
	}
	
	return maxsize;
}
 
const size_t HttpData::getBodySize(){
	
	return this->body.getBodySize();
}
 
const size_t HttpData::getRawBodySize(){
	
	return this->body.getRawBody().size();
}
 
const size_t HttpData::setEntitybody(const char * buffer, const size_t size){
	
	if(size && isEndHeaders()){
		
		size_t length = 0;
		
		string cl = getHeader("content-length");
		
		if(!cl.empty() && Anyks::isNumber(cl)) length = ::atoi(cl.c_str());
		
		else {
			
			string ch = getHeader("transfer-encoding");
			
			if(!ch.empty() && (ch.find("chunked") != string::npos)) length = 2;
			
			else if(isClose()){
				
				smatch match;
				
				regex e(
					"^(?:" PROXY_HTTP_METHODS ")"
					"\\s+[^\\r\\n\\s]+\\s+[A-Za-z]+\\/[\\d\\.]+",
					regex::ECMAScript | regex::icase
				);
				
				regex_search(this->http, match, e);
				
				if(match.empty()) length = 1;
			}
		}
		
		return this->body.addData(buffer, size, length);
	}
	
	return size;
}
 
const size_t HttpData::size(){
	
	if(this->raw.empty()) data();
	
	return this->raw.size();
}
 
const u_int HttpData::getPort(){
	
	return (!this->port.empty() ? ::atoi(this->port.c_str()) : 80);
}
 
const u_int HttpData::getStatus(){
	
	return this->status;
}
 
const u_short HttpData::getAuth(){
	
	return this->auth;
}
 
const float HttpData::getVersion(){
	
	return (!this->version.empty() ? ::atof(this->version.c_str()) : 1.0);
}
 
const string HttpData::getHttp(){
	
	return this->http;
}
 
const string HttpData::getMethod(){
	
	return this->method;
}
 
const string HttpData::getHost(){
	
	return this->host;
}
 
const string HttpData::getPath(){
	
	return this->path;
}
 
const string HttpData::getProtocol(){
	
	return this->protocol;
}
 
const string HttpData::getLogin(){
	
	return this->login;
}
 
const string HttpData::getPassword(){
	
	return this->password;
}
 
const string HttpData::getUseragent(){
	
	return getHeader("user-agent");
}
 
const string HttpData::getLocation(){
	
	return getHeader("location");
}
 
const string HttpData::getHeader(const string key){
	
	return this->headers.getHeader(key).value;
}
 
const string HttpData::getResponseHeaders(){
	
	return createHeadResponse();
}
 
const string HttpData::getRequestHeaders(){
	
	return createHeadRequest();
}
 
const string HttpData::getBody(const bool chunked){
	
	string body;
	
	if(isEndBody()){
		
		if(!this->body.isIntCompress()
		&& !this->body.isExtCompress()) this->unsetGzip();
		
		if(chunked){
			
			rmHeader("content-length");
			
			setHeader("Transfer-Encoding", "chunked");
		
		} else {
			
			rmHeader("transfer-encoding");
			
			setHeader("Content-Length", to_string(getBodySize()));
		}
		
		body = this->body.getBody(chunked);
	}
	
	return body;
}
 
const string HttpData::getResponseData(const bool chunked){
	
	string result;
	
	string body = getBody(chunked);
	
	string headers = getResponseHeaders();
	
	if(!headers.empty()){
		
		result.append(headers);
		
		if(!body.empty() && isEndBody()) result.append(body);
		
		else if(!body.empty()) result.clear();
	}
	
	return result;
}
 
const string HttpData::getRequestData(){
	
	string result;
	
	string body = getBody(false);
	
	string headers = getRequestHeaders();
	
	if(!headers.empty()){
		
		result.append(headers);
		
		if(!body.empty() && isEndBody()) result.append(body);
		
		else if(!body.empty()) result.clear();
	}
	
	return result;
}
 
const string HttpData::getRawResponseData(){
	
	string result;
	
	if(isEndBody()){
		
		string body = this->body.getRawBody();
		
		rmHeader("transfer-encoding");
		
		setHeader("Content-Length", to_string(body.size()));
		
		string headers = getResponseHeaders();
		
		regex ce("Content\\-Encoding\\s*\\:[^\\r\\n]*gzip[^\\r\\n]*\\r\\n", regex::ECMAScript | regex::icase);
		
		headers = regex_replace(headers, ce, "");
		
		regex et("ETag\\s*\\:\\s*W\\/", regex::ECMAScript | regex::icase);
		
		headers = regex_replace(headers, et, "ETag: ");
		
		if(!headers.empty()){
			
			result.append(headers);
			
			if(!body.empty()) result.append(body);
		}
	}
	
	return result;
}
 
const string HttpData::getRawRequestData(){
	
	string result;
	
	if(isEndBody()){
		
		string body = this->body.getRawBody();
		
		rmHeader("transfer-encoding");
		
		setHeader("Content-Length", to_string(body.size()));
		
		string headers = getRequestHeaders();
		
		regex ce("Content\\-Encoding\\s*\\:[^\\r\\n]*gzip[^\\r\\n]*\\r\\n", regex::ECMAScript | regex::icase);
		
		headers = regex_replace(headers, ce, "");
		
		regex et("ETag\\s*\\:\\s*W\\/", regex::ECMAScript | regex::icase);
		
		headers = regex_replace(headers, et, "ETag: ");
		
		if(!headers.empty()){
			
			result.append(headers);
			
			if(!body.empty()) result.append(body);
		}
	}
	
	return result;
}
 
const u_char * HttpData::data(){
	
	if(isEndBody() && isEndHeaders()){
		
		Dump sizes = {
			sizeof(this->status),
			sizeof(this->options),
			sizeof(this->auth),
			this->http.size(),
			this->path.size(),
			this->host.size(),
			this->port.size(),
			this->login.size(),
			this->method.size(),
			this->appName.size(),
			this->version.size(),
			this->protocol.size(),
			this->password.size(),
			this->appVersion.size(),
			this->headers.size(),
			this->body.size()
		};
		
		const size_t size = sizeof(sizes);
		
		const u_char * map = reinterpret_cast <const u_char *> (&sizes);
		
		copy(map, map + size, back_inserter(this->raw));
		
		const u_char * status = reinterpret_cast <const u_char *> (&this->status);
		
		copy(status, status + sizes.status, back_inserter(this->raw));
		
		const u_char * options = reinterpret_cast <const u_char *> (&this->options);
		
		copy(options, options + sizes.options, back_inserter(this->raw));
		
		const u_char * auth = reinterpret_cast <const u_char *> (&this->auth);
		
		copy(auth, auth + sizes.auth, back_inserter(this->raw));
		
		copy(this->http.begin(), this->http.end(), back_inserter(this->raw));
		
		copy(this->path.begin(), this->path.end(), back_inserter(this->raw));
		
		copy(this->host.begin(), this->host.end(), back_inserter(this->raw));
		
		copy(this->port.begin(), this->port.end(), back_inserter(this->raw));
		
		copy(this->login.begin(), this->login.end(), back_inserter(this->raw));
		
		copy(this->method.begin(), this->method.end(), back_inserter(this->raw));
		
		copy(this->appName.begin(), this->appName.end(), back_inserter(this->raw));
		
		copy(this->version.begin(), this->version.end(), back_inserter(this->raw));
		
		copy(this->protocol.begin(), this->protocol.end(), back_inserter(this->raw));
		
		copy(this->password.begin(), this->password.end(), back_inserter(this->raw));
		
		copy(this->appVersion.begin(), this->appVersion.end(), back_inserter(this->raw));
		
		const u_char * headers = this->headers.data();
		
		copy(headers, headers + sizes.headers, back_inserter(this->raw));
		
		const u_char * body = this->body.data();
		
		copy(body, body + sizes.body, back_inserter(this->raw));
	}
	
	return this->raw.data();
}
 
void HttpData::set(const u_char * data, size_t size){
	
	if(size){
		
		size_t size_map = sizeof(Dump);
		
		if(size_map < size){
			
			clear();
			
			size_t size_it = size_map;
			
			for(size_t i = 0, j = 0; i < size_map; i += sizeof(size_t), j++){
				
				size_t size_data;
				
				memcpy(&size_data, data + i, sizeof(size_t));
				
				if(size_data && ((size_data + size_it) <= size)){
					
					switch(j){
						
						case 0: Anyks::cpydata(data, size_data, size_it, &this->status); break;
						
						case 1: Anyks::cpydata(data, size_data, size_it, &this->options); break;
						
						case 2: Anyks::cpydata(data, size_data, size_it, &this->auth); break;
						
						case 3: Anyks::cpydata(data, size_data, size_it, this->http); break;
						
						case 4: Anyks::cpydata(data, size_data, size_it, this->path); break;
						
						case 5: Anyks::cpydata(data, size_data, size_it, this->host); break;
						
						case 6: Anyks::cpydata(data, size_data, size_it, this->port); break;
						
						case 7: Anyks::cpydata(data, size_data, size_it, this->login); break;
						
						case 8: Anyks::cpydata(data, size_data, size_it, this->method); break;
						
						case 9: Anyks::cpydata(data, size_data, size_it, this->appName); break;
						
						case 10: Anyks::cpydata(data, size_data, size_it, this->version); break;
						
						case 11: Anyks::cpydata(data, size_data, size_it, this->protocol); break;
						
						case 12: Anyks::cpydata(data, size_data, size_it, this->password); break;
						
						case 13: Anyks::cpydata(data, size_data, size_it, this->appVersion); break;
						
						case 14: {
							
							u_char * buffer = new u_char [size_data];
							
							memcpy(buffer, data + size_it, size_data);
							
							this->headers.set(buffer, size_data);
							
							size_it += size_data;
							
							delete [] buffer;
						} break;
						
						case 15: {
							
							u_char * buffer = new u_char [size_data];
							
							memcpy(buffer, data + size_it, size_data);
							
							
							setHeader("Content-Length", to_string(size_data));
							
							if(compressIsAllowed()) setGzip();
							
							initBody();
							
							this->body.set(buffer, size_data);
							
							size_it += size_data;
							
							delete [] buffer;
						} break;
					}
				}
			}
		}
	}
}
 
void HttpData::clear(){
	
	this->auth = AMING_AUTH_BASIC;
	
	this->status = 0;
	
	this->intGzip = false;
	this->extGzip = false;
	this->http.clear();
	this->method.clear();
	this->path.clear();
	this->protocol.clear();
	this->version.clear();
	this->host.clear();
	this->port.clear();
	this->login.clear();
	this->password.clear();
	
	this->headers.clear();
	
	this->body.clear();
	
	this->raw.clear();
}
 
void HttpData::initBody(){
	
	int level = (this->gzipParams ? this->gzipParams->level : Z_DEFAULT_COMPRESSION);
	
	size_t chunk = (this->gzipParams ? this->gzipParams->chunk : MOD_GZIP_ZLIB_CHUNK);
	
	HttpBody body = HttpBody(chunk, level, this->intGzip, this->extGzip);
	
	this->body = body;
}
 
void HttpData::rmHeader(const string key){
	
	if(!key.empty()) this->headers.remove(key);
}
 
void HttpData::unsetGzip(){
	
	if(this->intGzip || this->extGzip){
		
		string etag = this->getHeader("etag");
		
		if(!etag.empty() && (etag.find("W/") != string::npos)){
			
			this->setHeader("ETag", etag.replace(0, 2, ""));
		}
		
		this->rmHeader("content-encoding");
		
		this->intGzip = false;
		this->extGzip = false;
	}
}
 
void HttpData::setGzip(const bool intGzip, const bool extGzip){
	
	if(intGzip || extGzip){
		
		string etag = this->getHeader("etag");
		
		if(!etag.empty() && (etag.find("W/") == string::npos)){
			
			this->setHeader("ETag", string("W/") + etag);
		}
		
		this->setHeader("Content-Encoding", "gzip");
		
		this->rmHeader("accept-ranges");
		
		this->intGzip = intGzip;
		this->extGzip = extGzip;
	}
}
 
void HttpData::setBodyEnd(){
	
	this->body.setEnd();
}
 
void HttpData::setHeader(const string key, const string value){
	
	if(!key.empty() && !value.empty()){
		
		this->headers.append(key, value);
	}
}
 
void HttpData::setData(const char * buffer, const size_t size){
	
	if(size){
		
		clear();
		
		smatch match;
		
		string str(buffer, size);
		
		regex e(
			"^((?:(?:" PROXY_HTTP_METHODS ")"
			"\\s+[^\\r\\n\\s]+\\s+[A-Za-z]+\\/([\\d\\.]+)\\r\\n)|(?:[A-Za-z]+"
			"\\/([\\d\\.]+)\\s+(\\d+)(?:\\s+[^\\r\\n]+)?\\r\\n))((?:[\\w\\-]+\\s*\\:"
			"\\s*[^\\r\\n]+\\r\\n)+\\r\\n)",
			regex::ECMAScript | regex::icase
		);
		
		regex_search(str, match, e);
		
		if(!match.empty()){
			
			string version = match[2].str();
			
			if(version.empty()) version = match[3].str();
			
			this->version = version;
			
			this->http = match[1].str();
			
			this->http = Anyks::trim(this->http);
			
			this->status = ::atoi(match[4].str().c_str());
			
			this->headers.create(match[0].str().c_str());
		}
		
		genDataConnect();
	}
}
 
void HttpData::setGzipParams(void * params){
	
	if(params) this->gzipParams = reinterpret_cast <Gzip *> (params);
}
 
void HttpData::setOptions(const u_short options){
	
	this->options = options;
}
 
void HttpData::setMethod(const string str){
	
	this->method = str;
}
 
void HttpData::setHost(const string str){
	
	this->host = str;
}
 
void HttpData::setPort(const u_int number){
	
	this->port = to_string(number);
}
 
void HttpData::setStatus(const u_int number){
	
	this->status = number;
}
 
void HttpData::setAuth(const u_short auth){
	
	this->auth = auth;
}
 
void HttpData::setPath(const string str){
	
	this->path = str;
}
 
void HttpData::setProtocol(const string str){
	
	this->protocol = str;
}
 
void HttpData::setVersion(const float number){
	
	this->version = to_string(number);
	
	if(this->version.length() == 1) this->version += ".0";
}
 
void HttpData::setUseragent(const string str){
	
	setHeader("User-Agent", str);
}
 
void HttpData::setClose(){
	
	setHeader("Connection", "close");
}
 
void HttpData::addHeader(const char * buffer){
	
	if(strlen(buffer)){
		
		smatch match;
		
		regex e(
			"((?:(?:" PROXY_HTTP_METHODS ")"
			"\\s+[^\\r\\n\\s]+\\s+[A-Za-z]+\\/([\\d\\.]+))|"
			"(?:[A-Za-z]+\\/([\\d\\.]+)\\s+(\\d+)(?:\\s+[A-Za-z\\s\\-]+)?))|"
			"(?:([\\w\\-]+)\\s*\\:\\s*([^\\r\\n]+))",
			regex::ECMAScript | regex::icase
		);
		
		string str = buffer;
		
		regex_search(str, match, e);
		
		if(!match.empty()){
			
			if(match[1].str().empty()){
				
				string key = match[5].str();
				string val = match[6].str();
				
				this->headers.append(key, val);
				
				if((Anyks::toCase(key).compare("content-encoding") == 0)
				&& (val.find("gzip") != string::npos)) this->extGzip = true;
			
			} else {
				
				clear();
				
				string version = match[2].str();
				
				if(version.empty()) version = match[3].str();
				
				this->http = match[1].str();
				
				this->version = version;
				
				this->status = ::atoi(match[4].str().c_str());
			}
		}
	
	} else if(buffer) this->headers.setEnd();
}
 
void HttpData::largeRequest(){
	
	createRequest(509);
}
 
void HttpData::brokenRequest(){
	
	createRequest(510);
}
 
void HttpData::faultConnect(){
	
	createRequest(502);
}
 
void HttpData::pageNotFound(){
	
	createRequest(404);
}
 
void HttpData::faultAuth(){
	
	createRequest(403);
}
 
void HttpData::requiredAuth(){
	
	createRequest(407);
}
 
void HttpData::authSuccess(){
	
	createRequest(200);
}
 
void HttpData::create(const u_short options, Config * config, LogApp * log){
	
	if(config != nullptr){
		
		clear();
		
		this->log = log;
		
		this->config = config;
		
		this->appName = this->config->proxy.name;
		
		this->options = options;
		
		this->appVersion = APP_VERSION;
	}
}
 
HttpData::HttpData(const u_short options, Config * config, LogApp * log){
	
	if(config != nullptr){
		 
		auto redirect = [this](const u_short code, const string url){
			
			const string headers = Anyks::strFormat(
				"Proxy-Connection: close\r\n"
				"Connection: close\r\n"
				"Content-type: text/html; charset=utf-8\r\n"
				"Location: %s\r\n\r\n",
				url.c_str()
			);
			
			pair <u_short, Http> result = {code, {301, "Moved Permanently", headers, ""}};
			
			return result;
		};
		 
		auto htemplate = [this](const u_short code, const string message, const string body, const string header = ""){
			
			const string headers = Anyks::strFormat(
				"Proxy-Connection: close\r\n"
				"Connection: close\r\n"
				"Content-type: text/html; charset=utf-8\r\n"
				"%s%s\r\n",
				header.c_str(),
				(!header.empty() ? "\r\n" : "")
			);
			
			pair <u_short, Http> result = {code, {code, message, headers, body + "\r\n"}};
			
			return result;
		};
		
		ACodes templates(config, log);
		
		auto t400 = templates.get(400);
		auto t403 = templates.get(403);
		auto t404 = templates.get(404);
		auto t407 = templates.get(407);
		auto t500 = templates.get(500);
		auto t502 = templates.get(502);
		auto t503 = templates.get(503);
		auto t509 = templates.get(509);
		auto t510 = templates.get(510);
		
		this->response = {
			{100, {100, "Continue", "\r\n", ""}},
			{200, {200, "Connection established", "\r\n", "OK"}}
		};
		
		const string authMessage = Anyks::strFormat("Proxy-Authenticate: Basic realm=\"%s: Please enter your password\"", config->proxy.name.c_str());
		
		this->response.insert(t400.type == AMING_HTTP_ADDRESS ? redirect(t400.code, t400.data) : htemplate(t400.code, "Bad Request", t400.data));
		this->response.insert(t403.type == AMING_HTTP_ADDRESS ? redirect(t403.code, t403.data) : htemplate(t403.code, "Forbidden", t403.data));
		this->response.insert(t404.type == AMING_HTTP_ADDRESS ? redirect(t404.code, t404.data) : htemplate(t404.code, "Not Found", t404.data));
		this->response.insert(t407.type == AMING_HTTP_ADDRESS ? redirect(t407.code, t407.data) : htemplate(t407.code, "Proxy Authentication Required", t407.data, authMessage));
		this->response.insert(t500.type == AMING_HTTP_ADDRESS ? redirect(t500.code, t500.data) : htemplate(t500.code, "Internal Error", t500.data));
		this->response.insert(t502.type == AMING_HTTP_ADDRESS ? redirect(t502.code, t502.data) : htemplate(t502.code, "Bad Gateway", t502.data));
		this->response.insert(t503.type == AMING_HTTP_ADDRESS ? redirect(t503.code, t503.data) : htemplate(t503.code, "Service Unavailable", t503.data));
		this->response.insert(t509.type == AMING_HTTP_ADDRESS ? redirect(t509.code, t509.data) : htemplate(t509.code, "Bandwidth Limit Exceeded", t509.data));
		this->response.insert(t510.type == AMING_HTTP_ADDRESS ? redirect(t510.code, t510.data) : htemplate(t510.code, "Not Extended", t510.data));
		
		create(options, config, log);
	}
}
 
HttpData::~HttpData(){
	
	clear();
	
	vector <u_char> ().swap(this->raw);
}
 
const bool Http::isHttp(const string buffer){
	
	if(!buffer.empty()){
		
		char buf[4];
		
		char cmds[8][4] = {"get", "hea", "pos", "put", "pat", "del", "tra", "con"};
		
		strncpy(buf, buffer.c_str(), 3);
		
		buf[3] = '\0';
		
		for(u_short i = 0; i < 8; i++) if(Anyks::toCase(buf).compare(cmds[i]) == 0) return true;
	}
	
	return false;
}
 
const size_t Http::parse(const char * buffer, const size_t size){
	
	HttpData httpData;
	
	size_t bytes = httpData.parse(buffer, size, this->options, this->config, this->log);
	
	if(bytes) this->httpData.push_back(httpData);
	
	return bytes;
}
 
void Http::modify(vector <char> &data){
	
	const char * headers = data.data();
	
	HttpData httpData;
	
	httpData.create(this->options, this->config, this->log);
	
	httpData.setData(headers, data.size());
	
	if(httpData.isEndHeaders()){
		
		size_t pos = (strstr(headers, "\r\n\r\n") - headers) + 4;
		
		string last(data.data() + pos, data.size() - pos);
		
		string query = httpData.getResponseHeaders();
		
		query.append(last);
		
		data.assign(query.begin(), query.end());
	}
}
 
void Http::clear(){
	
	this->httpData.clear();
}
 
void Http::create(const u_short options, Config * config, LogApp * log){
	
	if(config != nullptr){
		
		this->log = log;
		
		this->config = config;
		
		this->options = options;
		
		this->version = APP_VERSION;
	}
}
 
Http::Http(const u_short options, Config * config, LogApp * log){
	
	if(config != nullptr) create(options, config, log);
}
 
Http::~Http(){
	
	clear();
	
	vector <HttpData> ().swap(this->httpData);
}