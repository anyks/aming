/* SERVER PROXY AMING */
/*
*  author:     Yuriy Lobarev
*  phone:      +7(910)983-95-90
*  telegram:   @forman
*  email:      info@anyks.com
*  date:       11/23/2017 17:50:05
*  copyright:  © 2017 anyks.com
*/
 
 

#include "http.h"


using namespace std;

 
void ConnectClients::Client::add(void * ctx){
	
	this->mtx.lock();
	
	BufferHttpProxy * http = reinterpret_cast <BufferHttpProxy *> (ctx);
	
	if(http != nullptr){
		
		http->frze = &this->freeze;
		
		http->remove = [this](){
			
			rm();
		};
		
		http->isFull = [this](){
			
			return (this->connects >= this->max);
		};
		
		http->activeConnects = [this](){
			
			return (this->connects < this->max ? this->connects : this->max);
		};
		
		this->key = (http->proxy->config->connects.key == AMING_MAC ? http->client.mac : http->client.ip);
		
		this->max = http->auser.connects.connect;
		
		std::thread thr(&HttpProxy::connection, http);
		
		thr.detach();
		
		this->connects++;
	}
	
	this->mtx.unlock();
}
 
void ConnectClients::Client::rm(){
	
	this->mtx.lock();
	
	this->connects--;
	
	this->mtx.unlock();
	
	if(this->connects && (this->connects < this->max))
		
		this->freeze.cond.notify_one();
	
	else if(!this->connects) this->remove(this->key);
}
 
void ConnectClients::add(void * ctx){
	
	BufferHttpProxy * http = reinterpret_cast <BufferHttpProxy *> (ctx);
	
	if(http != nullptr){
		
		string key = (http->proxy->config->connects.key == AMING_MAC ? http->client.mac : http->client.ip);
		
		if(this->clients.count(key) < 1){
			
			unique_ptr <Client> client(new Client);
			
			this->clients.insert(pair <string, unique_ptr <Client>> (key, move(client)));
		}
		
		Client * client = (this->clients.find(key)->second).get();
		
		client->remove = [this](const string key){
			
			rm(key);
		};
		
		client->add(http);
	}
}
 
void ConnectClients::rm(const string key){
	
	this->mtx.lock();
	
	if(this->clients.count(key) > 0){
		
		auto it = this->clients.find(key);
		
		this->clients.erase(it);
	}
	
	this->mtx.unlock();
}
 
void BufferHttpProxy::free_socket(evutil_socket_t * fd){
	
	if((* fd) > -1){
		
		shutdown(* fd, SHUT_RDWR);
		
		::close(* fd);
		
		* fd = -1;
	}
}
 
void BufferHttpProxy::free_event(struct bufferevent ** event){
	
	if((* event) != nullptr){
		
		bufferevent_free(* event);
		
		* event = nullptr;
	}
}
 
void BufferHttpProxy::closeClient(){
	
	this->mtx.lock();
	
	free_socket(&this->sockets.client);
	
	free_event(&this->events.client);
	
	this->mtx.unlock();
}
 
void BufferHttpProxy::closeServer(){
	
	this->mtx.lock();
	
	free_socket(&this->sockets.server);
	
	free_event(&this->events.server);
	
	this->mtx.unlock();
}
 
void BufferHttpProxy::close(){
	
	closeClient();
	
	closeServer();
	
	this->mtx.lock();
	
	event_base_loopbreak(this->base);
	
	this->mtx.unlock();
}
 
void BufferHttpProxy::freeze(){
	
	if(this->isFull()){
		
		ConnectClients::Freeze * frze = reinterpret_cast <ConnectClients::Freeze *> (this->frze);
		
		unique_lock <mutex> locker(frze->mtx);
		
		frze->cond.wait(locker);
	}
}
 
void BufferHttpProxy::checkUpgrade(){
	
	if(this->httpResponse.isUpgrade()
	&& (this->httpResponse.getStatus() == 101)){
		
		if(this->auser.options & OPT_UPGRADE){
			
			
			this->client.connect = true;
			
			this->server.upgrade = true;
		
		} else this->httpResponse.faultAuth();
	}
}
 
void BufferHttpProxy::checkClose(){
	
	if(!this->client.connect
	&& this->httpResponse.isEndHeaders()
	&& this->httpResponse.isClose()){
		
		this->httpResponse.setBodyEnd();
		
		this->sendClient();
		
		this->httpRequest.clear();
		
		string log = (this->client.request + "\r\n\r\n");
		
		log.append(this->httpResponse.getRawResponseData());
		
		this->proxy->log->write_data(this->client.ip, log);
		
		this->proxy->cache->setCache(this->httpResponse);
	
	} else close();
}
 
void BufferHttpProxy::auserUpdate(AParams::AUser auser){
	
	if(auser.auth){
		
		this->mtx.lock();
		
		this->auser = auser;
		
		this->parser.create(this->auser.options, this->proxy->config, this->proxy->log);
		
		this->readTimeout = this->auser.timeouts.read;
		
		this->writeTimeout = this->auser.timeouts.write;
		
		vector <string> resolver;
		
		switch(this->proxy->config->proxy.extIPv){
			
			case 4: resolver = this->auser.ipv4.resolver; break;
			
			case 6: resolver = this->auser.ipv6.resolver; break;
		}
		
		if(this->dns != nullptr) this->dns->replaceServers(resolver);
		
		this->mtx.unlock();
	}
}
 
void BufferHttpProxy::sleep(const size_t size, const bool type){
	
	int seconds = 0;
	
	float max = float(type ? this->auser.buffers.read : this->auser.buffers.write);
	
	if(max > 0){
		
		max = (max / /f/l/o/a/t/(/t/h/i/s/-/>/a/c/t/i/v/e/C/o/n/n/e/c/t/s/(/)/)/)/;/
/	/	
		if((max > 0) && (size > max)) seconds = (size / /m/a/x/)/;/
/	/	
		
		
		this_thread::sleep_for(chrono::seconds(seconds));
	}
}
 
