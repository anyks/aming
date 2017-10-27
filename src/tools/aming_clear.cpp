/* SERVER PROXY AMING */
/*
*  author:     Yuriy Lobarev
*  phone:      +7(910)983-95-90
*  telegram:   @forman
*  email:      info@anyks.com
*  date:       10/27/2017 18:42:35
*  copyright:  © 2017 anyks.com
*/
 
 

#include <string>
#include <ctime>
#include <clocale>
#include <stdio.h>


#define APP_TARGET "SERVER PROXY"
#define APP_NAME "AMING"
#define APP_VERSION "1.0"
#define APP_YEAR "2017"
#define APP_COPYRIGHT "ANYKS LLC"
#define APP_DOMAIN "anyks.com"
#define APP_SITE "http://anyks.com"
#define APP_EMAIL "info@anyks.com"
#define APP_SUPPORT "support@anyks.com"
#define APP_AUTHOR "Yuriy Lobarev"
#define APP_TELEGRAM "@forman"
#define APP_PHONE "+7(910)983-95-90"


using namespace std;


bool inInlineLastEscape = false;
bool inMultilineLastStar = false;
bool inStringLastEscape = false;
bool inCharLastEscape = false;



 
typedef void Parser(char, FILE *);
Parser* currentParser;

void InStringParser(char c, FILE *output_file);
void InCharParser(char c, FILE *output_file);
void AfterSlashParser(char c, FILE *output_file);
void InInlineCommentParser(char c, FILE *output_file);
void InMultilineCommentParser(char c, FILE *output_file);

 
void NormalParser(char c, FILE * output_file){
	switch (c){
		case '\"': currentParser = InStringParser; break;
		case '\'': currentParser = InCharParser; break;
		
		case '/': currentParser = AfterSlashParser; return;
	}
    fprintf(output_file, "%c", c);
}
 
void InStringParser(char c, FILE * output_file){
	fprintf(output_file, "%c", c);
	if((c == '\"') && !inStringLastEscape)
		currentParser = NormalParser; 
	inStringLastEscape = (c == '\\') && !inStringLastEscape;
}
 
void InCharParser(char c, FILE * output_file){
	fprintf(output_file, "%c", c);
	if((c == '\'') && !inCharLastEscape)
		currentParser = NormalParser; 
	inCharLastEscape = (c == '\\') && !inCharLastEscape;
}
 
void AfterSlashParser(char c, FILE * output_file){
	switch(c){
		case '/': currentParser = InInlineCommentParser; break;
		case '*': currentParser = InMultilineCommentParser; break;
		default: fprintf(output_file, "/%c", c);
	}
}
 
void InInlineCommentParser(char c, FILE * output_file){
	if(c == '\n' && !inInlineLastEscape){
		fprintf(output_file, "\n");
		currentParser = NormalParser;
	}
	inInlineLastEscape = (c == '\\') && !inInlineLastEscape;
}
 
void InMultilineCommentParser(char c, FILE * output_file){
	if(c == '/' && inMultilineLastStar){
		fprintf(output_file, " ");
		currentParser = NormalParser; 
	}
	inMultilineLastStar = (c == '*');
}
 
int main(int argc, char * argv[]){
	
	
	setlocale(LC_ALL, "en_US.UTF-8");
	
	string inFile = (argc >= 2 ? argv[1] : "");
	
	string outFile = (argc >= 3 ? argv[2] : "");
	
	if(!inFile.empty() && !outFile.empty()){
		
		currentParser = NormalParser;
		
		FILE * input_file = fopen(inFile.c_str(), "r");
		
		FILE * output_file = fopen(outFile.c_str(), "w");
		
		if((input_file == nullptr) || (output_file == nullptr)) perror("Error opening file");
		
		else {
			
			char date[80], year[5];
			
			time_t seconds = time(nullptr);
			
			struct tm * timeinfo = localtime(&seconds);
			
			const int yearlen = strftime(year, sizeof(year), "%Y", timeinfo);
			
			const int datelen = strftime(date, sizeof(date), "%m/%d/%Y %H:%M:%S", timeinfo);
			
			year[yearlen] = '\0';
			
			date[datelen] = '\0';
			
			const string zyear(year, yearlen);
			
			const string zdate(date, datelen);
			
			const string header = "/* %s %s */\r\n/*\r\n"
			"*  author:     %s\r\n"
			"*  phone:      %s\r\n"
			"*  telegram:   %s\r\n"
			"*  email:      %s\r\n"
			"*  date:       %s\r\n"
			"*  copyright:  © %s %s\r\n*/\r\n";
			
			fprintf(output_file, header.c_str(), APP_TARGET, APP_NAME, APP_AUTHOR, APP_PHONE, APP_TELEGRAM, APP_EMAIL, zdate.c_str(), zyear.c_str(), APP_DOMAIN);
			
			char symbol;
			
			while((symbol = fgetc(input_file)) != EOF)
				
				currentParser(symbol, output_file);
			
			fclose(input_file);
			fclose(output_file);
		}
	}
	
	return 0;
}
