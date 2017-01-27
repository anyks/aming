/* МОДУЛЬ УПРАВЛЕНИЯ СИСТЕМНЫМИ НАСТРОЙКАМИ ANYKS */
/*
*	автор:				Юрий Николаевич Лобарев
*	skype:				efrantick
*	телефон:			+7(920)672-33-22
*	авторские права:	Все права принадлежат автору © Юрий Лобарев, 2017
*/

#include "system.h"

// Устанавливаем область видимости
using namespace std;

/**
 * clearMsgPids Метод очистки процесса передачи данных пидов между воркерами
 * @param key ключ для отправки сообщения
 */
void System::clearMsgPids(int key){
	// Ключ по которому передаются данные между процессов
	key_t msgkey = ftok(".", key);
	// Создаем идентификатор сообщения
	int qid = msgget(msgkey, IPC_CREAT | 0666);
	// Освобождаем процесс
	msgctl(qid, IPC_RMID, 0);
}
/**
 * readPids Метод получения идентификаторов пидов дочерних воркеров
 * @param  key ключ для отправки сообщения
 * @return     структура с данными пидов
 */
System::Pids System::readPids(int key){
	// Создаем объект с параметрами для получения пидов
	ParamsSendPids msg;
	// Создаем результирующий объект
	Pids result;
	// Вычисляем размер отправляемых данных
	size_t length = sizeof(msg) - sizeof(long);
	// Ключ по которому передаются данные между процессов
	key_t msgkey = ftok(".", key);
	// Создаем идентификатор сообщения
	int qid = msgget(msgkey, IPC_CREAT | 0666);
	// Если сообщение не может быть отправлено выводим сообщение об ошибке
	if(qid < 0){
		// Выводим сообщение в консоль
		this->log->write(LOG_ERROR, "[-] cannot create message id for read fork pids");
		// Выходим из приложения
		exit(23);
	}
	// Выполняем чтение сообщения
	if(msgrcv(qid, &msg, length, 1, 0) < 0) {
		// Выводим сообщение в консоль
		this->log->write(LOG_ERROR, "[-] cannot read message children fork pids");
		// Освобождаем процесс
		msgctl(qid, IPC_RMID, 0);
		// Выходим из приложения
		exit(23);
	}
	// Запоминаем размер массива с пидами
	result.len = msg.len;
	// Копируем полученный массив пидов
	memcpy(result.pids, msg.pids, sizeof(pid_t) * result.len);
	// Освобождаем процесс
	msgctl(qid, IPC_RMID, 0);
	// Выводим результат
	return result;
}
/**
 * sendPids Метод отправки идентификаторов пидов родительским воркерам
 * @param pids указатель на массив пидов
 * @param len  размер массива пидов
 * @param key  ключ для отправки сообщения
 */
void System::sendPids(pid_t * pids, size_t len, int key){
	// Очищаем ранее созданные сессии
	clearMsgPids(key);
	// Создаем объект с параметрами для отправки
	ParamsSendPids msg;
	// Вычисляем размер отправляемых данных
	size_t length = sizeof(msg) - sizeof(long);
	// Ключ по которому передаются данные между процессов
	key_t msgkey = ftok(".", key);
	// Создаем идентификатор сообщения
	int qid = msgget(msgkey, IPC_CREAT | 0666);
	// Если сообщение не может быть отправлено выводим сообщение об ошибке
	if(qid < 0){
		// Выводим сообщение в консоль
		this->log->write(LOG_ERROR, "[-] cannot create message id for send in worker %d to %d", getpid(), getppid());
		// Выходим из приложения
		exit(23);
	}
	// Устанавливаем тип сообщения
	msg.type = 1;
	// Запоминаем количество пидов
	msg.len = len;
	// Заполняем структуру массива пидов
	memcpy(msg.pids, pids, sizeof(pid_t) * msg.len);
	// Выполняем отправку сообщения
	if(msgsnd(qid, &msg, length, IPC_NOWAIT) < 0){
		// Выводим сообщение в консоль
		this->log->write(LOG_ERROR, "[-] cannot send message in worker %d to %d", getpid(), getppid());
		// Выходим из приложения
		exit(23);
	}
}
/**
 * reload Метод перезагрузки конфигурационных данных
 */
void System::reload(){
	// Удаляем созданный ранее объект конфигурационного файла
	delete this->config;
	// Создаем объект конфигурации
	this->config = new Config(this->configfile);
}
/**
 * setConfigFile Метод установки адреса конфигурационного файла
 * @param configfile адрес конфигурационного файла
 */
void System::setConfigFile(string configfile){
	// Запоминаем адрес конфигурационного файла
	this->configfile = configfile;
	// Выполняем перезагрузку конфигурационного файла
	reload();
}
/**
 * System Конструктор
 * @param configfile адрес конфигурационного файла
 */
System::System(string configfile){
	// Запоминаем адрес конфигурационного файла
	this->configfile = configfile;
	// Создаем объект конфигурации
	this->config = new Config(this->configfile);
	// Создаем модуль лога
	this->log = new LogApp(&this->config, TOLOG_FILES | TOLOG_CONSOLE);
	// Устанавливаем настройки операционной системы
	this->os = new Os(this->log, &this->config);
}
/**
 * ~System Деструктор
 */
System::~System(){
	// Удаляем созданные ранее объекты
	delete this->os;
	delete this->log;
	delete this->config;
}