void BufferHttpProxy::setTimeout(const u_short type, const bool read, const bool write){
	
	this->mtx.lock();
	
	struct timeval readTimeout = {this->readTimeout, 0};
	
	struct timeval writeTimeout = {this->writeTimeout, 0};
	
	bool readValue	= read;
	bool writeValue	= write;
	
	if(this->readTimeout < 1)	readValue	= false;
	if(this->writeTimeout < 1)	writeValue	= false;
	
	if((type & TM_SERVER) && this->events.server){
		
		bufferevent_set_timeouts(this->events.server, (readValue ? &readTimeout : nullptr), (writeValue ? &writeTimeout : nullptr));
	}
	
	if((type & TM_CLIENT) && this->events.client){
		
		bufferevent_set_timeouts(this->events.client, (readValue ? &readTimeout : nullptr), (writeValue ? &writeTimeout : nullptr));
	}
	
	this->mtx.unlock();
}
 
void BufferHttpProxy::sendClient(){
	
	this->cache.clear();
	
	this->checkUpgrade();
	 
	
	this->httpResponse.setMethod(this->httpRequest.getMethod());
	
	this->httpResponse.setPath(this->httpRequest.getPath());
	
	this->httpResponse.setHost(this->httpRequest.getHost());
	
	this->httpResponse.setPort(this->httpRequest.getPort());
	
	this->httpResponse.setProtocol(this->httpRequest.getProtocol());
	
	this->proxy->headers->modify({
		this->client.ip,
		this->client.mac,
		this->server.ip,
		&this->auser
	}, this->httpResponse);
	 
	
	const string response = this->httpResponse.getResponseData(!this->httpResponse.isClose() && (this->httpResponse.getVersion() > 1));
	
	if(this->server.upgrade){
		
		this->readTimeout = this->auser.timeouts.upgrade;
		
		this->setTimeout(TM_CLIENT | TM_SERVER, true, true);
	
	} else if(!this->httpResponse.isClose()) {
		
		this->sleep(response.size(), false);
		
		this->setTimeout(TM_CLIENT, true, true);
	
	} else this->setTimeout(TM_CLIENT, false, true);
	
	bufferevent_setwatermark(this->events.client, EV_WRITE, response.size(), 0);
	
	bufferevent_enable(this->events.client, EV_WRITE);
	
	bufferevent_write(this->events.client, response.data(), response.size());
}
 
void BufferHttpProxy::sendServer(){
	
	this->proxy->headers->modify({
		this->client.ip,
		this->client.mac,
		this->server.ip,
		&this->auser
	}, this->httpRequest);
	
	this->client.request = this->httpRequest.getRequestData();
	
	this->sleep(this->client.request.size(), false);
	
	this->setTimeout(TM_SERVER, true, true);
	
	bufferevent_write(this->events.server, this->client.request.data(), this->client.request.size());
}
 
void BufferHttpProxy::next(){
	
	this->httpRequest.clear();
	
	if(!this->parser.httpData.empty()){
		
		this->parser.httpData.erase(this->parser.httpData.begin());
	}
	
	if(this->auser.proxy.pipelining){
		
		if(!this->parser.httpData.empty()
		&& !this->httpResponse.isClose()) HttpProxy::do_request(this);
	}
}
 
BufferHttpProxy::BufferHttpProxy(const string ip, const string mac, System * proxy){
	
	if((proxy != nullptr) && !ip.empty() && !mac.empty()){
		
		this->mtx.lock();
		try {
			
			this->proxy = proxy;
			
			this->base = event_base_new();
			
			this->client.ip = ip;
			
			this->client.mac = mac;
			
			this->auser = this->proxy->ausers->searchUser(this->client.ip, this->client.mac);
			
			this->auth = this->auser.auth;
			
			this->httpResponse = HttpData(this->auser.options, this->proxy->config, this->proxy->log);
			
			this->parser.create(this->auser.options, this->proxy->config, this->proxy->log);
			
			this->readTimeout = this->auser.timeouts.read;
			
			this->writeTimeout = this->auser.timeouts.write;
			
			vector <string> resolver;
			
			switch(this->proxy->config->proxy.extIPv){
				
				case 4: resolver = this->auser.ipv4.resolver; break;
				
				case 6: resolver = this->auser.ipv6.resolver; break;
			}
			
			this->dns = new DNSResolver(this->proxy->log, this->proxy->cache, this->base, resolver);
		
		} catch(const bad_alloc&) {}
		
		this->mtx.unlock();
	}
}
 
BufferHttpProxy::~BufferHttpProxy(){
	
	this->mtx.lock();
	
	if(this->dns != nullptr) delete this->dns;
	
	if(this->base != nullptr) event_base_free(this->base);
	
	this->mtx.unlock();
}
 
void HttpProxy::create_client(const string ip, const string mac, const evutil_socket_t fd){
	
	socket_nonblocking(fd, this->server->log);
	
	socket_reuseable(fd, this->server->log);
	
	socket_nosigpipe(fd, this->server->log);
	
	this->server->log->write(LOG_ACCESS, 0, "client connect to proxy server, host = %s, mac = %s, socket = %d", ip.c_str(), mac.c_str(), fd);
	
	BufferHttpProxy * http = new BufferHttpProxy(ip, mac, this->server);
	
	http->sockets.client = fd;
	
	this->clients.add(http);
}
 
const string HttpProxy::get_mac(void * ctx){
	
	char buff[256];
	
	struct sockaddr * s = reinterpret_cast <struct sockaddr *> (ctx);
	
	u_char * ptr = (u_char *) s->sa_data;
	
	sprintf(
		buff, "%02X:%02X:%02X:%02X:%02X:%02X",
		(ptr[0] & 0xff), (ptr[1] & 0xff), (ptr[2] & 0xff),
		(ptr[3] & 0xff), (ptr[4] & 0xff), (ptr[5] & 0xff)
	);
	
	return buff;
}
 
const string HttpProxy::get_ip(const int family, void * ctx){
	
	switch(family){
		
		case AF_INET: {
			
			char ipstr[INET_ADDRSTRLEN];
			
			struct sockaddr_in * s = reinterpret_cast <struct sockaddr_in *> (ctx);
			
			inet_ntop(family, &s->sin_addr, ipstr, sizeof(ipstr));
			
			return ipstr;
		}
		
		case AF_INET6: {
			
			char ipstr[INET6_ADDRSTRLEN];
			
			struct sockaddr_in6 * s = reinterpret_cast <struct sockaddr_in6 *> (ctx);
			
			inet_ntop(family, &s->sin6_addr, ipstr, sizeof(ipstr));
			
			return ipstr;
		}
	}
	
	return "";
}
 
