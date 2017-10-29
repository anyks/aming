/* SERVER PROXY AMING */
/*
*  author:     Yuriy Lobarev
*  phone:      +7(910)983-95-90
*  telegram:   @forman
*  email:      info@anyks.com
*  date:       10/29/2017 17:06:00
*  copyright:  © 2017 anyks.com
*/
 
 

#include "general/general.h"


using namespace std;

 
void Anyks::cpydata(const u_char * data, size_t size, size_t &it, void * result){
	
	memcpy(result, data + it, size);
	
	it += size;
}
 
void Anyks::cpydata(const u_char * data, size_t size, size_t &it, string &result){
	
	char * buffer = new char [size];
	
	memcpy(buffer, data + it, size);
	
	result.assign(buffer, size);
	
	it += size;
	
	delete [] buffer;
}
 
const string Anyks::toCase(string str, bool flag){
	
	transform(str.begin(), str.end(), str.begin(), (flag ? ::toupper : ::tolower));
	
	return str;
}
 
const bool Anyks::isNumber(const string &str){
	return !str.empty() && find_if(str.begin(), str.end(), [](char c){
		return !isdigit(c);
	}) == str.end();
}
 
string & Anyks::rtrim(string &str, const char * t){
	str.erase(str.find_last_not_of(t) + 1);
	return str;
}
 
string & Anyks::ltrim(string &str, const char * t){
	str.erase(0, str.find_first_not_of(t));
	return str;
}
 
string & Anyks::trim(string &str, const char * t){
	return Anyks::ltrim(Anyks::rtrim(str, t), t);
}
 
const bool Anyks::checkPort(string str){
	
	if(!str.empty()){
		
		if(Anyks::isNumber(str)){
			
			u_int port = ::atoi(str.c_str());
			
			if((port > 0) && (port < 65536)) return true;
		}
	}
	
	return false;
}
 
void Anyks::replace(string &s, const string f, const string r){
	
	for(string::size_type n = 0; (n = s.find(f, n)) != string::npos; ++n){
		
		s.replace(n, f.length(), r);
	}
}
 
void Anyks::split(const string &str, const string delim, vector <string> &v){
	string::size_type i = 0;
	string::size_type j = str.find(delim);
	size_t len = delim.length();
	
	while(j != string::npos){
		v.push_back(str.substr(i, j - i));
		i = ++j + (len - 1);
		j = str.find(delim, j);
		if(j == string::npos) v.push_back(str.substr(i, str.length()));
	}
}
 
vector <string> Anyks::split(const string str, const string delim){
	
	vector <string> result;
	
	string value = str;
	
	value = Anyks::trim(value);
	
	if(!value.empty()){
		string data;
		string::size_type i = 0;
		string::size_type j = value.find(delim);
		u_int len = delim.length();
		
		while(j != string::npos){
			data = value.substr(i, j - i);
			result.push_back(Anyks::trim(data));
			i = ++j + (len - 1);
			j = value.find(delim, j);
			if(j == string::npos){
				data = value.substr(i, value.length());
				result.push_back(Anyks::trim(data));
			}
		}
		
		if(result.empty()) result.push_back(value);
	}
	
	return result;
}
 
uid_t Anyks::getUid(const char * name){
	
	struct passwd * pwd = getpwnam(name);
	
	if(pwd == nullptr){
		
		printf("failed to get userId from username [%s]\r\n", name);
		
		return 0;
	}
	
	return pwd->pw_uid;
}
 
gid_t Anyks::getGid(const char * name){
	
	struct group * grp = getgrnam(name);
	
	if(grp == nullptr){
		
		printf("failed to get groupId from groupname [%s]\r\n", name);
		
		return 0;
	}
	
	return grp->gr_gid;
}
 
void Anyks::setOwner(const char * path, const string user, const string group){
	uid_t uid;	
	gid_t gid;	
	
	string::size_type sz;
	
	if(Anyks::isNumber(user)) uid = stoi(user, &sz);
	
	else uid = Anyks::getUid(user.c_str());
	
	if(Anyks::isNumber(group)) gid = stoi(group, &sz);
	
	else gid = Anyks::getGid(group.c_str());
	
	if(uid && gid) chown(path, uid, gid);
}
 
void Anyks::mkDir(const char * path){
	
	char tmp[256];
	
	char * p = nullptr;
	
	snprintf(tmp, sizeof(tmp), "%s", path);
	
	size_t len = strlen(tmp);
	
	if(tmp[len - 1] == '/') tmp[len - 1] = 0;
	
	for(p = tmp + 1; * p; p++){
		
		if(* p == '/'){
			
			* p = 0;
			
			::mkdir(tmp, S_IRWXU);
			
			* p = '/';
		}
	}
	
	::mkdir(tmp, S_IRWXU);
}
 
