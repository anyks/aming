/* МОДУЛЬ УПРАВЛЕНИЯ ПОЛЬЗОВАТЕЛЯМИ И ГРУППАМИ ПОЛЬЗОВАТЕЛЕЙ ПРОКСИ AMING */
/*
*	автор:				Юрий Николаевич Лобарев
*	skype:				efrantick
*	телефон:			+7(920)672-33-22
*	авторские права:	Все права принадлежат автору © Юрий Лобарев, 2017
*/

#include "ausers/ausers.h"

// Устанавливаем область видимости
using namespace std;

/**
 * getPasswordsFromFile Метод получения данных паролей из файла для всех пользователей
 * @param path  путь где расположен файл с паролями
 * @param log   объект ведения логов
 * @param object объект пользователей или групп
 * @param flag   флаг обозначающий тип объекта
 */
void AUsers::getPasswordsFromFile(const string path, LogApp * log, void * object, const u_short flag){
	// Проверяем входящие данные
	if(!path.empty() && (object != nullptr)
	&& ((flag == AMING_USER) || (flag == AMING_GROUP))){
		// Если пароль является адресом файла
		if(Anyks::getTypeAmingByString(path) == AMING_ADDRESS){
			// Проверяем существует ли такой файл
			if(Anyks::isFileExist(path.c_str())){
				// Открываем файл на чтение
				ifstream file(path.c_str());
				// Если файл открыт
				if(file.is_open()){
					/**
					 * readData Функция считывающая из файла паролей, пароли и присваивает их
					 * @param object объект групп или пользователей
					 * @param data   список данных групп или пользователей
					 * @param flag   флаг указывающий, с чем конкретно мы сейчас имеем дело
					 */
					auto readData = [&file](void * object, void * data, const u_short flag){
						// Строка чтения из файла
						string filedata;
						// Считываем до тех пор пока все удачно
						while(file.good()){
							// Считываем строку из файла
							getline(file, filedata);
							// Если строка существует
							if(!filedata.empty()){
								// Результат работы регулярного выражения
								smatch match;
								// Создаем регулярное выражение
								regex e("^([A-Za-z]+|\\d+)\\:((?:CL|MD5|SHA1|SHA256|SHA512)\\:.{3,128})$", regex::ECMAScript | regex::icase);
								// Выполняем извлечение данных
								regex_search(filedata, match, e);
								// Если данные найдены
								if(!match.empty()){
									// Получаем имя пользователя или группы
									const string subject = Anyks::toCase(match[1].str());
									// Определяем тип объекта для работы
									switch(flag){
										// Если это группы
										case AMING_GROUP: {
											// Получаем данные групп
											auto * groups = reinterpret_cast <const vector <const DataGroup *> *> (data);
											// Получаем идентификатор группы
											const gid_t gid = (Anyks::isNumber(subject) ? ::atoi(subject.c_str()) : -1);
											// Переходим по всему списку групп
											for(auto it = groups->cbegin(); it != groups->cend(); ++it){
												// Если тип пользователя работа с файлами
												if(((* it)->type == 0) && (((gid > -1)
												&& ((* it)->id == gid))
												|| (subject.compare(Anyks::toCase((* it)->name)) == 0))) (reinterpret_cast <Groups *> (object))->setPassword(gid, match[2].str());
											}
										} break;
										// Если это пользователи
										case AMING_USER: {
											// Получаем данные пользователей
											auto * users = reinterpret_cast <const vector <const DataUser *> *> (data);
											// Получаем идентификатор пользователя
											const uid_t uid = (Anyks::isNumber(subject) ? ::atoi(subject.c_str()) : -1);
											// Переходим по всему списку пользователей
											for(auto it = users->cbegin(); it != users->cend(); ++it){
												// Если тип пользователя работа с файлами
												if(((* it)->type == 0) && (((uid > -1)
												&& ((* it)->id == uid))
												|| (subject.compare(Anyks::toCase((* it)->name)) == 0))) (reinterpret_cast <Users *> (object))->setPassword(uid, match[2].str());
											}
										} break;
									}
								}
							}
						}
						// Выводим результат
						return true;
					};
					// Определяем тип объекта для работы
					switch(flag){
						// Если это группы
						case AMING_GROUP: {
							// Получаем данные групп
							auto data = (reinterpret_cast <Groups *> (object))->getAllGroups();
							// Выполняем чтение данных
							readData(object, &data, flag);
						} break;
						// Если это пользователи
						case AMING_USER: {
							// Получаем данные пользователей
							auto data = (reinterpret_cast <Users *> (object))->getAllUsers();
							// Выполняем чтение данных
							readData(object, &data, flag);
						} break;
					}
					// Закрываем файл
					file.close();
				// Выводим сообщение об ошибке
				} else if(log != nullptr) log->write(LOG_ERROR, 0, "password file (%s) is cannot open", path.c_str());
			// Выводим сообщение что файл не существует
			} else if(log != nullptr) log->write(LOG_ERROR, 0, "password file (%s) does not exist", path.c_str());
		}
	}
}
/**
 * getPasswordFromFile Метод получения данных паролей из файла
 * @param path путь где расположен файл с паролями
 * @param log  объект ведения логов
 * @param uid  идентификатор пользователя
 * @param name название пользователя
 */
