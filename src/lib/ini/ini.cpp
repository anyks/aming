// Read an INI file into easy-to-access name/value pairs.

// inih and INI are released under the New BSD license (see LICENSE.txt).
// Go to the project home page for more info:
//
// https://github.com/benhoyt/inih
#include "ini/ini.h"

using namespace std;

INI::INI(const string &filename){
	_error = iniParse(filename, ValueHandler, this);
}

int INI::ParseError() const {
	return _error;
}

string INI::Get(const string &section, const string &name, const string &default_value) const {
	string key = MakeKey(section, name);
	// Use _values.find() here instead of _values.at() to support pre C++11 compilers
	return _values.count(key) ? _values.find(key)->second : default_value;
}

long INI::GetInteger(const string &section, const string &name, long default_value) const {
	string valstr = Get(section, name, "");
	const char * value = valstr.c_str();
	char * end;
	// This parses "1234" (decimal) and also "0x4D2" (hex)
	long n = strtol(value, &end, 0);
	return (end > value ? n : default_value);
}

double INI::GetReal(const string &section, const string &name, double default_value) const {
	string valstr = Get(section, name, "");
	const char * value = valstr.c_str();
	char * end;
	double n = strtod(value, &end);
	return (end > value ? n : default_value);
}

bool INI::GetBoolean(const string &section, const string &name, bool default_value) const {
	string valstr = Get(section, name, "");
	// Convert to lower case to make string comparisons case-insensitive
	transform(valstr.begin(), valstr.end(), valstr.begin(), ::tolower);
	if((valstr.compare("true") == 0) || (valstr.compare("yes") == 0)
	|| (valstr.compare("on") == 0) || (valstr.compare("1") == 0)) return true;
	else if((valstr.compare("false") == 0) || (valstr.compare("no") == 0)
	|| (valstr.compare("off") == 0) || (valstr.compare("0") == 0)) return false;
	else return default_value;
}

string INI::MakeKey(const string& section, const string& name){
	string key = section + "=" + name;
	// Convert to lower case to make section/name lookups case-insensitive
	transform(key.begin(), key.end(), key.begin(), ::tolower);
	return key;
}

int INI::ValueHandler(void * user, const char * section, const char * name, const char * value){
	INI * reader = (INI *) user;
	string key = MakeKey(section, name);
	if(reader->_values[key].size() > 0) reader->_values[key] += "\n";
	reader->_values[key] += value;
	return 1;
}

/* Strip whitespace chars off end of given string, in place. Return s. */
char * INI::rstrip(char * s){
	char * p = s + strlen(s);
	while(p > s && isspace((unsigned char)(*--p))) *p = '\0';
	return s;
}

/* Return pointer to first non-whitespace char in given string. */
char * INI::lskip(const char * s){
	while(*s && isspace((unsigned char)(*s))) s++;
	return (char*)s;
}

/* Return pointer to first char (of chars) or inline comment in given string,
   or pointer to null at end of string if neither found. Inline comment must
   be prefixed by a whitespace character to register as a comment. */
char * INI::findCharsOrComment(const char * s, const char * chars){
#if INI_ALLOW_INLINE_COMMENTS
	int was_space = 0;
	while(*s && (!chars || !strchr(chars, *s))
	&& !(was_space && strchr(INI_INLINE_COMMENT_PREFIXES, *s))){
		was_space = isspace((unsigned char)(*s));
		s++;
	}
#else
	while(*s && (!chars || !strchr(chars, *s))) s++;
#endif
	return (char *) s;
}

/* Version of strncpy that ensures dest (size bytes) is null-terminated. */
char * INI::strncpy0(char * dest, const char * src, size_t size){
	strncpy(dest, src, size);
	dest[size - 1] = '\0';
	return dest;
}

/* See documentation in header file. */
int INI::iniParseStream(ini_reader reader, void * stream, ini_handler handler, void * user){
	/* Uses a fair bit of stack (use heap instead if you need to) */
#if INI_USE_STACK
	char line[INI_MAX_LINE];
#else
	char * line;
#endif
	char section[MAX_SECTION] = "";
	char prev_name[MAX_NAME] = "";

	char * start;
	char * end;
	char * name;
	char * value;
	int lineno = 0;
	int error = 0;

#if !INI_USE_STACK
	line = new char[INI_MAX_LINE];
	if(!line) return -2;
#endif

#if INI_HANDLER_LINENO
#define HANDLER(u, s, n, v) handler(u, s, n, v, lineno)
#else
#define HANDLER(u, s, n, v) handler(u, s, n, v)
#endif

	/* Scan through stream line by line */
	while(reader(line, INI_MAX_LINE, stream)){
		lineno++;
		start = line;
#if INI_ALLOW_BOM
		if(lineno == 1 && (unsigned char)start[0] == 0xEF
		&& (unsigned char) start[1] == 0xBB
		&& (unsigned char) start[2] == 0xBF) start += 3;
#endif
		start = lskip(rstrip(start));

		if((* start == ';') || (* start == '#')){
			/* Per Python configparser, allow both ; and # comments at the
			   start of a line */
		}
#if INI_ALLOW_MULTILINE
		else if((* prev_name) && ((* start) && (start > line))) {
			/* Non-blank line with leading whitespace, treat as continuation
			   of previous name's value (as per Python configparser). */
			if(!HANDLER(user, section, prev_name, start) && !error) error = lineno;
		}
#endif
		else if(* start == '[') {
			/* A "[section]" line */
			end = findCharsOrComment(start + 1, "]");
			if(* end == ']'){
				* end = '\0';
				strncpy0(section, start + 1, sizeof(section));
				* prev_name = '\0';
			} else if(!error) {
				/* No ']' found on section line */
				error = lineno;
			}
		} else if(* start) {
			/* Not a comment, must be a name[=:]value pair */
			end = findCharsOrComment(start, "=:");
			if((* end == '=') || (*end == ':')){
				* end = '\0';
				name = rstrip(start);
				value = end + 1;
#if INI_ALLOW_INLINE_COMMENTS
				end = findCharsOrComment(value, NULL);
				if(* end) * end = '\0';
#endif
				value = lskip(value);
				rstrip(value);

				/* Valid name[=:]value pair found, call handler */
				strncpy0(prev_name, name, sizeof(prev_name));
				if(!HANDLER(user, section, name, value) && !error) error = lineno;
			} else if(!error) {
				/* No '=' or ':' found on name[=:]value line */
				error = lineno;
			}
		}
#if INI_STOP_ON_FIRST_ERROR
	if(error) break;
#endif
	}
#if !INI_USE_STACK
	delete [] line;
#endif
	return error;
}

/* See documentation in header file. */
int INI::iniParseFile(FILE * file, ini_handler handler, void * user){
	return iniParseStream((ini_reader) fgets, file, handler, user);
}

/* See documentation in header file. */
int INI::iniParse(string filename, ini_handler handler, void * user){
	FILE * file = fopen(filename.c_str(), "r");
	if(!file) return -1;
	int error = iniParseFile(file, handler, user);
	fclose(file);
	return error;
}