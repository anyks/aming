/* МОДУЛЬ АВТОРИЗАЦИИ ПРОКСИ AMING */
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
 * checkLdap Метод проверки корректности пароля c помощью LDAP сервера
 * @param username имя пользователя
 * @param password пароль пользователя
 * @param type     тип кодировки пароля передаваемый в http запросе
 */
const bool AUsers::Auth::checkLdap(const uid_t uid, const string password, const u_short type){
	// Результат проверки
	bool result = false;
	// Если данные переданы
	if((uid > 0) && !password.empty()){

	}
	// Выводим результат
	return result;
}
/**
 * checkPam Метод проверки корректности пароля c помощью операционной системы
 * @param username имя пользователя
 * @param password пароль пользователя
 * @param type     тип кодировки пароля передаваемый в http запросе
 */
const bool AUsers::Auth::checkPam(const uid_t uid, const string password, const u_short type){
	// Результат проверки
	bool result = false;
	// Если данные переданы
	if((uid > 0) && !password.empty()){

	}
	// Выводим результат
	return result;
}
/**
 * checkPam Метод проверки корректности пароля c помощью конфигурационного файла
 * @param username имя пользователя
 * @param password пароль пользователя
 * @param type     тип кодировки пароля передаваемый в http запросе
 */
const bool AUsers::Auth::checkFile(const uid_t uid, const string password, const u_short type){
	// Результат проверки
	bool result = false;
	// Если данные переданы
	if((uid > 0) && !password.empty()){

	}
	// Выводим результат
	return result;
}
/**
 * check Метод проверки корректности пароля
 * @param username имя пользователя
 * @param password пароль пользователя
 * @param type     тип кодировки пароля передаваемый в http запросе
 */
const bool AUsers::Auth::check(const string username, const string password, const u_short type){
	// Результат проверки
	bool result = false;
	// Если данные переданы
	if(!username.empty() && !password.empty()){

	}
	// Выводим результат
	return result;
}
/**
 * Auth Конструктор
 * @param config конфигурационные данные
 * @param log    объект лога для вывода информации
 * @param groups объект групп пользователей
 * @param users  объект пользователей
 */
AUsers::Auth::Auth(Config * config, LogApp * log, void * groups, void * users){
	// Если конфигурационные данные переданы
	if((config != nullptr) && (groups != nullptr) && (users != nullptr)){
		// Запоминаем данные логов
		this->log = log;
		// Запоминаем конфигурационные данные
		this->config = config;
		// Запоминаем данные групп
		this->groups = groups;
		// Запоминаем данные пользователей
		this->users = users;
	}
}
