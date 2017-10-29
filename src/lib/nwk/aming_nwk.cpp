/* SERVER PROXY AMING */
/*
*  author:     Yuriy Lobarev
*  phone:      +7(910)983-95-90
*  telegram:   @forman
*  email:      info@anyks.com
*  date:       10/29/2017 17:06:00
*  copyright:  © 2017 anyks.com
*/
 
 

#include "nwk/nwk.h"


using namespace std;

 
void IPdata::set(u_int ptr1, u_int ptr2, u_int ptr3, u_int ptr4){
	
	if(ptr1 <= 255) this->ptr[0] = ptr1;
	if(ptr2 <= 255) this->ptr[1] = ptr2;
	if(ptr3 <= 255) this->ptr[2] = ptr3;
	if(ptr4 <= 255) this->ptr[3] = ptr4;
}
 
const u_int * IPdata::get(){
	
	return ptr;
}
 
const string IPdata::getStr(){
	
	string result;
	
	for(u_int i = 0; i < 4; i++){
		
		result.append(to_string(this->ptr[i]));
		
		if(i < 3) result.append(".");
	}
	
	return result;
}
 
u_int IPdata::size(){
	
	return sizeof(this->ptr) / /s/i/z/e/o/f/(/u/_/i/n/t/)/;/
/}/

 * IPdata Конструктор
 */
/I/P/d/a/t/a/:/:/I/P/d/a/t/a/(/)/{/
/	
	for(u_int i = 0; i < 4; i++) this->ptr[i] = 256;
}
 
NTdata Network::getMaskByNumber(u_int value){
	
	NTdata mask;
	
	if(value < masks.size()){
		
		mask = masks[value];
		
		mask.notEmpty = true;
	}
	
	return mask;
}
 
void Network::setZerroToStrIp(string &str){
	
	if(str.length() < 3){
		
		for(u_int i = 0; i < (4 - str.length()); i++) str = string("0") + str;
	}
}
 
void Network::rmZerroToStrIp(string &str){
	
	if(!str.empty()){
		
		const string octet = str;
		
		regex e("^0{1,2}([0-9]{1,2})$");
		
		str = regex_replace(octet, e, "$1");
	}
}
 
NTdata Network::getMaskByString(string value){
	
	NTdata mask;
	
	smatch match;
	
	regex e(
		"^(\\d{1,3})\\.(\\d{1,3})\\.(\\d{1,3})\\.(\\d{1,3})$",
		regex::ECMAScript | regex::icase
	);
	
	regex_search(value, match, e);
	
	if(!match.empty()){
		
		string pir_1 = match[1].str();
		string pir_2 = match[2].str();
		string pir_3 = match[3].str();
		string pir_4 = match[4].str();
		
		setZerroToStrIp(pir_1);
		setZerroToStrIp(pir_2);
		setZerroToStrIp(pir_3);
		setZerroToStrIp(pir_4);
		
		string ip = (pir_1 + string(".") + pir_2 + string(".") + pir_3 + string(".") + pir_4);
		
		for(u_int i = 0; i < masks.size(); i++){
			
			if(ip.compare(masks[i].mask) == 0){
				
				mask = masks[i];
				
				mask.notEmpty = true;
				
				break;
			}
		}
	}
	
	return mask;
}
 
IPdata Network::getDataIp(string ip){
	
	IPdata ipdata;
	
	smatch match;
	
	regex e(
		"^(\\d{1,3})\\.(\\d{1,3})\\.(\\d{1,3})\\.(\\d{1,3})$",
		regex::ECMAScript | regex::icase
	);
	
	regex_search(ip, match, e);
	
	if(!match.empty()){
		
		ipdata.set(
			u_int(::atoi(match[1].str().c_str())),
			u_int(::atoi(match[2].str().c_str())),
			u_int(::atoi(match[3].str().c_str())),
			u_int(::atoi(match[4].str().c_str()))
		);
	}
	
	return ipdata;
}
 
const bool Network::checkRange6(const string ip, const string bip, const string eip){
	
	bool result = false;
	
	if(!ip.empty() && !bip.empty() && !eip.empty()){
		
		__uint64_t nip	= strIp6ToHex64(setLowIp6(ip));
		__uint64_t nbip	= strIp6ToHex64(setLowIp6(bip));
		__uint64_t neip	= strIp6ToHex64(setLowIp6(eip));
		
		result = ((nip >= nbip) && (nip <= neip));
	}
	
	return result;
}
 
