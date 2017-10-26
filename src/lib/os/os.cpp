/* МОДУЛЬ ОПТИМИЗАЦИИ ОПЕРАЦИОННОЙ СИСТЕМЫ AMING */
/*
*  author:     Yuriy Lobarev
*  phone:      +7(910)983-95-90
*  telegram:   @forman
*  email:      info@anyks.com
*  copyright:  © 2017 anyks.com
*/

#include "os/os.h"

// Устанавливаем область видимости
using namespace std;

/**
 * getOsName Функция определения операционной системы
 * @return название операционной системы
 */
Os::OsData Os::getOsName(){
	// Результат
	OsData result;
	// Определяем операционную систему
	#ifdef _WIN32
		// Заполняем структуру
		result = {"Windows 32-bit", 1};
	#elif _WIN64
		// Заполняем структуру
		result = {"Windows 64-bit", 2};
	#elif __APPLE__ || __MACH__
		// Заполняем структуру
		result = {"MacOS X", 3};
	#elif __linux__
		// Заполняем структуру
		result = {"Linux", 4};
	#elif __FreeBSD__
		// Заполняем структуру
		result = {"FreeBSD", 5};
	#elif __unix || __unix__
		// Заполняем структуру
		result = {"Unix", 6};
	#else
		// Заполняем структуру
		result = {"Other", 7};
	#endif
	// Выводим результат
	return result;
}
/**
 * mkPid Функция создания pid файла
 */
void Os::mkPid(){
	// Если конфигурационный объект существует
	if(* config){
		// Создаем адрес pid файла
		string filename = ((* config)->proxy.piddir + string("/") + (* config)->proxy.name + ".pid");
		// Открываем файл на запись
		FILE * f = fopen(filename.c_str(), "w");
		// Если файл открыт
		if(f){
			// Записываем в файл pid процесса
			fprintf(f, "%u", getpid());
			// Закрываем файл
			fclose(f);
		}
	}
}
/**
 * rmPid Функция удаления pid файла
 * @param ext тип ошибки
 */
void Os::rmPid(int ext){
	// Если конфигурационный объект существует
	if(* config){
		// Создаем адрес pid файла
		string filename = ((* config)->proxy.piddir + string("/") + (* config)->proxy.name + ".pid");
		// Удаляем файл
		remove(filename.c_str());
		// Выходим из приложения
		exit(ext);
	}
}
/**
 * set_fd_limit Функция установки количество разрешенных файловых дескрипторов
 * @return количество установленных файловых дескрипторов
 */
int Os::setFdLimit(){
	// Структура для установки лимитов
	struct rlimit lim;
	// зададим текущий лимит на кол-во открытых дискриптеров
	lim.rlim_cur = (* this->config)->connects.fds;
	// зададим максимальный лимит на кол-во открытых дискриптеров
	lim.rlim_max = (* this->config)->connects.fds;
	// установим указанное кол-во
	return setrlimit(RLIMIT_NOFILE, &lim);
}
// Если это не Linux
#ifndef __linux__
/**
 * getNumberParam Метод получения заначений ядра sysctl
 * @param  name название параметра
 * @return      значение параметра
 */
long Os::getNumberParam(string name){
	// Параметр искомого значения
	int param = 0;
	// Получаем размер искомого параметра
	size_t len = sizeof(param);
	// Запрашиваем искомые данные
	if(sysctlbyname(name.c_str(), &param, &len, nullptr, 0) < 0){
		// Выводим сообщение в консоль
		if(this->log) this->log->write(LOG_ERROR, 0, "filed get param: %s", name.c_str());
	}
	// Выводим результат
	return param;
}
/**
 * getParam Метод получения заначений ядра sysctl
 * @param  name название параметра
 * @return      значение параметра
 */
string Os::getStringParam(string name){
	// Создаем буфер для чтения данных
	char buffer[128];
	// Получаем размер буфера
	size_t len = sizeof(buffer);
	// Заполняем буфер нулями
	memset(buffer, 0, len);
	// Запрашиваем искомые данные
	if(sysctlbyname(name.c_str(), &buffer, &len, nullptr, 0) < 0){
		// Выводим сообщение в консоль
		if(this->log) this->log->write(LOG_ERROR, 0, "filed get param: %s", name.c_str());
	}
	// Выводим результат
	return buffer;
}
/**
 * setParam Метод установки значений ядра sysctl
 * @param name  название параметра
 * @param param данные параметра
 */
