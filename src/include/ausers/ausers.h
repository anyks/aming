/* ПОЛЬЗОВАТЕЛИ ПРОКСИ AMING */
/*
*	автор:				Юрий Николаевич Лобарев
*	skype:				efrantick
*	телефон:			+7(920)672-33-22
*	авторские права:	Все права принадлежат автору © Юрий Лобарев, 2017
*/
#ifndef _USERS_PROXY_AMING_
#define _USERS_PROXY_AMING_

#include <regex>
#include <string>
#include <random>
#include <vector>
#include <iostream>
#include <fstream>
#include <algorithm>
#include <unordered_map>
#include <grp.h>
#include <pwd.h>
#include <time.h>
#include <stdlib.h>
#include <unistd.h>
#include <sys/types.h>
#include <security/pam_appl.h>
#include "ini/ini.h"
#include "nwk/nwk.h"
#include "system/system.h"
#include "general/general.h"
#include "ldap2/ldap.h"

// Устанавливаем область видимости
using namespace std;

/**
 * AParams Основные схемы параметров модуля
 */
namespace AParams {
	/**
	 * Gzip Структура параметров сжатия данных на уровне прокси сервера
	 */
	struct Gzip {
		bool vary;					// Разрешает или запрещает выдавать в ответе поле заголовка “Vary: Accept-Encoding”
		int level;					// Тип сжатия (default - по умолчанию, best - лучшее сжатие, speed - лучшая скорость, no - без сжатия)
		long length;				// Минимальная длина данных после которых включается сжатие (работает только с Content-Length)
		size_t chunk;				// Максимальный размер чанка в байтах
		string regex;				// Не сжимать контент, UserAgent которого соответсвует регулярному выражению
		vector <string> vhttp;		// Версия http протокола
		vector <string> proxied;	// Разрешает или запрещает сжатие ответа методом gzip для проксированных запросов
		vector <string> types;		// Разрешает сжатие ответа методом gzip для указанных MIME-типов
	};
	/**
	 * IP Структура ip адреса
	 */
	struct IP {
		vector <string> ip;			// Список ip адресов
		vector <string> resolver;	// Список dns серверов
	};
	/**
	 * Proxy Структура параметров самого прокси-сервера
	 */
	struct Proxy {
		bool reverse;		// Обратный прокси (доступ из сети в локальную сеть)
		bool transfer;		// Активация поддержки прокси листа
		bool forward;		// Прямой прокси (доступ во внешнюю сеть)
		bool subnet;		// Активация режима мульти-сетевого взаимодействия когда выход на сеть определяется по ip адресу и в зависимости от него выбирается ipv6 или ipv4 сеть
		bool pipelining;	// Активация конвеерной обработки
	};
	/**
	 * Connects Структура контроля подключений клиента к серверу
	 */
	struct Connects {
		size_t size;	// Максимальный размер скачиваемых данных в байтах
		u_int connect;	// Максимальное количество подключений (одного клиента к прокси серверу)
	};
	/**
	 * Timeout Структура таймаутов
	 */
	struct Timeouts {
		size_t read;	// Таймаут на чтение данных
		size_t write;	// Таймаут на запись данных
		size_t upgrade;	// Таймаут на работу в режиме переключения протоколов
	};
	/**
	 * BufferSize Структура размеров буфера
	 */
	struct BufferSize {
		long read;		// Буфер на чтение
		long write;		// Буфер на запись
	};
	/**
	 * Keepalive Структура параметров постоянного подключения
	 */
	struct Keepalive {
		int keepcnt;	// Максимальное количество попыток
		int keepidle;	// Интервал времени в секундах через которое происходит проверка подключения
		int keepintvl;	// Интервал времени в секундах между попытками
	};
	/**
	 * Group Структура данных группы
	 */
	struct Group {
		gid_t gid;		// Идентификатор группы
		string name;	// Название группы
		string desc;	// Описание группы
		string pass;	// Пароль группы
	};
	/**
	 * User Структура данных пользователя
	 */
	struct User {
		uid_t uid;		// Идентификатор пользователя
		string name;	// Имя пользователя
		string desc;	// Описание пользователя
		string pass;	// Пароль пользователя
	};
	/**
	 * AUser Структура данных пользователя прокси сервера AMING
	 */
	struct AUser {
		bool auth = false;		// Результат авторизации клиента
		vector <string> idnt;	// Список ip4/ip6/mac адресов
		vector <Group> groups;	// Данные группы
		User user;				// Данные пользователя
		IP ipv4;				// Блок ip адресов для протокола версии 4
		IP ipv6;				// Блок ip адресов для протокола версии 6
		Gzip gzip;				// Параметры gzip
		Proxy proxy;			// Параметры самого прокси-сервера
		Connects connects;		// Контроль подключений клиента к серверу
		Timeouts timeouts;		// Таймауты подключений
		BufferSize buffers;		// Размеры буферов передачи данных
		Keepalive keepalive;	// Постоянное подключение
	};
	/**
	 * Client Структура клиента подключившегося к прокси серверу
	 */
	struct Client {
		const string ip;				// IP адрес клиента
		const string mac;				// MAC адрес клиента
		const string sip;				// IP адрес удаленного сервера куда подключается клиент
		const AUser * user = nullptr;	// Указатель на объект данных пользователя (если он авторизован)
	};
};