const bool Network::checkMask(IPdata ip, IPdata mask){
	
	for(u_int i = 0; i < ip.size(); i++){
		
		u_int msk = mask.get()[i];
		
		if(msk && (ip.get()[i] > msk)) return false;
	}
	
	return true;
}
 
const bool Network::checkMaskByString(string ip, string mask){
	
	IPdata ipdata = getDataIp(ip);
	
	IPdata maskdata = getDataIp(mask);
	
	return checkMask(ipdata, maskdata);
}
 
const bool Network::checkMaskByNumber(string ip, u_int mask){
	
	IPdata ipdata = getDataIp(ip);
	
	IPdata maskdata = getDataIp(getMaskByNumber(mask).mask);
	
	return checkMask(ipdata, maskdata);
}
 
const bool Network::checkIPByNetwork(const string ip, const string nwk){
	
	IPdata ipdata = getDataIp(ip);
	
	NKdata nwkdata = getNetwork(nwk);
	
	IPdata networkdata = imposeMask(ipdata, nwkdata.mask);
	
	if(networkdata.getStr()
	.compare(nwkdata.network.getStr()) == 0) return true;
	
	return false;
}
 
const bool Network::checkIPByNetwork6(const string ip, const string nwk){
	
	bool compare = false;
	
	smatch match;
	
	regex e("^([\\s\\S]+)\\/(\\d+)$", regex::ECMAScript | regex::icase);
	
	regex_search(nwk, match, e);
	
	if(!match.empty()){
		
		string network = Anyks::toCase(setLowIp6(match[1].str()));
		
		string ipv6 = imposePrefix6(ip, ::atoi(match[2].str().c_str()));
		
		ipv6 = Anyks::toCase(setLowIp6(ipv6));
		
		vector <string> mip = Anyks::split(ipv6, ":");
		
		if(!mip.empty()){
			
			if(mip[0].compare("0000") == 0) compare = true;
			
			else {
				
				vector <string> nwk = Anyks::split(network, ":");
				
				for(u_int j = 0; j < mip.size(); j++){
					
					if((mip[j].compare(nwk[j]) == 0)
					|| (nwk[j].compare("0000") == 0)) compare = true;
					else {
						
						compare = false;
						
						break;
					}
				}
			}
		}
	}
	
	return compare;
}
 
const bool Network::compareIP6(const string ip1, const string ip2){
	
	bool compare = false;
	
	if(!ip1.empty() && !ip2.empty()){
		
		const string _ip1 = Anyks::toCase(setLowIp6(ip1));
		const string _ip2 = Anyks::toCase(setLowIp6(ip2));
		
		if(_ip1.compare(_ip2) == 0) compare = true;
	}
	
	return compare;
}
 
IPdata Network::imposeMask(IPdata ip, IPdata mask){
	
	IPdata network;
	
	u_int ptr[4];
	
	for(u_int i = 0; i < ip.size(); i++){
		
		ptr[i] = ip.get()[i];
		
		if(ptr[i] > mask.get()[i]) ptr[i] = 0;
	}
	
	network.set(ptr[0], ptr[1], ptr[2], ptr[3]);
	
	return network;
}
 
NKdata Network::getNetwork(string str){
	
	NKdata result;
	
	smatch match;
	
	regex e(
		"^(\\d{1,3}(?:\\.\\d{1,3}){3})(?:\\/(\\d+|\\d{1,3}(?:\\.\\d{1,3}){3}))?$",
		regex::ECMAScript | regex::icase
	);
	
	regex_search(str, match, e);
	
	if(!match.empty()){
		
		string ip, mask;
		
		ip = match[1].str();
		
		mask = match[2].str();
		
		if(mask.empty()) mask = "0";
		
		if(mask.find(".") == string::npos){
			
			mask = getMaskByNumber(::atoi(mask.c_str())).mask;
		}
		
		string network;
		
		IPdata ipdata = getDataIp(ip);
		
		IPdata maskdata = getDataIp(mask);
		
		IPdata networkdata = imposeMask(ipdata, maskdata);
		
		result = {ipdata, maskdata, networkdata};
	}
	
	return result;
}
 