const int HttpProxy::nosigpipe(LogApp * log){
	
	struct sigaction act;
	
	memset(&act, 0, sizeof(act));
	
	act.sa_handler = SIG_IGN;
	
	act.sa_flags = SA_RESTART;
	
	if(sigaction(SIGPIPE, &act, nullptr)){
		
		log->write(LOG_ERROR, 0, "cannot set SIG_IGN on signal SIGPIPE");
		
		return -1;
	}
	
	return 0;
}
 
const int HttpProxy::socket_nosigpipe(const evutil_socket_t fd, LogApp * log){

#ifdef __APPLE__ || __MACH__ || __FreeBSD__
	
	int nosigpipe = 1;
	
	if(setsockopt(fd, SOL_SOCKET, SO_NOSIGPIPE, &nosigpipe, sizeof(nosigpipe)) < 0){
		
		log->write(LOG_ERROR, 0, "cannot set SO_NOSIGPIPE option on socket %d", fd);
		
		return -1;
	}
#endif
	
	return 0;
}
 
const int HttpProxy::socket_nonblocking(const evutil_socket_t fd, LogApp * log){
	int flags;
	flags = fcntl(fd, F_GETFL);
	if(flags < 0) return flags;
	flags |= O_NONBLOCK;
	
	if(fcntl(fd, F_SETFL, flags) < 0){
		
		log->write(LOG_ERROR, 0, "cannot set NON_BLOCK option on socket %d", fd);
		
		return -1;
	}
	
	return 0;
}
 
const int HttpProxy::socket_tcpcork(const evutil_socket_t fd, LogApp * log){
	
	int tcpcork = 1;

#ifdef __linux__
	
	if(setsockopt(fd, IPPROTO_TCP, TCP_CORK, &tcpcork, sizeof(tcpcork)) < 0){
		
		log->write(LOG_ERROR, 0, "cannot set TCP_CORK option on socket %d", fd);
		
		return -1;
	}

#elif __APPLE__ || __MACH__ || __FreeBSD__
	
	if(setsockopt(fd, IPPROTO_TCP, TCP_NOPUSH, &tcpcork, sizeof(tcpcork)) < 0){
		
		log->write(LOG_ERROR, 0, "cannot set TCP_NOPUSH option on socket %d", fd);
		
		return -1;
	}
#endif
	
	return 0;
}
 
const int HttpProxy::socket_tcpnodelay(const evutil_socket_t fd, LogApp * log){
	
	int tcpnodelay = 1;
	
	if(setsockopt(fd, IPPROTO_TCP, TCP_NODELAY, &tcpnodelay, sizeof(tcpnodelay)) < 0){
		
		log->write(LOG_ERROR, 0, "cannot set TCP_NODELAY option on socket %d", fd);
		
		return -1;
	}
	
	return 0;
}
 
const int HttpProxy::socket_reuseable(const evutil_socket_t fd, LogApp * log){
	
	int reuseaddr = 1;
	
	if(setsockopt(fd, SOL_SOCKET, SO_REUSEADDR, &reuseaddr, sizeof(reuseaddr)) < 0){
		
		log->write(LOG_ERROR, 0, "cannot set SO_REUSEADDR option on socket %d", fd);
		
		return -1;
	}
	
	return 0;
}
 
const int HttpProxy::socket_ipv6only(const evutil_socket_t fd, const bool mode, LogApp * log){
	
	int only6 = (mode ? 1 : 0);
	
	if(setsockopt(fd, IPPROTO_IPV6, IPV6_V6ONLY, &only6, sizeof(only6)) < 0){
		
		log->write(LOG_ERROR, 0, "cannot set IPV6_V6ONLY option on socket %d", fd);
		
		return -1;
	}
	
	return 0;
}
 
const int HttpProxy::socket_keepalive(const evutil_socket_t fd, LogApp * log, const int cnt, const int idle, const int intvl){
	
	int keepalive = 1;
	
	if(setsockopt(fd, SOL_SOCKET, SO_KEEPALIVE, &keepalive, sizeof(int))){
		
		log->write(LOG_ERROR, 0, "cannot set SO_KEEPALIVE option on socket %d", fd);
		
		return -1;
	}
	
	if(setsockopt(fd, IPPROTO_TCP, TCP_KEEPCNT, &cnt, sizeof(int))){
		
		log->write(LOG_ERROR, 0, "cannot set TCP_KEEPCNT option on socket %d", fd);
		
		return -1;
	}

#ifdef __APPLE__ || __MACH__
	
	if(setsockopt(fd, IPPROTO_TCP, TCP_KEEPALIVE, &idle, sizeof(int))){
		
		log->write(LOG_ERROR, 0, "cannot set TCP_KEEPALIVE option on socket %d", fd);
		
		return -1;
	}

#elif __linux__ || __FreeBSD__
	
	if(setsockopt(fd, IPPROTO_TCP, TCP_KEEPIDLE, &idle, sizeof(int))){
		
		log->write(LOG_ERROR, 0, "cannot set TCP_KEEPIDLE option on socket %d", fd);
		
		return -1;
	}
#endif
	
	if(setsockopt(fd, IPPROTO_TCP, TCP_KEEPINTVL, &intvl, sizeof(int))){
		
		log->write(LOG_ERROR, 0, "cannot set TCP_KEEPINTVL option on socket %d", fd);
		
		return -1;
	}
	
	return 0;
}
 
const int HttpProxy::socket_buffersize(const evutil_socket_t fd, const int read_size, const int write_size, const u_int maxcon, LogApp * log){
	
	int readSize	= read_size;
	int writeSize	= write_size;
	
	socklen_t read_optlen	= sizeof(readSize);
	socklen_t write_optlen	= sizeof(write_size);
	
	if(readSize > 0){
		
		readSize = (readSize / /m/a/x/c/o/n/)/;/
/	/	
		setsockopt(fd, SOL_SOCKET, SO_RCVBUF, (char *) &readSize, read_optlen);
	}
	
	if(writeSize > 0){
		
		writeSize = (writeSize / /m/a/x/c/o/n/)/;/
/	/	
		setsockopt(fd, SOL_SOCKET, SO_SNDBUF, (char *) &writeSize, write_optlen);
	}
	
	if((getsockopt(fd, SOL_SOCKET, SO_RCVBUF, &readSize, &read_optlen) < 0)
	|| (getsockopt(fd, SOL_SOCKET, SO_SNDBUF, &writeSize, &write_optlen) < 0)){
		
		log->write(LOG_ERROR, 0, "get buffer wrong on socket %d", fd);
		
		return -1;
	}
	
	return 0;
}
 