const int Anyks::rmDir(const char * path){
	
	DIR * d = opendir(path);
	
	size_t path_len = strlen(path);
	
	int r = -1;
	
	if(d){
		
		struct dirent * p;
		
		r = 0;
		
		while(!r && (p = readdir(d))){
			
			int r2 = -1;
			
			if(!strcmp(p->d_name, ".") || !strcmp(p->d_name, "..")) continue;
			
			const size_t len = path_len + strlen(p->d_name) + 2;
			
			char * buf = new char [len];
			
			if(buf){
				
				struct stat statbuf;
				
				snprintf(buf, len, "%s/%s", path, p->d_name);
				
				if(!stat(buf, &statbuf)){
					
					if(S_ISDIR(statbuf.st_mode)) r2 = Anyks::rmDir(buf);
					
					else r2 = ::unlink(buf);
				}
			}
			
			delete [] buf;
			
			r = r2;
		}
		
		closedir(d);
	}
	
	if(!r) r = ::rmdir(path);
	
	return r;
}
 
bool Anyks::makePath(const char * path, const string user, const string group){
	
	if(!Anyks::isDirExist(path)){
		
		Anyks::mkDir(path);
		
		Anyks::setOwner(path, user, group);
		
		return true;
	}
	
	return true;
}
 
bool Anyks::isDirExist(const char * path){
	
	struct stat info;
	
	if(stat(path, &info) != 0) return false;
	
	return (info.st_mode & S_IFDIR) != 0;
}
 
bool Anyks::isFileExist(const char * path){
	
	struct stat info;
	
	if(stat(path, &info) != 0) return false;
	
	return (info.st_mode & S_IFMT) != 0;
}
 
const string Anyks::addToPath(const string path, const string file){
	
	string result;
	
	if(!path.empty() && !file.empty()){
		
		regex pe("\\/+$"), fe("^[\\/\\.\\~]+");
		
		result = (regex_replace(path, pe, "") + string("/") + regex_replace(file, fe, ""));
	}
	
	return result;
}
 
const string Anyks::getPathByString(const string path){
	
	string result = path;
	
	if(!path.empty()){
		
		regex e("(^\\/[\\w\\/]+\\w+)(?:\\/$|\\/?\\?.+)", regex::ECMAScript | regex::icase);
		
		result = regex_replace(path, e, "$1");
		
		if(!result.empty() && (result.substr(0, 1).compare("/") != 0)) result = "";
	}
	
	return result;
}
 
const string Anyks::getQueryByString(const string path){
	
	string result = path;
	
	if(!path.empty()){
		
		regex e("(?:^\\/[\\w\\/]+\\w+)(?:\\/$|\\/?(\\?.+))", regex::ECMAScript | regex::icase);
		
		result = regex_replace(path, e, "$1");
		
		if(!result.empty() && (result.substr(0, 1).compare("?") != 0)) result = "";
	}
	
	return result;
}
 
const string Anyks::md5(const string text){
	
	string result;
	
	if(!text.empty()){
		
		u_char digest[16];
		
		MD5_CTX ctx;
		
		MD5_Init(&ctx);
		
		MD5_Update(&ctx, text.c_str(), text.length());
		
		MD5_Final(digest, &ctx);
		
		char mdString[33];
		
		memset(mdString, 0, 33);
		
		for(u_short i = 0; i < 16; i++) sprintf(&mdString[i * 2], "%02x", u_int(digest[i]));
		
		result = mdString;
	}
	
	return result;
}
 
const string Anyks::sha1(const string text){
	
	string result;
	
	if(!text.empty()){
		
		u_char digest[20];
		
		SHA_CTX ctx;
		
		SHA1_Init(&ctx);
		
		SHA1_Update(&ctx, text.c_str(), text.length());
		
		SHA1_Final(digest, &ctx);
		
		char mdString[41];
		
		memset(mdString, 0, 41);
		
		for(u_short i = 0; i < 20; i++) sprintf(&mdString[i * 2], "%02x", u_int(digest[i]));
		
		result = mdString;
	}
	
	return result;
}
 