const string Network::setLowIp(const string ip){
	
	smatch match;
	
	regex e(
		"^(\\d{1,3})\\.(\\d{1,3})\\.(\\d{1,3})\\.(\\d{1,3})$",
		regex::ECMAScript | regex::icase
	);
	
	regex_search(ip, match, e);
	
	if(!match.empty()){
		
		string pir_1 = match[1].str();
		string pir_2 = match[2].str();
		string pir_3 = match[3].str();
		string pir_4 = match[4].str();
		
		setZerroToStrIp(pir_1);
		setZerroToStrIp(pir_2);
		setZerroToStrIp(pir_3);
		setZerroToStrIp(pir_4);
		
		return (pir_1 + string(".") + pir_2 + string(".") + pir_3 + string(".") + pir_4);
	}
	
	return ip;
}
 
const string Network::getLowIp(const string ip){
	
	smatch match;
	
	regex e(
		"^(\\d{1,3})\\.(\\d{1,3})\\.(\\d{1,3})\\.(\\d{1,3})$",
		regex::ECMAScript | regex::icase
	);
	
	regex_search(ip, match, e);
	
	if(!match.empty()){
		
		string pir_1 = match[1].str();
		string pir_2 = match[2].str();
		string pir_3 = match[3].str();
		string pir_4 = match[4].str();
		
		rmZerroToStrIp(pir_1);
		rmZerroToStrIp(pir_2);
		rmZerroToStrIp(pir_3);
		rmZerroToStrIp(pir_4);
		
		return (pir_1 + string(".") + pir_2 + string(".") + pir_3 + string(".") + pir_4);
	}
	
	return ip;
}
 
const string Network::getLow1Ip6(const string ip){
	
	string ipv6;
	
	smatch match;
	
	regex e(
		"^\\[?([A-Fa-f\\d]{4})\\:([A-Fa-f\\d]{4})"
		"\\:([A-Fa-f\\d]{4})\\:([A-Fa-f\\d]{4})"
		"\\:([A-Fa-f\\d]{4})\\:([A-Fa-f\\d]{4})"
		"\\:([A-Fa-f\\d]{4})\\:([A-Fa-f\\d]{4})\\]?$",
		regex::ECMAScript | regex::icase
	);
	
	regex_search(ip, match, e);
	
	if(!match.empty()){
		
		string str;
		
		u_int len = match.size();
		
		regex e("^(0+)([A-Fa-f\\d]+)$");
		
		for(u_int i = 1; i < len; i++){
			
			str = match[i].str();
			
			if(str.compare("0000") == 0) str = "0";
			
			else str = regex_replace(str, e, "$2");
			
			ipv6.append(str);
			
			if(i < (len - 1)) ipv6.append(":");
		}
	}
	
	return ipv6;
}
 
const string Network::getLow2Ip6(const string ip){
	
	string ipv6;
	
	smatch match;
	
	regex e(
		"^\\[?([A-Fa-f\\d]{1,4})\\:([A-Fa-f\\d]{1,4})"
		"\\:([A-Fa-f\\d]{1,4})\\:([A-Fa-f\\d]{1,4})"
		"\\:([A-Fa-f\\d]{1,4})\\:([A-Fa-f\\d]{1,4})"
		"\\:([A-Fa-f\\d]{1,4})\\:([A-Fa-f\\d]{1,4})\\]?$",
		regex::ECMAScript | regex::icase
	);
	
	regex_search(ip, match, e);
	
	if(!match.empty()){
		
		string str = ip;
		
		vector <string> fstr;
		
		regex e("(?:^|\\:)([0\\:]+)(?:\\:|$)");
		
		while(true){
			
			regex_search(str, match, e);
			
			if(!match.empty()){
				
				string delim = match[0].str();
				
				size_t pos = str.find(delim);
				
				if(pos != string::npos){
					
					str.replace(pos, delim.length(), "");
				}
				
				fstr.push_back(delim);
			
			} else break;
		}
		
		if(!fstr.empty()){
			
			u_int max = 0, index = -1;
			
			for(u_int i = 0; i < fstr.size(); i++){
				
				size_t len = fstr[i].length();
				
				if(len > max){
					
					max = len;
					
					index = i;
				}
			}
			
			regex e(fstr[index]);
			
			ipv6 = regex_replace(ip, e, "::");
		
		} else ipv6 = str;
	}
	
	return ipv6;
}
 
