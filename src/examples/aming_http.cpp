/* SERVER PROXY AMING */
/*
*  author:     Yuriy Lobarev
*  phone:      +7(910)983-95-90
*  telegram:   @forman
*  email:      info@anyks.com
*  date:       10/29/2017 16:59:42
*  copyright:  © 2017 anyks.com
*/
 
 
#include "http.h"


using namespace std;

 
inline size_t Connects::get(){
	
	return this->connects;
}
 
inline bool Connects::end(){
	
	return (this->connects < 1);
}
 
inline void Connects::inc(){
	
	this->connects++;
}
 
inline void Connects::dec(){
	
	if(this->connects > 0) this->connects--;
}
 
inline void Connects::signal(){
	
	this->condition.notify_one();
}
 
inline void Connects::broadcastSignal(){
	
	this->condition.notify_all();
}
 
inline void Connects::wait(recursive_mutex &mutx){
	
	unique_lock <recursive_mutex> locker(mutx);
	
	this->condition.wait(locker);
	
	
}
 
Connects::Connects(){
	
	this->connects = 1;
}
 
inline void BufferHttpProxy::lock(){
	
	this->lock_thread.lock();
}
 
inline void BufferHttpProxy::unlock(){
	
	this->lock_thread.unlock();
}
 
Connects * ClientConnects::get(const string client){
	
	Connects * connect = nullptr;
	
	this->mtx.lock();
	
	if(this->connects.count(client) > 0){
		
		connect = (this->connects.find(client)->second).get();
	}
	
	this->mtx.unlock();
	
	return connect;
}
 
void ClientConnects::add(const string client){
	
	this->mtx.lock();
	
	if(this->connects.count(client) < 1){
		
		unique_ptr <Connects> connect(new Connects);
		
		this->connects.insert(pair <string, unique_ptr <Connects>> (client, move(connect)));
	}
	
	this->mtx.unlock();
}
 
void ClientConnects::rm(const string client){
	
	this->mtx.lock();
	
	if(this->connects.count(client) > 0){
		
		auto it = this->connects.find(client);
		
		this->connects.erase(it);
	}
	
	this->mtx.unlock();
}
 
void BufferHttpProxy::appconn(const bool flag){
	
	this->lock();
	
	Connects * connect = (* this->connects).get(this->client.ip);
	
	if(connect){
		
		if(flag) connect->inc();
		
		else {
			
			connect->dec();
			
			connect->signal();
		}
		
		if(connect->end()) (* this->connects).rm(this->client.ip);
	
	} else if(flag) (* this->connects).add(this->client.ip);
	
	this->unlock();
}
 
void BufferHttpProxy::free_socket(evutil_socket_t * fd){
	
	if(*fd > -1){
		
		shutdown(*fd, SHUT_RDWR);
		
		::close(*fd);
		
		*fd = -1;
	}
}
 
void BufferHttpProxy::free_event(struct bufferevent ** event){
	
	if(*event){
		
		bufferevent_set_timeouts(*event, nullptr, nullptr);
		
		bufferevent_free(*event);
		
		*event = nullptr;
	}
}
 
void BufferHttpProxy::blockconnect(){
	
	Connects * connect = (* this->connects).get(this->client.ip);
	
	if(connect){
		
		this->myconns = connect->get();
		
		if(this->myconns >= this->proxy->config->connects.max){
			
			connect->wait(this->lock_connect);
		}
	}
	
	this->appconn(true);
}
 
void BufferHttpProxy::close_client(){
	
	this->lock();
	
	free_socket(&this->sockets.client);
	
	free_event(&this->events.client);
	
	this->unlock();
}
 
void BufferHttpProxy::close_server(){
	
	this->lock();
	
	free_socket(&this->sockets.server);
	
	free_event(&this->events.server);
	
	this->unlock();
}
 
void BufferHttpProxy::close(){
	
	this->lock();
	
	close_client();
	
	close_server();
	
	appconn(false);
	
	event_base_loopexit(this->base, nullptr);
	
	this->unlock();
}
 
void BufferHttpProxy::set_timeout(const u_short type, bool read, bool write){
	
	struct timeval _read = {this->proxy->config->timeouts.read, 0};
	
	struct timeval _write = {this->proxy->config->timeouts.write, 0};
	
	if(this->proxy->config->timeouts.read < 1)	read	= false;
	if(this->proxy->config->timeouts.write < 1)	write	= false;
	
	if((type & TM_SERVER) && this->events.server)
		
		bufferevent_set_timeouts(this->events.server, (read ? &_read : nullptr), (write ? &_write : nullptr));
	
	if((type & TM_CLIENT) && this->events.client)
		
		bufferevent_set_timeouts(this->events.client, (read ? &_read : nullptr), (write ? &_write : nullptr));
}
 
