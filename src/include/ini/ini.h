/* МОДУЛЬ ЧТЕНИЯ КОНФИГУРАЦИОННЫХ ФАЙЛОВ ANYKS */
/*
*	автор:				Юрий Николаевич Лобарев
*	skype:				efrantick
*	телефон:			+7(920)672-33-22
*	авторские права:	Все права принадлежат автору © Юрий Лобарев, 2017
*/
#ifndef _CONFIG_INI_ANYKS_
#define _CONFIG_INI_ANYKS_

#include <regex>
#include <vector>
#include <string>
#include <fstream>
#include <iostream>
#include <unordered_map>
#include <stdio.h>
#include <stdlib.h>
#include <sys/types.h>
#include "general/general.h"

// Устанавливаем область видимости
using namespace std;

/**
 * INI Класс чтения конфигурационных данных из ini файлов
 */
class INI {
	private:
		/**
		 * Params Структура параметров конфигурационного файла
		 */
		struct Params {
			string key;		// Ключ
			string value;	// Значение
		};
		// Ошибка чтения файла
		bool error = false;
		// Название раздела
		string section;
		// Собранные конфигурационные данные
		unordered_map <string, vector <Params>> data;
		/**
		 * getParams Метод извлечения параметров
		 * @param  str строка для проверки
		 * @return     данные параметров
		 */
		const Params getParams(const string str);
		/**
		 * getSection Метод извлечения раздела
		 * @param  str строка для проверки
		 * @return     название раздела
		 */
		const string getSection(const string str);
		/**
		 * get Метод извлечения данных
		 * @param  section раздел
		 * @param  key     ключ
		 * @return         искомый результат
		 */
		const string get(const string section, const string key);
	public:
		/**
		 * getSections Получить список разделов
		 * @return список разделов
		 */
		const vector <string> getSections();
		/**
		 * getParamsInSection Получить список параметров в разделе
		 * @param  section раздел
		 * @return         список параметров
		 */
		const vector <Params> getParamsInSection(const string section);
		/**
		 * getFloat Получить значение числа с плавающей точкой
		 * @param  section раздел
		 * @param  key     ключ
		 * @param  defval  значение по умолчанию, если ключ не найден
		 * @return         искомый результат
		 */
		const double getFloat(const string section, const string key, const double defval = 0.0);
		/**
		 * getNumber Получить значение числа в знаковой форме
		 * @param  section раздел
		 * @param  key     ключ
		 * @param  defval  значение по умолчанию, если ключ не найден
		 * @return         искомый результат
		 */
		const int64_t getNumber(const string section, const string key, const int64_t defval = 0);
		/**
		 * getUNumber Получить значение числа в беззнаковой форме
		 * @param  section раздел
		 * @param  key     ключ
		 * @param  defval  значение по умолчанию, если ключ не найден
		 * @return         искомый результат
		 */
		const size_t getUNumber(const string section, const string key, const size_t defval = 0);
		/**
		 * getBoolean Получить значение в булевом виде
		 * @param  section раздел
		 * @param  key     ключ
		 * @param  defval  значение по умолчанию, если ключ не найден
		 * @return         искомый результат
		 */
		const bool getBoolean(const string section, const string key, const bool defval = false);
		/**
		 * getString Получить значение строки
		 * @param  section раздел
		 * @param  key     ключ
		 * @param  defval  значение по умолчанию, если ключ не найден
		 * @return         искомый результат
		 */
		const string getString(const string section, const string key, const string defval = "");
		/**
		 * addData Метод добавления данных
		 * @param  section название раздела
		 * @param  key     название параметра
		 * @param  value   значение параметра
		 * @return         результат добавления
		 */
		const bool addData(const string section, const string key, const string value);
		/**
		 * delData Метод удаления данных
		 * @param  section название раздела
		 * @param  key     название параметра
		 * @return         результат удаления
		 */
		const bool delData(const string section, const string key);
		/**
		 * addSection Метод добавления раздела
		 * @param  name название раздела
		 * @return      результат добавления
		 */
		const bool addSection(const string name);
		/**
		 * delSection Метод удаления раздела
		 * @param  name название раздела
		 * @return      результат удаления
		 */
		const bool delSection(const string name);
		/**
		 * isError Метод проверяющий на возникновение ошибки чтения
		 * @return результат проверки
		 */
		const bool isError();
		/**
		 * read Метод чтения данных из файла
		 * @param filename адрес конфигурационного файла
		 */
		void read(const string filename);
		/**
		 * write Метод записи данных в файл
		 * @param filename адрес конфигурационного файла
		 */
		void write(const string filename);
		/**
		 * INI Конструктор
		 * @param filename адрес конфигурационного файла
		 */
		INI(const string filename);
		/**
		 * ~INI Деструктор
		 */
		~INI();
};

#endif // _CONFIG_INI_ANYKS_