void Os::setParam(string name, int param){
	// Устанавливаем новые параметры настройки ядра
	if(sysctlbyname(name.c_str(), nullptr, 0, &param, sizeof(param)) < 0){
		// Выводим сообщение в консоль
		if(this->log) this->log->write(LOG_ERROR, 0, "filed set param: %s -> %i", name.c_str(), param);
	}
}
/**
 * setParam Метод установки значений ядра sysctl
 * @param name  название параметра
 * @param param данные параметра
 */
void Os::setParam(string name, string param){
	// Получаем значение параметра для установки
	const char * value = param.c_str();
	// Устанавливаем новые параметры настройки ядра
	if(sysctlbyname(name.c_str(), nullptr, 0, (void *) value, param.size()) < 0){
		// Выводим сообщение в консоль
		if(this->log) this->log->write(LOG_ERROR, 0, "filed set param: %s -> %s", name.c_str(), param.c_str());
	}
}
#endif
/**
 * enableCoreDumps Функция активации создания дампа ядра
 * @return результат установки лимитов дампов ядра
 */
bool Os::enableCoreDumps(){
	// Если отладка включена
	if((* this->config)->proxy.debug){
		// Структура лимитов дампов
		struct rlimit limit;
		// Устанавливаем текущий лимит равный бесконечности
		limit.rlim_cur = RLIM_INFINITY;
		// Устанавливаем максимальный лимит равный бесконечности
		limit.rlim_max = RLIM_INFINITY;
		// Выводим результат установки лимита дампов ядра
		return (setrlimit(RLIMIT_CORE, &limit) == 0);
	}
	// Сообщаем что операция не удачная
	return false;
}
/**
 * privBind Функция запускает приложение от имени указанного пользователя
 */
void Os::privBind(){
	uid_t uid;	// Идентификатор пользователя
	gid_t gid;	// Идентификатор группы
	// Размер строкового типа данных
	string::size_type sz;
	// Если идентификатор пользователя пришел в виде числа
	if(Anyks::isNumber((* config)->proxy.user))
		// Получаем идентификатор пользователя
		uid = stoi((* config)->proxy.user, &sz);
	// Если идентификатор пользователя пришел в виде названия
	else uid = Anyks::getUid((* config)->proxy.user.c_str());
	// Если идентификатор группы пришел в виде числа
	if(Anyks::isNumber((* config)->proxy.group))
		// Получаем идентификатор группы пользователя
		gid = stoi((* config)->proxy.group, &sz);
	// Если идентификатор группы пришел в виде названия
	else gid = Anyks::getGid((* config)->proxy.group.c_str());
	// Устанавливаем идентификатор пользователя
	setuid(uid);
	// Устанавливаем идентификатор группы
	setgid(gid);
}
/**
 * exec Метод запуска внешней оболочки
 * @param cmd       команда запуска
 * @param multiline данные должны вернутся многострочные
 */
string Os::exec(string cmd, bool multiline){
	// Устанавливаем размер буфера
	const int MAX_BUFFER = 2048;
	// Полученный результат
	string result;
	// Создаем буфер для чтения результата
	char buffer[MAX_BUFFER];
	// Зануляем весь буфер
	memset(buffer, '\0', MAX_BUFFER);
	// Создаем пайп для чтения результата работы OS
	FILE * stream = popen(cmd.c_str(), "r");
	// Если пайп открыт
	if(stream){
		// Считываем до тех пор пока все не прочитаем
		while(!feof(stream)){
			// Считываем данные из потока в буфер
			fgets(buffer, MAX_BUFFER, stream);
			// Добавляем полученный результат
			result.append(buffer);
			// Если это не мультилайн то выходим
			if(!multiline) break;
		}
		// Закрываем пайп
		pclose(stream);
		// Если данные в буфере существуют
		if(result.empty() && this->log) this->log->write(LOG_ERROR, 0, "filed set param: %s", buffer);
	}
	// Выводим результат
	return result;
}
/**
 * getCongestionControl Метод определения алгоритма сети
 * @param  str строка с выводмом доступных алгоритмов из sysctl
 * @return     строка с названием алгоритма
 */