void BufferHttpProxy::sleep(size_t size, bool type){
	
	int seconds = 0;
	
	float max = float(type ? this->proxy->config->buffers.read : this->proxy->config->buffers.write);
	
	if(max > 0){
		
		max = (max / /f/l/o/a/t/(/t/h/i/s/-/>/m/y/c/o/n/n/s/)/)/;/
/	/	
		if((max > 0) && (size > max)) seconds = (size / /m/a/x/)/;/
/	/}/
/	
	this_thread::sleep_for(chrono::seconds(seconds));
}
 
BufferHttpProxy::BufferHttpProxy(System * proxy){
	
	this->proxy = proxy;
	
	this->base = event_base_new();
	
	Http parser(this->proxy->config->proxy.name, this->proxy->config->options);
	
	this->parser = parser;
	
	switch(this->proxy->config->proxy.ipver){
		
		case 4: this->dns = new DNSResolver(this->proxy->log, this->base, AF_INET, this->proxy->config->proxy.resolver);	break;
		
		case 6: this->dns = new DNSResolver(this->proxy->log, this->base, AF_INET6, this->proxy->config->proxy.resolver);	break;
	}
}
 
BufferHttpProxy::~BufferHttpProxy(){
	
	close();
	
	delete this->dns;
	
	event_base_free(this->base);
}
 
string HttpProxy::get_mac(void * ctx){
	
	char buff[256];
	
	struct sockaddr * s = reinterpret_cast <struct sockaddr *> (ctx);
	
	unsigned char * ptr = (unsigned char *) s->sa_data;
	
	sprintf(
		buff, "%02X:%02X:%02X:%02X:%02X:%02X",
		(ptr[0] & 0xff), (ptr[1] & 0xff), (ptr[2] & 0xff),
		(ptr[3] & 0xff), (ptr[4] & 0xff), (ptr[5] & 0xff)
	);
	
	return buff;
}
 