const string AUsers::getPasswordFromFile(const string path, LogApp * log, const uid_t uid, const string name){
	// Результат работы
	string result = path;
	// Проверяем входящие данные
	if(((uid > 0) || !name.empty()) && !path.empty()){
		// Если пароль является адресом файла
		if(Anyks::getTypeAmingByString(path) == AMING_ADDRESS){
			// Проверяем существует ли такой файл
			if(Anyks::isFileExist(path.c_str())){
				// Строка чтения из файла
				string filedata;
				// Открываем файл на чтение
				ifstream file(path.c_str());
				// Если файл открыт
				if(file.is_open()){
					// Считываем до тех пор пока все удачно
					while(file.good()){
						// Считываем строку из файла
						getline(file, filedata);
						// Если строка существует
						if(!filedata.empty()){
							// Результат работы регулярного выражения
							smatch match;
							// Создаем регулярное выражение
							regex e("^([A-Za-z]+|\\d+)\\:((?:CL|MD5|SHA1|SHA256|SHA512)\\:.{3,128})$", regex::ECMAScript | regex::icase);
							// Выполняем извлечение данных
							regex_search(filedata, match, e);
							// Если данные найдены
							if(!match.empty()){
								// Получаем имя пользователя
								const string user = Anyks::toCase(match[1].str());
								// Если пользователь соответствует то выводим пароль
								if((Anyks::isNumber(user) && (uid_t(::atoi(user.c_str())) == uid))
								|| (Anyks::toCase(name).compare(user) == 0)){
									// Запоминаем пароль пользователя
									result = match[2].str();
									// Выходим из цикла
									break;
								}
							}
						}
					}
					// Закрываем файл
					file.close();
				// Выводим сообщение об ошибке
				} else if(log != nullptr) log->write(LOG_ERROR, 0, "password file (%s) is cannot open", path.c_str());
			// Выводим сообщение что файл не существует
			} else if(log != nullptr) log->write(LOG_ERROR, 0, "password file (%s) does not exist", path.c_str());
		}
	}
	// Выводим путь так как он пришел (если это пароль)
	return result;
}
/**
 * getUser Метод получения данных пользователя
 * @param uid идентификатор пользователя
 * @return    объект с зданными пользователя
 */
const AParams::AUser AUsers::getUser(const uid_t uid){
	// Создаем объект данных пользователя
	AParams::AUser result;
	// Если идентификатор пользователя передан верный
	if((uid > 0) && (this->users != nullptr) && (this->groups != nullptr)){
		// Получаем данные пользователя
		auto * user = this->users->getDataById(uid);
		// Если данные пользователя получены
		if(user != nullptr){
			// Запрашиваем группы в которых он состоит
			auto gits = this->groups->getGroupIdByUser(uid);
			// Переходим по всему списку идентификаторов групп
			for(auto it = gits.cbegin(); it != gits.cend(); it++){
				// Запрашиваем данные группы
				auto * group = this->groups->getDataById(* it);
				// Если данные группы получены
				if(group != nullptr){
					// Добавляем группы в список групп пользователя
					result.groups.push_back({group->id, group->name, group->desc, group->pass});
				}
			}
			// Добавляем данные пользователя
			result.user = {user->id, user->name, user->desc, user->pass};
			// Запоминаем остальные параметры
			result.idnt = user->idnt;
			result.ipv4 = user->ipv4;
			result.ipv6 = user->ipv6;
			result.gzip = user->gzip;
			result.proxy = user->proxy;
			result.connects = user->connects;
			result.timeouts = user->timeouts;
			result.buffers = user->buffers;
			result.keepalive = user->keepalive;
		}
	}
	// Выводим результат
	return result;
}
/**
 * getUser Метод получения данных пользователя
 * @param userName имя пользователя
 * @return         объект с зданными пользователя
 */
