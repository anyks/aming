/* SERVER PROXY AMING */
/*
*  author:     Yuriy Lobarev
*  phone:      +7(910)983-95-90
*  telegram:   @forman
*  email:      info@anyks.com
*  date:       10/29/2017 17:06:00
*  copyright:  © 2017 anyks.com
*/
 
 

#include "proxy/proxy.h"


using namespace std;

 
void Proxy::signal_log(int num, void * ctx){
	
	Proxy * proxy = reinterpret_cast <Proxy *> (ctx);
	
	if(proxy){
		
		switch(num){
			case SIGABRT:	proxy->sys->log->write(LOG_ERROR, 0, "[-] Process abort signal [%d]", num);									break;
			case SIGALRM:	proxy->sys->log->write(LOG_ERROR, 0, "[-] Alarm clock [%d]", num);											break;
			case SIGBUS:	proxy->sys->log->write(LOG_ERROR, 0, "[-] Access to an undefined portion of a memory object [%d]", num);	break;
			case SIGCHLD:	proxy->sys->log->write(LOG_ERROR, 0, "[-] Child process terminated, stopped, or continued [%d]", num);		break;
			case SIGCONT:	proxy->sys->log->write(LOG_ERROR, 0, "[-] Continue executing, if stopped [%d]", num);						break;
			case SIGFPE:	proxy->sys->log->write(LOG_ERROR, 0, "[-] Erroneous arithmetic operation [%d]", num);						break;
			case SIGHUP:	proxy->sys->log->write(LOG_ERROR, 0, "[-] Hangup [%d]", num);												break;
			case SIGILL:	proxy->sys->log->write(LOG_ERROR, 0, "[-] Illegal instruction [%d]", num);									break;
			case SIGINT:	proxy->sys->log->write(LOG_ERROR, 0, "[-] Terminal interrupt signal [%d]", num);							break;
			case SIGKILL:	proxy->sys->log->write(LOG_ERROR, 0, "[-] Kill (cannot be caught or ignored) [%d]", num);					break;
			case SIGPIPE:	proxy->sys->log->write(LOG_ERROR, 0, "[-] Write on a pipe with no one to read it [%d]", num);				break;
			case SIGQUIT:	proxy->sys->log->write(LOG_ERROR, 0, "[-] Terminal quit signal [%d]", num);									break;
			case SIGSEGV:	proxy->sys->log->write(LOG_ERROR, 0, "[-] Invalid memory reference [%d]", num);								break;
			case SIGSTOP:	proxy->sys->log->write(LOG_ERROR, 0, "[-] Stop executing (cannot be caught or ignored) [%d]", num);			break;
			case SIGTERM:	proxy->sys->log->write(LOG_ERROR, 0, "[-] Termination signal [%d]", num);									break;
			case SIGTSTP:	proxy->sys->log->write(LOG_ERROR, 0, "[-] Terminal stop signal [%d]", num);									break;
			case SIGTTIN:	proxy->sys->log->write(LOG_ERROR, 0, "[-] Background process attempting read [%d]", num);					break;
			case SIGTTOU:	proxy->sys->log->write(LOG_ERROR, 0, "[-] Background process attempting write [%d]", num);					break;
			case SIGUSR1:	proxy->sys->log->write(LOG_MESSAGE, 0, "[=] User-defined signal 1 [%d]", num);								break;
			case SIGUSR2:	proxy->sys->log->write(LOG_MESSAGE, 0, "[=] User-defined signal 2 [%d]", num);								break;
			
			case SIGPROF:	proxy->sys->log->write(LOG_ERROR, 0, "[-] Profiling timer expired [%d]", num);								break;
			case SIGSYS:	proxy->sys->log->write(LOG_ERROR, 0, "[-] Bad sys call [%d]", num);											break;
			case SIGTRAP:	proxy->sys->log->write(LOG_ERROR, 0, "[-] Trace / breakpoint trap [%d]", num);								break;
			case SIGURG:	proxy->sys->log->write(LOG_ERROR, 0, "[-] High bandwidth data is available at a socket [%d]", num);			break;
			case SIGVTALRM:	proxy->sys->log->write(LOG_ERROR, 0, "[-] Virtual timer expired [%d]", num);								break;
			case SIGXCPU:	proxy->sys->log->write(LOG_ERROR, 0, "[-] CPU time limit exceeded [%d]", num);								break;
			case SIGXFSZ:	proxy->sys->log->write(LOG_ERROR, 0, "[-] File size limit exceeded [%d]", num);								break;
		}
	}
	
	return;
}
 
