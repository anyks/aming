/* SERVER PROXY AMING */
/*
*  author:     Yuriy Lobarev
*  phone:      +7(910)983-95-90
*  telegram:   @forman
*  email:      info@anyks.com
*  date:       10/29/2017 16:59:43
*  copyright:  © 2017 anyks.com
*/
 
 

#include "os/os.h"


using namespace std;

 
Os::OsData Os::getOsName(){
	
	OsData result;
	
	#ifdef _WIN32
		
		result = {"Windows 32-bit", 1};
	#elif _WIN64
		
		result = {"Windows 64-bit", 2};
	#elif __APPLE__ || __MACH__
		
		result = {"MacOS X", 3};
	#elif __linux__
		
		result = {"Linux", 4};
	#elif __FreeBSD__
		
		result = {"FreeBSD", 5};
	#elif __unix || __unix__
		
		result = {"Unix", 6};
	#else
		
		result = {"Other", 7};
	#endif
	
	return result;
}
 
void Os::mkPid(){
	
	if(* config){
		
		string filename = ((* config)->proxy.piddir + string("/") + (* config)->proxy.name + ".pid");
		
		FILE * f = fopen(filename.c_str(), "w");
		
		if(f){
			
			fprintf(f, "%u", getpid());
			
			fclose(f);
		}
	}
}
 
void Os::rmPid(int ext){
	
	if(* config){
		
		string filename = ((* config)->proxy.piddir + string("/") + (* config)->proxy.name + ".pid");
		
		remove(filename.c_str());
		
		exit(ext);
	}
}
 
int Os::setFdLimit(){
	
	struct rlimit lim;
	
	lim.rlim_cur = (* this->config)->connects.fds;
	
	lim.rlim_max = (* this->config)->connects.fds;
	
	return setrlimit(RLIMIT_NOFILE, &lim);
}

#ifndef __linux__
 
long Os::getNumberParam(string name){
	
	int param = 0;
	
	size_t len = sizeof(param);
	
	if(sysctlbyname(name.c_str(), &param, &len, nullptr, 0) < 0){
		
		if(this->log) this->log->write(LOG_ERROR, 0, "filed get param: %s", name.c_str());
	}
	
	return param;
}
 
string Os::getStringParam(string name){
	
	char buffer[128];
	
	size_t len = sizeof(buffer);
	
	memset(buffer, 0, len);
	
	if(sysctlbyname(name.c_str(), &buffer, &len, nullptr, 0) < 0){
		
		if(this->log) this->log->write(LOG_ERROR, 0, "filed get param: %s", name.c_str());
	}
	
	return buffer;
}
 
void Os::setParam(string name, int param){
	
	if(sysctlbyname(name.c_str(), nullptr, 0, &param, sizeof(param)) < 0){
		
		if(this->log) this->log->write(LOG_ERROR, 0, "filed set param: %s -> %i", name.c_str(), param);
	}
}
 
void Os::setParam(string name, string param){
	
	const char * value = param.c_str();
	
	if(sysctlbyname(name.c_str(), nullptr, 0, (void *) value, param.size()) < 0){
		
		if(this->log) this->log->write(LOG_ERROR, 0, "filed set param: %s -> %s", name.c_str(), param.c_str());
	}
}
#endif
 
bool Os::enableCoreDumps(){
	
	if((* this->config)->proxy.debug){
		
		struct rlimit limit;
		
		limit.rlim_cur = RLIM_INFINITY;
		
		limit.rlim_max = RLIM_INFINITY;
		
		return (setrlimit(RLIMIT_CORE, &limit) == 0);
	}
	
	return false;
}
 
void Os::privBind(){
	uid_t uid;	
	gid_t gid;	
	
	string::size_type sz;
	
	if(Anyks::isNumber((* config)->proxy.user))
		
		uid = stoi((* config)->proxy.user, &sz);
	
	else uid = Anyks::getUid((* config)->proxy.user.c_str());
	
	if(Anyks::isNumber((* config)->proxy.group))
		
		gid = stoi((* config)->proxy.group, &sz);
	
	else gid = Anyks::getGid((* config)->proxy.group.c_str());
	
	setuid(uid);
	
	setgid(gid);
}
 
string Os::exec(string cmd, bool multiline){
	
	const int MAX_BUFFER = 2048;
	
	string result;
	
	char buffer[MAX_BUFFER];
	
	memset(buffer, '\0', MAX_BUFFER);
	
	FILE * stream = popen(cmd.c_str(), "r");
	
	if(stream){
		
		while(!feof(stream)){
			
			fgets(buffer, MAX_BUFFER, stream);
			
			result.append(buffer);
			
			if(!multiline) break;
		}
		
		pclose(stream);
		
		if(result.empty() && this->log) this->log->write(LOG_ERROR, 0, "filed set param: %s", buffer);
	}
	
	return result;
}
 
string Os::getCongestionControl(string str){
	
	smatch match;
	
	regex e("(cubic|htcp)", regex::ECMAScript | regex::icase);
	
	regex_search(str, match, e);
	
	if(!match.empty() && (match.size() == 2)) return match[1].str();
	
	return "";
}
 