const AParams::AUser AUsers::getUser(const string userName){
	// Создаем объект данных пользователя
	AParams::AUser result;
	// Если имя пользователя получено
	if(!userName.empty()){
		// Если пользователи найдены
		const uid_t uid = (this->users != nullptr ? this->users->getIdByName(userName) : -1);
		// Если идентификатор найден то запрашиваем данные пользователя
		if(uid > 0) result = getUser(uid);
	}
	// Выводим результат
	return result;
}
/**
 * getAllGroups Метод получения данных всех групп
 * @return      список данных всех групп
 */
const vector <const AUsers::DataGroup *> AUsers::getAllGroups(){
	// Результат получения данных
	vector <const AUsers::DataGroup *> result;
	// Если объект групп существует
	if(this->groups != nullptr) result = this->groups->getAllGroups();
	// Выводим результат
	return result;
}
/**
 * getIdUsersInGroup Метод получения списка пользователей в группе
 * @param  gid идентификатор группы
 * @return     список идентификаторов пользователей
 */
const vector <uid_t> AUsers::getIdUsersInGroup(const gid_t gid){
	// Результат работы
	vector <uid_t> result;
	// Если идентификатор группы передан
	if(gid && (this->groups != nullptr)){
		// Запрашиваем данные
		result = this->groups->getIdUsers(gid);
	}
	// Выводим результат
	return result;
}
/**
 * getIdUsersInGroup Метод получения списка пользователей в группе
 * @param  groupName название группы
 * @return           список идентификаторов пользователей
 */
const vector <uid_t> AUsers::getIdUsersInGroup(const string groupName){
	// Результат работы
	vector <uid_t> result;
	// Если идентификатор группы передан
	if(!groupName.empty() && (this->groups != nullptr)){
		// Запрашиваем данные
		result = this->groups->getIdUsers(groupName);
	}
	// Выводим результат
	return result;
}
/**
 * getNameUsersInGroup Метод получения списка пользователей в группе
 * @param  gid   идентификатор группы
 * @return       список имен пользователей
 */
const vector <string> AUsers::getNameUsersInGroup(const gid_t gid){
	// Результат работы
	vector <string> result;
	// Если идентификатор группы передан
	if(gid && (this->groups != nullptr)){
		// Запрашиваем данные
		result = this->groups->getNameUsers(gid);
	}
	// Выводим результат
	return result;
}
/**
 * getNameUsersInGroup Метод получения списка пользователей в группе
 * @param  groupName название группы
 * @return           список имен пользователей
 */
const vector <string> AUsers::getNameUsersInGroup(const string groupName){
	// Результат работы
	vector <string> result;
	// Если идентификатор группы передан
	if(!groupName.empty() && (this->groups != nullptr)){
		// Запрашиваем данные
		result = this->groups->getNameUsers(groupName);
	}
	// Выводим результат
	return result;
}
/**
 * getGidByName Метод извлечения идентификатора группы по ее имени
 * @param  groupName название группы
 * @return           идентификатор группы
 */
const gid_t AUsers::getGidByName(const string groupName){
	// Результат работы
	gid_t result = -1;
	// Если пользователи найдены
	if(this->groups != nullptr) result = this->groups->getIdByName(groupName);
	// Выводим результат
	return result;
}
/**
 * getUidByName Метод извлечения идентификатора пользователя по его имени
 * @param  userName название пользователя
 * @return          идентификатор пользователя
 */
const uid_t AUsers::getUidByName(const string userName){
	// Результат работы
	uid_t result = -1;
	// Если пользователи найдены
	if(this->users != nullptr) result = this->users->getIdByName(userName);
	// Выводим результат
	return result;
}
/**
 * checkUserInGroup Метод проверки принадлежности пользователя к группе
 * @param  gid идентификатор группы
 * @param  uid идентификатор пользователя
 * @return     результат проверки
 */
const bool AUsers::checkUserInGroup(const gid_t gid, const uid_t uid){
	// Результат работы
	bool result = false;
	// Если идентификаторы группы и пользователя переданы
	if(gid && uid && (this->groups != nullptr)){
		// Выполняем проверку
		result = this->groups->checkUser(gid, uid);
	}
	// Выводим результат
	return result;
}
/**
 * checkUserInGroup Метод проверки принадлежности пользователя к группе
 * @param  gid      идентификатор группы
 * @param  userName название пользователя
 * @return          результат проверки
 */