void Proxy::clear_fantoms(int signal, void * ctx){
	
	Proxy * proxy = reinterpret_cast <Proxy *> (ctx);
	
	if(proxy){
		
		signal_log(signal, proxy);
		
		System::Pids pids = proxy->sys->readPids();
		
		for(u_int i = 0; i < pids.len; i++) kill(pids.pids[i], SIGTERM);
		
		if(proxy->cpid || proxy->mpid){
			
			if(proxy->cpid) kill(proxy->cpid, SIGTERM);
			if(proxy->mpid) kill(proxy->mpid, SIGTERM);
			
			if(proxy->sys->os) proxy->sys->os->rmPid(EXIT_FAILURE);
		}
	}
	
	return;
}
 
void Proxy::siginfo_cb(evutil_socket_t fd, short event, void * ctx){
	
	SignalBuffer * buffer = reinterpret_cast <SignalBuffer *> (ctx);
	
	if(buffer) signal_log(buffer->signal, buffer->proxy);
	
	return;
}
 
void Proxy::siguser_cb(evutil_socket_t fd, short event, void * ctx){
	
	SignalBuffer * buffer = reinterpret_cast <SignalBuffer *> (ctx);
	
	if(buffer){
		
		signal_log(buffer->signal, buffer->proxy);
		
		Proxy * proxy = reinterpret_cast <Proxy *> (buffer->proxy);
		
		if(proxy){
			
			System::Pids pids = proxy->sys->readPids();
			
			proxy->sys->sendPids(pids.pids, pids.len);
			
			for(u_int i = 0; i < pids.len; i++) kill(pids.pids[i], SIGTERM);
		}
	}
	
	return;
}
 
 
 
void Proxy::sigsegv_cb(evutil_socket_t fd, short event, void * ctx){
	
	SignalBuffer * buffer = reinterpret_cast <SignalBuffer *> (ctx);
	
	if(buffer){
		
		clear_fantoms(buffer->signal, buffer->proxy);
		
		signal(buffer->signal, SIG_DFL);
		
		exit(SIGQUIT);
	}
	
	return;
}
 
void Proxy::sigexit_cb(evutil_socket_t fd, short event, void * ctx){
	
	SignalBuffer * buffer = reinterpret_cast <SignalBuffer *> (ctx);
	
	if(buffer){
		
		clear_fantoms(buffer->signal, buffer->proxy);
		
		exit(EXIT_SUCCESS);
	}
	
	return;
}
 
void Proxy::create_proxy(){
	
	this->sys->log->welcome();
	
	this->sys->os->privBind();
	
	this->sys->os->setFdLimit();
	
	HttpProxy http(this->sys);
}
 
void Proxy::run_worker(){
	
	switch(this->cpid = fork()){
		
		case -1: {
			
			perror("fork");
			
			exit(EXIT_FAILURE);
		} break;
		
		case 0: create_proxy(); break;
		
		default: {
			
			int status;
			
			this->sys->log->write(LOG_MESSAGE, 900, "[+] start balancer proccess, pid = %d", this->cpid);
			
			do {
				
				pid_t pid = waitpid(this->cpid, &status, WUNTRACED | WCONTINUED);
				
				if(pid < 0){
					perror("waitpid");
					exit(EXIT_FAILURE);
				}
				
				if(WIFEXITED(status)){
					
					this->sys->log->write(LOG_ERROR, 0, "exited, pid = %d, status = %d", this->cpid, WEXITSTATUS(status));
					
					if(WEXITSTATUS(status) == SIGSTOP) exit(SIGSTOP);
				}
				
				else if(WIFSIGNALED(status)) this->sys->log->write(LOG_ERROR, 0, "killed by pid = %d, signal %d", this->cpid, WTERMSIG(status));
				
				else if(WIFSTOPPED(status)) this->sys->log->write(LOG_ERROR, 0, "stopped by pid = %d, signal %d", this->cpid, WSTOPSIG(status));
				
				
			
			} while(!WIFEXITED(status) && !WIFSIGNALED(status));
			
			System::Pids pids = this->sys->readPids();
			
			for(u_int i = 0; i < pids.len; i++) kill(pids.pids[i], SIGTERM);
			
			run_worker();
			
			exit(EXIT_SUCCESS);
		}
	}
}
 