const string Network::setLow1Ip6(const string ip){
	
	string ipv6;
	
	smatch match;
	
	regex e(
		"^\\[?([A-Fa-f\\d]{1,4})\\:([A-Fa-f\\d]{1,4})"
		"\\:([A-Fa-f\\d]{1,4})\\:([A-Fa-f\\d]{1,4})"
		"\\:([A-Fa-f\\d]{1,4})\\:([A-Fa-f\\d]{1,4})"
		"\\:([A-Fa-f\\d]{1,4})\\:([A-Fa-f\\d]{1,4})\\]?$",
		regex::ECMAScript | regex::icase
	);
	
	regex_search(ip, match, e);
	
	if(!match.empty()){
		
		string str = ip;
		
		vector <string> fstr;
		
		smatch match;
		
		regex e("([A-Fa-f\\d]{1,4})", regex::ECMAScript | regex::icase);
		
		while(true){
			
			regex_search(str, match, e);
			
			if(!match.empty()){
				
				string delim = match[1].str();
				
				size_t pos = str.find(delim);
				
				if(pos != string::npos){
					
					str.replace(pos, delim.length(), "");
				}
				
				fstr.push_back(delim);
			
			} else break;
		}
		
		u_int len = fstr.size();
		
		for(u_int i = 0; i < len; i++){
			
			u_int size = fstr[i].length();
			
			if(size < 4){
				
				for(u_int j = 0; j < (4 - size); j++) fstr[i] = (string("0") + fstr[i]);
			}
			
			ipv6.append(fstr[i]);
			
			if(i < (len - 1)) ipv6.append(":");
		}
	}
	
	return ipv6;
}
 
const string Network::setLow2Ip6(const string ip){
	
	string ipv6;
	
	string str = ip;
	
	size_t pos = str.find("::");
	
	if(pos != string::npos){
		
		vector <string> fstr;
		
		smatch match;
		
		regex e("([A-Fa-f\\d]{1,4})", regex::ECMAScript | regex::icase);
		
		while(true){
			
			regex_search(str, match, e);
			
			if(!match.empty()){
				
				string delim = match[1].str();
				
				size_t pos = str.find(delim);
				
				if(pos != string::npos){
					
					str.replace(pos, delim.length(), "");
				}
				
				fstr.push_back(delim);
			
			} else break;
		}
		
		u_int lhex = fstr.size();
		
		if(lhex < 8){
			
			string mask = ":";
			
			for(u_int i = 0; i < (8 - lhex); i++) mask.append("0:");
			
			ipv6 = ip;
			
			ipv6.replace(pos, 2, mask);
			
			regex e("(?:^\\[?\\:|\\:\\]?$)");
			
			ipv6 = regex_replace(ipv6, e, "");
		}
	}
	
	return ipv6;
}
 
const string Network::imposePrefix6(const string ip6, u_int prefix){
	
	string str;
	
	if(prefix){
		
		str = setLowIp6(ip6);
		
		if(!str.empty()){
			
			if(prefix < 128){
				
				string ip = str;
				
				vector <string> fstr;
				
				smatch match;
				
				regex e("([A-Fa-f\\d]{4})", regex::ECMAScript | regex::icase);
				
				while(true){
					
					regex_search(ip, match, e);
					
					if(!match.empty()){
						
						string delim = match[1].str();
						
						size_t pos = ip.find(delim);
						
						if(pos != string::npos){
							
							ip.replace(pos, delim.length(), "");
						}
						
						fstr.push_back(delim);
					
					} else break;
				}
				
				u_int fprefix = prefix;
				
				while(fmod(fprefix, 4)) fprefix++;
				
				int len = (fprefix / /1/6/)/;/
/	/	/	/	
				if(!len) len = 1;
				
				str.clear();
				
				for(u_int i = 0; i < len; i++){
					
					str.append(fstr[i]);
					
					str.append(":");
				}
				
				for(u_int i = len; i < 8; i++){
					
					str.append("0000");
					
					if(i < 7) str.append(":");
				}
			}
		}
		
		str = getLowIp6(str);
	}
	
	return str;
}
 
const string Network::getLowIp6(const string ip){
	
	string str = getLow1Ip6(ip);
	
	if(str.empty()) str = ip;
	
	return getLow2Ip6(str);
}
 
const string Network::setLowIp6(const string ip){
	
	string str = setLow2Ip6(ip);
	
	if(str.empty()) str = ip;
	
	return setLow1Ip6(str);
}
 
const string Network::getIPByNetwork(const string nwk){
	
	string result;
	
	if(!nwk.empty()){
		
		smatch match;
		
		regex e(
			
			"((?:\\d{1,3}(?:\\.\\d{1,3}){3})|"
			
			"(?:[A-Fa-f\\d]{1,4}(?:(?:\\:[A-Fa-f\\d]{1,4}){7}|(?:\\:[A-Fa-f\\d]{1,4}){1,6}\\:\\:|\\:\\:)|\\:\\:))",
			regex::ECMAScript | regex::icase
		);
		
		regex_search(nwk, match, e);
		
		if(!match.empty()) result = match[1].str();
	}
	
	return result;
}
 
