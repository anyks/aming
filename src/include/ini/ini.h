// Read an INI file into easy-to-access name/value pairs.

// inih and INI are released under the New BSD license (see LICENSE.txt).
// Go to the project home page for more info:
//
// https://github.com/benhoyt/inih

#ifndef __INIREADER_H__
#define __INIREADER_H__

#include <map>
#include <string>
#include <cctype>
#include <cstdlib>
#include <cstring>
#include <stdlib.h>
#include <algorithm>

/* Max params */
#define MAX_SECTION 50
#define MAX_NAME 50

/* Nonzero to allow multi-line value parsing, in the style of Python's
	configparser. If allowed, ini_parse() will call the handler with the same
	name for each subsequent line parsed. */
#ifndef INI_ALLOW_MULTILINE
#define INI_ALLOW_MULTILINE 1
#endif

/* Nonzero to allow a UTF-8 BOM sequence (0xEF 0xBB 0xBF) at the start of
	the file. See http://code.google.com/p/inih/issues/detail?id=21 */
#ifndef INI_ALLOW_BOM
#define INI_ALLOW_BOM 1
#endif

/* Nonzero to allow inline comments (with valid inline comment characters
	specified by INI_INLINE_COMMENT_PREFIXES). Set to 0 to turn off and match
	Python 3.2+ configparser behaviour. */
#ifndef INI_ALLOW_INLINE_COMMENTS
#define INI_ALLOW_INLINE_COMMENTS 1
#endif
#ifndef INI_INLINE_COMMENT_PREFIXES
#define INI_INLINE_COMMENT_PREFIXES ";"
#endif

/* Nonzero to use stack, zero to use heap (malloc/free). */
#ifndef INI_USE_STACK
#define INI_USE_STACK 1
#endif

/* Stop parsing on first error (default is to keep parsing). */
#ifndef INI_STOP_ON_FIRST_ERROR
#define INI_STOP_ON_FIRST_ERROR 0
#endif

/* Maximum line length for any line in INI file. */
#ifndef INI_MAX_LINE
#define INI_MAX_LINE 200
#endif

using namespace std;

// Read an INI file into easy-to-access name/value pairs. (Note that I've gone
// for simplicity here rather than speed, but it should be pretty decent.)
class INI {
	public:
		// Construct INI and parse given filename. See ini.h for more info
		// about the parsing.
		INI(const string &filename);
		// Return the result of ini_parse(), i.e., 0 on success, line number of
		// first error on parse error, or -1 on file open error.
		int ParseError() const;
		// Get a string value from INI file, returning default_value if not found.
		string Get(const string &section, const string &name, const string &default_value) const;
		// Get an integer (long) value from INI file, returning default_value if
		// not found or not a valid integer (decimal "1234", "-1234", or hex "0x4d2").
		long GetInteger(const string &section, const string &name, long default_value) const;
		// Get a real (floating point double) value from INI file, returning
		// default_value if not found or not a valid floating point value
		// according to strtod().
		double GetReal(const string &section, const string &name, double default_value) const;
		// Get a boolean value from INI file, returning default_value if not found or if
		// not a valid true/false value. Valid true values are "true", "yes", "on", "1",
		// and valid false values are "false", "no", "off", "0" (not case sensitive).
		bool GetBoolean(const string &section, const string &name, bool default_value) const;
	private:
		int _error;
		map<string, string> _values;
		/* Typedef for prototype of handler function. */
		#if INI_HANDLER_LINENO
			typedef int (* ini_handler) (void * user, const char * section, const char * name, const char * value, int lineno);
		#else
			typedef int (* ini_handler) (void * user, const char * section, const char * name, const char * value);
		#endif
		/* Typedef for prototype of fgets-style reader function. */
		typedef char * (* ini_reader) (char * str, int num, void * stream);
		/* Other function */
		static string MakeKey(const string &section, const string &name);
		static int ValueHandler(void * user, const char * section, const char * name, const char * value);
		/* Strip whitespace chars off end of given string, in place. Return s. */
		static char * rstrip(char * s);
		/* Return pointer to first non-whitespace char in given string. */
		static char * lskip(const char * s);
		/* Return pointer to first char (of chars) or inline comment in given string,
		   or pointer to null at end of string if neither found. Inline comment must
		   be prefixed by a whitespace character to register as a comment. */
		static char * findCharsOrComment(const char * s, const char * chars);
		/* Version of strncpy that ensures dest (size bytes) is null-terminated. */
		static char * strncpy0(char * dest, const char * src, size_t size);
		/* See documentation in header file. */
		int iniParseStream(ini_reader reader, void * stream, ini_handler handler, void * user);
		/* See documentation in header file. */
		int iniParseFile(FILE * file, ini_handler handler, void * user);
		/* See documentation in header file. */
		int iniParse(string filename, ini_handler handler, void * user);
};

#endif  // __INIREADER_H__