string Os::getCongestionControl(string str){
	// Результат работы регулярного выражения
	smatch match;
	// Устанавливаем правило регулярного выражения
	regex e("(cubic|htcp)", regex::ECMAScript | regex::icase);
	// Выполняем поиск протокола
	regex_search(str, match, e);
	// Если протокол найден
	if(!match.empty() && (match.size() == 2)) return match[1].str();
	// Если ничего не найдено то выводим пустую строку
	return "";
}
/**
 * getCPU Метод получения данных процессора
 */
void Os::getCPU(){
	// Определяем тип операционной системы
	OsData os = getOsName();
	// Количество ядер процессора
	u_int ncpu = 1;
	// Название процессора
	string cpu = "Unknown";
	// Определяем os
	switch(os.type){
// Если это не Linux
#ifndef __linux__
		// Если это MacOS X
		case 3: {
			// Получаем количество ядер
			ncpu = (u_int) getNumberParam("hw.ncpu");
			// Получаем название процессора
			cpu = getStringParam("machdep.cpu.brand_string");
		} break;
#endif
		// Если это Linux
		case 4: {
			// Результат работы регулярного выражения
			smatch match;
			// Получаем данные о процессоре
			string str = exec("lscpu", true);
			// Устанавливаем правило регулярного выражения
			regex e("CPU\\(s\\)\\:\\s+(\\d+)[\\s\\S]+Model\\s+name\\:\\s+([^\\r\\n]+)", regex::ECMAScript | regex::icase);
			// Выполняем поиск протокола
			regex_search(str, match, e);
			// Если протокол найден
			if(!match.empty() && (match.size() == 3)){
				// Получаем количество ядер
				ncpu = ::atoi(match[1].str().c_str());
				// Получаем название процессора
				cpu = match[2].str();
			}
		} break;
// Если это не Linux
#ifndef __linux__
		// Если это FreeBSD
		case 5: {
			// Получаем количество ядер
			ncpu = (u_int) getNumberParam("hw.ncpu");
			// Получаем название процессора
			cpu = exec("grep -w CPU: /var/run/dmesg.boot");
			// Результат работы регулярного выражения
			smatch match;
			// Устанавливаем правило регулярного выражения
			regex e("CPU\\:\\s+([^\\r\\n]+)", regex::ECMAScript | regex::icase);
			// Выполняем поиск протокола
			regex_search(cpu, match, e);
			// Если протокол найден
			if(!match.empty() && (match.size() == 2)) cpu = match[1].str();
		} break;
#endif
	}
	// Формируем структуру данных операционной системы
	(* config)->os = {ncpu, cpu, os.name};
}
/**
 * optimos Метод запуска оптимизации
 * @return результат работы
 */
