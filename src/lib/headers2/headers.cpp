/* МОДУЛЬ УПРАВЛЕНИЯ ЗАГОЛОВКАМИ AMING */
/*
*	автор:				Юрий Николаевич Лобарев
*	skype:				efrantick
*	телефон:			+7(920)672-33-22
*	авторские права:	Все права принадлежат автору © Юрий Лобарев, 2017
*/
#include "headers2/headers.h"

// Устанавливаем область видимости
using namespace std;

/**
 * readFromLDAP Метод чтения данных из LDAP сервера
 */
void Headers2::readFromLDAP(){

}
/**
 * readFromFile Метод чтения данных из файла
 */
void Headers2::readFromFile(){
	// Получаем название файла
	const string name = getName();
	// Если название существует и файл тоже
	if(!name.empty() && checkAvailable(name)){
		// Получаем данные каталога где хранится файл с правилами
		const string dir = this->config->proxy.dir;
		// Получаем имя файла
		const string filename = Anyks::addToPath(dir, name + ".headers");
		// Проверяем на существование адреса
		if(!filename.empty()
		// Проверяем существует ли такой каталог
		&& Anyks::isDirExist(dir.c_str())
		// Проверяем существует ли такой файл
		&& Anyks::isFileExist(filename.c_str())){
			// Устанавливаем права на файл
			Anyks::setOwner(filename.c_str(), this->config->proxy.user, this->config->proxy.group);
			// Строка чтения из файла
			string filedata;
			// Открываем файл на чтение
			ifstream file(filename.c_str());
			// Если файл открыт
			if(file.is_open()){
				// Считываем до тех пор пока все удачно
				while(file.good()){
					// Считываем строку из файла
					getline(file, filedata);
					// Ищем комментарий
					size_t pos = filedata.find("#");
					// Если комментарий найден, вырезаем его из строки
					if(pos != string::npos) filedata = filedata.replace(pos, filedata.length() - pos, "");
					// Если строка существует
					if(!filedata.empty()){
						// Результат работы регулярного выражения
						smatch match;
						// Создаем регулярное выражение
						regex e(
							// Action
							"((?:ADD|RM|\\*)(?:\\s*\\|\\s*(?:ADD|RM|\\*))*)(?:\\s+|\\t+)"
							// Traffic
							"((?:IN|OUT|\\*)(?:\\s*\\|\\s*(?:IN|OUT|\\*))*)(?:\\s+|\\t+)"
							// Server
							"((?:\\!?[\\w\\.\\-\\@\\:\\!\\/]+|\\*)(?:\\s*\\|\\s*(?:\\!?[\\w\\.\\-\\@\\:\\!\\/]+|\\*))*)(?:\\s+|\\t+)"
							// Method
							"(\\!?(?:OPTIONS|GET|HEAD|POST|PUT|PATCH|DELETE|TRACE|CONNECT|\\*)(?:\\s*\\|\\s*\\!?(?:OPTIONS|GET|HEAD|POST|PUT|PATCH|DELETE|TRACE|CONNECT|\\*))*)(?:\\s+|\\t+)"
							// Path
							"((?:\\!?\\/[\\w\\-\\_]*(?:\\/[\\w\\-\\_]*)*|\\*)(?:\\s*\\|\\s*(?:\\!?\\/[\\w\\-\\_]*(?:\\/[\\w\\-\\_]*)*|\\*))*)(?:\\s+|\\t+)"
							// Query
							"((?:\\!?\\?\\w+\\=[^\\s\\r\\n\\t]+|\\*)(?:\\s*\\|\\s*(?:\\!?\\?\\w+\\=[^\\s\\r\\n\\t]+|\\*))*)(?:\\s+|\\t+)"
							// Agent
							"([^\\s\\r\\n\\t]+|\\*)(?:\\s+|\\t+)"
							// User
							"((?:\\!?[\\w\\.\\-\\@\\:\\!\\/]+|\\*)(?:\\s*\\|\\s*(?:\\!?[\\w\\.\\-\\@\\:\\!\\/]+|\\*))*)(?:\\s+|\\t+)"
							// Group
							"((?:\\!?[\\w\\.\\-\\@\\:\\!\\/]+|\\*)(?:\\s*\\|\\s*(?:\\!?[\\w\\.\\-\\@\\:\\!\\/]+|\\*))*)(?:\\s+|\\t+)"
							// Headers
							"([^\\r\\n\\t]+)$",
							regex::ECMAScript | regex::icase
						);
						// Выполняем извлечение данных
						regex_search(filedata, match, e);
						// Если данные найдены
						if(!match.empty()){
							// Получаем блок данных экшенов
							auto raw_actions = Anyks::split(match[1].str(), "|");
							// Получаем блок данных направления трафика
							auto raw_traffic = Anyks::split(match[2].str(), "|");
							// Получаем блок данных серверов
							auto raw_servers = Anyks::split(match[3].str(), "|");
							// Получаем блок данных методов
							auto raw_methods = Anyks::split(match[4].str(), "|");
							// Получаем блок данных путей
							auto raw_paths = Anyks::split(match[5].str(), "|");
							// Получаем блок данных запросов
							auto raw_queries = Anyks::split(match[6].str(), "|");
							// Получаем блок данных агента
							auto raw_agent = match[7].str();
							// Получаем блок данных пользователей
							auto raw_users = Anyks::split(match[8].str(), "|");
							// Получаем блок данных групп
							auto raw_groups = Anyks::split(match[9].str(), "|");
							// Получаем блок данных заголовков
							auto raw_headers = Anyks::split(match[10].str(), "|");
							/* Начинаем извлечение данных */
							// Получаем список всех групп
							auto data_groups = this->groups->getAllGroups();
							// Если группы существуют
							if(!data_groups.empty()){
								// Список клиентов
								vector <Clients> clients;
								// Список серверов
								vector <Servers> servers;
								// Список заголовков
								vector <string> headers;
								// Заполняем данные заголовков
								headers.assign(raw_headers.cbegin(), raw_headers.cend());
								// Переходим по списку пользователей
								for(auto it = raw_users.cbegin(); it != raw_users.cend(); ++it){
									// Извлекаем строку с типом
									string str = * it;
									// Объект клиента
									Clients client;
									// Если это не звездочка
									if(str.compare("*") != 0){
										// Определяем тип записи
										const u_int type = Anyks::getTypeAmingByString(* it);
										// Определяем что нужно выполнить для данного типа
										switch(type){
											// Запоминаем mac адрес
											case AMING_MAC: client.mac = str; break;
											// Если это ip адрес версии протокола 4
											case AMING_IPV4: client.ip4 = str; break;
											// Если это ip адрес версии протокола 6
											case AMING_IPV6: client.ip6 = str; break;
											// Если это сеть
											case AMING_NETWORK: {
												// Создаем объект сети
												Network nwk;
												// Получаем данные сети
												string ipNwk = nwk.getIPByNetwork(str);
												// Получаем версию протокола
												u_int version = nwk.checkNetworkByIp(ipNwk);
												// Запоминаем данные префикса
												client.prefix = nwk.getPrefixByNetwork(str);
												// Проверяем тип ip адреса
												switch(version){
													case 4: client.ip4 = ipNwk; break;
													case 6: client.ip6 = ipNwk; break;
												}
											}
										}
									// Если это звездочка
									} else {

									}
								}



							}
						}
					}
				}
				// Закрываем файл
				file.close();
			// Выводим сообщение об ошибке
			} else if(this->log) this->log->write(LOG_ERROR, 0, "headers file (%s) is cannot open", filename.c_str());
		// Выводим сообщение в лог
		} else if(!filename.empty() && this->log){
			// Выводим сообщение в лог, что файл не найден
			this->log->write(LOG_WARNING, 0, "headers file (%s) is not found", filename.c_str());
		}
	}
}
/**
 * checkAvailable Метод проверки на существование параметров заголовков
 * @param  name название файла с параметрами
 * @return      результат проверки
 */
