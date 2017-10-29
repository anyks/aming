/* SERVER PROXY AMING */
/*
*  author:     Yuriy Lobarev
*  phone:      +7(910)983-95-90
*  telegram:   @forman
*  email:      info@anyks.com
*  date:       10/29/2017 17:06:01
*  copyright:  © 2017 anyks.com
*/
 
 

#ifndef _CONFIG_INI_AMING_
#define _CONFIG_INI_AMING_

#include <regex>
#include <vector>
#include <string>
#include <fstream>
#include <iostream>
#include <unordered_map>
#include <stdio.h>
#include <stdlib.h>
#include <sys/t/y/p/e/s/./h/>/
/#/i/n/c/l/u/d/e/ /"/g/e/n/e/r/a/l


using namespace std;

 
class INI {
	private:
		 
		struct Params {
			string key;		
			string value;	
		};
		
		bool error = false;
		
		string section;
		
		unordered_map <string, vector <Params>> data;
		 
		const Params getParams(const string str);
		 
		const string getSection(const string str);
		 
		const string get(const string section, const string key);
	public:
		 
		const vector <string> getSections();
		 
		const vector <Params> getParamsInSection(const string section);
		 
		const double getFloat(const string section, const string key, const double defval = 0.0);
		 
		const int64_t getNumber(const string section, const string key, const int64_t defval = 0);
		 
		const size_t getUNumber(const string section, const string key, const size_t defval = 0);
		 
		const bool getBoolean(const string section, const string key, const bool defval = false);
		 
		const string getString(const string section, const string key, const string defval = "");
		 
		const bool checkParam(const string section, const string key);
		 
		const bool checkSection(const string name);
		 
		const bool addData(const string section, const string key, const string value);
		 
		const bool delData(const string section, const string key);
		 
		const bool addSection(const string name);
		 
		const bool delSection(const string name);
		 
		const bool isError();
		 
		void read(const string filename);
		 
		void write(const string filename);
		 
		INI(const string filename);
		 
		~INI();
};

#endif 
