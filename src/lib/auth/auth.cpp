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
 */
const bool AUsers::Auth::checkLdap(const uid_t uid, const string password){
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
 */
const bool AUsers::Auth::checkPam(const uid_t uid, const string password){
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
 */
const bool AUsers::Auth::checkFile(const uid_t uid, const string password){
	// Результат проверки
	bool result = false;
	// Если данные переданы
	if((uid > 0) && !password.empty()){
		// Получаем данные пользователя
		auto * user = (reinterpret_cast <Users *> (this->users))->getDataById(uid);
		// Если пользователь найден
		if(user != nullptr){
			// Результат работы регулярного выражения
			smatch match;
			// CL:bigsecret
			// MD5:ee11cbb19052e40b07aac0ca060c23ee (32)
			// SHA1:12dea96fec20593566ab75692c9949596833adc9 (40)
			// SHA256:04f8996da763b7a969b1028ee3007569eaf3a635486ddab211d512c85b9df8fb (64)
			// SHA512:b14361404c078ffd549c03db443c3fede2f3e534d73f78f77301ed97d4a436a9fd9db05ee8b325c0ad36438b43fec8510c204fc1c1edb21d0941c00e9e2c1ce2 (128)
			// Определяем тип пароля
			regex e("(?:(CL)\\:(.+)|(MD5)\\:(\\w{32})|(SHA1)\\:(\\w{40})|(SHA256)\\:(\\w{64})|(SHA512)\\:(\\w{128}))", regex::ECMAScript | regex::icase);
			// Выполняем проверку типа пароля
			regex_search(user->pass, match, e);
			// Если данные найдены
			if(!match.empty()){
				// Если это чистый пароль
				if(!match[1].str().empty()) result = (password.compare(match[2].str()) == 0);
				// Если это md5
				else if(!match[3].str().empty()) result = (Anyks::md5(password).compare(match[4].str()) == 0);
				// Если это sha1
				else if(!match[5].str().empty()) result = (Anyks::sha1(password).compare(match[6].str()) == 0);
				// Если это sha256
				else if(!match[7].str().empty()) result = (Anyks::sha256(password).compare(match[8].str()) == 0);
				// Если это sha512
				else if(!match[9].str().empty()) result = (Anyks::sha512(password).compare(match[10].str()) == 0);
			}
		}
	}
	// Выводим результат
	return result;
}
/**
 * check Метод проверки корректности пароля
 * @param username имя пользователя
 * @param password пароль пользователя
 */
const bool AUsers::Auth::check(const string username, const string password){
	// Результат проверки
	bool result = false;
	// Если данные переданы
	if(!username.empty() && !password.empty() && (this->users != nullptr)){
		// Получаем данные пользователя
		auto * user = (reinterpret_cast <Users *> (this->users))->getDataByName(username);
		// Если пользователь найден
		if(user != nullptr){
			// Определяем тип пользователя
			switch(user->type){
				// Если это проверка по файлам
				case 0: result = checkFile(user->id, password);	break;
				// Если это проверка операционной системы
				case 1: result = checkPam(user->id, password);	break;
				// Если это проверка LDAP
				case 2: result = checkLdap(user->id, password);	break;
			}
		}		
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
