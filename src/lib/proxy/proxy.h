/* МОДУЛЬ ПРОКСИ СЕРВЕРА ANYKS */
/*
*	автор:				Юрий Николаевич Лобарев
*	skype:				efrantick
*	телефон:			+7(920)672-33-22
*	авторские права:	Все права принадлежат автору © Юрий Лобарев, 2017
*/
#ifndef _PROXY_APP_ANYKS_
#define _PROXY_APP_ANYKS_

#include <vector>
#include <string>
#include <errno.h>
#include <unistd.h>
#include <signal.h>
#include <sys/stat.h>
#include <sys/wait.h>
#include <sys/file.h>
#include <sys/signal.h>
#include <sys/resource.h>
#include <event2/event.h>
#include "../system/system.h"
#include "../../proxy/http.h"

// Устанавливаем область видимости
using namespace std;

/**
 * Proxy Класс прокси сервера
 */
class Proxy {
	public:
		/**
		 * SignalBuffer Структура буфера сигнала
		 */
		struct SignalBuffer {
			int signal;		// Номер сигнала
			void * proxy;	// Объект прокси сервера
		};
	private:
		// Пид дочернего воркера балансера
		pid_t cpid = 0;
		// Пид воркера поднимающего балансер
		pid_t mpid = 0;
		// Объект управления системными настройками
		System * sys = NULL;
		// База данных событий
		struct event_base * base = NULL;
		// Буферы сигналов
		vector <SignalBuffer> siginfo;
		vector <SignalBuffer> sigexit;
		vector <SignalBuffer> sigsegv;
		// События сигналов
		vector <struct event *> signals;
		/**
		 * signal_log Функция вывода значения сигнала в лог
		 * @param num номер сигнала
		 * @param ctx объект прокси сервера
		 */
		static void signal_log(int num, void * ctx);
		/**
		 * clear_fantoms Функция чистки фантомных процессов
		 * @param signal сигнал
		 * @param ctx    объект прокси сервера
		 */
		static void clear_fantoms(int signal, void * ctx);
		/**
		 * siginfo_cb Функция обработки сигнала SIGPIPE
		 * @param fd    файловый дескриптор (сокет)
		 * @param event возникшее событие
		 * @param ctx   объект прокси сервера
		 */
		static void siginfo_cb(evutil_socket_t fd, short event, void * ctx);
		/**
		 * sigsegv_cb Функция обработки сигналов ошибки сегментации SIGSEGV
		 * @param fd    файловый дескриптор (сокет)
		 * @param event возникшее событие
		 * @param ctx   объект прокси сервера
		 */
		static void sigsegv_cb(evutil_socket_t fd, short event, void * ctx);
		/**
		 * sigexit_cb Функция обработки сигналов безусловного завершения работы
		 * @param fd    файловый дескриптор (сокет)
		 * @param event возникшее событие
		 * @param ctx   объект прокси сервера
		 */
		static void sigexit_cb(evutil_socket_t fd, short event, void * ctx);
		/**
		 * create_proxy Функция создания прокси-сервера
		 */
		void create_proxy();
		/**
		 * run_worker Функция запуска воркера
		 */
		void run_worker();
	public:
		/**
		 * Proxy Конструктор
		 * @param configfile адрес конфигурационного файла
		 */
		Proxy(const string configfile);
		/**
		 * ~Proxy Деструктор
		 */
		~Proxy();
};

#endif // _PROXY_APP_ANYKS_