Proxy::Proxy(string configfile){
	
	this->sys = new System(configfile);
	
	if(!this->sys->config->proxy.debug
	&& this->sys->config->proxy.daemon){
		
		pid_t pid = fork();
		
		if(pid < 0) this->sys->os->rmPid(EXIT_FAILURE);
		
		if(pid > 0) this->sys->os->rmPid(EXIT_SUCCESS);
		
		umask(0);
		
		
		pid_t sid = setsid();
		
		if(sid < 0) this->sys->os->rmPid(EXIT_FAILURE);
		
		if((chdir("/")) < 0) this->sys->os->rmPid(EXIT_FAILURE);
		
		close(STDIN_FILENO);
		close(STDOUT_FILENO);
		close(STDERR_FILENO);
		
		this->sys->os->mkPid();
	}
	
	if(!this->sys->config->proxy.debug){
		
		this->mpid = fork();
		
		switch(this->mpid){
			
			case -1: {
				
				perror("fork");
				
				exit(EXIT_FAILURE);
			} break;
			
			case 0: run_worker(); break;
			default: {
				
				this->sys->log->write(LOG_MESSAGE, 500, "[+] start master proccess, pid = %d", getpid());
				this->sys->log->write(LOG_MESSAGE, 700, "[+] start slave proccess, pid = %d", this->mpid);
				
				this->base = event_base_new();
				
				
				this->siguser.push_back({SIGUSR1, this});
				
				this->siginfo.push_back({SIGPIPE, this});
				
				this->sigexit.push_back({SIGHUP, this});
				this->sigexit.push_back({SIGINT, this});
				this->sigexit.push_back({SIGQUIT, this});
				this->sigexit.push_back({SIGTERM, this});
				this->sigexit.push_back({SIGTSTP, this});
				
				this->sigexit.push_back({SIGTTIN, this});
				this->sigexit.push_back({SIGTTOU, this});
				
				this->sigsegv.push_back({SIGILL, this});
				this->sigsegv.push_back({SIGTRAP, this});
				this->sigsegv.push_back({SIGFPE, this});
				this->sigsegv.push_back({SIGBUS, this});
				this->sigsegv.push_back({SIGSEGV, this});
				this->sigsegv.push_back({SIGSYS, this});
				this->sigsegv.push_back({SIGXCPU, this});
				this->sigsegv.push_back({SIGXFSZ, this});
				
				
				for(u_int i = 0; i < this->siginfo.size(); i++)
					
					this->signals.push_back(
						evsignal_new(
							this->base,
							this->siguser[i].signal,
							&Proxy::siguser_cb,
							&this->siguser[i]
						)
					);
				
				for(u_int i = 0; i < this->siginfo.size(); i++)
					
					this->signals.push_back(
						evsignal_new(
							this->base,
							this->siginfo[i].signal,
							&Proxy::siginfo_cb,
							&this->siginfo[i]
						)
					);
				
				for(u_int i = 0; i < this->sigexit.size(); i++)
					
					this->signals.push_back(
						evsignal_new(
							this->base,
							this->sigexit[i].signal,
							&Proxy::sigexit_cb,
							&this->sigexit[i]
						)
					);
				
				for(u_int i = 0; i < this->sigsegv.size(); i++)
					
					this->signals.push_back(
						evsignal_new(
							this->base,
							this->sigsegv[i].signal,
							&Proxy::sigsegv_cb,
							&this->sigsegv[i]
						)
					);
				
				for(u_int i = 0; i < this->signals.size(); i++)
					
					evsignal_add(this->signals[i], nullptr);
				
				event_base_loop(this->base, EVLOOP_NO_EXIT_ON_EMPTY);
			}
		}
	
	} else create_proxy();
}
 
Proxy::~Proxy(){
	
	for(u_int i = 0; i < this->signals.size(); i++){
		
		if(this->signals[i]) event_free(this->signals[i]);
	}
	
	if(this->base) event_base_free(this->base);
	
	delete this->sys;
}