const bool HttpProxy::check_auth(void * ctx){
	
	BufferHttpProxy * http = reinterpret_cast <BufferHttpProxy *> (ctx);
	
	if(http != nullptr){
		
		const string login = http->httpRequest.getLogin();
		
		const string password = http->httpRequest.getPassword();
		
		auto auser = http->proxy->ausers->authenticate(login, password);
		
		http->auserUpdate(auser);
		
		return auser.auth;
		
		http->proxy->log->write(LOG_MESSAGE, 0, "auth client [%s] to proxy wrong!", http->client.ip.c_str());
	}
	
	return false;
}
 
const bool HttpProxy::isallow_remote_connect(const string ip, void * ctx){
	
	BufferHttpProxy * http = reinterpret_cast <BufferHttpProxy *> (ctx);
	
	if(http != nullptr){
		
		Network nwk;
		
		int check = -1;
		
		u_int nettype = http->proxy->config->proxy.extIPv;
		
		if(http->auser.proxy.subnet){
			
			nettype = nwk.checkNetworkByIp(ip);
		}
		
		switch(nettype){
			
			case 4: check = nwk.isLocal(ip);	break;
			
			case 6: check = nwk.isLocal6(ip);	break;
		}
		
		switch(check){
			
			case -1: return false;
			
			case 0: return http->auser.proxy.reverse;
			
			case 1: return http->auser.proxy.forward;
		}
	}
	
	return false;
}
 
const int HttpProxy::connect_server(void * ctx){
	
	BufferHttpProxy * http = reinterpret_cast <BufferHttpProxy *> (ctx);
	
	if(http != nullptr){
		
		if(http->events.server == nullptr){
			
			string bindhost;
			
			socklen_t sinlen = 0, sotlen = 0;
			
			struct sockaddr * sin = nullptr, * sot = nullptr;
			
			struct sockaddr_in server4_addr, client4_addr;
			
			struct sockaddr_in6 server6_addr, client6_addr;
			
			u_int nettype = http->proxy->config->proxy.extIPv;
			
			if(http->auser.proxy.subnet){
				
				Network nwk;
				
				nettype = nwk.checkNetworkByIp(http->server.ip);
			}
			
			switch(nettype){
				
				case 4: {
					
					auto ips = http->proxy->config->ipv4.external;
					
					if(ips.size() > 1){
						
						srand(time(0));
						
						bindhost = ips[rand() % ips.size()];
					
					} else bindhost = ips[0];
					
					struct hostent * client = gethostbyname2(bindhost.c_str(), AF_INET);
					
					memset(&client4_addr, 0, sizeof(client4_addr));
					
					memset(&server4_addr, 0, sizeof(server4_addr));
					
					client4_addr.sin_family = AF_INET;
					server4_addr.sin_family = AF_INET;
					
					client4_addr.sin_port = htons(0);
					
					server4_addr.sin_port = htons(http->server.port);
					
					client4_addr.sin_addr.s_addr = * ((u_long *) client->h_addr);
					
					server4_addr.sin_addr.s_addr = inet_addr(http->server.ip.c_str());
					
					bzero(&server4_addr.sin_zero, sizeof(server4_addr.sin_zero));
					
					sinlen = sizeof(client4_addr);
					sotlen = sizeof(server4_addr);
					
					sin	= reinterpret_cast <struct sockaddr *> (&client4_addr);
					sot	= reinterpret_cast <struct sockaddr *> (&server4_addr);
					
					http->sockets.server = socket(AF_INET, SOCK_STREAM, IPPROTO_TCP);
				} break;
				
				case 6: {
					
					auto ips = http->proxy->config->ipv6.external;
					
					if(ips.size() > 1){
						
						srand(time(0));
						
						bindhost = ips[rand() % ips.size()];
					
					} else bindhost = ips[0];
					
					char host_client[INET6_ADDRSTRLEN], host_server[INET6_ADDRSTRLEN];
					
					memset(&client6_addr, 0, sizeof(client6_addr));
					
					memset(&server6_addr, 0, sizeof(server6_addr));
					
					client6_addr.sin6_family = AF_INET6;
					server6_addr.sin6_family = AF_INET6;
					
					client6_addr.sin6_port = htons(0);
					
					server6_addr.sin6_port = htons(http->server.port);
					
					inet_pton(AF_INET6, bindhost.c_str(), &(client6_addr.sin6_addr));
					inet_pton(AF_INET6, http->server.ip.c_str(), &(server6_addr.sin6_addr));
					
					inet_ntop(AF_INET6, &client6_addr.sin6_addr, host_client, sizeof(host_client));
					inet_ntop(AF_INET6, &server6_addr.sin6_addr, host_server, sizeof(host_server));
					
					sinlen = sizeof(client6_addr);
					sotlen = sizeof(server6_addr);
					
					sin	= reinterpret_cast <struct sockaddr *> (&client6_addr);
					sot	= reinterpret_cast <struct sockaddr *> (&server6_addr);
					
					http->sockets.server = socket(AF_INET6, SOCK_STREAM, IPPROTO_TCP);
				} break;
				
				default: {
					
					http->proxy->log->write(
						LOG_ERROR, 0,
						"network not allow from server = %s, port = %d, client = %s",
						http->server.ip.c_str(),
						http->server.port,
						http->client.ip.c_str()
					);
					
					return 0;
				}
			}
			
			http->server.mac = get_mac(sot);
			
			if(http->sockets.server < 0){
				
				http->proxy->log->write(
					LOG_ERROR, 0,
					"creating socket to server = %s, port = %d, client = %s",
					http->server.ip.c_str(),
					http->server.port,
					http->client.ip.c_str()
				);
				
				return 0;
			}
			
			socket_nonblocking(http->sockets.server, http->proxy->log);
			
			socket_reuseable(http->sockets.server, http->proxy->log);
			
			socket_nosigpipe(http->sockets.server, http->proxy->log);
			
			if(http->client.alive){
				
				socket_tcpnodelay(http->sockets.server, http->proxy->log);
				socket_tcpnodelay(http->sockets.client, http->proxy->log);
				
				socket_keepalive(
					http->sockets.server,
					http->proxy->log,
					http->auser.keepalive.keepcnt,
					http->auser.keepalive.keepidle,
					http->auser.keepalive.keepintvl
				);
			}
			
			if(::bind(http->sockets.server, sin, sinlen) < 0){
				
				http->proxy->log->write(LOG_ERROR, 0, "bind local network [%s] error", bindhost.c_str());
				
				return 0;
			}
			
			http->events.server = bufferevent_socket_new(http->base, http->sockets.server, BEV_OPT_CLOSE_ON_FREE | BEV_OPT_DEFER_CALLBACKS);
			
			
			
			bufferevent_setcb(http->events.server, &HttpProxy::read_server_cb, nullptr, &HttpProxy::event_cb, http);
			
			bufferevent_flush(http->events.server, EV_READ | EV_WRITE, BEV_FINISHED);
			
			bufferevent_enable(http->events.server, EV_READ | EV_WRITE);
			
			if(bufferevent_socket_connect(http->events.server, sot, sotlen) < 0){
				
				http->proxy->log->write(
					LOG_ERROR, 0,
					"connecting to server = %s, port = %d, client = %s",
					http->server.ip.c_str(),
					http->server.port,
					http->client.ip.c_str()
				);
				
				return -1;
			}
			
			http->proxy->log->write(
				LOG_MESSAGE, 0,
				"connect client [%s] to host = %s [%s:%d], mac = %s, method = %s, path = %s, useragent = %s, socket = %d",
				http->client.ip.c_str(),
				http->httpRequest.getHost().c_str(),
				http->server.ip.c_str(),
				http->server.port,
				http->server.mac.c_str(),
				http->httpRequest.getMethod().c_str(),
				http->httpRequest.getPath().c_str(),
				http->httpRequest.getUseragent().c_str(),
				http->sockets.client
			);
			
			return 1;
		
		} else {
			
			http->proxy->log->write(
				LOG_MESSAGE, 0,
				"last connect client [%s] to host = %s [%s:%d], mac = %s, method = %s, path = %s, useragent = %s, socket = %d",
				http->client.ip.c_str(),
				http->httpRequest.getHost().c_str(),
				http->server.ip.c_str(),
				http->server.port,
				http->server.mac.c_str(),
				http->httpRequest.getMethod().c_str(),
				http->httpRequest.getPath().c_str(),
				http->httpRequest.getUseragent().c_str(),
				http->sockets.client
			);
			
			return 2;
		}
	}
	
	return -1;
}
 