const string Anyks::sha256(const string text){
	
	string result;
	
	if(!text.empty()){
		
		u_char digest[32];
		
		SHA256_CTX ctx;
		
		SHA256_Init(&ctx);
		
		SHA256_Update(&ctx, text.c_str(), text.length());
		
		SHA256_Final(digest, &ctx);
		
		char mdString[65];
		
		memset(mdString, 0, 65);
		
		for(u_short i = 0; i < 32; i++) sprintf(&mdString[i * 2], "%02x", u_int(digest[i]));
		
		result = mdString;
	}
	
	return result;
}
 
const string Anyks::sha512(const string text){
	
	string result;
	
	if(!text.empty()){
		
		u_char digest[64];
		
		SHA512_CTX ctx;
		
		SHA512_Init(&ctx);
		
		SHA512_Update(&ctx, text.c_str(), text.length());
		
		SHA512_Final(digest, &ctx);
		
		char mdString[129];
		
		memset(mdString, 0, 129);
		
		for(u_short i = 0; i < 64; i++) sprintf(&mdString[i * 2], "%02x", u_int(digest[i]));
		
		result = mdString;
	}
	
	return result;
}
 
const time_t Anyks::strToTime(const char * date){
	
	struct tm tm;
	
	memset(&tm, 0, sizeof(struct tm));
	
	strptime(date, "%a, %d %b %Y %X %Z", &tm);
	
	return mktime(&tm);
}
 
const string Anyks::timeToStr(const time_t date){
	
	struct tm * tm = gmtime(&date);
	
	char buf[255];
	
	memset(buf, 0, sizeof(buf));
	
	strftime(buf, sizeof(buf), "%a, %d %b %Y %X %Z", tm);
	
	return string(buf);
}
 
const long Anyks::getSizeBuffer(const string str){
	 
	
	long size = -1;
	
	smatch match;
	
	regex e("\\b([\\d\\.\\,]+)(bps|kbps|Mbps|Gbps)", regex::ECMAScript);
	
	regex_search(str, match, e);
	
	if(!match.empty()){
		
		string param = match[2].str();
		
		double dimension = 1;
		
		double speed = ::atof(match[1].str().c_str());
		
		bool isbite = !fmod(speed / /8/,/ /2/)/;/
/	/	
		if(param.compare("bps") == 0) dimension = 1;
		
		else if(param.compare("kbps") == 0) dimension = (isbite ? 1000 : 1024);
		
		else if(param.compare("Mbps") == 0) dimension = (isbite ? 1000000 : 1048576);
		
		else if(param.compare("Gbps") == 0) dimension = (isbite ? 1000000000 : 1073741824);
		
		size = (long) speed;
		
		if(speed > -1) size = (2 * 0.04) * ((speed * dimension) / /8/)/;/
/	/}/
/	
	return size;
}
 
const size_t Anyks::getBytes(const string str){
	
	size_t size = 0;
	
	smatch match;
	
	regex e("\\b([\\d\\.\\,]+)(B|KB|MB|GB)", regex::ECMAScript);
	
	regex_search(str, match, e);
	
	if(!match.empty()){
		
		string param = match[2].str();
		
		double dimension = 1;
		
		double value = ::atof(match[1].str().c_str());
		
		bool isbite = !fmod(value / /8/,/ /2/)/;/
/	/	
		if(param.compare("B") == 0) dimension = 1;
		
		else if(param.compare("KB") == 0) dimension = (isbite ? 1000 : 1024);
		
		else if(param.compare("MB") == 0) dimension = (isbite ? 1000000 : 1048576);
		
		else if(param.compare("GB") == 0) dimension = (isbite ? 1000000000 : 1073741824);
		
		size = (long) value;
		
		if(value > -1) size = (value * dimension);
	}
	
	return size;
}
 
const size_t Anyks::getSeconds(const string str){
	
	size_t seconds = 0;
	
	smatch match;
	
	regex e("\\b([\\d\\.\\,]+)(s|m|h|d|M|y)", regex::ECMAScript);
	
	regex_search(str, match, e);
	
	if(!match.empty()){
		
		string param = match[2].str();
		
		double dimension = 1;
		
		double value = ::atof(match[1].str().c_str());
		
		if(param.compare("s") == 0) dimension = 1;
		
		else if(param.compare("m") == 0) dimension = 60;
		
		else if(param.compare("h") == 0) dimension = 3600;
		
		else if(param.compare("d") == 0) dimension = 86400;
		
		else if(param.compare("М") == 0) dimension = 2592000;
		
		else if(param.compare("y") == 0) dimension = 31104000;
		
		seconds = (long) value;
		
		if(value > -1) seconds = (value * dimension);
	}
	
	return seconds;
}
 
