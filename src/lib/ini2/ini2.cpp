/* МОДУЛЬ ЧТЕНИЯ КОНФИГУРАЦИОННЫХ ФАЙЛОВ ANYKS */
/*
*	автор:				Юрий Николаевич Лобарев
*	skype:				efrantick
*	телефон:			+7(920)672-33-22
*	авторские права:	Все права принадлежат автору © Юрий Лобарев, 2017
*/
#include "ini2/ini2.h"

// Устанавливаем область видимости
using namespace std;

/**
 * getParams Метод извлечения параметров
 * @param  str строка для проверки
 * @return     данные параметров
 */
const INI::Params INI::getParams(const string str){
	// Полученные параметры
	Params params;
	// Если строка передана
	if(!str.empty()){
		// Результат работы регулярного выражения
		smatch match;
		// Устанавливаем правило регулярного выражения
		regex e("^\\s*(\\w+)\\s*\\=\\s*([^\\r\\n\\;\\#]+)", regex::ECMAScript | regex::icase);
		// Выполняем проверку
		regex_search(str, match, e);
		// Если раздел найден, выводим результат
		if(!match.empty()){
			// Получаем ключ
			params.key = match[1].str();
			// Получаем значение
			params.value = match[2].str();
			// Исправляем значения
			params.value = ::trim(params.value);
		}
	}
	// Выводим результат
	return params;
}
/**
 * getSection Метод извлечения раздела
 * @param  str строка для проверки
 * @return     название раздела
 */
const string INI::getSection(const string str){
	// Полученные параметры
	string result;
	// Если строка передана
	if(!str.empty()){
		// Результат работы регулярного выражения
		smatch match;
		// Устанавливаем правило регулярного выражения
		regex e("^\\s*\\[(\\w+)\\]", regex::ECMAScript | regex::icase);
		// Выполняем проверку
		regex_search(str, match, e);
		// Если раздел найден, выводим результат
		if(!match.empty()) result = match[1].str();
	}
	// Выводим результат
	return result;
}
/**
 * get Метод извлечения данных
 * @param  section раздел
 * @param  key     ключ
 * @return         искомый результат
 */
const string INI::get(const string section, const string key){
	// Результат
	string result;
	// Если данные переданы
	if(!section.empty() && !key.empty()){
		// Если раздел существует
		if(this->data.count(section)){
			// Получаем данные раздела
			auto params = this->data.find(section)->second;
			// Выполняем перебор полученных данных
			for(auto it = params.cbegin(); it != params.cend(); ++it){
				// Если ключ найден, выводим результат
				if(key.compare(it->key) == 0) return it->value;
			}
		}
	}
	// Выводим результат
	return result;
}
/**
 * getNumber Получить значение числа в знаковой форме
 * @param  section раздел
 * @param  key     ключ
 * @param  defval  значение по умолчанию, если ключ не найден
 * @return         искомый результат
 */
const int64_t INI::getNumber(const string section, const string key, const int64_t defval){
	// Выводим результат
	return defval;
}
/**
 * getUNumber Получить значение числа в беззнаковой форме
 * @param  section раздел
 * @param  key     ключ
 * @param  defval  значение по умолчанию, если ключ не найден
 * @return         искомый результат
 */
const size_t INI::getUNumber(const string section, const string key, const size_t defval){
	// Выводим результат
	return defval;
}
/**
 * getBoolean Получить значение в булевом виде
 * @param  section раздел
 * @param  key     ключ
 * @param  defval  значение по умолчанию, если ключ не найден
 * @return         искомый результат
 */
const bool INI::getBoolean(const string section, const string key, const bool defval){
	// Выводим результат
	return defval;
}
/**
 * getString Получить значение строки
 * @param  section раздел
 * @param  key     ключ
 * @param  defval  значение по умолчанию, если ключ не найден
 * @return         искомый результат
 */
const string INI::getString(const string section, const string key, const string defval){
	// Запрашиваем данные
	string data = get(section, key);
	// Выводим результат
	return (!data.empty() ? data : defval);
}
/**
 * read Метод чтения данных из файла
 * @param filename адрес конфигурационного файла
 */
void INI::read(const string filename){
	// Если файл передан
	if(!filename.empty()){
		// Открываем файл на чтение
		ifstream config(filename.c_str());
		// Если файл открыт
		if(config.is_open()){
			// Строка чтения из файла
			string filedata;
			// Считываем до тех пор пока все удачно
			while(config.good()){
				// Считываем строку из файла
				getline(config, filedata);
				// Получаем название раздела
				const string section = getSection(filedata);
				// Получаем параметры конфига
				const Params params = getParams(filedata);
				// Если раздел существует, запоминаем
				if(!section.empty()) this->section = section;
				// Если параметры существуют
				else if(!params.key.empty()){
					// Если раздел не существует
					if(!this->data.count(this->section))
						// Добавляем параметры
						this->data.insert(pair <string, vector <Params>>(this->section, {params}));
					// Если такой раздел существует
					else this->data.find(this->section)->second.push_back(params);
				}
			}
			// Очищаем название раздела
			this->section.clear();
			// Закрываем файл
			config.close();
		}
	}
}
/**
 * write Метод записи данных в файл
 * @param filename адрес конфигурационного файла
 */
void INI::write(const string filename){
	
}
/**
 * INI Конструктор
 * @param filename адрес конфигурационного файла
 */
INI::INI(const string filename){
	// Выполняем чтение данных из файла конфига
	read(filename);
}
/**
 * ~INI Деструктор
 */
INI::~INI(){
	// Очищаем данные
	this->data.clear();
}