void HttpProxy::event_cb(struct bufferevent * bev, const short events, void * ctx){
	
	BufferHttpProxy * http = reinterpret_cast <BufferHttpProxy *> (ctx);
	
	if(http != nullptr){
		
		const evutil_socket_t current_fd = bufferevent_getfd(bev);
		
		const bool isClient = (current_fd == http->sockets.client ? true : false);
		
		if(events & BEV_EVENT_CONNECTED){
			
			if(!isClient){
				
				http->proxy->log->write(
					LOG_ACCESS, 0,
					"connect client [%s], useragent = %s, socket = %d to server [%s:%d]",
					http->client.ip.c_str(),
					http->client.useragent.c_str(),
					current_fd,
					http->server.ip.c_str(),
					http->server.port
				);
			}
		
		} else if(events & (BEV_EVENT_ERROR | BEV_EVENT_EOF | BEV_EVENT_TIMEOUT)) {
			
			if(events & BEV_EVENT_ERROR){
				
				int err = bufferevent_socket_get_dns_error(bev);
				
				if(err) http->proxy->log->write(LOG_ERROR, 0, "DNS error: %s", evutil_gai_strerror(err));
			}
			
			if(isClient){
				
				http->proxy->log->write(
					LOG_ACCESS, 0,
					"closing client [%s] from server [%s:%d], socket = %d",
					http->client.ip.c_str(),
					http->server.ip.c_str(),
					http->server.port,
					current_fd
				);
				
				http->close();
			
			} else {
				
				http->proxy->log->write(
					LOG_ACCESS, 0,
					"closing server [%s:%d] from client [%s], socket = %d",
					http->server.ip.c_str(),
					http->server.port,
					http->client.ip.c_str(),
					current_fd
				);
				
				if(http->httpResponse.isClose() && !http->httpResponse.getBodySize()){
					
					socket_tcpcork(http->sockets.client, http->proxy->log);
				}
				
				http->checkClose();
			}
		}
	}
	
	return;
}
 
void HttpProxy::send_http_data(void * ctx){
	
	BufferHttpProxy * http = reinterpret_cast <BufferHttpProxy *> (ctx);
	
	if(http != nullptr){
		
		struct evbuffer * input		= bufferevent_get_input(http->events.server);
		struct evbuffer * output	= bufferevent_get_output(http->events.client);
		
		const size_t len = evbuffer_get_length(input);
		
		if(len){
			
			char * buffer = new char[len];
			
			evbuffer_copyout(input, buffer, len);
			
			size_t size = http->httpResponse.setEntitybody(buffer, len);
			
			if(http->httpResponse.getRawBodySize() > http->auser.connects.size){
				
				http->closeServer();
				
				socket_tcpcork(http->sockets.client, http->proxy->log);
				
				http->httpResponse.brokenRequest();
				
				http->sendClient();
			
			} else {
				
				if(!http->httpResponse.isClose()){
					
					if(!http->httpResponse.isIntGzip()){
						
						struct evbuffer * tmp = evbuffer_new();
						
						evbuffer_add(tmp, buffer, size);
						
						http->setTimeout(TM_CLIENT, true, true);
						
						evbuffer_add_buffer(output, tmp);
						
						evbuffer_free(tmp);
					
					} else if(http->httpResponse.isEndBody()) http->sendClient();
					
					if(http->httpResponse.isEndBody()){
						
						string log = (http->client.request + "\r\n\r\n");
						
						log.append(http->httpResponse.getRawResponseData());
						
						http->proxy->log->write_data(http->client.ip, log);
						
						http->proxy->cache->setCache(http->httpResponse);
						
						http->next();
					}
				}
				
				evbuffer_drain(input, size);
			}
			
			delete [] buffer;
		}
	}
	
	return;
}
 
