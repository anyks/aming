/* SERVER PROXY AMING */
/*
*  author:     Yuriy Lobarev
*  phone:      +7(910)983-95-90
*  telegram:   @forman
*  email:      info@anyks.com
*  date:       11/08/2017 16:52:47
*  copyright:  © 2017 anyks.com
*/
 
 

#include "proxy/proxy.h"


using namespace std;

 
int main(int argc, char * argv[]){
	
	
	setlocale(LC_ALL, "en_US.UTF-8");
	
	string configfile;
	
	string param = (argc >= 2 ? argv[1] : "");
	
	if(param.compare("-c") == 0)
		
		configfile = (argc >= 3 ? argv[2] : "");
	
	else if(param.find("--config=") != string::npos)
		
		configfile = param.replace(0, 9, "");
	
	Proxy proxy(configfile);
	
	return 0;
}
