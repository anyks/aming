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
 * getAllGroups Метод получения данных всех групп
 * @return      список данных всех групп
 */
const vector <const AUsers::DataGroups *> AUsers::getAllGroups(){
	// Результат получения данных
	vector <const AUsers::DataGroups *> result;
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
	if(gid && (this->groups != nullptr) && (this->users != nullptr)){
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
	if(!groupName.empty() && (this->groups != nullptr) && (this->users != nullptr)){
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
		// Добавляем объект групп пользователям
		if(this->users != nullptr) this->users->setGroups(this->groups);
		// Добавляем объект пользователей группам
		if(this->groups != nullptr) this->groups->setUsers(this->users);
	}
}
/**
 * ~AUsers Деструктор
 */
AUsers::~AUsers(){
	// Очищаем память
	if(this->users != nullptr)	delete this->users;
	if(this->groups != nullptr)	delete this->groups;
}
