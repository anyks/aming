/* МОДУЛЬ ЧТЕНИЯ КОНФИГУРАЦИОННЫХ ФАЙЛОВ ANYKS */
/*
*	автор:				Юрий Николаевич Лобарев
*	skype:				efrantick
*	телефон:			+7(920)672-33-22
*	авторские права:	Все права принадлежат автору © Юрий Лобарев, 2017
*/
#include "ini/ini.h"

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
 * getSections Получить список разделов
 * @return список разделов
 */
const vector <string> INI::getSections(){
	// Результат полученных данных
	vector <string> result;
	// Если раздел передан
	if(!this->data.empty()){
		// Переходим по всем разделам
		for(auto it = this->data.cbegin(); it != this->data.cend(); ++it){
			// Добавляем данные в массив результатов
			result.push_back(it->first);
		}
	}
	// Выводим результат
	return result;
}
/**
 * getParamsInSection Получить список параметров в разделе
 * @param  section раздел
 * @return         список параметров
 */
const vector <INI::Params> INI::getParamsInSection(const string section){
	// Результат полученных данных
	vector <Params> result;
	// Если раздел передан
	if(!section.empty() && !this->data.empty()){
		// Переходим по всем разделам
		for(auto it = this->data.cbegin(); it != this->data.cend(); ++it){
			// Получаем данные раздела
			auto params = it->second;
			// Выполняем перебор полученных данных
			for(auto it = params.cbegin(); it != params.cend(); ++it){
				// Добавляем данные в массив результатов
				result.push_back({it->key, it->value});
			}
		}
	}
	// Выводим результат
	return result;
}
/**
 * getFloat Получить значение числа с плавающей точкой
 * @param  section раздел
 * @param  key     ключ
 * @param  defval  значение по умолчанию, если ключ не найден
 * @return         искомый результат
 */
const double INI::getFloat(const string section, const string key, const double defval){
	// Запрашиваем данные
	string data = get(section, key);
	// Если данные пришли верные
	if(!data.empty()){
		// Результат работы регулярного выражения
		smatch match;
		// Устанавливаем правило регулярного выражения
		regex e("^\\-?(?:\\d+(?:\\.|\\,)\\d+|\\d+)$", regex::ECMAScript | regex::icase);
		// Выполняем проверку на то является ли это числом
		regex_search(data, match, e);
		// Если это число, выводим результат
		if(!match.empty()) return ::atof(match[0].str().c_str());
		// Если это не число то нужно прверить на булевые значения
		else {
			// Устанавливаем правило регулярного выражения
			regex e("^(true|false)$", regex::ECMAScript | regex::icase);
			// Выполняем проверку на то является ли это булевым значением
			regex_search(data, match, e);
			// Если это булевое значение, определяем результат
			if(!match.empty()){
				// Получаем данные зачения
				string value = match[1].str();
				// Определяем булевое значение
				if(::toCase(value).compare("true") == 0)
					return 1.0;
				else return 0.0;
			}
		}
	}
	// Выводим результат
	return defval;
}
/**
 * getNumber Получить значение числа в знаковой форме
 * @param  section раздел
 * @param  key     ключ
 * @param  defval  значение по умолчанию, если ключ не найден
 * @return         искомый результат
 */