void Os::optimos(){
	// Данные оптимизаций операционной системы берет от сюда: http://fasterdata.es.net/host-tuning/freebsd/
	// Определяем тип операционной системы
	OsData os = getOsName();
	// Определяем os
	switch(os.type){
		// Если это Windows
		case 1:
		case 2: {
			// Vista/7 also includes "Compound TCP (CTCP)", which is similar to cubic on Linux. To enable this, set the following:
			exec("netsh interface tcp set global congestionprovider=ctcp");
			// If you even need to enable autotuning, here are the commands:
			exec("netsh interface tcp set global autotuninglevel=normal");
		} break;
// Если это не Linux
#ifndef __linux__
		// Если это MacOS X
		case 3: {
			// OSX default of 3 is not big enough
			setParam("net.inet.tcp.win_scale_factor", 8);
			// increase OSX TCP autotuning maximums
			setParam("net.inet.tcp.autorcvbufmax", 33554432);
			setParam("net.inet.tcp.autosndbufmax", 33554432);
			// for other customs
			setParam("net.inet.tcp.sendspace", 1042560);
			setParam("net.inet.tcp.recvspace", 1042560);
			setParam("net.inet.tcp.slowstart_flightsize", 20);
			setParam("net.inet.tcp.local_slowstart_flightsize", 20);
			// for 10G hosts it would be nice to increase this too, but
			// 4G seems to be the limit for some OSX installations
			setParam("kern.ipc.maxsockbuf", 6291456);
			// for max connections
			setParam("kern.ipc.somaxconn", 49152);
		} break;
#endif
		// Если это Linux
		case 4: {
			// Если режим отладки включен
			if((* this->config)->proxy.debug){
				// for debug, create dump core
				exec("sysctl -w kernel.core_uses_pid=1");
				exec("sysctl -w kernel.core_pattern=/tmp/core-%e-%p");
			}
			// for max connections
			exec("sysctl -w net.core.somaxconn=49152");
			// allow testing with buffers up to 128MB
			exec("sysctl -w net.core.rmem_max=134217728");
			exec("sysctl -w net.core.wmem_max=134217728");
			// increase Linux autotuning TCP buffer limit to 64MB
			exec("sysctl -w net.ipv4.tcp_rmem=\"4096 87380 33554432\"");
			exec("sysctl -w net.ipv4.tcp_wmem=\"4096 65536 33554432\"");
			// recommended for hosts with jumbo frames enabled
			exec("sysctl -w net.ipv4.tcp_mtu_probing=1");
			// recommended for CentOS7/Debian8 hosts
			exec("sysctl -w net.core.default_qdisc=fq");
			// recommended default congestion control is htcp
			// you can check which are available using net.ipv4.tcp_available_congestion_control
			// Get which are available algorithm
			string algorithm = getCongestionControl(exec("sysctl net.ipv4.tcp_available_congestion_control"));
			// If algorithm exist
			if(!algorithm.empty()) exec(string("sysctl -w net.ipv4.tcp_congestion_control=") + algorithm);
		} break;
// Если это не Linux
#ifndef __linux__
		// Если это FreeBSD
		case 5: {
			// set to at least 16MB for 10GE hosts
			exec("sysctl -w kern.ipc.maxsockbuf=16777216");
			// set autotuning maximum to at least 16MB too
			exec("sysctl -w net.inet.tcp.sendbuf_max=16777216");
			exec("sysctl -w net.inet.tcp.recvbuf_max=16777216");
			// for other customs
			exec("sysctl -w net.inet.tcp.sendspace=1042560");
			exec("sysctl -w net.inet.tcp.recvspace=1042560");
			// enable send/recv autotuning
			exec("sysctl -w net.inet.tcp.sendbuf_auto=1");
			exec("sysctl -w net.inet.tcp.recvbuf_auto=1");
			// increase autotuning step size
			exec("sysctl -w net.inet.tcp.sendbuf_inc=16384");
			exec("sysctl -w net.inet.tcp.recvbuf_inc=524288");
			// set this on test/measurement hosts
			exec("sysctl -w net.inet.tcp.hostcache.expire=1");
			// for max connections
			exec("sysctl -w kern.ipc.somaxconn=49152");
			// you can check which are available using net.inet.tcp.cc.available
			// Get which are available algorithm
			string algorithm = getCongestionControl(getStringParam("net.inet.tcp.cc.available"));
			// If algorithm exist
			if(!algorithm.empty()) setParam("net.inet.tcp.cc.algorithm", algorithm);
		} break;
#endif
		// Если это Solaris
		case 7: {
			// increase max tcp window
			// Rule-of-thumb: max_buf = 2 x cwnd_max (congestion window)
			exec("ndd -set /dev/tcp tcp_max_buf 33554432");
			exec("ndd -set /dev/tcp tcp_cwnd_max 16777216");
			// increase DEFAULT tcp window size
			exec("ndd -set /dev/tcp tcp_xmit_hiwat 65536");
			exec("ndd -set /dev/tcp tcp_recv_hiwat 65536");
		} break;
	}
}
/**
 * Os Конструктор
 * @param config  объект конфигурационных файлов
 * @param log     объект лога для вывода информации
 */
Os::Os(Config ** config, LogApp * log){
	// Если конфигурационный файл передан
	if(config){
		// Запоминаем настройки системы
		this->log		= log;
		this->config	= config;
		// Получаем данные процессора
		getCPU();
		// Активируем лимиты дампов ядра
		enableCoreDumps();
		// Если модуль активирован тогда запускаем активацию
		if((* config)->proxy.optimos) optimos();
	}
}