void HttpProxy::read_server_cb(struct bufferevent * bev, void * ctx){
	
	BufferHttpProxy * http = reinterpret_cast <BufferHttpProxy *> (ctx);
	
	if(http != nullptr){
		
		struct evbuffer * input		= bufferevent_get_input(http->events.server);
		struct evbuffer * output	= bufferevent_get_output(http->events.client);
		
		size_t len = evbuffer_get_length(input);
		
		http->sleep(len, true);
		
		http->setTimeout(TM_CLIENT | TM_SERVER, true, true);
		
		const bool redirect = (http->proxy->config->proxy.type & AMING_TYPE_REDIRECT);
		
		if(redirect || http->client.connect) evbuffer_add_buffer(output, input);
		
		else if(http->httpResponse.isEndHeaders()) send_http_data(http);
		
		else if(len){
			
			while(!http->httpResponse.isEndHeaders()){
				
				const char * line = evbuffer_readln(input, &len, EVBUFFER_EOL_CRLF_STRICT);
				
				if(!line) break;
				
				http->httpResponse.addHeader(line);
			}
			
			if(http->httpResponse.isEndHeaders()){
				
				const u_int status = http->httpResponse.getStatus();
				
				http->httpResponse.setGzipParams(&http->auser.gzip);
				
				if((status == 304) && !http->cache.empty()){
					
					const string age = http->httpResponse.getHeader("age");
					
					http->httpResponse.set(http->cache.data(), http->cache.size());
					
					if(!age.empty()) http->httpResponse.setHeader("Age", age);
					
					http->sendClient();
					
					http->next();
					
					return;
				
				} else if(http->httpResponse.isRedirect()){
					
					http->sendClient();
					
					http->next();
					
					return;
				}
				
				const string cl = http->httpResponse.getHeader("content-length");
				
				if(!cl.empty() && (::atoi(cl.c_str()) > http->auser.connects.size)){
					
					http->closeServer();
					
					socket_tcpcork(http->sockets.client, http->proxy->log);
					
					http->httpResponse.largeRequest();
					
					http->sendClient();
					
					return;
				}
				
				if(!http->client.connect
				&& http->httpResponse.compressIsAllowed(http->client.useragent)){
					
					http->httpResponse.setGzip();
				}
				
				http->httpResponse.initBody();
				
				if(!http->httpResponse.isIntGzip()
				&& !http->httpResponse.isClose()) http->sendClient();
				
				if(evbuffer_get_length(input)) send_http_data(http);
			}
		
		} else http->close();
	}
	
	return;
}
 
void HttpProxy::rresolve_cb(const string ip, void * ctx){
	
	BufferHttpProxy * http = reinterpret_cast <BufferHttpProxy *> (ctx);
	
	if(http != nullptr){
		
		if(!ip.empty()){
			
			http->server.ip = ip;
			
			const int connect = connect_server(http);
			
			if(connect > 0){
				
				http->setTimeout(TM_SERVER | TM_CLIENT, true, true);
				
				struct evbuffer * input		= bufferevent_get_input(http->events.client);
				struct evbuffer * output	= bufferevent_get_output(http->events.server);
				
				evbuffer_add_buffer(output, input);
			
			} else if(connect == 0){
				
				http->httpResponse.faultConnect();
				
				socket_tcpcork(http->sockets.client, http->proxy->log);
				
				http->sendClient();
			
			} else {
				
				http->close();
				
				return;
			}
		
		} else {
			
			http->proxy->log->write(
				LOG_ERROR, 0,
				"host server for redirect = %s is not found, port = %d, client = %s, socket = %d",
				http->server.ip.c_str(),
				http->server.port,
				http->client.ip.c_str(),
				http->sockets.client
			);
			
			http->httpResponse.pageNotFound();
			
			socket_tcpcork(http->sockets.client, http->proxy->log);
			
			http->sendClient();
		}
	}
	
	return;
}
 
void HttpProxy::resolve_cb(const string ip, void * ctx){
	
	BufferHttpProxy * http = reinterpret_cast <BufferHttpProxy *> (ctx);
	
	if(http != nullptr){
		
		if(!ip.empty()){
			
			if(isallow_remote_connect(ip, http)){
				
				const bool conn_enabled = (http->auser.options & OPT_CONNECT);
				
				const bool upge_enabled = (http->auser.options & OPT_UPGRADE);
				
				if(!http->auth) http->auth = check_auth(http);
				
				if(!http->auth && (http->httpRequest.getLogin().empty()
				|| http->httpRequest.getPassword().empty())){
					
					http->httpResponse.requiredAuth();
				
				} else if(!http->auth) {
					
					http->httpResponse.faultAuth();
				
				} else {
					
					const u_int port = http->httpRequest.getPort();
					
					if((http->events.server != nullptr)
					&& ((http->server.ip.compare(ip) != 0)
					|| (http->server.port != port))) http->closeServer();
					
					http->server.ip		= ip;
					http->server.port	= port;
					
					http->client.alive		= http->httpRequest.isAlive();
					http->client.https		= http->httpRequest.isHttps();
					http->client.connect	= http->httpRequest.isConnect();
					http->client.upgrade	= http->httpRequest.isUpgrade();
					http->client.useragent	= http->httpRequest.getUseragent();
					
					if(!http->client.connect){
						
						auto cache = http->proxy->cache->getCache(http->httpRequest);
						
						if(!cache.http.empty()){
							
							if(!cache.valid){
								
								http->httpResponse.setGzipParams(&http->auser.gzip);
								
								http->httpResponse.set(cache.http.data(), cache.http.size());
								
								if(cache.age) http->httpResponse.setHeader("Age", to_string(cache.age));
								
								http->sendClient();
								
								http->next();
								
								return;
							
							} else {
								
								if(!cache.modified.empty()) http->httpRequest.setHeader("If-Modified-Since", cache.modified);
								
								if(!cache.etag.empty()){
									
									const string gzip = http->httpRequest.getHeader("accept-encoding");
									
									if(!gzip.empty()) cache.etag = (string("W/") + cache.etag);
									
									http->httpRequest.setHeader("If-None-Match", cache.etag);
								}
								
								http->cache.assign(cache.http.begin(), cache.http.end());
							}
						}
					}
					
					if((http->client.connect && !(http->client.https || conn_enabled))
					|| (http->client.upgrade && !upge_enabled)) http->httpResponse.faultAuth();
					
					else {
						
						const int connect = connect_server(http);
						
						if(connect > 0){
							
							if(http->client.connect) http->httpResponse.authSuccess();
							
							else {
								
								if(!http->client.alive) http->httpRequest.setClose();
								
								http->sendServer();
								
								return;
							}
						
						} else if(connect == 0) http->httpResponse.faultConnect();
						
						else {
							
							http->close();
							
							return;
						}
					}
				}
			
			} else http->httpResponse.faultAuth();
		
		} else {
			
			http->proxy->log->write(
				LOG_ERROR, 0,
				"host server = %s is not found, port = %d, client = %s, socket = %d",
				http->httpRequest.getHost().c_str(),
				http->httpRequest.getPort(),
				http->client.ip.c_str(),
				http->sockets.client
			);
			
			http->httpResponse.pageNotFound();
		}
		
		if(http->httpResponse.isClose()) socket_tcpcork(http->sockets.client, http->proxy->log);
		
		http->sendClient();
	}
	
	return;
}
 
