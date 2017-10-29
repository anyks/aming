/* SERVER PROXY AMING */
/*
*  author:     Yuriy Lobarev
*  phone:      +7(910)983-95-90
*  telegram:   @forman
*  email:      info@anyks.com
*  date:       10/29/2017 16:59:43
*  copyright:  © 2017 anyks.com
*/
 
 

#include "system/system.h"


using namespace std;

 
void System::clearMsgPids(){
	
	key_t msgkey = ftok(".", 222);
	
	int qid = msgget(msgkey, IPC_CREAT | 0666);
	
	msgctl(qid, IPC_RMID, 0);
}
 
System::Pids System::readPids(){
	
	ParamsSendPids msg;
	
	Pids result;
	
	size_t length = sizeof(msg) - sizeof(long);
	
	key_t msgkey = ftok(".", 222);
	
	int qid = msgget(msgkey, IPC_CREAT | 0666);
	
	if(qid < 0){
		
		this->log->write(LOG_ERROR, 0, "[-] cannot create message id for read fork pids");
		
		exit(23);
	}
	
	if(msgrcv(qid, &msg, length, 1, 0) < 0) {
		
		this->log->write(LOG_ERROR, 0, "[-] cannot read message children fork pids");
		
		msgctl(qid, IPC_RMID, 0);
		
		exit(23);
	}
	
	result.len = msg.len;
	
	memcpy(result.pids, msg.pids, sizeof(pid_t) * result.len);
	
	msgctl(qid, IPC_RMID, 0);
	
	return result;
}
 
void System::sendPids(pid_t * pids, size_t len){
	
	clearMsgPids();
	
	ParamsSendPids msg;
	
	size_t length = sizeof(msg) - sizeof(long);
	
	key_t msgkey = ftok(".", 222);
	
	int qid = msgget(msgkey, IPC_CREAT | 0666);
	
	if(qid < 0){
		
		this->log->write(LOG_ERROR, 0, "[-] cannot create message id for send in worker %d to %d", getpid(), getppid());
		
		exit(23);
	}
	
	msg.type = 1;
	
	msg.len = len;
	
	memcpy(msg.pids, pids, sizeof(pid_t) * msg.len);
	
	if(msgsnd(qid, &msg, length, IPC_NOWAIT) < 0){
		
		this->log->write(LOG_ERROR, 0, "[-] cannot send message in worker %d to %d", getpid(), getppid());
		
		exit(23);
	}
}
 
void System::reload(){
	
	delete this->config;
	
	this->config = new Config(this->configfile);
}
 
void System::setConfigFile(string configfile){
	
	this->configfile = configfile;
	
	reload();
}
 
System::System(string configfile){
	
	this->configfile = configfile;
	
	this->config = new Config(this->configfile);
	
	this->log = new LogApp(&this->config, TOLOG_FILES | TOLOG_CONSOLE);
	
	this->os = new Os(&this->config, this->log);
	
	this->cache = new Cache(this->config, this->log);
	
	this->headers = new Headers(this->config, this->log);
}
 
System::~System(){
	
	delete this->headers;
	delete this->cache;
	delete this->os;
	delete this->log;
	delete this->config;
}