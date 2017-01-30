/* МОДУЛЬ УПРАВЛЕНИЯ СИСТЕМНЫМИ НАСТРОЙКАМИ ANYKS */
/*
*	автор:				Юрий Николаевич Лобарев
*	skype:				efrantick
*	телефон:			+7(920)672-33-22
*	авторские права:	Все права принадлежат автору © Юрий Лобарев, 2017
*/
#ifndef _SYSTEM_PARAMS_ANYKS_
#define _SYSTEM_PARAMS_ANYKS_

#include <string>
#include <iostream>
#include <sys/ipc.h>
#include <sys/msg.h>
#include <sys/types.h>
#include "os/os.h"
#include "log/log.h"
#include "config/conf.h"

// Устанавливаем область видимости
using namespace std;

/**
 * System Класс управляющий системными настройками
 */
class System {
	private:
		/**
		 * ParamsSendPids Структура для передачи пидов между воркерами
		 */
		struct ParamsSendPids {
			long	type;
			pid_t	pids[MMAX_WORKERS];
			int		len;
		};
		// Адрес конфигурационного файла
		string configfile;
	public:
		/**
		 * Pids Структура содержащая данные полученных пидов
		 */
		struct Pids {
			pid_t pids[MMAX_WORKERS];
			int len;
		};
		// Объект log модуля
		LogApp * log = NULL;
		// Объект конфигурационного файла
		Config * config = NULL;
		// Объект взаимодействия с ОС
		Os * os = NULL;
		/**
		 * clearMsgPids Метод очистки процесса передачи данных пидов между воркерами
		 */
		void clearMsgPids();
		/**
		 * readPids Метод получения идентификаторов пидов дочерних воркеров
		 * @return структура с данными пидов
		 */
		Pids readPids();
		/**
		 * sendPids Метод отправки идентификаторов пидов родительским воркерам
		 * @param pids указатель на массив пидов
		 * @param len  размер массива пидов
		 */
		void sendPids(pid_t * pids, size_t len);
		/**
		 * reload Метод перезагрузки конфигурационных данных
		 */
		void reload();
		/**
		 * setConfigFile Метод установки адреса конфигурационного файла
		 * @param configfile адрес конфигурационного файла
		 */
		void setConfigFile(const string configfile);
		/**
		 * System Конструктор
		 * @param configfile адрес конфигурационного файла
		 */
		System(const string configfile);
		/**
		 * ~System Деструктор
		 */
		~System();

};

#endif // _SYSTEM_PARAMS_ANYKS_