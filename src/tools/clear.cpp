/* SERVER PROXY AMING */
/*
*  author:     Yuriy Lobarev
*  phone:      +7(910)983-95-90
*  telegram:   @forman
*  email:      info@anyks.com
*  copyright:  © 2017 anyks.com
*/

#include <string>
#include <ctime>
#include <clocale>
#include <stdio.h>

// Данные приложения
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

// Устанавливаем пространство имен
using namespace std;

// Основные флаги
bool inInlineLastEscape = false;
bool inMultilineLastStar = false;
bool inStringLastEscape = false;
bool inCharLastEscape = false;

// g++ ./tools/clear.cpp -o ./bin/clear

/*
* Прототипы функций
*/
typedef void Parser(char, FILE *);
Parser* currentParser;

void InStringParser(char c, FILE *output_file);
void InCharParser(char c, FILE *output_file);
void AfterSlashParser(char c, FILE *output_file);
void InInlineCommentParser(char c, FILE *output_file);
void InMultilineCommentParser(char c, FILE *output_file);

/**
 * NormalParser Функция установки нормального парсера
 * @param c           символ
 * @param output_file файловый поток для записи
 */
void NormalParser(char c, FILE * output_file){
	switch (c){
		case '\"': currentParser = InStringParser; break;
		case '\'': currentParser = InCharParser; break;
		// no need to print
		case '/': currentParser = AfterSlashParser; return;
	}
    fprintf(output_file, "%c", c);
}
/**
 * InStringParser Функция определения строки в парсере
 * @param c           символ
 * @param output_file файловый поток для записи
 */
void InStringParser(char c, FILE * output_file){
	fprintf(output_file, "%c", c);
	if((c == '\"') && !inStringLastEscape)
		currentParser = NormalParser; // inStringLastEscape will be false
	inStringLastEscape = (c == '\\') && !inStringLastEscape;
}
/**
 * InCharParser Функция определения символа в парсере
 * @param c           символ
 * @param output_file файловый поток для записи
 */
void InCharParser(char c, FILE * output_file){
	fprintf(output_file, "%c", c);
	if((c == '\'') && !inCharLastEscape)
		currentParser = NormalParser; // inCharLastEscape will be false
	inCharLastEscape = (c == '\\') && !inCharLastEscape;
}
/**
 * AfterSlashParser Функция определения типа парсера
 * @param c           символ
 * @param output_file файловый поток для записи
 */
void AfterSlashParser(char c, FILE * output_file){
	switch(c){
		case '/': currentParser = InInlineCommentParser; break;
		case '*': currentParser = InMultilineCommentParser; break;
		default: fprintf(output_file, "/%c", c);
	}
}
/**
 * InInlineCommentParser Функция парсера однострочных комментариев
 * @param c           символ
 * @param output_file файловый поток для записи
 */
void InInlineCommentParser(char c, FILE * output_file){
	if(c == '\n' && !inInlineLastEscape){
		fprintf(output_file, "\n");
		currentParser = NormalParser;
	}
	inInlineLastEscape = (c == '\\') && !inInlineLastEscape;
}
/**
 * InMultilineCommentParser Функция парсера мультистрочных комментариев
 * @param c           символ
 * @param output_file файловый поток для записи
 */
void InMultilineCommentParser(char c, FILE * output_file){
	if(c == '/' && inMultilineLastStar){
		fprintf(output_file, " ");
		currentParser = NormalParser; // inMultilineLastStar will be false
	}
	inMultilineLastStar = (c == '*');
}
/**
 * main Главная функция приложения
 * @param  argc длина массива параметров
 * @param  argv массив параметров
 * @return      код выхода из приложения
 */
int main(int argc, char * argv[]){
	// Активируем локаль приложения
	// setlocale(LC_ALL, "");
	setlocale(LC_ALL, "en_US.UTF-8");
	// Определяем параметр для входного файла
	string inFile = (argc >= 2 ? argv[1] : "");
	// Определяем параметр для выходного файла
	string outFile = (argc >= 3 ? argv[2] : "");
	// Если файлы существуют
	if(!inFile.empty() && !outFile.empty()){
		// Запоминаем данные парсера
		currentParser = NormalParser;
		// Открываем файл на чтение
		FILE * input_file = fopen(inFile.c_str(), "r");
		// Открываем файл на запись
		FILE * output_file = fopen(outFile.c_str(), "w");
		// Если какой-то из файлов не открыт
		if((input_file == nullptr) || (output_file == nullptr)) perror("Error opening file");
		// Если файлы открыты удачно
		else {
			// Создаем буфер для хранения даты
			char date[80], year[5];
			// Определяем количество секунд
			time_t seconds = time(nullptr);
			// Получаем структуру локального времени
			struct tm * timeinfo = localtime(&seconds);
			// Копируем в буфер полученный год
			const int yearlen = strftime(year, sizeof(year), "%Y", timeinfo);
			// Копируем в буфер полученную дату и время
			const int datelen = strftime(date, sizeof(date), "%m/%d/%Y %H:%M:%S", timeinfo);
			// Устанавливаем конец строки
			year[yearlen] = '\0';
			// Устанавливаем конец строки
			date[datelen] = '\0';
			// Создаем строку с годом
			const string zyear(year, yearlen);
			// Создаем строку с датой
			const string zdate(date, datelen);
			// Создаем заголовок для вставки
			const string header = "/* %s %s */\r\n/*\r\n"
			"*  author:     %s\r\n"
			"*  phone:      %s\r\n"
			"*  telegram:   %s\r\n"
			"*  email:      %s\r\n"
			"*  date:       %s\r\n"
			"*  copyright:  © %s %s\r\n*/\r\n";
			// Вставляем заголовки в каждый файл
			fprintf(output_file, header.c_str(), APP_TARGET, APP_NAME, APP_AUTHOR, APP_PHONE, APP_TELEGRAM, APP_EMAIL, zdate.c_str(), zyear.c_str(), APP_DOMAIN);
			// Символ считываемый из входного файла
			char symbol;
			// Начинаем считывать по символу из файла до тех пор пока все не прочитаем
			while((symbol = fgetc(input_file)) != EOF)
				// Подключаем наш парсер который удаляет все комментарии из кода
				currentParser(symbol, output_file);
			// Закрываем все файлы
			fclose(input_file);
			fclose(output_file);
		}
	}
	// Завершаем выполнение приложения с удачным кодом
	return 0;
}
