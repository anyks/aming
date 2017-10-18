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
 * conversation Функция проверки валидности pam авторизации
 * @param num_msg     номер сообщения
 * @param msg         сообщение
 * @param resp        ответ системы
 * @param appdata_ptr указатель на объект данных системы
 */
int AUsers::Auth::Pam::conversation(int num_msg, const struct pam_message ** msg, struct pam_response ** resp, void * appdata_ptr){
	// Запоминаем объект с введенными данными
	* resp = pamReply;
	// Выводим результат
	return PAM_SUCCESS;
}
/**
 * checkLdap Метод проверки корректности пароля c помощью LDAP сервера
 * @param username имя пользователя
 * @param password пароль пользователя
 */
const bool AUsers::Auth::checkLdap(const uid_t uid, const string password){
	// Результат проверки
	bool result = false;
	// Если данные переданы
	if((uid > 0) && !password.empty() && (this->users != nullptr)){
		// Получаем имя пользователя
		const string login = (reinterpret_cast <Users *> (this->users))->getNameById(uid);
		// Если логин найден
		if(!login.empty()){
			// Заменяем ключ на логин пользователя
			const string dn = (this->ldap.keyUser + string("=") + Anyks::toCase(login) + string(",") + this->ldap.dnUser);
			// Создаем объект подключения LDAP
			ALDAP ldap(this->config, this->log);
			// Выполняем проверку авторизации
			result = ldap.checkAuth(dn, password, this->ldap.scopeUser, this->ldap.filterUser);
			// Если пароль не соответствует то проверяем соответствует ли он паролям группы
			if(!result && (this->groups != nullptr)){
				// Получаем объект групп
				Groups * groups = reinterpret_cast <Groups *> (this->groups);
				// Если группы получены
				if(groups != nullptr){
					// Получаем список групп в которых состоит пользователь
					auto gids = groups->getGroupIdByUser(uid);
					// Если список групп получен
					if(!gids.empty()){
						// Переходим по всему списку групп
						for(auto gid = gids.cbegin(); gid != gids.cend(); ++gid){
							// Получаем данные группы
							auto * group = groups->getDataById(* gid);
							// Если данные группы найдены
							if(group != nullptr){
								// Заменяем ключ на логин пользователя
								const string dn = (this->ldap.keyGroup + string("=") + Anyks::toCase(group->name) + string(",") + this->ldap.dnGroup);
								// Выполняем проверку корректности пароля
								result = ldap.checkAuth(dn, password, this->ldap.scopeGroup, this->ldap.filterGroup);
								// Если пароль корректный то выходим из цикла
								if(result) break;
							}
						}
					}
				}
			}
		}
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
			/**
			 * checkPassword Функция проверки паролей
			 * @param password1 Пароль из базы пользователей (зашифрованный)
			 * @param password2 Пароль для проверки (не зашифрованный)
			 */
			auto checkPassword = [](const string password1, const string password2){
				// Результат проверки
				bool result = false;
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
				regex_search(password1, match, e);
				// Если данные найдены
				if(!match.empty()){
					// Если это чистый пароль
					if(!match[1].str().empty()) result = (password2.compare(match[2].str()) == 0);
					// Если это md5
					else if(!match[3].str().empty()) result = (Anyks::md5(password2).compare(match[4].str()) == 0);
					// Если это sha1
					else if(!match[5].str().empty()) result = (Anyks::sha1(password2).compare(match[6].str()) == 0);
					// Если это sha256
					else if(!match[7].str().empty()) result = (Anyks::sha256(password2).compare(match[8].str()) == 0);
					// Если это sha512
					else if(!match[9].str().empty()) result = (Anyks::sha512(password2).compare(match[10].str()) == 0);
				}
				// Выводим результат
				return result;
			};
			// Выполняем проверку корректности пароля
			result = checkPassword(user->pass, user->name + password);
			// Если пароль не соответствует то проверяем соответствует ли он паролям группы
			if(!result && (this->groups != nullptr)){
				// Получаем объект групп
				Groups * groups = reinterpret_cast <Groups *> (this->groups);
				// Если группы получены
				if(groups != nullptr){
					// Получаем список групп в которых состоит пользователь
					auto gids = groups->getGroupIdByUser(user->id);
					// Если список групп получен
					if(!gids.empty()){
						// Переходим по всему списку групп
						for(auto gid = gids.cbegin(); gid != gids.cend(); ++gid){
							// Получаем данные группы
							auto * group = groups->getDataById(* gid);
							// Если данные группы найдены
							if(group != nullptr){
								// Выполняем проверку корректности пароля
								result = checkPassword(group->pass, group->name + password);
								// Если пароль корректный то выходим из цикла
								if(result) break;
							}
						}
					}
				}
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
	if(!username.empty() && !password.empty() && (this->users != nullptr) && this->config->auth.enabled){
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
	// Если авторизация не требуется тогда сообщаем что все удачно
	} else if(!this->config->auth.enabled) result = true;
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
		// Устанавливаем параметры подключения LDAP
		this->ldap = {
			"uid",
			"cn",
			"ou=users,dc=agro24,dc=dev",
			"ou=groups,dc=agro24,dc=dev",
			"one",
			"one",
			"(&(!(agro24CoJpDismissed=TRUE))(objectClass=inetOrgPerson))",
			"(objectClass=posixGroup)"
		};
	}
}