string HttpProxy::get_ip(int family, void * ctx){
	
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
 
int HttpProxy::socket_nosigpipe(evutil_socket_t fd, LogApp * log){
	
	int nosigpipe = 1;
	
	if(setsockopt(fd, SOL_SOCKET, SO_NOSIGPIPE, &nosigpipe, sizeof(nosigpipe)) < 0){
		
		log->write(LOG_ERROR, 0, "cannot set SO_NOSIGPIPE option on socket %d", fd);
		
		return -1;
	}
	
	return 0;
}
 
int HttpProxy::socket_nonblocking(evutil_socket_t fd, LogApp * log){
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
 
int HttpProxy::socket_tcpnodelay(evutil_socket_t fd, LogApp * log){
	
	int tcpnodelay = 1;
	
	if(setsockopt(fd, IPPROTO_TCP, TCP_NODELAY, &tcpnodelay, sizeof(tcpnodelay)) < 0){
		
		log->write(LOG_ERROR, 0, "cannot set TCP_NODELAY option on socket %d", fd);
		
		return -1;
	}
	
	return 0;
}
 
int HttpProxy::socket_tcpcork(evutil_socket_t fd, LogApp * log){
	
	int tcpcork = 1;

#ifdef __linux__
	
	if(setsockopt(fd, IPPROTO_TCP, TCP_CORK, &tcpcork, sizeof(tcpcork)) < 0){
		
		log->write(LOG_ERROR, 0, "cannot set TCP_CORK option on socket %d", fd);
		
		return -1;
	}

#elif __APPLE__ || __FreeBSD__
	
	if(setsockopt(fd, IPPROTO_TCP, TCP_NOPUSH, &tcpcork, sizeof(tcpcork)) < 0){
		
		log->write(LOG_ERROR, 0, "cannot set TCP_NOPUSH option on socket %d", fd);
		
		return -1;
	}
#endif
	
	return 0;
}
 
int HttpProxy::socket_reuseable(evutil_socket_t fd, LogApp * log){
	
	int reuseaddr = 1;
	
	if(setsockopt(fd, SOL_SOCKET, SO_REUSEADDR, &reuseaddr, sizeof(reuseaddr)) < 0){
		
		log->write(LOG_ERROR, 0, "cannot set SO_REUSEADDR option on socket %d", fd);
		
		return -1;
	}
	
	return 0;
}
 
int HttpProxy::socket_keepalive(evutil_socket_t fd, LogApp * log, int cnt, int idle, int intvl){
	
	int keepalive = 1;
	
	if(setsockopt(fd, SOL_SOCKET, SO_KEEPALIVE, &keepalive, sizeof(int))){
		
		log->write(LOG_ERROR, 0, "cannot set SO_KEEPALIVE option on socket %d", fd);
		
		return -1;
	}
	
	if(setsockopt(fd, IPPROTO_TCP, TCP_KEEPCNT, &cnt, sizeof(int))){
		
		log->write(LOG_ERROR, 0, "cannot set TCP_KEEPCNT option on socket %d", fd);
		
		return -1;
	}

#ifdef __APPLE__
	
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
 
int HttpProxy::socket_buffersize(evutil_socket_t fd, int read_size, int write_size, u_int maxcon, LogApp * log){
	
	socklen_t read_optlen	= sizeof(read_size);
	socklen_t write_optlen	= sizeof(write_size);
	
	if(read_size > 0){
		
		read_size = read_size / /m/a/x/c/o/n/;/
/	/	
		setsockopt(fd, SOL_SOCKET, SO_RCVBUF, (char *) &read_size, read_optlen);
	}
	
	if(write_size > 0){
		
		write_size = write_size / /m/a/x/c/o/n/;/
/	/	
		setsockopt(fd, SOL_SOCKET, SO_SNDBUF, (char *) &write_size, write_optlen);
	}
	
	if((getsockopt(fd, SOL_SOCKET, SO_RCVBUF, &read_size, &read_optlen) < 0)
	|| (getsockopt(fd, SOL_SOCKET, SO_SNDBUF, &write_size, &write_optlen) < 0)){
		
		log->write(LOG_ERROR, 0, "get buffer wrong on socket %d", fd);
		
		return -1;
	}
	
	return 0;
}
 
bool HttpProxy::check_auth(void * ctx){
	
	BufferHttpProxy * http = reinterpret_cast <BufferHttpProxy *> (ctx);
	
	if(http){
		
		const char * username = "zdD786KeuS";
		
		const char * password = "k.frolovv@gmail.com";
		
		if(!strcmp(http->httpData.getLogin().c_str(), username)
		&& !strcmp(http->httpData.getPassword().c_str(), password)) return true;
		
		http->proxy->log->write(LOG_MESSAGE, 0, "auth client [%s] to proxy wrong!", http->client.ip.c_str());
	}
	
	return false;
}
 
bool HttpProxy::isallow_remote_connect(const string ip, void * ctx){
	
	BufferHttpProxy * http = reinterpret_cast <BufferHttpProxy *> (ctx);
	
	if(http){
		
		Network nwk;
		
		int check = -1;
		
		switch(http->proxy->config->proxy.ipver){
			
			case 4: check = nwk.isLocal(ip);	break;
			
			case 6: check = nwk.isLocal6(ip);	break;
		}
		
		switch(check){
			
			case -1: return false;
			
			case 0: return http->proxy->config->proxy.reverse;
			
			case 1: return http->proxy->config->proxy.forward;
		}
	}
	
	return false;
}
 
int HttpProxy::connect_server(void * ctx){
	
	BufferHttpProxy * http = reinterpret_cast <BufferHttpProxy *> (ctx);
	
	if(http){
		
		if(!http->events.server){
			
			string bindhost;
			
			socklen_t sinlen = 0, sotlen = 0;
			
			struct sockaddr * sin = nullptr, * sot = nullptr;
			
			struct sockaddr_in server4_addr, client4_addr;
			
			struct sockaddr_in6 server6_addr, client6_addr;
			
			switch(http->proxy->config->proxy.ipver){
				
				case 4: {
					
					bindhost = http->proxy->config->ipv4.external;
					
					struct hostent * client = gethostbyname2(bindhost.c_str(), AF_INET);
					
					memset(&client4_addr, 0, sizeof(client4_addr));
					
					memset(&server4_addr, 0, sizeof(server4_addr));
					
					client4_addr.sin_family = AF_INET;
					server4_addr.sin_family = AF_INET;
					
					client4_addr.sin_port = htons(0);
					
					server4_addr.sin_port = htons(http->server.port);
					
					client4_addr.sin_addr.s_addr = * ((unsigned long *) client->h_addr);
					
					server4_addr.sin_addr.s_addr = inet_addr(http->server.host.c_str());
					
					bzero(&server4_addr.sin_zero, sizeof(server4_addr.sin_zero));
					
					sinlen = sizeof(client4_addr);
					sotlen = sizeof(server4_addr);
					
					sin	= reinterpret_cast <struct sockaddr *> (&client4_addr);
					sot	= reinterpret_cast <struct sockaddr *> (&server4_addr);
					
					http->sockets.server = socket(AF_INET, SOCK_STREAM, IPPROTO_TCP);
				} break;
				
				case 6: {
					
					char host_client[50], host_server[50];
					
					bindhost = http->proxy->config->ipv6.external;
					
					struct hostent * client	= gethostbyname2(bindhost.c_str(), AF_INET6);
					struct hostent * server	= gethostbyname2(http->server.host.c_str(), AF_INET6);
					
					inet_ntop(AF_INET6, (struct in_addr *) client->h_addr, host_client, sizeof(host_client));
					inet_ntop(AF_INET6, (struct in_addr *) server->h_addr, host_server, sizeof(host_server));
					
					memset(&client6_addr, 0, sizeof(client6_addr));
					
					memset(&server6_addr, 0, sizeof(server6_addr));
					
					client6_addr.sin6_family = AF_INET6;
					server6_addr.sin6_family = AF_INET6;
					
					client6_addr.sin6_port = htons(0);
					
					server6_addr.sin6_port = htons(http->server.port);
					
					memcpy(client6_addr.sin6_addr.s6_addr, host_client, sizeof(host_client));
					memcpy(server6_addr.sin6_addr.s6_addr, host_server, sizeof(host_server));
					
					sinlen = sizeof(client6_addr);
					sotlen = sizeof(server6_addr);
					
					sin	= reinterpret_cast <struct sockaddr *> (&client6_addr);
					sot	= reinterpret_cast <struct sockaddr *> (&server6_addr);
					
					http->sockets.server = socket(AF_INET6, SOCK_STREAM, IPPROTO_TCP);
				} break;
			}
			
			http->server.mac = get_mac(sot);
			
			if(http->sockets.server < 0){
				
				http->proxy->log->write(
					LOG_ERROR, 0,
					"creating socket to server = %s, port = %d, client = %s",
					http->server.host.c_str(),
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
					http->proxy->config->keepalive.keepcnt,
					http->proxy->config->keepalive.keepidle,
					http->proxy->config->keepalive.keepintvl
				);
			
			} else {
				
				socket_tcpcork(http->sockets.server, http->proxy->log);
				socket_tcpcork(http->sockets.client, http->proxy->log);
			}
			
			if(::bind(http->sockets.server, sin, sinlen) < 0){
				
				http->proxy->log->write(LOG_ERROR, 0, "bind local network [%s] error", bindhost.c_str());
				
				return 0;
			}
			
			http->events.server = bufferevent_socket_new(http->base, http->sockets.server, BEV_OPT_CLOSE_ON_FREE | BEV_OPT_DEFER_CALLBACKS);
			
			bufferevent_setwatermark(http->events.server, EV_READ | EV_WRITE, 1, 0);
			
			bufferevent_setcb(http->events.server, &HttpProxy::read_server_cb, nullptr, &HttpProxy::event_cb, http);
			
			bufferevent_enable(http->events.server, EV_READ | EV_WRITE);
			
			bufferevent_flush(http->events.server, EV_READ | EV_WRITE, BEV_FINISHED);
			
			if(bufferevent_socket_connect(http->events.server, sot, sotlen) < 0){
				
				http->proxy->log->write(
					LOG_ERROR, 0,
					"connecting to server = %s, port = %d, client = %s",
					http->server.host.c_str(),
					http->server.port,
					http->client.ip.c_str()
				);
				
				return -1;
			}
			
			http->proxy->log->write(
				LOG_MESSAGE, 0,
				"connect client [%s] to host = %s [%s:%d], mac = %s, method = %s, path = %s, useragent = %s, socket = %d",
				http->client.ip.c_str(),
				http->httpData.getHost().c_str(),
				http->server.host.c_str(),
				http->server.port,
				http->server.mac.c_str(),
				http->httpData.getMethod().c_str(),
				http->httpData.getPath().c_str(),
				http->httpData.getUseragent().c_str(),
				http->sockets.client
			);
			
			return 1;
		
		} else {
			
			http->proxy->log->write(
				LOG_MESSAGE, 0,
				"last connect client [%s] to host = %s [%s:%d], mac = %s, method = %s, path = %s, useragent = %s, socket = %d",
				http->client.ip.c_str(),
				http->httpData.getHost().c_str(),
				http->server.host.c_str(),
				http->server.port,
				http->server.mac.c_str(),
				http->httpData.getMethod().c_str(),
				http->httpData.getPath().c_str(),
				http->httpData.getUseragent().c_str(),
				http->sockets.client
			);
			
			return 2;
		}
	}
	
	return -1;
}
 
void HttpProxy::event_cb(struct bufferevent * bev, short events, void * ctx){
	
	BufferHttpProxy * http = reinterpret_cast <BufferHttpProxy *> (ctx);
	
	if(http){
		
		http->lock();
		
		evutil_socket_t current_fd = bufferevent_getfd(bev);
		
		string subject = (current_fd == http->sockets.client ? "client" : "server");
		
		if(events & BEV_EVENT_CONNECTED){
			
			if(subject.compare("server") == 0){
				
				http->proxy->log->write(
					LOG_ACCESS, 0,
					"connect client [%s], useragent = %s, socket = %d to server [%s:%d]",
					http->client.ip.c_str(),
					http->client.useragent.c_str(),
					current_fd,
					http->server.host.c_str(),
					http->server.port
				);
			}
		
		} else if(events & (BEV_EVENT_ERROR | BEV_EVENT_TIMEOUT | BEV_EVENT_EOF)) {
			
			if(events & BEV_EVENT_ERROR){
				
				int err = bufferevent_socket_get_dns_error(bev);
				
				if(err) http->proxy->log->write(LOG_ERROR, 0, "DNS error: %s", evutil_gai_strerror(err));
			}
			
			if(subject.compare("client") == 0){
				
				http->proxy->log->write(
					LOG_ACCESS, 0,
					"closing client [%s] from server [%s:%d], socket = %d",
					http->client.ip.c_str(),
					http->server.host.c_str(),
					http->server.port,
					current_fd
				);
			
			} else {
				
				http->proxy->log->write(
					LOG_ACCESS, 0,
					"closing server [%s:%d] from client [%s], socket = %d",
					http->server.host.c_str(),
					http->server.port,
					http->client.ip.c_str(),
					current_fd
				);
			}
			
			http->close();
		}
		
		http->unlock();
	}
	
	return;
}
 
void HttpProxy::read_server_cb(struct bufferevent * bev, void * ctx){
	
	BufferHttpProxy * http = reinterpret_cast <BufferHttpProxy *> (ctx);
	
	if(http){
		
		struct evbuffer * input		= bufferevent_get_input(http->events.server);
		struct evbuffer * output	= bufferevent_get_output(http->events.client);
		
		http->set_timeout(TM_CLIENT, true, true);
		
		size_t len = evbuffer_get_length(input);


		 


		
		http->sleep(len, true);
		
		if(http->client.connect || http->headers.getFullHeaders()) evbuffer_add_buffer(output, input);
		
		else if(len){
			
			if(!http->headers.size()){
				
				HttpData httpData(http->proxy->config->proxy.name, http->proxy->config->options);
				
				http->headers = httpData;
			}
			
			while(true){
				
				const char * line = evbuffer_readln(input, &len, EVBUFFER_EOL_CRLF_STRICT);
				
				if(line && !strlen(line)) http->headers.setFullHeaders();
				
				if(!line || !strlen(line)) break;
				
				http->headers.addHeader(line);
			}
			
			if(http->headers.getFullHeaders()){
				
				if((http->headers.getHttp().find("101 Switching Protocols") != string::npos)
				&& !http->headers.getHeader("upgrade").empty()
				&& (http->headers.getHeader("connection").find("Upgrade") != string::npos)){
					
					
					http->client.connect = true;
				}
				
				struct evbuffer * tmp = evbuffer_new();
				
				string headers = http->headers.getResponseHeaders();
				
				evbuffer_add(tmp, headers.data(), headers.length());
				
				len = evbuffer_get_length(input);
				
				char * buffer = new char[len];
				
				evbuffer_copyout(input, buffer, len);
				
				evbuffer_add(tmp, buffer, len);
				
				delete [] buffer;
				
				evbuffer_add_buffer(output, tmp);
				
				evbuffer_drain(input, len);
				
				evbuffer_free(tmp);
			}
		
		} else http->close();
	}
	
	return;
}
 
void HttpProxy::resolve_cb(const string ip, void * ctx){
	
	BufferHttpProxy * http = reinterpret_cast <BufferHttpProxy *> (ctx);
	
	if(http){
		
		auto httpData = http->parser.httpData.begin();
		
		if(!ip.empty()){
			
			if(isallow_remote_connect(ip, http)){
				
				bool conn_enabled = (http->proxy->config->options & OPT_CONNECT);
				
				if(!http->auth) http->auth = check_auth(http);
				
				if(!http->auth && (http->httpData.getLogin().empty()
				|| http->httpData.getPassword().empty())){
					
					http->response = http->httpData.requiredAuth();
				
				} else if(!http->auth) {
					
					http->response = http->httpData.faultAuth();
				
				} else {
					
					u_int port = http->httpData.getPort();
					
					if(http->events.server
					&& ((http->server.host.compare(ip) != 0)
					|| (http->server.port != port))) http->close_server();
					
					http->server.host = ip;
					http->server.port = port;
					
					http->client.alive		= http->httpData.isAlive();
					http->client.https		= http->httpData.isHttps();
					http->client.connect	= http->httpData.isConnect();
					http->client.useragent	= http->httpData.getUseragent();
					
					int connect = connect_server(http);
					
					if(connect > 0){
						
						if(http->client.connect && (conn_enabled || http->client.https))
							
							http->response = http->httpData.authSuccess();
						
						else if(!conn_enabled && http->client.connect)
							
							http->response = http->httpData.faultConnect();
						
						else {
							
							if(!http->client.alive) http->httpData.setClose();
							
							http->response = http->httpData.getRequest();
							
							if(!http->client.alive)
								
								http->set_timeout(TM_SERVER, true, true);
							
							else http->set_timeout(TM_SERVER, false, true);
							
							http->sleep(http->response.size(), false);
							
							bufferevent_write(http->events.server, http->response.data(), http->response.size());
							
							http->parser.httpData.erase(httpData);
							
							if(!http->parser.httpData.empty()) do_request(http, true);
							
							else http->httpData.clear();
							
							return;
						}
					
					} else if(connect == 0) http->response = http->httpData.faultConnect();
					
					else {
						
						http->close();
						
						return;
					}
				}
			
			} else http->response = http->httpData.faultAuth();
		
		} else {
			
			http->proxy->log->write(
				LOG_ERROR, 0,
				"host server = %s not found, port = %d, client = %s, socket = %d",
				http->httpData.getHost().c_str(),
				http->httpData.getPort(),
				http->client.ip.c_str(),
				http->sockets.client
			);
			
			http->response = http->httpData.faultConnect();
		}
		
		if(!http->response.empty()){
			
			if(http->response.code == 200) http->set_timeout(TM_CLIENT, true, true);
			
			else http->set_timeout(TM_CLIENT, false, true);
			
			if(http->response.code == 200) http->sleep(http->response.size(), false);
			
			bufferevent_setwatermark(http->events.client, EV_WRITE, http->response.size(), 0);
			
			bufferevent_write(http->events.client, http->response.data(), http->response.size());
			
			http->parser.httpData.erase(httpData);
		}
	}
	
	return;
}
 
void HttpProxy::do_request(void * ctx, bool flag){
	
	BufferHttpProxy * http = reinterpret_cast <BufferHttpProxy *> (ctx);
	
	if(http){
		
		if(!http->parser.httpData.empty() && (!http->httpData.size() || flag)){
			
			http->set_timeout(TM_CLIENT);
			
			http->response.clear();
			
			auto httpData = http->parser.httpData.begin();
			
			http->httpData = * httpData;
			
			http->dns->resolve(http->httpData.getHost(), &HttpProxy::resolve_cb, http);
		}
	}
	
	return;
}
 
void HttpProxy::write_client_cb(struct bufferevent * bev, void * ctx){
	
	BufferHttpProxy * http = reinterpret_cast <BufferHttpProxy *> (ctx);
	
	if(http != NULL){
		
		if(!http->response.empty()
		&& (http->response.code != 200)) http->close();
	}
	
	return;
}
 
void HttpProxy::read_client_cb(struct bufferevent * bev, void * ctx){
	
	BufferHttpProxy * http = reinterpret_cast <BufferHttpProxy *> (ctx);
	
	if(http != NULL){
		
		http->set_timeout(TM_CLIENT | TM_SERVER);
		
		bool conn_enabled = (http->proxy->config->options & OPT_CONNECT);
		
		struct evbuffer * input = bufferevent_get_input(bev);
		
		size_t len = evbuffer_get_length(input);
		
		http->sleep(len, true);
		
		if(http->client.connect && (conn_enabled || http->client.https)){
			
			struct evbuffer * output = bufferevent_get_output(http->events.server);
			
			if(!http->client.alive)
				
				http->set_timeout(TM_SERVER, true, true);
			
			else http->set_timeout(TM_SERVER, false, true);
			
			evbuffer_add_buffer(output, input);
		
		} else if(!http->client.connect) {
			
			char * buffer = new char[len];
			
			evbuffer_copyout(input, buffer, len);
			
			size_t size = http->parser.parse(buffer, len);
			
			evbuffer_drain(input, size);
			
			delete [] buffer;
			
			do_request(http);
		
		} else http->close();
	}
	
	return;
}
 
void HttpProxy::connection(void * ctx){
	
	BufferHttpProxy * http = reinterpret_cast <BufferHttpProxy *> (ctx);
	
	if(http != NULL){
		
		http->blockconnect();
		
		http->events.client = bufferevent_socket_new(http->base, http->sockets.client, BEV_OPT_CLOSE_ON_FREE | BEV_OPT_DEFER_CALLBACKS);
		
		http->set_timeout(TM_CLIENT, true);
		
		bufferevent_setwatermark(http->events.client, EV_READ | EV_WRITE, 5, 0);
		
		bufferevent_setcb(http->events.client, &HttpProxy::read_client_cb, &HttpProxy::write_client_cb, &HttpProxy::event_cb, http);
		
		bufferevent_enable(http->events.client, EV_READ | EV_WRITE);
		
		bufferevent_flush(http->events.client, EV_READ | EV_WRITE, BEV_FINISHED);
		
		event_base_dispatch(http->base);
		
		delete http;
	}
	
	return;
}
 
void HttpProxy::accept_cb(evutil_socket_t fd, short event, void * ctx){
	
	HttpProxy * proxy = reinterpret_cast <HttpProxy *> (ctx);
	
	if(proxy != NULL){
		
		string ip, mac;
		
		evutil_socket_t socket = -1;
		
		switch(proxy->server->config->proxy.ipver){
			
			case 4: {
				
				struct sockaddr_in client;
				
				socklen_t len = sizeof(client);
				
				socket = accept(fd, reinterpret_cast <struct sockaddr *> (&client), &len);
				
				if(socket < 1) return;
				
				ip = get_ip(AF_INET, &client);
				
				mac = get_mac(&client);
			} break;
			
			case 6: {
				
				struct sockaddr_in6 client;
				
				socklen_t len = sizeof(client);
				
				socket = accept(fd, reinterpret_cast <struct sockaddr *> (&client), &len);
				
				if(socket < 1) return;
				
				ip = get_ip(AF_INET6, &client);
				
				mac = get_mac(&client);
			} break;
		}
		
		socket_nonblocking(socket, proxy->server->log);
		
		socket_reuseable(socket, proxy->server->log);
		
		socket_nosigpipe(socket, proxy->server->log);
		
		proxy->server->log->write(LOG_ACCESS, 0, "client connect to proxy server, host = %s, mac = %s, socket = %d", ip.c_str(), mac.c_str(), socket);
		
		BufferHttpProxy * http = new BufferHttpProxy(proxy->server);
		
		http->connects = &proxy->connects;
		
		http->sockets.client = socket;
		
		http->client.mac = mac;
		
		http->client.ip = ip;
		
		std::thread thr(&HttpProxy::connection, http);
		
		thr.detach();
	}
}
 
evutil_socket_t HttpProxy::create_server(){
	
	evutil_socket_t sock = -1;
	
	socklen_t sinlen = 0;
	
	struct sockaddr * sin = NULL;
	
	struct sockaddr_in server4_addr;
	
	struct sockaddr_in6 server6_addr;
	
	switch(this->server->config->proxy.ipver){
		
		case 4: {
			
			memset(&server4_addr, 0, sizeof(server4_addr));
			
			struct hostent * bindhost = gethostbyname2(this->server->config->ipv4.internal.c_str(), AF_INET);
			
			server4_addr.sin_family = AF_INET;
			
			server4_addr.sin_addr.s_addr = * ((unsigned long *) bindhost->h_addr);
			
			server4_addr.sin_port = htons(this->server->config->proxy.port);
			
			sinlen = sizeof(server4_addr);
			
			sin = reinterpret_cast <struct sockaddr *> (&server4_addr);
			
			bzero(&server4_addr.sin_zero, sizeof(server4_addr.sin_zero));
			
			sock = socket(AF_INET, SOCK_STREAM, IPPROTO_TCP);
		} break;
		
		case 6: {
			
			char straddr[50];
			
			memset(&server6_addr, 0, sizeof(server6_addr));
			
			struct hostent * bindhost = gethostbyname2(this->server->config->ipv6.internal.c_str(), AF_INET6);
			
			inet_ntop(AF_INET6, (struct in_addr *) bindhost->h_addr, straddr, sizeof(straddr));
			
			memcpy(server6_addr.sin6_addr.s6_addr, straddr, sizeof(straddr));
			
			server6_addr.sin6_family = AF_INET6;
			
			server6_addr.sin6_port = htons(this->server->config->proxy.port);
			
			sinlen = sizeof(server6_addr);
			
			sin = reinterpret_cast <struct sockaddr *> (&server6_addr);
			
			sock = socket(AF_INET6, SOCK_STREAM, IPPROTO_TCP);
		} break;
	}
	
	if(sock < 0){
		
		this->server->log->write(LOG_ERROR, 0, "[-] could not create socket");
		
		return -1;
	}
	
	socket_nonblocking(sock, this->server->log);
	
	socket_reuseable(sock, this->server->log);
	
	socket_tcpnodelay(sock, this->server->log);
	
	socket_nosigpipe(sock, this->server->log);
	
	if(::bind(sock, sin, sinlen) < 0){
		
		this->server->log->write(LOG_ERROR, 0, "[-] bind error");
		
		return -1;
	}
	
	if(listen(sock, this->server->config->connects.all) < 0){
		
		this->server->log->write(LOG_ERROR, 0, "[-] listen error");
		
		return -1;
	}
	
	return sock;
}
 
void HttpProxy::run_server(evutil_socket_t socket, void * ctx){
	
	HttpProxy * proxy = reinterpret_cast <HttpProxy *> (ctx);
	
	if(proxy != NULL){
		
		proxy->server->log->write(LOG_MESSAGE, 0, "[+] start service: pid = %i, socket = %i", getpid(), socket);
		
		proxy->base = event_base_new();
		
		struct event * evnt = event_new(proxy->base, socket, EV_READ | EV_PERSIST, &HttpProxy::accept_cb, proxy);
		
		event_add(evnt, NULL);
		
		event_base_loop(proxy->base, EVLOOP_NO_EXIT_ON_EMPTY);
		
		shutdown(socket, SHUT_RDWR);
		
		close(socket);
		
		event_del(evnt);
		
		event_free(evnt);
		
		event_base_free(proxy->base);
	}
}
 
void HttpProxy::run_works(pid_t * pids, evutil_socket_t socket, size_t cur, size_t max, void * ctx){
	
	HttpProxy * proxy = reinterpret_cast <HttpProxy *> (ctx);
	
	if((pids != NULL) && (proxy != NULL) && (socket > 0) && max){
		
		if(cur < max){
			
			switch(pids[cur] = fork()){
				
				case -1: {
					
					proxy->server->log->write(LOG_ERROR, 0, "[-] create fork error");
					
					exit(SIGHUP);
				}
				
				case 0: run_server(socket, proxy); break;
				
				default: run_works(pids, socket, cur + 1, max, proxy);
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
	
	if(proxy != NULL){
		
		this->server = proxy;
		
		evutil_socket_t socket = create_server();
		
		if(socket > -1){
			
			if(!this->server->config->proxy.debug){
				
				u_int max_works = (
					this->server->config->proxy.workers
					? this->server->config->proxy.workers
					: this->server->config->os.ncpu
				);
				
				if(max_works > MMAX_WORKERS) max_works = MMAX_WORKERS;
				
				pids = new pid_t[max_works];
				
				run_works(pids, socket, 0, max_works, this);
			
			} else {
				
				pid_t pids[] = {getpid()};
				
				this->server->sendPids(pids, 1);
				
				run_server(socket, this);
			}
		
		} else exit(SIGSTOP);
	}
}
 
HttpProxy::~HttpProxy(){
	
	if(pids != NULL) delete [] pids;
}