void Os::getCPU(){
	
	OsData os = getOsName();
	
	u_int ncpu = 1;
	
	string cpu = "Unknown";
	
	switch(os.type){

#ifndef __linux__
		
		case 3: {
			
			ncpu = (u_int) getNumberParam("hw.ncpu");
			
			cpu = getStringParam("machdep.cpu.brand_string");
		} break;
#endif
		
		case 4: {
			
			smatch match;
			
			string str = exec("lscpu", true);
			
			regex e("CPU\\(s\\)\\:\\s+(\\d+)[\\s\\S]+Model\\s+name\\:\\s+([^\\r\\n]+)", regex::ECMAScript | regex::icase);
			
			regex_search(str, match, e);
			
			if(!match.empty() && (match.size() == 3)){
				
				ncpu = ::atoi(match[1].str().c_str());
				
				cpu = match[2].str();
			}
		} break;

#ifndef __linux__
		
		case 5: {
			
			ncpu = (u_int) getNumberParam("hw.ncpu");
			
			cpu = exec("grep -w CPU: /var/run/dmesg.boot");
			
			smatch match;
			
			regex e("CPU\\:\\s+([^\\r\\n]+)", regex::ECMAScript | regex::icase);
			
			regex_search(cpu, match, e);
			
			if(!match.empty() && (match.size() == 2)) cpu = match[1].str();
		} break;
#endif
	}
	
	(* config)->os = {ncpu, cpu, os.name};
}
 
void Os::optimos(){
	
	
	OsData os = getOsName();
	
	switch(os.type){
		
		case 1:
		case 2: {
			
			exec("netsh interface tcp set global congestionprovider=ctcp");
			
			exec("netsh interface tcp set global autotuninglevel=normal");
		} break;

#ifndef __linux__
		
		case 3: {
			
			setParam("net.inet.tcp.win_scale_factor", 8);
			
			setParam("net.inet.tcp.autorcvbufmax", 33554432);
			setParam("net.inet.tcp.autosndbufmax", 33554432);
			
			setParam("net.inet.tcp.sendspace", 1042560);
			setParam("net.inet.tcp.recvspace", 1042560);
			setParam("net.inet.tcp.slowstart_flightsize", 20);
			setParam("net.inet.tcp.local_slowstart_flightsize", 20);
			
			
			setParam("kern.ipc.maxsockbuf", 6291456);
			
			setParam("kern.ipc.somaxconn", 49152);
		} break;
#endif
		
		case 4: {
			
			if((* this->config)->proxy.debug){
				
				exec("sysctl -w kernel.core_uses_pid=1");
				exec("sysctl -w kernel.core_pattern=/tmp/core-%e-%p");
			}
			
			exec("sysctl -w net.core.somaxconn=49152");
			
			exec("sysctl -w net.core.rmem_max=134217728");
			exec("sysctl -w net.core.wmem_max=134217728");
			
			exec("sysctl -w net.ipv4.tcp_rmem=\"4096 87380 33554432\"");
			exec("sysctl -w net.ipv4.tcp_wmem=\"4096 65536 33554432\"");
			
			exec("sysctl -w net.ipv4.tcp_mtu_probing=1");
			
			exec("sysctl -w net.core.default_qdisc=fq");
			
			
			
			string algorithm = getCongestionControl(exec("sysctl net.ipv4.tcp_available_congestion_control"));
			
			if(!algorithm.empty()) exec(string("sysctl -w net.ipv4.tcp_congestion_control=") + algorithm);
		} break;

#ifndef __linux__
		
		case 5: {
			
			exec("sysctl -w kern.ipc.maxsockbuf=16777216");
			
			exec("sysctl -w net.inet.tcp.sendbuf_max=16777216");
			exec("sysctl -w net.inet.tcp.recvbuf_max=16777216");
			
			exec("sysctl -w net.inet.tcp.sendspace=1042560");
			exec("sysctl -w net.inet.tcp.recvspace=1042560");
			
			exec("sysctl -w net.inet.tcp.sendbuf_auto=1");
			exec("sysctl -w net.inet.tcp.recvbuf_auto=1");
			
			exec("sysctl -w net.inet.tcp.sendbuf_inc=16384");
			exec("sysctl -w net.inet.tcp.recvbuf_inc=524288");
			
			exec("sysctl -w net.inet.tcp.hostcache.expire=1");
			
			exec("sysctl -w kern.ipc.somaxconn=49152");
			
			
			string algorithm = getCongestionControl(getStringParam("net.inet.tcp.cc.available"));
			
			if(!algorithm.empty()) setParam("net.inet.tcp.cc.algorithm", algorithm);
		} break;
#endif
		
		case 7: {
			
			
			exec("ndd -set /dev/tcp tcp_max_buf 33554432");
			exec("ndd -set /dev/tcp tcp_cwnd_max 16777216");
			
			exec("ndd -set /dev/tcp tcp_xmit_hiwat 65536");
			exec("ndd -set /dev/tcp tcp_recv_hiwat 65536");
		} break;
	}
}
 
Os::Os(Config ** config, LogApp * log){
	
	if(config){
		
		this->log		= log;
		this->config	= config;
		
		getCPU();
		
		enableCoreDumps();
		
		if((* config)->proxy.optimos) optimos();
	}
}