__uint64_t Network::strIp6ToHex64(const string ip){
	
	__uint64_t result = 0;
	
	stringstream strm;
	
	regex e("[^A-Fa-f\\d]", regex::ECMAScript | regex::icase);
	
	string str = regex_replace(ip, e, "");
	
	if(str.length() > 16) str = str.erase(15, str.length());
	
	strm << str;
	
	strm >> std::hex >> result;
	
	return result;
}
 
const u_int Network::checkNetworkByIp(const string ip){
	
	smatch match;
	
	regex e(
		"^\\d{1,3}(?:\\.\\d{1,3}){3}$",
		regex::ECMAScript | regex::icase
	);
	
	regex_search(ip, match, e);
	
	if(!match.empty()) return 4;
	
	else {
		
		regex e(
			"^\\[?[A-Fa-f\\d]{1,4}(?:\\:[A-Fa-f\\d]{1,4}){7}\\]?$",
			regex::ECMAScript | regex::icase
		);
		
		const string ip2 = setLow2Ip6(ip);
		
		regex_search(ip2, match, e);
		
		if(!match.empty()) return 6;
		
		else return 0;
	}
}
 
const u_int Network::getPrefixByNetwork(const string nwk){
	
	u_int result = 0;
	
	if(!nwk.empty()){
		
		smatch match;
		
		regex e(
			
			"(?:\\d{1,3}(?:\\.\\d{1,3}){3}|(?:[A-Fa-f\\d]{1,4}(?:(?:\\:[A-Fa-f\\d]{1,4}){7}|(?:\\:[A-Fa-f\\d]{1,4}){1,6}\\:\\:|\\:\\:)|\\:\\:))\\/(\\d{1,3}(?:\\.\\d{1,3}){3}|\\d+)",
			regex::ECMAScript | regex::icase
		);
		
		regex_search(nwk, match, e);
		
		if(!match.empty()){
			
			string mask = match[1].str();
			
			if(Anyks::isNumber(mask)) result = ::atoi(mask.c_str());
			
			else {
				
				NTdata data = getMaskByString(mask);
				
				result = data.number;
			}
		}
	}
	
	return result;
}
 
const int Network::isLocal(const string ip){
	
	smatch match;
	
	regex e(
		"^\\d{1,3}(?:\\.\\d{1,3}){3}$",
		regex::ECMAScript | regex::icase
	);
	
	regex_search(ip, match, e);
	
	if(!match.empty()){
		
		IPdata ipdata = getDataIp(ip);
		
		for(u_int i = 0; i < locals.size(); i++){
			
			if(checkMaskByNumber(ip, locals[i].mask)){
				
				string mask = getMaskByNumber(locals[i].mask).mask;
				
				IPdata maskdata = getDataIp(mask);
				
				IPdata networkdata = imposeMask(ipdata, maskdata);
				
				if(networkdata.getStr()
				.compare(locals[i].network) == 0){
					
					if(locals[i].allow) return 0;
					
					else return -1;
				}
			}
		}
		
		return 1;
	}
	
	return -1;
}
 
const int Network::isLocal6(const string ip){
	
	int result = 1;
	
	bool compare = false;
	
	for(u_int i = 0; i < this->locals6.size(); i++){
		
		if(!this->locals6[i].eip.empty())
			
			compare = checkRange6(ip, this->locals6[i].ip, this->locals6[i].eip);
		else {
			
			string network = Anyks::toCase(setLowIp6(this->locals6[i].ip));
			
			string ipv6 = imposePrefix6(ip, this->locals6[i].prefix);
			
			ipv6 = Anyks::toCase(setLowIp6(ipv6));
			
			vector <string> mip = Anyks::split(ipv6, ":");
			
			if(!mip.empty()){
				
				if(mip[0].compare("0000") == 0) compare = true;
				
				else {
					
					vector <string> nwk = Anyks::split(network, ":");
					
					for(u_int j = 0; j < mip.size(); j++){
						
						if((mip[j].compare(nwk[j]) == 0) || (nwk[j].compare("0000") == 0)) compare = true;
						else {
							
							compare = false;
							
							break;
						}
					}
				}
			}
		}
		
		if(compare) result = (!locals6[i].allow ? -1 : 0);
	}
	
	return result;
}