/**
 * AUsers Класс пользователей AMING
 */
class AUsers {
	private:
		/**
		 * DataGroup Структура данных группы
		 */
		struct DataGroup {
			gid_t id;						// Идентификатор группы
			u_short options;				// Основные параметры прокси
			u_short type;					// Тип группы (0 - файлы, 1 - PAM, 2 - LDAP)
			string name;					// Название группы
			string pass;					// Пароль группы
			string auth;					// Тип авторизации клиентов
			string desc;					// Описание групп
			vector <string> idnt;			// Список ip4/ip6/nwk/mac адресов
			AParams::IP ipv4;				// Блок ip адресов для протокола версии 4
			AParams::IP ipv6;				// Блок ip адресов для протокола версии 6
			AParams::Gzip gzip;				// Параметры gzip
			AParams::Proxy proxy;			// Параметры самого прокси-сервера
			AParams::Connects connects;		// Контроль подключений клиента к серверу
			AParams::Timeouts timeouts;		// Таймауты подключений
			AParams::BufferSize buffers;	// Размеры буферов передачи данных
			AParams::Keepalive keepalive;	// Постоянное подключение
			vector <uid_t> users;	// Список идентификаторов пользователей
		};
		/**
		 * DataUser Структура данных пользователя
		 */
		struct DataUser {
			uid_t id;						// Идентификатор пользователя
			u_short options;				// Основные параметры прокси
			u_short type;					// Тип пользователя (0 - файлы, 1 - PAM, 2 - LDAP)
			string name;					// Название пользователя
			string pass;					// Пароль пользователя
			string auth;					// Тип авторизации клиентов
			string desc;					// Описание пользователя
			vector <string> idnt;			// Список ip4/ip6/mac адресов
			AParams::IP ipv4;				// Блок ip адресов для протокола версии 4
			AParams::IP ipv6;				// Блок ip адресов для протокола версии 6
			AParams::Gzip gzip;				// Параметры gzip
			AParams::Proxy proxy;			// Параметры самого прокси-сервера
			AParams::Connects connects;		// Контроль подключений клиента к серверу
			AParams::Timeouts timeouts;		// Таймауты подключений
			AParams::BufferSize buffers;	// Размеры буферов передачи данных
			AParams::Keepalive keepalive;	// Постоянное подключение
		};
		/**
		 * getPasswordsFromFile Метод получения данных паролей из файла для всех пользователей
		 * @param path   путь где расположен файл с паролями
		 * @param log    объект ведения логов
		 * @param object объект пользователей или групп
		 * @param flag   флаг обозначающий тип объекта
		 */
		static void getPasswordsFromFile(const string path, LogApp * log = nullptr, void * object = nullptr, const u_short flag = AMING_USER);
		/**
		 * getPasswordFromFile Метод получения данных паролей из файла
		 * @param path путь где расположен файл с паролями
		 * @param log  объект ведения логов
		 * @param uid  идентификатор пользователя
		 * @param name название пользователя
		 * @return     пароль из файла
		 */
		static const string getPasswordFromFile(const string path, LogApp * log = nullptr, const uid_t uid = -1, const string name = "");
		/**
		 * Groups Класс групп пользователей
		 */
		class Groups {
			private:
				/**
				 * Ldap Структура ldap
				 */
				struct Ldap {
					string dnGroup;
					string dnUser;
					string dnConfig;
					string scopeGroup;
					string scopeUser;
					string scopeConfig;
					string filterGroup;
					string filterUser;
					string filterConfig;
				};
				// Время в течение которого обновлять нельзя
				time_t maxUpdate = 0;
				// Время последнего обновления данных
				time_t lastUpdate = 0;
				// Тип поиска групп (0 - Из файла, 1 - из PAM, 2 - из LDAP)
				u_short typeSearch = 0;
				// Тип поиска конфигурационных данных (0 - Из файла, 1 - из LDAP)
				u_short typeConfigs = 0;
				// Максимальное количество групп пользователя для PAM
				int maxPamGroupsUser = 0;
				// Объект ldap подклчюения
				Ldap ldap;
				// Объект лога
				LogApp * log = nullptr;
				// Конфигурационные данные
				Config * config = nullptr;
				// Объект пользователей
				void * users = nullptr;
				// Список групп
				unordered_map <gid_t, DataGroup> data;
				/**
				 * setProxyOptions Функция добавления опций прокси
				 * @param option       опция для добавления
				 * @param proxyOptions список существующих опций
				 * @param flag         флаг добавления или удаления опции
				 */
				void setProxyOptions(const u_short option, u_short &proxyOptions, const bool flag = false);
				/**
				 * setDataGroupFromLdap Метод заполнения данных группы из LDAP
				 * @param group объект группы
				 */
				void setDataGroupFromLdap(DataGroup &group);
				/**
				 * setDataGroupFromFile Метод заполнения данных группы из конфигурационного файла
				 * @param group объект группы
				 * @param ini   указатель на объект конфигурации
				 */
				void setDataGroupFromFile(DataGroup &group, INI * ini = nullptr);
				/**
				 * setDataGroup Метод заполнения данных группы
				 * @param group объект группы
				 * @param ini   указатель на объект конфигурации
				 */
				void setDataGroup(DataGroup &group, INI * ini = nullptr);
				/**
				 * createDefaultData Метод создания группы с параметрами по умолчанию
				 * @param  id   идентификатор групыы
				 * @param  name название группы
				 * @return      созданная группа
				 */
				const DataGroup createDefaultData(const gid_t id, const string name);
				/**
				 * readGroupsFromLdap Метод чтения данных групп из LDAP сервера
				 * @return результат операции
				 */
				const bool readGroupsFromLdap();
				/**
				 * readGroupsFromPam Метод чтения данных групп из операционной системы
				 * @return результат операции
				 */
				const bool readGroupsFromPam();
				/**
				 * readGroupsFromFile Метод чтения данных групп из файла
				 * @return результат операции
				 */
				const bool readGroupsFromFile();
				/**
				 * update Метод обновления групп
				 * @return статус обновления
				 */
				const bool update();
			public:
				/**
				 * getAllGroups Метод получения данных всех групп
				 * @return      список данных всех групп
				 */
				const vector <const DataGroup *> getAllGroups();
				/**
				 * getDataById Метод получения данные группы по идентификатору группы
				 * @param  gid идентификатор группы
				 * @return     данные группы
				 */
				const DataGroup * getDataById(const gid_t gid);
				/**
				 * getDataByName Метод получения данные группы по имени группы
				 * @param  groupName название группы
				 * @return           данные группы
				 */
				const DataGroup * getDataByName(const string groupName);
				/**
				 * getGroupIdByUser Метод получения идентификатор группы по идентификатору пользователя
				 * @param  uid идентификатор пользователя
				 * @return     идентификатор группы
				 */
				const vector <gid_t> getGroupIdByUser(const uid_t uid);
				/**
				 * getGroupIdByUser Метод получения идентификатор группы по имени пользователя
				 * @param  userName название пользователя
				 * @return          идентификатор группы
				 */
				const vector <gid_t> getGroupIdByUser(const string userName);
				/**
				 * getGroupNameByUser Метод получения название группы по идентификатору пользователя
				 * @param  uid идентификатор пользователя
				 * @return     название группы
				 */
				const vector <string> getGroupNameByUser(const uid_t uid);
				/**
				 * getGroupNameByUser Метод получения название группы по имени пользователя
				 * @param  userName название пользователя
				 * @return          название группы
				 */
				const vector <string> getGroupNameByUser(const string userName);
				/**
				 * checkUser Метод проверки принадлежности пользователя к группе
				 * @param  gid идентификатор группы
				 * @param  uid идентификатор пользователя
				 * @return     результат проверки
				 */
				const bool checkUser(const gid_t gid, const uid_t uid);
				/**
				 * checkUser Метод проверки принадлежности пользователя к группе
				 * @param  gid      идентификатор группы
				 * @param  userName название пользователя
				 * @return          результат проверки
				 */
				const bool checkUser(const gid_t gid, const string userName);
				/**
				 * checkUser Метод проверки принадлежности пользователя к группе
				 * @param  groupName название группы
				 * @param  uid       идентификатор пользователя
				 * @return           результат проверки
				 */
				const bool checkUser(const string groupName, const uid_t uid);
				/**
				 * checkUser Метод проверки принадлежности пользователя к группе
				 * @param  groupName название группы
				 * @param  userName  название пользователя
				 * @return           результат проверки
				 */
				const bool checkUser(const string groupName, const string userName);
				/**
				 * checkGroupById Метод проверки на существование группы
				 * @param  gid идентификатор группы
				 * @return     результат проверки
				 */
				const bool checkGroupById(const gid_t gid);
				/**
				 * checkGroupByName Метод проверки на существование группы
				 * @param  groupName название группы
				 * @return           результат проверки
				 */
				const bool checkGroupByName(const string groupName);
				/**
				 * getUidByUserName Метод извлечения идентификатора пользователя по его имени
				 * @param  userName название пользователя
				 * @return          идентификатор пользователя
				 */
				const uid_t getUidByUserName(const string userName);
				/**
				 * getIdByName Метод извлечения идентификатора группы по ее имени
				 * @param  groupName название группы
				 * @return           идентификатор группы
				 */
				const gid_t getIdByName(const string groupName);
				/**
				 * getUserNameByUid Метод извлечения имени пользователя по его идентификатору
				 * @param  uid идентификатор пользователя
				 * @return     название пользователя
				 */
				const string getUserNameByUid(const uid_t uid);
				/**
				 * getNameById Метод извлечения имени группы по ее идентификатору
				 * @param  gid идентификатор группы
				 * @return     название группы
				 */
				const string getNameById(const gid_t gid);
				/**
				 * getNameUsers Метод получения списка пользователей в группе
				 * @param  gid идентификатор группы
				 * @return     список имен пользователей
				 */
				const vector <string> getNameUsers(const gid_t gid);
				/**
				 * getNameUsers Метод получения списка пользователей в группе
				 * @param  groupName название группы
				 * @return           список имен пользователей
				 */
				const vector <string> getNameUsers(const string groupName);
				/**
				 * getIdAllUsers Метод получения списка всех пользователей
				 * @return список идентификаторов пользователей
				 */
				const vector <uid_t> getIdAllUsers();
				/**
				 * getIdUsers Метод получения списка пользователей в группе
				 * @param  gid идентификатор группы
				 * @return     список идентификаторов пользователей
				 */
				const vector <uid_t> getIdUsers(const gid_t gid);
				/**
				 * getIdUsers Метод получения списка пользователей в группе
				 * @param  groupName название группы
				 * @return           список идентификаторов пользователей
				 */
				const vector <uid_t> getIdUsers(const string groupName);
				/**
				 * addUser Метод добавления пользователя
				 * @param  gid идентификатор группы
				 * @param  uid идентификатор пользователя
				 * @return     результат добавления
				 */
				const bool addUser(const gid_t gid, const uid_t uid);
				/**
				 * addUser Метод добавления пользователя
				 * @param  gid       идентификатор группы
				 * @param  userName  название пользователя
				 * @return           результат добавления
				 */
				const bool addUser(const gid_t gid, const string userName);
				/**
				 * addUser Метод добавления пользователя
				 * @param  groupName название группы
				 * @param  uid       идентификатор пользователя
				 * @return           результат добавления
				 */
				const bool addUser(const string groupName, const uid_t uid);
				/**
				 * addUser Метод добавления пользователя
				 * @param  groupName название группы
				 * @param  userName  название пользователя
				 * @return           результат добавления
				 */
				const bool addUser(const string groupName, const string userName);
				/**
				 * addGroup Метод добавления группы
				 * @param  gid  идентификатор группы
				 * @param  name название группы
				 * @return      результат добавления
				 */
				const bool addGroup(const gid_t gid, const string name);
				/**
				 * setPassword Метод установки пароля группы
				 * @param gid      идентификатор группы
				 * @param password пароль группы
				 */
				void setPassword(const gid_t gid, const string password);
				/**
				 * setUsers Метод добавления объекта пользователей
				 * @param users объект пользователей
				 */
				void setUsers(void * users = nullptr);
				/**
				 * Groups Конструктор
				 * @param config конфигурационные данные
				 * @param log    объект лога для вывода информации
				 */
				Groups(Config * config = nullptr, LogApp * log = nullptr);
		};
		/**
		 * Users Класс пользователей
		 */
		class Users {
			private:
				/**
				 * Ldap Структура ldap
				 */
				struct Ldap {
					string dnUser;
					string dnConfig;
					string scopeUser;
					string scopeConfig;
					string filterUser;
					string filterConfig;
				};
				// Время в течение которого обновлять нельзя
				time_t maxUpdate = 0;
				// Время последнего обновления данных
				time_t lastUpdate = 0;
				// Тип поиска пользователей (0 - Из файла, 1 - из PAM, 2 - из LDAP)
				u_short typeSearch = 0;
				// Тип поиска конфигурационных данных (0 - Из файла, 1 - из LDAP)
				u_short typeConfigs = 0;
				// Объект ldap подклчюения
				Ldap ldap;
				// Объект лога
				LogApp * log = nullptr;
				// Конфигурационные данные
				Config * config = nullptr;
				// Объект пользователей
				void * groups = nullptr;
				// Список пользователей
				unordered_map <uid_t, DataUser> data;
				/**
				 * setProxyOptions Функция добавления опций прокси
				 * @param option       опция для добавления
				 * @param proxyOptions список существующих опций
				 * @param flag         флаг добавления или удаления опции
				 */
				void setProxyOptions(const u_short option, u_short &proxyOptions, const bool flag = false);
				/**
				 * setDataUserFromLdap Метод заполнения данных пользователя из LDAP
				 * @param user объект пользователя
				 */
				void setDataUserFromLdap(DataUser &user);
				/**
				 * setDataUserFromFile Метод заполнения данных пользователя из конфигурационного файла
				 * @param user объект пользователя
				 * @param ini  указатель на объект конфигурации
				 */
				void setDataUserFromFile(DataUser &user, INI * ini = nullptr);
				/**
				 * setDataUser Метод заполнения данных пользователя
				 * @param user объект пользователя
				 * @param ini  указатель на объект конфигурации
				 */
				void setDataUser(DataUser &user, INI * ini = nullptr);
				/**
				 * createDefaultData Метод создания пользователя с параметрами по умолчанию
				 * @param  id   идентификатор пользователя
				 * @param  name название пользователя
				 * @return      созданный пользователь
				 */
				const DataUser createDefaultData(const uid_t id, const string name);
				/**
				 * readUsersFromLdap Метод чтения данных пользователей из LDAP сервера
				 * @return результат операции
				 */
				const bool readUsersFromLdap();
				/**
				 * readUsersFromPam Метод чтения данных пользователей из операционной системы
				 * @return результат операции
				 */
				const bool readUsersFromPam();
				/**
				 * readUsersFromFile Метод чтения данных пользователей из файла
				 * @return результат операции
				 */
				const bool readUsersFromFile();
				/**
				 * update Метод обновления пользователей
				 * @return статус обновления
				 */
				const bool update();
			public:
				/**
				 * getAllUsers Метод получения данных всех пользователей
				 * @return     список данных всех пользователей
				 */
				const vector <const DataUser *> getAllUsers();
				/**
				 * getUserByConnect Метод поиска данных пользователя по данным коннекта
				 * @param ip  адрес интернет протокола клиента
				 * @param mac аппаратный адрес сетевого интерфейса клиента
				 * @return    данные пользователя
				 */
				const DataUser * getUserByConnect(const string ip = "", const string mac = "");
				/**
				 * getDataById Метод получения данные пользователя по идентификатору
				 * @param  uid идентификатор пользователя
				 * @return     данные пользователя
				 */
				const DataUser * getDataById(const uid_t uid);
				/**
				 * getDataByName Метод получения данные пользователя по имени
				 * @param  groupName название пользователя
				 * @return           данные пользователя
				 */
				const DataUser * getDataByName(const string userName);
				/**
				 * checkUserById Метод проверки на существование пользователя
				 * @param  uid идентификатор пользователя
				 * @return     результат проверки
				 */
				const bool checkUserById(const uid_t uid);
				/**
				 * checkGroupByName Метод проверки на существование пользователя
				 * @param  userName название пользователя
				 * @return          результат проверки
				 */
				const bool checkUserByName(const string userName);
				/**
				 * getIdByName Метод извлечения идентификатора пользователя по его имени
				 * @param  userName название пользователя
				 * @return          идентификатор пользователя
				 */
				const uid_t getIdByName(const string userName);
				/**
				 * getNameById Метод извлечения имени пользователя по его идентификатору
				 * @param  uid идентификатор пользователя
				 * @return     название пользователя
				 */
				const string getNameById(const uid_t uid);
				/**
				 * getIdAllUsers Метод получения списка всех пользователей
				 * @return список идентификаторов пользователей
				 */
				const vector <uid_t> getIdAllUsers();
				/**
				 * setPassword Метод установки пароля пользователя
				 * @param uid      идентификатор пользователя
				 * @param password пароль пользователя
				 */
				void setPassword(const uid_t uid, const string password);
				/**
				 * setGroups Метод добавления объекта групп
				 * @param groups объект групп
				 */
				void setGroups(void * groups = nullptr);
				/**
				 * Users Конструктор
				 * @param config конфигурационные данные
				 * @param log    объект лога для вывода информации
				 */
				Users(Config * config = nullptr, LogApp * log = nullptr);
		};
		/**
		 * Auth Класс авторизации AMING
		 */
		class Auth {
			private:
				/**
				 * Ldap Структура ldap
				 */
				struct Ldap {
					string keyUser;
					string keyGroup;
					string dnUser;
					string dnGroup;
					string scopeUser;
					string scopeGroup;
					string filterUser;
					string filterGroup;
				};
				// Объект ldap подклчюения
				Ldap ldap;
				// Объект лога
				LogApp * log = nullptr;
				// Конфигурационные данные
				Config * config = nullptr;
				// Объект с данными групп
				void * groups = nullptr;
				// Объект с данными пользователей
				void * users = nullptr;
				/**
				 * pamconv Функция проверки валидности pam авторизации
				 * @param num_msg     номер сообщения
				 * @param msg         сообщение
				 * @param resp        ответ системы
				 * @param appdata_ptr указатель на объект данных системы
				 * @return            результат проверки
				 */
				static int pamconv(int num_msg = 0, const struct pam_message ** msg = nullptr, struct pam_response ** resp = nullptr, void * appdata_ptr = nullptr);
				/**
				 * checkLdap Метод проверки корректности пароля c помощью LDAP сервера
				 * @param username имя пользователя
				 * @param password пароль пользователя
				 * @return         результат проверки
				 */
				const bool checkLdap(const uid_t uid, const string password);
				/**
				 * checkPam Метод проверки корректности пароля c помощью операционной системы
				 * @param username имя пользователя
				 * @param password пароль пользователя
				 * @return         результат проверки
				 */
				const bool checkPam(const uid_t uid, const string password);
				/**
				 * checkPam Метод проверки корректности пароля c помощью конфигурационного файла
				 * @param username имя пользователя
				 * @param password пароль пользователя
				 * @return         результат проверки
				 */
				const bool checkFile(const uid_t uid, const string password);
			public:
				/**
				 * check Метод проверки корректности пароля
				 * @param username имя пользователя
				 * @param password пароль пользователя
				 * @return         результат проверки
				 */
				const bool check(const string username = "", const string password = "");
				/**
				 * Auth Конструктор
				 * @param config конфигурационные данные
				 * @param log    объект лога для вывода информации
				 * @param groups объект групп пользователей
				 * @param users  объект пользователей
				 */
				Auth(Config * config = nullptr, LogApp * log = nullptr, void * groups = nullptr, void * users = nullptr);
		};
		// Объект лога
		LogApp * log = nullptr;
		// Конфигурационные данные
		Config * config = nullptr;
		// Объект с данными групп
		Groups * groups = nullptr;
		// Объект с данными пользователей
		Users * users = nullptr;
		// Объект с данными авторизации
		Auth * auth = nullptr;
		/**
		 * getUser Метод получения данных пользователя
		 * @param uid идентификатор пользователя
		 * @return    объект с зданными пользователя
		 */
		const AParams::AUser getUser(const uid_t uid);
		/**
		 * getUser Метод получения данных пользователя
		 * @param userName имя пользователя
		 * @return         объект с зданными пользователя
		 */
		const AParams::AUser getUser(const string userName);
	public:
		/**
		 * getAllGroups Метод получения данных всех групп
		 * @return      список данных всех групп
		 */
		const vector <const DataGroup *> getAllGroups();
		/**
		 * getIdUsersInGroup Метод получения списка пользователей в группе
		 * @param  gid идентификатор группы
		 * @return     список идентификаторов пользователей
		 */
		const vector <uid_t> getIdUsersInGroup(const gid_t gid);
		/**
		 * getIdUsersInGroup Метод получения списка пользователей в группе
		 * @param  groupName название группы
		 * @return           список идентификаторов пользователей
		 */
		const vector <uid_t> getIdUsersInGroup(const string groupName);
		/**
		 * getNameUsersInGroup Метод получения списка пользователей в группе
		 * @param  gid   идентификатор группы
		 * @param  users объект пользователей
		 * @return       список имен пользователей
		 */
		const vector <string> getNameUsersInGroup(const gid_t gid);
		/**
		 * getNameUsersInGroup Метод получения списка пользователей в группе
		 * @param  groupName название группы
		 * @param  users     объект пользователей
		 * @return           список имен пользователей
		 */
		const vector <string> getNameUsersInGroup(const string groupName);
		/**
		 * getGroupNameByGid Метод извлечения имени группы по ее идентификатору
		 * @param  gid идентификатор группы
		 * @return     название группы
		 */
		const string getGroupNameByGid(const gid_t gid);
		/**
		 * getUserNameByUid Метод извлечения имени пользователя по его идентификатору
		 * @param  uid идентификатор пользователя
		 * @return     имя пользователя
		 */
		const string getUserNameByUid(const uid_t uid);
		/**
		 * getGidByName Метод извлечения идентификатора группы по ее имени
		 * @param  groupName название группы
		 * @return           идентификатор группы
		 */
		const gid_t getGidByName(const string groupName);
		/**
		 * getUidByName Метод извлечения идентификатора пользователя по его имени
		 * @param  userName название пользователя
		 * @return          идентификатор пользователя
		 */
		const uid_t getUidByName(const string userName);
		/**
		 * getOptionsByUid Метод получения опций прокси сервера по идентификатору пользователя
		 * @param uid идентификатор пользователя
		 * @return    опции прокси сервера
		 */
		const u_short getOptionsByUid(const uid_t uid);
		/**
		 * getOptionsByUserName Метод получения опций прокси сервера по имени пользователя
		 * @param userName название пользователя
		 * @return         опции прокси сервера
		 */
		const u_short getOptionsByUserName(const string userName);
		/**
		 * checkUserInGroup Метод проверки принадлежности пользователя к группе
		 * @param  gid идентификатор группы
		 * @param  uid идентификатор пользователя
		 * @return     результат проверки
		 */
		const bool checkUserInGroup(const gid_t gid, const uid_t uid);
		/**
		 * checkUserInGroup Метод проверки принадлежности пользователя к группе
		 * @param  gid      идентификатор группы
		 * @param  userName название пользователя
		 * @return          результат проверки
		 */
		const bool checkUserInGroup(const gid_t gid, const string userName);
		/**
		 * checkUserInGroup Метод проверки принадлежности пользователя к группе
		 * @param  groupName название группы
		 * @param  uid       идентификатор пользователя
		 * @return           результат проверки
		 */
		const bool checkUserInGroup(const string groupName, const uid_t uid);
		/**
		 * checkUserInGroup Метод проверки принадлежности пользователя к группе
		 * @param  groupName название группы
		 * @param  userName  название пользователя
		 * @return           результат проверки
		 */
		const bool checkUserInGroup(const string groupName, const string userName);
		/**
		 * checkGroupById Метод проверки на существование группы
		 * @param  gid идентификатор группы
		 * @return     результат проверки
		 */
		const bool checkGroupById(const gid_t gid);
		/**
		 * searchUser Метод поиска данных пользователя по данным коннекта
		 * @param ip  адрес интернет протокола клиента
		 * @param mac аппаратный адрес сетевого интерфейса клиента
		 * @return    данные пользователя
		 */
		const AParams::AUser searchUser(const string ip = "", const string mac = "");
		/**
		 * authenticate Метод авторизации пользователя в системе
		 * @param login логин пользователя
		 * @param pass  пароль пользователя
		 * @return      данные пользователя
		 */
		const AParams::AUser authenticate(const string login, const string pass);
		/**
		 * AUsers Конструктор
		 * @param config конфигурационные данные
		 * @param log    объект лога для вывода информации
		 */
		AUsers(Config * config = nullptr, LogApp * log = nullptr);
		/**
		 * ~AUsers Деструктор
		 */
		~AUsers();
};

#endif // _USERS_PROXY_AMING_
