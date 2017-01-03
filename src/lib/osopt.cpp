/* МОДУЛЬ ОПТИМИЗАЦИИ ОПЕРАЦИОННОЙ СИСТЕМЫ ANYKS */
/*
*	автор:				Юрий Николаевич Лобарев
*	skype:				efrantick
*	телефон:			+7(920)672-33-22
*	авторские права:	Все права принадлежат автору © Юрий Лобарев, 2016
*/
#include "osopt.h"

// Устанавливаем область видимости
using namespace std;

/**
 * getOsName Функция определения операционной системы
 * @return название операционной системы
 */
OsOpt::OsData OsOpt::getOsName(){
	// Результат
	OsData result;
	// Определяем операционную систему
	#ifdef _WIN32
		// Заполняем структуру
		result = {"Windows 32-bit", 1};
	#elif _WIN64
		// Заполняем структуру
		result = {"Windows 64-bit", 2};
	#elif __unix || __unix__
		// Заполняем структуру
		result = {"Unix", 3};
	#elif __APPLE__ || __MACH__
		// Заполняем структуру
		result = {"Mac OSX", 4};
	#elif __linux__
		// Заполняем структуру
		result = {"Linux", 5};
	#elif __FreeBSD__
		// Заполняем структуру
		result = {"FreeBSD", 6};
	#else
		// Заполняем структуру
		result = {"Other", 7};
	#endif
	// Выводим результат
	return result;
}
/**
 * exec Метод запуска внешней оболочки
 * @param cmd команда запуска
 */
string OsOpt::exec(string cmd){
	// Устанавливаем размер буфера
	const int MAX_BUFFER = 2048;
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
		}
		// Закрываем пайп
		pclose(stream);
		// Если данные в буфере существуют
		if(strlen(buffer)) this->log->write(LOG_MESSAGE, "system set: %s", buffer);
	}
	// Выводим результат
	return buffer;
}
/**
 * getCongestionControl Метод определения алгоритма сети
 * @param  str строка с выводмом доступных алгоритмов из sysctl
 * @return     строка с названием алгоритма
 */
string OsOpt::getCongestionControl(string str){
	// Результат работы регулярного выражения
	smatch match;
	// Устанавливаем правило регулярного выражения
	regex e("^[\\w\\._\\-]+\\s*\\:.*\\s*(cubic|htcp)", regex::ECMAScript | regex::icase);
	// Выполняем поиск протокола
	regex_search(str, match, e);
	// Если протокол найден
	if(!match.empty() && (match.size() == 2)) return match[1].str();
	// Если ничего не найдено то выводим пустую строку
	return "";
}
/**
 * run Метод запуска оптимизации
 * @return результат работы
 */
void OsOpt::run(){
	// Если модуль активирован тогда запускаем активацию
	if(this->enabled){
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
			// Если это MacOS X
			case 4: {
				// OSX default of 3 is not big enough
				exec("sysctl -w net.inet.tcp.win_scale_factor=8");
				// increase OSX TCP autotuning maximums
				exec("sysctl -w net.inet.tcp.autorcvbufmax=33554432");
				exec("sysctl -w net.inet.tcp.autosndbufmax=33554432");
				// for other customs
				exec("sysctl -w net.inet.tcp.sendspace=1042560");
				exec("sysctl -w net.inet.tcp.recvspace=1042560");
				exec("sysctl -w net.inet.tcp.slowstart_flightsize=20");
				exec("sysctl -w net.inet.tcp.local_slowstart_flightsize=20");
				// for 10G hosts it would be nice to increase this too, but
				// 4G seems to be the limit for some OSX installations
				exec("sysctl -w kern.ipc.maxsockbuf=6291456");
				// for max connections
				exec("sysctl -w kern.ipc.somaxconn=49152");
			} break;
			// Если это Linux
			case 5: {
				// for max connections
				exec("sysctl -w net.core.somaxconn=49152");
				// allow testing with buffers up to 128MB
				exec("sysctl -w net.core.rmem_max=134217728");
				exec("sysctl -w net.core.wmem_max=134217728");
				// increase Linux autotuning TCP buffer limit to 64MB
				exec("sysctl -w net.ipv4.tcp_rmem=4096 87380 67108864");
				exec("sysctl -w net.ipv4.tcp_wmem=4096 65536 67108864");
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
			// Если это FreeBSD
			case 6: {
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
				// turn off inflight limiting
				exec("sysctl -w net.inet.tcp.inflight.enable=0");
				// set this on test/measurement hosts
				exec("sysctl -w net.inet.tcp.hostcache.expire=1");
				// for max connections
				exec("sysctl -w kern.ipc.somaxconn=49152");
				// you can check which are available using net.inet.tcp.cc.available
				// Get which are available algorithm
				string algorithm = getCongestionControl(exec("sysctl net.inet.tcp.cc.available"));
				// If algorithm exist
				if(!algorithm.empty()) exec(string("sysctl -w net.inet.tcp.cc.algorithm=") + algorithm);
			} break;
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
}
/**
 * enable Метод активации модуля
 */
void OsOpt::enable(){
	// Запоминаем что модуль активирован
	this->enabled = true;
}
/**
 * enable Метод деактивации модуля
 */
void OsOpt::disable(){
	// Запоминаем что модуль деактивирован
	this->enabled = false;
}
/**
 * OsOpt Конструктор
 * @param log     объект лога для вывода информации
 * @param enabled модуль активирован или деактивирован
 */
OsOpt::OsOpt(LogApp * log, bool enabled){
	// Запоминаем настройки системы
	this->log		= log;
	this->enabled	= enabled;
	// Если модуль активирован тогда запускаем активацию
	if(this->enabled) run();
}