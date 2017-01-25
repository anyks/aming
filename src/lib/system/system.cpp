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