void HttpProxy::do_redirect(void * ctx){
	
	BufferHttpProxy * http = reinterpret_cast <BufferHttpProxy *> (ctx);
	
	if(http != nullptr){
		
		if(!http->auser.proxy.redirect.empty()){
			
			string host;
			
			http->setTimeout(TM_CLIENT, true);
			
			if(http->auser.proxy.redirect.size() > 1){
				
				srand(time(0));
				
				host = http->auser.proxy.redirect[rand() % http->auser.proxy.redirect.size()];
			
			} else host = http->auser.proxy.redirect[0];
			
			if(!host.empty()){
				
				auto server = Anyks::split(host, ":");
				
				http->server.ip = server[0];
				
				const u_int port = http->proxy->config->proxy.port;
				
				http->server.port = (server.size() > 1 ? (Anyks::isNumber(server[1]) ? ::atoi(server[1].c_str()) : port) : port);
				
				if(http->dns != nullptr){
					
					if(!http->auser.proxy.subnet){
						
						switch(http->proxy->config->proxy.extIPv){
							
							case 4: http->dns->resolve(http->server.ip, AF_INET, &HttpProxy::rresolve_cb, http); break;
							
							case 6: http->dns->resolve(http->server.ip, AF_INET6, &HttpProxy::rresolve_cb, http); break;
						}
					
					} else http->dns->resolve(http->server.ip, AF_UNSPEC, &HttpProxy::rresolve_cb, http);
				
				} else http->close();
			
			} else http->close();
		
		} else http->close();
	}
	
	return;
}
 
void HttpProxy::do_request(void * ctx){
	
	BufferHttpProxy * http = reinterpret_cast <BufferHttpProxy *> (ctx);
	
	if(http != nullptr){
		
		http->setTimeout(TM_CLIENT, true);
		
		if(!http->parser.httpData.empty()
		&& http->httpRequest.isEmpty()){
			
			http->httpResponse.create(
				http->auser.options,
				http->proxy->config,
				http->proxy->log
			);
			
			auto httpData = http->parser.httpData.begin();
			
			http->httpRequest = * httpData;
			
			if(http->dns != nullptr){
				
				if(!http->auser.proxy.subnet){
					
					switch(http->proxy->config->proxy.extIPv){
						
						case 4: http->dns->resolve(http->httpRequest.getHost(), AF_INET, &HttpProxy::resolve_cb, http); break;
						
						case 6: http->dns->resolve(http->httpRequest.getHost(), AF_INET6, &HttpProxy::resolve_cb, http); break;
					}
				
				} else http->dns->resolve(http->httpRequest.getHost(), AF_UNSPEC, &HttpProxy::resolve_cb, http);
			
			} else http->close();
		}
	}
	
	return;
}
 
void HttpProxy::write_client_cb(struct bufferevent * bev, void * ctx){
	
	BufferHttpProxy * http = reinterpret_cast <BufferHttpProxy *> (ctx);
	
	if(http != nullptr){
		
		struct evbuffer * output = bufferevent_get_output(http->events.client);
		
		const size_t len = evbuffer_get_length(output);
		
		if(!len && (http->httpResponse.isClose() || http->httpResponse.isRedirect())) http->close();
	}
	
	return;
}
 
void HttpProxy::read_client_cb(struct bufferevent * bev, void * ctx){
	
	BufferHttpProxy * http = reinterpret_cast <BufferHttpProxy *> (ctx);
	
	if(http != nullptr){
		
		if(http->proxy->config->proxy.type & AMING_TYPE_REDIRECT) do_redirect(http);
		
		else {
			
			struct evbuffer * input = bufferevent_get_input(bev);
			
			const size_t len = evbuffer_get_length(input);
			
			http->sleep(len, true);
			
			if(http->client.connect && ((http->auser.options & OPT_CONNECT) || http->client.https)){
				
				http->setTimeout(TM_SERVER | TM_CLIENT, true, true);
				
				struct evbuffer * output = bufferevent_get_output(http->events.server);
				
				evbuffer_add_buffer(output, input);
			
			} else if(!http->client.connect) {
				
				char * buffer = new char[len];
				
				evbuffer_copyout(input, buffer, len);
				
				evbuffer_drain(input, http->parser.parse(buffer, len));
				
				delete [] buffer;
				
				do_request(http);
			
			} else http->close();
		}
	}
	
	return;
}
 
void HttpProxy::connection(void * ctx){
	
	BufferHttpProxy * http = reinterpret_cast <BufferHttpProxy *> (ctx);
	
	if(http != nullptr){
		
		function <void (void)> remove = http->remove;
		
		http->events.client = bufferevent_socket_new(http->base, http->sockets.client, BEV_OPT_CLOSE_ON_FREE | BEV_OPT_DEFER_CALLBACKS);
		
		if(http->isFull()) http->close();
		
		else {
			
			http->setTimeout(TM_CLIENT, true);
			
			
			
			bufferevent_setcb(http->events.client, &HttpProxy::read_client_cb, &HttpProxy::write_client_cb, &HttpProxy::event_cb, http);
			
			bufferevent_flush(http->events.client, EV_READ | EV_WRITE, BEV_FINISHED);
			
			bufferevent_enable(http->events.client, EV_READ | EV_WRITE);
			
			event_base_dispatch(http->base);
		}
		
		delete http;
		
		remove();
	}
	
	return;
}
 
void HttpProxy::accept_cb(const evutil_socket_t fd, const short event, void * ctx){
	
	HttpProxy * proxy = reinterpret_cast <HttpProxy *> (ctx);
	
	if(proxy != nullptr){
		
		string ip, mac;
		
		evutil_socket_t socket = -1;
		
		switch(proxy->server->config->proxy.intIPv){
			
			case 4: {
				
				struct sockaddr_in client;
				
				socklen_t len = sizeof(client);
				
				socket = accept(fd, reinterpret_cast <struct sockaddr *> (&client), &len);
				
				if(socket < 0) return;
				
				ip = get_ip(AF_INET, &client);
				
				mac = get_mac(&client);
			} break;
			
			case 6: {
				
				struct sockaddr_in6 client;
				
				socklen_t len = sizeof(client);
				
				socket = accept(fd, reinterpret_cast <struct sockaddr *> (&client), &len);
				
				if(socket < 0) return;
				
				ip = get_ip(AF_INET6, &client);
				
				mac = get_mac(&client);
			} break;
		}
		
		proxy->create_client(ip, mac, socket);
	}
}
 
