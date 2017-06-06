/* МОДУЛЬ ЧТЕНИЯ КОНФИГУРАЦИОННЫХ ФАЙЛОВ ANYKS */
/*
*	автор:				Юрий Николаевич Лобарев
*	skype:				efrantick
*	телефон:			+7(920)672-33-22
*	авторские права:	Все права принадлежат автору © Юрий Лобарев, 2017
*/
#ifndef _CONFIG_INI_ANYKS_
#define _CONFIG_INI_ANYKS_

#include <map>
#include <regex>
#include <vector>
#include <string>
#include <fstream>
#include <iostream>
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
		// Название раздела
		string section;
		// Собранные конфигурационные данные
		map <string, vector <Params>> data;
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