const bool Headers2::checkAvailable(const string name){
	// Результат проверки
	bool result = false;
	// Если название существует
	if(!name.empty()){
		// Получаем данные каталога где хранится файл с правилами
		const string dir = this->config->proxy.dir;
		// Получаем имя файла
		const string filename = Anyks::addToPath(dir, name + ".headers");
		// Проверяем существуют ли параметры заголовков
		if(Anyks::isDirExist(dir.c_str()) && Anyks::isFileExist(filename.c_str())) result = true;
	}
	// Выводим результат
	return result;
}
/**
 * getName Метод получения имени конфига
 */
const string Headers2::getName(){
	// Результат
	string result;
	// Переходим по всему объекту имен
	for(auto it = this->names.cbegin(); it != this->names.cend(); ++it){
		// Проверяем существует ли файл конфигурации
		if(checkAvailable(* it)){
			// Запоминаем результат
			result = * it;
			// Выходим из цикла
			break;
		}
	}
	// Выводим результат
	return result;
}
/**
 * read Метод чтения из параметров
 */
void Headers2::read(){
	// Создаем текущее время генерации
	time_t curUpdate = time(nullptr);
	// Если время ожидания уже вышло, выполняем обновление данных
	if((this->lastUpdate + this->maxUpdate) < curUpdate){
		// Запоминаем текущее время
		this->lastUpdate = curUpdate;
		// Очищаем блок данных
		this->rules.clear();
		// Определяем тип работы
		switch(this->typeSearch){
			// Считываем данные из файлов
			case 0: readFromFile(); break;
			// Считываем данные из LDAP
			case 1: readFromLDAP(); break;
			// Считываем данные из файлов и LDAP
			case 2: {
				readFromFile();
				readFromLDAP();
			} break;
		}
	}
}
/**
 * clear Метод очистки данных
 */
void Headers2::clear(){
	// Очищаем данные правил
	this->rules.clear();
	this->names.clear();
}
/**
 * addName Метод добавления нового имени конфига
 * @param name название файла с параметрами
 */
void Headers2::addName(const string name){
	// Если название существует
	if(!name.empty()) this->names.push_front(name);
}
/**
 * Headers Конструктор
 * @param config конфигурационные данные
 * @param log    объект лога для вывода информации
 */
Headers2::Headers2(Config * config, LogApp * log, Groups * groups){
	// Если конфигурационные данные переданы
	if(config && groups){
		// Очищаем все параметры
		clear();
		// Запоминаем объект логов
		this->log = log;
		// Запоминаем параметры конфига
		this->config = config;
		// Запоминаем параметры групп
		this->groups = groups;
		// Запоминаем название конфигурационного файла
		this->names.push_front(this->config->proxy.name);
		// Запоминаем тип поиска параметров заголовков
		this->typeSearch = 0;
		// Запоминаем время в течение которого запрещено обновлять данные
		this->maxUpdate = 600;
		// Выполняем чтение файла конфигурации
		read();
	}
}