const int64_t INI::getNumber(const string section, const string key, const int64_t defval){
	// Запрашиваем данные
	string data = get(section, key);
	// Если данные пришли верные
	if(!data.empty()){
		// Результат работы регулярного выражения
		smatch match;
		// Устанавливаем правило регулярного выражения
		regex e("^\\-?\\d+$", regex::ECMAScript | regex::icase);
		// Выполняем проверку на то является ли это числом
		regex_search(data, match, e);
		// Если это число, выводим результат
		if(!match.empty()) return ::atoi(match[0].str().c_str());
		// Если это не число то нужно прверить на булевые значения
		else {
			// Устанавливаем правило регулярного выражения
			regex e("^(true|false)$", regex::ECMAScript | regex::icase);
			// Выполняем проверку на то является ли это булевым значением
			regex_search(data, match, e);
			// Если это булевое значение, определяем результат
			if(!match.empty()){
				// Получаем данные зачения
				string value = match[1].str();
				// Определяем булевое значение
				if(::toCase(value).compare("true") == 0)
					return 1;
				else return 0;
			}
		}
	}
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
	// Запрашиваем данные
	string data = get(section, key);
	// Если данные пришли верные
	if(!data.empty()){
		// Результат работы регулярного выражения
		smatch match;
		// Устанавливаем правило регулярного выражения
		regex e("^\\d+$", regex::ECMAScript | regex::icase);
		// Выполняем проверку на то является ли это числом
		regex_search(data, match, e);
		// Если это число, выводим результат
		if(!match.empty()) return ::atoi(match[0].str().c_str());
		// Если это не число то нужно прверить на булевые значения
		else {
			// Устанавливаем правило регулярного выражения
			regex e("^(true|false)$", regex::ECMAScript | regex::icase);
			// Выполняем проверку на то является ли это булевым значением
			regex_search(data, match, e);
			// Если это булевое значение, определяем результат
			if(!match.empty()){
				// Получаем данные зачения
				string value = match[1].str();
				// Определяем булевое значение
				if(::toCase(value).compare("true") == 0)
					return 1;
				else return 0;
			}
		}
	}
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
	// Запрашиваем данные
	string data = get(section, key);
	// Если данные пришли верные
	if(!data.empty()){
		// Результат работы регулярного выражения
		smatch match;
		// Устанавливаем правило регулярного выражения
		regex e("^\\-?\\d+$", regex::ECMAScript | regex::icase);
		// Выполняем проверку на то является ли это числом
		regex_search(data, match, e);
		// Если это число, выводим результат
		if(!match.empty()){
			// Преобразуем в число
			int64_t value = ::atoi(match[0].str().c_str());
			// Выводим результат
			if(value > 0) return true;
			else return false;
		// Если это не число то нужно прверить на булевые значения
		} else {
			// Устанавливаем правило регулярного выражения
			regex e("^(true|false)$", regex::ECMAScript | regex::icase);
			// Выполняем проверку на то является ли это булевым значением
			regex_search(data, match, e);
			// Если это булевое значение, определяем результат
			if(!match.empty()){
				// Получаем данные зачения
				string value = match[1].str();
				// Определяем булевое значение
				if(::toCase(value).compare("true") == 0)
					return true;
				else return false;
			}
		}
	}
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
 * checkParam Метод проверки существования параметра
 * @param  section название раздела
 * @param  key     ключ для проверки
 * @return         результат проверки
 */
const bool INI::checkParam(const string section, const string key){
	// Результат проверки
	bool result = false;
	// Если название раздела передано
	if(!section.empty() && !key.empty() && this->data.count(section)){
		// Получаем данные раздела
		auto params = this->data.find(section)->second;
		// Выполняем перебор полученных данных
		for(auto it = params.cbegin(); it != params.cend(); ++it){
			// Если ключ найден, выводим результат
			if(key.compare(it->key) == 0){
				// Запоминаем что результат найден
				result = true;
				// Выходим из цикла
				break;
			}
		}
	}
	// Выводим результат
	return result;
}
/**
 * checkSection Метод проверки существования раздела
 * @param  name название раздела
 * @return      результат проверки
 */
const bool INI::checkSection(const string name){
	// Результат проверки
	bool result = false;
	// Если название раздела передано
	if(!name.empty() && this->data.count(name)) result = true;
	// Выводим результат
	return result;
}
/**
 * addData Метод добавления данных
 * @param  section название раздела
 * @param  key     название параметра
 * @param  value   значение параметра
 * @return         результат добавления
 */
const bool INI::addData(const string section, const string key, const string value){
	// Результат удаления
	bool result = false;
	// Если данные переданы
	if(!section.empty() && !key.empty()){
		// Сообщаем что все удачно
		result = true;
		// Полученные параметры
		Params params = {key, value};
		// Если раздел существует
		if(this->data.count(section)){
			// Добавляем новые параметры в раздел
			this->data.find(section)->second.push_back(params);
		// Если раздел не найден
		} else {
			// Добавляем новые параметры
			this->data.insert(pair <string, vector <Params>>(section, {params}));
		}
	}
	// Выводим результат
	return result;
}
/**
 * delData Метод удаления данных
 * @param  section название раздела
 * @param  key     название параметра
 * @return         результат удаления
 */
const bool INI::delData(const string section, const string key){
	// Результат удаления
	bool result = false;
	// Если данные переданы
	if(!section.empty() && !key.empty()){
		// Если раздел существует
		if(this->data.count(section)){
			// Получаем данные раздела
			auto params = this->data.find(section)->second;
			// Выполняем перебор полученных данных
			for(auto it = params.cbegin(); it != params.cend(); ++it){
				// Если ключ найден, выводим результат
				if(key.compare(it->key) == 0){
					// Сообщаем что все удачно
					result = true;
					// Если ключ найден тогда удаляем элемент
					params.erase(it);
					// Удаляем раздел
					this->data.erase(section);
					// Добавляем новые параметры
					this->data.insert(pair <string, vector <Params>>(section, params));
					// Выходим из цикла
					break;
				}
			}
		}
	}
	// Выводим результат
	return result;
}
/**
 * addSection Метод добавления раздела
 * @param  name название раздела
 * @return      результат добавления
 */
const bool INI::addSection(const string name){
	// Результат добавления
	bool result = false;
	// Если раздел не существует
	if(!name.empty() && !this->data.count(name)){
		// Запоминаем что все удачно
		result = true;
		// Добавляем параметры
		this->data.insert(pair <string, vector <Params>>(name, {{}}));
	}
	// Выводим результат
	return result;
}
/**
 * delSection Метод удаления раздела
 * @param  name название раздела
 * @return      результат удаления
 */
const bool INI::delSection(const string name){
	// Результат удаления
	bool result = false;
	// Если раздел существует
	if(!name.empty() && this->data.count(name)){
		// Запоминаем что все удачно
		result = true;
		// Добавляем параметры
		this->data.erase(name);
	}
	// Выводим результат
	return result;
}
/**
 * isError Метод проверяющий на возникновение ошибки чтения
 * @return результат проверки
 */
const bool INI::isError(){
	// Выводим результат
	return this->error;
}
/**
 * read Метод чтения данных из файла
 * @param filename адрес конфигурационного файла
 */
void INI::read(const string filename){
	// Если файл передан
	if(!filename.empty() && isFileExist(filename.c_str())){
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
	// Запоминаем что приозошла ошибка
	} else this->error = true;
}
/**
 * write Метод записи данных в файл
 * @param filename адрес конфигурационного файла
 */
void INI::write(const string filename){
	// Если файл передан
	if(!filename.empty()){
		// Результирующая строка
		string result = "";
		// Переходим по всем разделам
		for(auto it = this->data.cbegin(); it != this->data.cend(); ++it){
			// Получаем данные раздела
			auto params = it->second;
			// Добавляем название раздела
			result.append(string("\r\n[") + it->first + string("]\r\n"));
			// Выполняем перебор полученных данных
			for(auto it = params.cbegin(); it != params.cend(); ++it){
				// Добавляем данные раздела
				result.append(it->key + string(" = ") + it->value + string("\r\n"));
			}
		}
		// Документ сформирован
		if(!result.empty()){
			// Открываем файл на чтение
			ofstream config(filename.c_str(), ios::binary);
			// Если файл открыт
			if(config.is_open()){
				// Выполняем запись данных в файл
				config.write((const char *) result.data(), result.size());
				// Закрываем файл
				config.close();
			}
		}
	}
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