const bool AUsers::checkUserInGroup(const gid_t gid, const string userName){
	// Результат работы
	bool result = false;
	// Если идентификаторы группы и пользователя переданы
	if(gid && !userName.empty() && (this->groups != nullptr)){
		// Выполняем проверку
		result = this->groups->checkUser(gid, userName);
	}
	// Выводим результат
	return result;
}
/**
 * checkUserInGroup Метод проверки принадлежности пользователя к группе
 * @param  groupName название группы
 * @param  uid       идентификатор пользователя
 * @return           результат проверки
 */
const bool AUsers::checkUserInGroup(const string groupName, const uid_t uid){
	// Результат работы
	bool result = false;
	// Если идентификаторы группы и пользователя переданы
	if(uid && !groupName.empty() && (this->groups != nullptr)){
		// Выполняем проверку
		result = this->groups->checkUser(groupName, uid);
	}
	// Выводим результат
	return result;
}
/**
 * checkUserInGroup Метод проверки принадлежности пользователя к группе
 * @param  groupName название группы
 * @param  userName  название пользователя
 * @return           результат проверки
 */
const bool AUsers::checkUserInGroup(const string groupName, const string userName){
	// Результат работы
	bool result = false;
	// Если идентификаторы группы и пользователя переданы
	if(!userName.empty() && !groupName.empty() && (this->groups != nullptr)){
		// Выполняем проверку
		result = this->groups->checkUser(groupName, userName);
	}
	// Выводим результат
	return result;
}
/**
 * checkGroupById Метод проверки на существование группы
 * @param  gid идентификатор группы
 * @return     результат проверки
 */
const bool AUsers::checkGroupById(const gid_t gid){
	// Результат работы
	bool result = false;
	// Если идентификаторы группы и пользователя переданы
	if(gid && (this->groups != nullptr)){
		// Выполняем проверку
		result = this->groups->checkGroupById(gid);
	}
	// Выводим результат
	return result;
}
/**
 * searchUser Метод поиска данных пользователя по данным коннекта
 * @param ip  адрес интернет протокола клиента
 * @param mac аппаратный адрес сетевого интерфейса клиента
 * @return    данные пользователя
 */
const AParams::AUser AUsers::searchUser(const string ip, const string mac){
	// Результат работы
	AParams::AUser result;
	// Если данные ip и мак адреса существуют
	if(!ip.empty() && !mac.empty()){
		// Выполняем поиск пользователя
		auto * user = this->users->getUserByConnect(ip, mac);
		// Если пользователь найден то выводим результат
		if(user != nullptr){
			// Получаем данные пользователя
			result = getUser(user->id);
			// Запоминаем что авторизация прошла успешно
			result.auth = true;
		}
	}
	// Выводим результат
	return result;
}
/**
 * authenticate Метод авторизации пользователя в системе
 * @param login логин пользователя
 * @param pass  пароль пользователя
 */
const AParams::AUser AUsers::authenticate(const string login, const string pass){
	// Результат работы
	AParams::AUser result;
	// Если параметры переданы верные
	if(!login.empty() && !pass.empty() && (this->auth != nullptr)){
		// Проверяем авторизацию пользователя
		if(this->auth->check(login, pass)){
			// Получаем данные пользователя
			result = getUser(login);
			// Запоминаем что авторизация прошла успешно
			result.auth = true;
		}
	}
	// Выводим результат
	return result;
}
/**
 * AUsers Конструктор
 * @param config конфигурационные данные
 * @param log    объект лога для вывода информации
 */
AUsers::AUsers(Config * config, LogApp * log){
	// Если конфигурационные данные переданы
	if(config){
		// Запоминаем данные логов
		this->log = log;
		// Запоминаем конфигурационные данные
		this->config = config;
		// Создаем объект групп
		this->groups = new Groups(this->config, this->log);
		// Создаем объект пользователей
		this->users = new Users(this->config, this->log);
		// Если объекты пользователей и групп созданы
		if((this->users != nullptr) && (this->groups != nullptr)){
			// Добавляем объект пользователей группам
			this->groups->setUsers(this->users);
			// Добавляем объект групп пользователям
			this->users->setGroups(this->groups);
			// Создаем объект авторизации
			this->auth = new Auth(this->config, this->log, this->groups, this->users);
		}
	}
}
/**
 * ~AUsers Деструктор
 */
AUsers::~AUsers(){
	// Очищаем память
	if(this->auth != nullptr)	delete this->auth;
	if(this->users != nullptr)	delete this->users;
	if(this->groups != nullptr)	delete this->groups;
}