const evutil_socket_t HttpProxy::create_server(){
	
	evutil_socket_t sock = -1;
	
	socklen_t sinlen = 0;
	
	struct sockaddr * sin = nullptr;
	
	struct sockaddr_in server4_addr;
	
	struct sockaddr_in6 server6_addr;
	
	switch(this->server->config->proxy.intIPv){
		
		case 4: {
			
			memset(&server4_addr, 0, sizeof(server4_addr));
			
			struct hostent * bindhost = gethostbyname2(this->server->config->ipv4.internal.c_str(), AF_INET);
			
			server4_addr.sin_family = AF_INET;
			
			server4_addr.sin_addr.s_addr = * ((u_long *) bindhost->h_addr);
			
			server4_addr.sin_port = htons(this->server->config->proxy.port);
			
			sinlen = sizeof(server4_addr);
			
			sin = reinterpret_cast <struct sockaddr *> (&server4_addr);
			
			bzero(&server4_addr.sin_zero, sizeof(server4_addr.sin_zero));
			
			sock = socket(AF_INET, SOCK_STREAM, IPPROTO_TCP);
		} break;
		
		case 6: {
			
			char straddr[INET6_ADDRSTRLEN];
			
			memset(&server6_addr, 0, sizeof(server6_addr));
			
			server6_addr.sin6_family = AF_INET6;
			
			server6_addr.sin6_port = htons(this->server->config->proxy.port);
			
			inet_pton(AF_INET6, this->server->config->ipv6.internal.c_str(), &(server6_addr.sin6_addr));
			
			inet_ntop(AF_INET6, &server6_addr.sin6_addr, straddr, sizeof(straddr));
			
			sinlen = sizeof(server6_addr);
			
			sin = reinterpret_cast <struct sockaddr *> (&server6_addr);
			
			sock = socket(AF_INET6, SOCK_STREAM, IPPROTO_TCP);
			
			socket_ipv6only(sock, this->server->config->proxy.ipv6only, this->server->log);
		} break;
	}
	
	if(sock < 0){
		
		this->server->log->write(LOG_ERROR, 0, "[-] could not create socket");
		
		return -1;
	}

#ifdef __linux__
	
	nosigpipe(this->server->log);

#elif __APPLE__ || __MACH__ || __FreeBSD__
	
	socket_nosigpipe(sock, this->server->log);
#endif
	
	socket_nonblocking(sock, this->server->log);
	
	socket_reuseable(sock, this->server->log);
	
	socket_tcpnodelay(sock, this->server->log);
	
	if(::bind(sock, sin, sinlen) < 0){
		
		this->server->log->write(LOG_ERROR, 0, "[-] bind error");
		
		return -1;
	}
	
	if(listen(sock, this->server->config->connects.total) < 0){
		
		this->server->log->write(LOG_ERROR, 0, "[-] listen error");
		
		return -1;
	}
	
	return sock;
}
 
void HttpProxy::run_server(const evutil_socket_t fd, void * ctx){
	
	HttpProxy * proxy = reinterpret_cast <HttpProxy *> (ctx);
	
	if(proxy != nullptr){
		
		this_thread::sleep_for(chrono::milliseconds(500));
		
		proxy->server->log->write(LOG_MESSAGE, 0, "[+] start service: pid = %i, socket = %i", getpid(), fd);
		
		proxy->base = event_base_new();
		
		struct event * evnt = event_new(proxy->base, fd, EV_READ | EV_PERSIST, &HttpProxy::accept_cb, proxy);
		
		event_add(evnt, nullptr);
		
		event_base_loop(proxy->base, EVLOOP_NO_EXIT_ON_EMPTY);
		
		shutdown(fd, SHUT_RDWR);
		
		close(fd);
		
		event_del(evnt);
		
		event_free(evnt);
		
		event_base_free(proxy->base);
	}
}
 
void HttpProxy::run_works(pid_t * pids, const evutil_socket_t fd, const size_t cur, const size_t max, void * ctx){
	
	HttpProxy * proxy = reinterpret_cast <HttpProxy *> (ctx);
	
	if((pids != nullptr) && (proxy != nullptr) && (fd > -1) && (max > 0)){
		
		if(cur < max){
			
			switch(pids[cur] = fork()){
				
				case -1: {
					
					proxy->server->log->write(LOG_ERROR, 0, "[-] create fork error");
					
					exit(SIGHUP);
				}
				
				case 0: run_server(fd, proxy); break;
				
				default: run_works(pids, fd, cur + 1, max, proxy);
			}
		
		} else {
			
			pids[max] = getpid();
			
			proxy->server->sendPids(pids, max + 1);
			
			int status;
			
			for(u_int i = 0; i < max; i++){
				
				waitpid(pids[i], &status, 0);
				
				proxy->server->log->write(LOG_ERROR, 0, "[-] end service: pid = %i, status = %i", pids[i], WTERMSIG(status));
			}
			
			exit(SIGHUP);
		}
	}
}
 
HttpProxy::HttpProxy(System * proxy){
	
	if(proxy != nullptr){
		
		this->server = proxy;
		
		evutil_socket_t socket = create_server();
		
		if(socket > -1){
			
			this->server->cache->rmAllCache();
			
			this->server->cache->rmAllDomains();
			
			if(!this->server->config->proxy.debug){
				
				u_int max_works = (
					this->server->config->proxy.workers
					? this->server->config->proxy.workers
					: this->server->config->os.ncpu
				);
				
				if(max_works > MMAX_WORKERS) max_works = MMAX_WORKERS;
				
				this->pids = new pid_t[max_works];
				
				run_works(this->pids, socket, 0, max_works, this);
			
			} else {
				
				pid_t pids[] = {getpid()};
				
				this->server->sendPids(pids, 1);
				
				run_server(socket, this);
			}
		
		} else exit(SIGSTOP);
	}
}
 
HttpProxy::~HttpProxy(){
	
	if(this->pids) delete [] this->pids;
}
