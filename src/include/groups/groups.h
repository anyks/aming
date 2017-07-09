/* ГРУППЫ ПОЛЬЗОВАТЕЛЕЙ ПРОКСИ AMING */
/*
*	автор:				Юрий Николаевич Лобарев
*	skype:				efrantick
*	телефон:			+7(920)672-33-22
*	авторские права:	Все права принадлежат автору © Юрий Лобарев, 2017
*/
#ifndef _GROUPS_PROXY_AMING_
#define _GROUPS_PROXY_AMING_

#include <regex>
#include <string>
#include <random>
#include <vector>
#include <iostream>
#include <algorithm>
#include <unordered_map>
#include <grp.h>
#include <pwd.h>
#include <time.h>
#include <zlib.h>
#include <stdlib.h>
#include <sys/types.h>
#include "ini/ini.h"
#include "system/system.h"
#include "general/general.h"
#include "ldap2/ldap.h"
#include "headers/headers.h"

// Устанавливаем область видимости
using namespace std;

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
		/**
		 * Timeout Структура таймаутов
		 */
		struct Timeouts {
			size_t read;	// Таймаут на чтение данных
			size_t write;	// Таймаут на запись данных
			size_t upgrade;	// Таймаут на работу в режиме переключения протоколов
		};
		/**
		 * IP Структура ip адреса
		 */
		struct IP {
			vector <string> ip;			// Список ip адресов
			vector <string> resolver;	// Список dns серверов
		};
		/**
		 * ID Структура идентификаторов подключений
		 */
		struct ID {
			vector <string> ip4;	// Список ip4 адресов
			vector <string> ip6;	// Список ip6 адресов
			vector <string> nwk4;	// Список сетей IPv4
			vector <string> nwk6;	// Список сетей IPv6
			vector <string> mac;	// Список mac адресов
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
		 * Connects Структура контроля подключений клиента к серверу
		 */
		struct Connects {
			size_t size;	// Максимальный размер скачиваемых данных в байтах
			u_int connect;	// Максимальное количество подключений (одного клиента к прокси серверу)
		};
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
		 * Data Структура данных группы
		 */
		struct Data {
			gid_t id;				// Идентификатор группы
			u_short options;		// Основные параметры прокси
			u_short type;			// Тип группы (0 - файлы, 1 - PAM, 2 - LDAP)
			string name;			// Название группы
			string pass;			// Пароль группы (пароль в зашифрованном виде)
			string desc;			// Описание групп
			ID idnt;				// Блок списков идентификации групп
			IP ipv4;				// Блок ip адресов для протокола версии 4
			IP ipv6;				// Блок ip адресов для протокола версии 6
			Gzip gzip;				// Параметры gzip
			Proxy proxy;			// Параметры самого прокси-сервера
			Headers headers;		// Объект управления заголовками
			Connects connects;		// Контроль подключений клиента к серверу
			Timeouts timeouts;		// Таймауты подключений
			BufferSize buffers;		// Размеры буферов передачи данных
			Keepalive keepalive;	// Постоянное подключение
			vector <uid_t> users;	// Список идентификаторов пользователей
		};
		// Время в течение которого обновлять нельзя
		time_t maxUpdate = 0;
		// Время последнего обновления данных
		time_t lastUpdate = 0;
		// Тип поиска групп (0 - Из файла, 1 - из PAM, 2 - из LDAP)
		u_short typeSearch = 0;
		// Тип поиска конфигурационных данных
		u_short typeConfigs = 0; // (0 - Из файла, 1 - из LDAP)
		// Максимальное количество групп пользователя для PAM
		int maxPamGroupsUser = 0;
		// Объект ldap подклчюения
		Ldap ldap;
		// Объект лога
		LogApp * log = nullptr;
		// Конфигурационные данные
		Config * config = nullptr;
		// Список групп
		unordered_map <gid_t, Data> data;
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
		void setDataGroupFromLdap(Data &group);
		/**
		 * setDataGroupFromFile Метод заполнения данных группы из конфигурационного файла
		 * @param group объект группы
		 * @param ini   указатель на объект конфигурации
		 */
		void setDataGroupFromFile(Data &group, INI * ini = nullptr);
		/**
		 * setDataGroup Метод заполнения данных группы
		 * @param group объект группы
		 * @param ini   указатель на объект конфигурации
		 */
		void setDataGroup(Data &group, INI * ini = nullptr);
		/**
		 * createDefaultData Метод создания группы с параметрами по умолчанию
		 * @param  id   идентификатор групыы
		 * @param  name название группы
		 * @return      созданная группа
		 */
		const Data createDefaultData(const gid_t id, const string name);
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
		 */
		const bool update();
	public:
		/**
		 * getAllGroups Метод получения данных всех групп
		 * @return      список данных всех групп
		 */
		const vector <Data> getAllGroups();
		/**
		 * getDataById Метод получения данные группы по идентификатору группы
		 * @param  gid идентификатор группы
		 * @return     данные группы
		 */
		const Data getDataById(const gid_t gid);
		/**
		 * getDataByName Метод получения данные группы по имени группы
		 * @param  groupName название группы
		 * @return           данные группы
		 */
		const Data getDataByName(const string groupName);
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
		 * @return     данные группы
		 */
		const bool checkGroupById(const gid_t gid);
		/**
		 * checkGroupByName Метод проверки на существование группы
		 * @param  groupName название группы
		 * @return           результат проверки
		 */
		const bool checkGroupByName(const string groupName);
		/**
		 * getUidByName Метод извлечения идентификатора пользователя по его имени
		 * @param  userName название пользователя
		 * @return          идентификатор пользователя
		 */
		const uid_t getUidByName(const string userName);
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
		 * @param  id   идентификатор группы
		 * @param  name название группы
		 * @return      результат добавления
		 */
		const bool addGroup(const gid_t id, const string name);
		/**
		 * Groups Конструктор
		 * @param config конфигурационные данные
		 * @param log    объект лога для вывода информации
		 */
		Groups(Config * config = nullptr, LogApp * log = nullptr);
};

#endif // _GROUPS_PROXY_AMING_