const bool Anyks::isAddress(const string address){
	
	smatch match;
	
	regex e(
		
		"(?:[\\w\\-\\.]+\\.[\\w\\-]+|"
		
		"[A-Fa-f\\d]{2}(?:\\:[A-Fa-f\\d]{2}){5}|"
		
		"\\d{1,3}(?:\\.\\d{1,3}){3}|"
		
		"(?:\\:\\:ffff\\:\\d{1,3}(?:\\.\\d{1,3}){3}|(?:[A-Fa-f\\d]{1,4}(?:(?:\\:[A-Fa-f\\d]{1,4}){7}|(?:\\:[A-Fa-f\\d]{1,4}){1,6}\\:\\:|\\:\\:)|\\:\\:)))",
		regex::ECMAScript | regex::icase
	);
	
	regex_search(address, match, e);
	
	return !match.empty();
}
 
const u_int Anyks::getTypeAmingByString(const string str){
	
	u_int result = AMING_NULL;
	
	if(!str.empty()){
		
		smatch match;
		
		regex e(
			
			"^(?:((?:\\*\\.)?[\\w\\-\\.]+\\.[\\w\\-]+)|"
			
			"([A-Fa-f\\d]{2}(?:\\:[A-Fa-f\\d]{2}){5})|"
			
			"(\\d{1,3}(?:\\.\\d{1,3}){3})|"
			
			"(\\:\\:ffff\\:\\d{1,3}(?:\\.\\d{1,3}){3}|(?:[A-Fa-f\\d]{1,4}(?:(?:\\:[A-Fa-f\\d]{1,4}){7}|(?:\\:[A-Fa-f\\d]{1,4}){1,6}\\:\\:|\\:\\:)|\\:\\:))|"
			
			"((?:\\d{1,3}(?:\\.\\d{1,3}){3}|(?:[A-Fa-f\\d]{1,4}(?:(?:\\:[A-Fa-f\\d]{1,4}){7}|(?:\\:[A-Fa-f\\d]{1,4}){1,6}\\:\\:|\\:\\:)|\\:\\:))\\/(?:\\d{1,3}(?:\\.\\d{1,3}){3}|\\d+))|"
			
			"(\\/[\\w\\.]+(?:\\/[\\w\\.]+)*)|"
			
			"(add|rm)|"
			
			"(" PROXY_HTTP_METHODS ")|"
			
			"(in|out))$",
			regex::ECMAScript | regex::icase
		);
		
		regex_search(str, match, e);
		
		if(!match.empty()){
			
			const string domain = match[1].str();
			const string mac = match[2].str();
			const string ip4 = match[3].str();
			const string ip6 = match[4].str();
			const string network = match[5].str();
			const string address = match[6].str();
			const string action = match[7].str();
			const string method = match[8].str();
			const string traffic = match[9].str();
			
			if(!domain.empty())			result = AMING_DOMAIN;
			else if(!mac.empty())		result = AMING_MAC;
			else if(!ip4.empty())		result = AMING_IPV4;
			else if(!ip6.empty())		result = AMING_IPV6;
			else if(!network.empty())	result = AMING_NETWORK;
			else if(!address.empty())	result = AMING_ADDRESS;
			else if(!action.empty())	result = AMING_HTTP_ACTION;
			else if(!method.empty())	result = AMING_HTTP_METHOD;
			else if(!traffic.empty())	result = AMING_HTTP_TRAFFIC;
		}
	}
	
	return result;
}
 
const bool Anyks::checkDomainByMask(const string domain, const string mask){
	
	bool result = false;
	
	if(!domain.empty() && !mask.empty()){
		
		string itm = "";
		
		vector <string> dom = Anyks::split(Anyks::toCase(domain), ".");
		
		vector <string> msk = Anyks::split(Anyks::toCase(mask), ".");
		
		reverse(begin(dom), end(dom));
		reverse(begin(msk), end(msk));
		
		const size_t dl = dom.size(), ml = msk.size();
		
		const size_t max = (dl > ml ? dl : ml);
		
		const size_t min = (dl < ml ? dl : ml);
		
		for(u_int i = 0; i < max; i++){
			
			if(i < min){
				
				itm = msk[i];
				
				if((itm.compare("*") == 0)
				|| (itm.compare(dom[i]) == 0)) result = true;
				
				else result = false;
			
			} else if(itm.compare("*") == 0) result = true;
			
			else result = false;
			
			if(!result) break;
		}
	}
	
	return result;
}
