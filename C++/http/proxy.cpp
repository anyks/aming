// sudo lsof -i | grep -E LISTEN
//  g++ -o socks5 http.cpp -lpthread
//  g++ -std=c++11 -Wall -pedantic -O3 -Werror=vla -lpthread -o http base64.cpp proxy.cpp

#include <cstdlib>
#include <cstring>
#include <unistd.h>
#include <stdint.h>
#include <signal.h>

#include <sys/socket.h>
#include <arpa/inet.h>
#include <netinet/in.h>
#include <netdb.h>

#include <pthread.h>

#include <iostream>
#include <memory>
#include <string>
#include <sstream>
#include <algorithm>
#include <set>
#include <sys/stat.h>
#include <stdio.h>

#include <sys/ipc.h>
#include <sys/msg.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <sys/resource.h>


// #include <sstream>
#include <vector>

#include "base64.h"

using namespace std;

// Приведение типа -Wint-to-void-pointer-cast
// Или нужно установить параметр сборки: -Wno-int-to-void-pointer-cast
#define INTTOVOID_POINTER(i) ((char *) NULL + (i))

// Структура для передачи идентификатора сокета между процессами
typedef struct fbuff {
	long type;
	int socket;
} fmess;

// Класс для работы с сервером
class TServer {
	pthread_mutex_t mutex;

	public:
		// Конструктор
		TServer(){pthread_mutex_init(&mutex, NULL);}
		// Деструктор
		~TServer(){pthread_mutex_destroy(&mutex);}
		// Блокировка
		inline void lock(){pthread_mutex_lock(&mutex);}
		// Разблокировка
		inline void unlock(){pthread_mutex_unlock(&mutex);}
};

// Класс для работы с потоками клиента
class TClient {
	pthread_mutex_t mutex;
	pthread_cond_t condition;

	public:
		// Конструктор
		TClient(){
			pthread_mutex_init(&mutex, 0);
			pthread_cond_init(&condition, 0);
		}
		// Деструктор
		~TClient(){
			pthread_mutex_destroy(&mutex);
			pthread_cond_destroy(&condition);
		}
		// Блокировка
		inline void lock(){pthread_mutex_lock(&mutex);}
		// Разблокировка
		inline void unlock(){pthread_mutex_unlock(&mutex);}
		// Сигнал
		inline void signal(){pthread_cond_signal(&condition);}
		// Сигнал вещания
		inline void broadcastSignal(){pthread_cond_broadcast(&condition);}
		// Ожидание
		inline void wait(){pthread_cond_wait(&condition, &mutex);}
};

// Класс содержит данные парсинга http запроса
class Http {
	private:
		// Структура подключения
		struct connect {
			char * host		= NULL;	// Хост
			char * port		= NULL;	// Порт
			char * protocol	= NULL;	// Протокол
		};
		// Основные переменные
		int socket			= 0;	// Идентификатор сокета
		char * query		= NULL;	// Исходный запрос
		char * command		= NULL;	// Команда запроса
		char * method		= NULL;	// Метод запроса
		char * host			= NULL;	// Хост запроса
		char * port			= NULL;	// Порт запроса
		char * path			= NULL;	// Путь запроса
		char * protocol		= NULL;	// Протокол запроса
		char * version		= NULL;	// Версия протокола
		char * connection	= NULL; // Заголовок connection
		char * pconnection	= NULL;	// Заголовок proxy-connection
		char * useragent	= NULL;	// UserAgent браузера
		char * auth			= NULL;	// Тип авторизации
		char * login		= NULL;	// Логин
		char * password		= NULL;	// Пароль
		char * entitybody	= NULL;	// Данные запроса для POST метода
		char * head			= NULL;	// Результирующий заголовок для запроса
		// Массив остальных заголовков которые для нас не имеют значения
		std::vector <std::string> other;
		// Массив заголовков
		std::vector <std::string> headers;
		// Функция разбиения строки на указанные составляющие
		std::vector<std::string> split(const std::string str, const char * delim){
			// Начальный символ для обрезки строки
			int i = 0;
			// Результирующий вектор
			std::vector<std::string> result;
			// Строка в которой производим поиск
			std::string s = str.c_str();
			// Позиция разделителя в строке
			int pos = s.find(delim);
			// Перебираем строку до тех пор пока не переберем её целиком
			while(pos != std::string::npos){
				// Добавляем найденную позицию в строке
				result.push_back(s.substr(i, pos - i));
				// Запоминаем новую позицию
				i = strlen(delim) + pos;
				// Выполняем поиск новой позиции
				pos = s.find(delim, i);
				// Если позиция достигнута тогда продолжаем дальше
				if(pos == std::string::npos) result.push_back(s.substr(i, s.length()));
			}
			// Выводим результат
			return result;
		}
		// Функция установки текстовой переменной
		void setVar(const char * str, char * &var){
			// Если данные там уже есть, очищаем память
			if(var != NULL) delete [] var;
			// Создаем новую пустую строку
			var = new char[strlen(str) + 1];
			// Выполняем копирование строки
			std::strcpy(var, str);
		}
		// Функция перевода в нижний регистр
		std::string toLowerCase(std::string str){
			// Переводим все в нижний регистр
			std::transform(str.begin(), str.end(), str.begin(), ::tolower);
			// Выводим результат
			return str;
		}
		// Функция перевода в верхний регистр
		std::string toUpperCase(std::string str){
			// Переводим все в верхний регистр
			std::transform(str.begin(), str.end(), str.begin(), ::toupper);
			// Выводим результат
			return str;
		}
		// Функция удаления пробелов и переносов строк
		std::string trim(std::string &str){
			str.erase(0, str.find_first_not_of(" "));	// Начальный пробел
			str.erase(str.find_last_not_of(" ") + 1);	// Конечный пробел
			// Выводим результат
			return str;
		}
		// Функция получения содержимое заголовка
		std::string getHeaderParam(const char * head, const char * param){
			// Запоминаем переданные параметры
			std::string str, h = head, p = std::string(param) + ":";
			// Выполняем поиск заголовка
			size_t pos = toLowerCase(h).find(toLowerCase(p).c_str());
			// Если заголовок найден
			if(pos != std::string::npos){
				// Извлекаем данные
				str = h.substr(pos + p.length(), (h.length() - (pos + p.length())));
				// Вырезаем пробелы
				trim(str);
			}
			// Выводим результат
			return str;
		}
		// Функция поиска значение заголовка
		std::string findHeaderParam(const char * st, const char * buf){
			// Результирующая строка
			std::string str, istr = toLowerCase(st) + ":";
			// Ищем начало заголовка Content-length
			size_t pos1 = toLowerCase(buf).find(istr.c_str());
			// Ищем конец заголовка Content-length
			size_t pos2 = toLowerCase(buf).find("\r\n", pos1);
			// Если заголовок найден
			if((pos1 != std::string::npos) && (pos2 != std::string::npos)){
				// Получаем размер всей строки
				int length = strlen(buf);
				// Определяем начальную позицию
				int start = pos1 + istr.length();
				// Определяем конечную позицию
				int end = length - ((length - start) + (length - pos2));
				// Извлекаем данные заголовка
				str = std::string(buf).substr(start, end);
				// Вырезаем пробелы
				trim(str);
			}
			// Выводим результат
			return str;
		}
		// Функция извлечения данных подключения
		connect getConnection(const char * str){
			// Полученные данные подключения
			connect data;
			// Создаем временную строковую переменную
			std::string s = str;
			// Выполняем поиск протокола
			size_t pos = s.find("://");
			// Размеры данных
			size_t size;
			// Если протокол найден
			if(pos != std::string::npos){
				// Выполняем разделение на протокол
				std::vector<std::string> prt = split(s.c_str(), "://");
				// Запоминаем размер массива
				size = prt.size();
				// Если протокол найден
				if(size){
					// Запоминаем версию протокола
					setVar(prt[0].c_str(), data.protocol);
					// Запоминаем оставшуюся часть строки
					s = prt[1];
				}
				// Очищаем память выделенную для вектора
				std::vector <std::string> ().swap(prt);
			// Устанавливаем протокол по умолчанию
			} else setVar(protocol, data.protocol);
			// Выполняем поиск порта и хоста
			pos = s.find(":");
			// Если хост и порт найдены
			if(pos != std::string::npos){
				// Выполняем разделение на протокол
				std::vector<std::string> prt = split(s.c_str(), ":");
				// Запоминаем размер массива
				size = prt.size();
				// Если порт и хост найдены
				if(size){
					// Запоминаем хост
					setVar(prt[0].c_str(), data.host);
					// Запоминаем порт
					setVar(prt[1].c_str(), data.port);
				}
				// Очищаем память выделенную для вектора
				std::vector <std::string> ().swap(prt);
			// Если хост и порт не найдены
			} else {
				// Запоминаем хост
				setVar(s.c_str(), data.host);
				// Запоминаем порт
				setVar("80", data.port);
			}
			// Устанавливаем номер порта в зависимости от типа протокола
			if(!strcmp(toLowerCase(data.protocol).c_str(), "https")) setVar("443", data.port);
			// Устанавливаем версию протокола в зависимости от порта
			else if(!strcmp(toLowerCase(data.port).c_str(), "443")) setVar("https", data.protocol);
			// Запоминаем найденный хост
			s = data.host;
			// Выполняем поиск дирректории в хосте
			pos = s.find("/");
			// Если дирректория найдена значит это не хост
			if(pos != std::string::npos){
				// Извлекаем название домена
				s = s.substr(0, pos);
				// Выполняем поиск точки в домене
				pos = s.find(".");
				// Если это домен
				if(pos != std::string::npos) setVar(s.c_str(), data.host);
				// Если это не домен то устанавливаем что это корень
				else setVar("/", data.host);
			}
			// Выводим результат
			return data;
		}
		// Функция парсера http запроса
		void parser(const char * buffer){
			// Если буфер верный
			if(strlen(buffer)){
				// Запоминаем первоначальный запрос
				setVar(buffer, query);
				// Выполняем парсинг заголовков
				headers = split(query, "\r\n");
				// Размеры данных
				size_t size = headers.size();
				// Если заголовки найдены
				if(size){
					// Запоминаем команду запроса
					setVar(headers[0].c_str(), command);
					// Разделяем на составляющие команду
					std::vector<std::string> cmd = split(command, " ");
					// Получаем размер массива
					size = cmd.size();
					// Если комманда существует
					if(size && (size == 3)){
						// Запоминаем метод запроса
						setVar(toLowerCase(cmd[0]).c_str(), method);
						// Запоминаем путь запроса
						setVar(cmd[1].c_str(), path);
						// Разбиваем протокол и тип протокола на части
						std::vector<std::string> prt = split(cmd[2].c_str(), "/");
						// Получаем размер массива
						size = prt.size();
						// Если данные найдены
						if(size && (size == 2)){
							// Запоминаем протокол запроса
							setVar(toLowerCase(prt[0]).c_str(), protocol);
							// Запоминаем версию протокола
							setVar(prt[1].c_str(), version);
							// Перебираем остальные заголовки
							for(int i = 1; i < headers.size(); i++){
								// Если все заголовки считаны тогда выходим
								if(headers[i].length()){
									// Получаем данные заголовок хоста
									std::string hst = getHeaderParam(headers[i].c_str(), "Host");
									// Получаем данные юзерагента
									std::string usg = getHeaderParam(headers[i].c_str(), "User-Agent");
									// Получаем данные заголовока коннекта
									std::string cnn = getHeaderParam(headers[i].c_str(), "Connection");
									// Получаем данные заголовока коннекта для прокси
									std::string pcn = getHeaderParam(headers[i].c_str(), "Proxy-Connection");
									// Получаем данные авторизации
									std::string ath = getHeaderParam(headers[i].c_str(), "Proxy-Authorization");
									// Если заголовок хоста найден
									if(hst.length()){
										// Выполняем получение параметров подключения
										connect gcon = getConnection(path);
										// Выполняем получение параметров подключения
										connect scon = getConnection(hst.c_str());
										// Создаем полный адрес запроса
										std::string fulladdr1 = std::string(scon.protocol) + std::string("://") + scon.host;
										std::string fulladdr2 = fulladdr1 + "/";
										std::string fulladdr3 = fulladdr1 + std::string(":") + scon.port;
										std::string fulladdr4 = fulladdr3 + "/";
										// Определяем путь
										if(strcmp(toLowerCase(method).c_str(), "connect")
										&& (!strcmp(toLowerCase(path).c_str(), toLowerCase(hst).c_str())
										|| !strcmp(toLowerCase(path).c_str(), toLowerCase(fulladdr1).c_str())
										|| !strcmp(toLowerCase(path).c_str(), toLowerCase(fulladdr2).c_str())
										|| !strcmp(toLowerCase(path).c_str(), toLowerCase(fulladdr3).c_str())
										|| !strcmp(toLowerCase(path).c_str(), toLowerCase(fulladdr4).c_str()))) setVar("/", path);
										// Запоминаем хост
										setVar(toLowerCase(scon.host).c_str(), host);
										// Запоминаем порт
										if(strcmp(scon.port, gcon.port) && !strcmp(gcon.port, "80")){
											// Запоминаем протокол
											setVar(toLowerCase(scon.protocol).c_str(), protocol);
											// Уделяем предпочтение 443 порту
											setVar(scon.port, port);
										// Запоминаем порт такой какой он есть
										} else {
											// Запоминаем порт
											setVar(gcon.port, port);
											// Запоминаем протокол
											setVar(toLowerCase(gcon.protocol).c_str(), protocol);
										}
									// Если авторизация найдена
									} else if(ath.length()){
										// Выполняем разделение на тип и данные авторизации
										std::vector<std::string> lgn = split(ath.c_str(), " ");
										// Запоминаем размер массива
										size = lgn.size();
										// Если данные получены
										if(size){
											// Запоминаем тип авторизации
											setVar(toLowerCase(lgn[0]).c_str(), auth);
											// Если это тип авторизация basic, тогда выполняем декодирования данных авторизации
											if(!strcmp(auth, "basic")){
												// Выполняем декодирование логина и пароля
												std::string dauth = base64_decode(lgn[1].c_str());
												// Выполняем поиск авторизационных данных
												size_t pos = dauth.find(":");
												// Если протокол найден
												if(pos != std::string::npos){
													// Выполняем разделение на логин и пароль
													std::vector<std::string> lp = split(dauth.c_str(), ":");
													// Запоминаем размер массива
													size = lp.size();
													// Если данные получены
													if(size){
														// Запоминаем логин
														setVar(lp[0].c_str(), login);
														// Запоминаем пароль
														setVar(lp[1].c_str(), password);
													}
													// Очищаем память выделенную для вектора
													std::vector <std::string> ().swap(lp);
												}
											}
										}
										// Очищаем память выделенную для вектора
										std::vector <std::string> ().swap(lgn);
									// Если юзерагент найден
									} else if(usg.length()) setVar(usg.c_str(), useragent);
									// Если заголовок коннекта прокси найден
									else if(pcn.length()) setVar(toLowerCase(pcn).c_str(), pconnection);
									// Если заголовок коннекта найден
									else if(cnn.length()) setVar(toLowerCase(cnn).c_str(), connection);
									// Если это все остальные заголовки то просто добавляем их в список
									else if(headers[i].length()) other.push_back(headers[i]);
								// Иначе выходим из цикла
								} else break;
							}
						}
						// Очищаем память выделенную для вектора
						std::vector <std::string> ().swap(prt);
					}
					// Очищаем память выделенную для вектора
					std::vector <std::string> ().swap(cmd);
				}
			}
		}
		/**
		 * Если версия протокола 1.0 то там нет keep-alive и должен выставлять значение clone (идет запрос, ответ и отсоединение)
		 * Если версия протокола 1.1 и там установлен keep-alive то отсоединение должно быть по таймеру которое выставлено на ожидание соединения (время жизни например 30 секунд)
		 */
		// Функция получения сформированного заголовка запроса
		void createHead(){
			// Если это не метод CONNECT то меняем заголовок Connection на close
			if((entitybody == NULL) && !strcmp(version, "1.0")) setVar("close", connection);
			// Переменная с заголовком
			std::string str = std::string(command) + "\r\n";
			// Добавляем данные хоста
			str += std::string("Host: ") + std::string(host) + std::string(":") + std::string(port) + "\r\n";
			// Добавляем useragent
			if(useragent != NULL) str += std::string("User-Agent: ") + std::string(useragent) + "\r\n";
			// Добавляем остальные заголовки
			for(int i = 0; i < other.size(); i++){
				// Если это не заголовок контента, то добавляем в список остальные заголовки
				if(toLowerCase(other[i]).find("content-length:") == std::string::npos) str += (other[i] + "\r\n");
			}
			// Добавляем заголовок connection
			if(connection != NULL) str += std::string("Connection: ") + std::string(connection) + "\r\n";
			// Если тело запроса существует то добавляем размер тела
			if(entitybody != NULL) str += std::string("Content-Length: ") + std::to_string(strlen(entitybody)) + "\r\n";
			// Добавляем конец запроса
			str += "\r\n";
			// Если существует тело запроса то добавляем его
			if(entitybody != NULL) str += std::string(entitybody);
			// Устанавливаем значение переменной
			setVar(str.c_str(), head);
		}
		// Функция чтения данных данных из сокета
		void readSocket(int sock){
			// Если сокет верный
			if(sock > -1){
				// Запоминаем сокет
				socket = sock;
				// Создаем буфер
				char * buffer = new char[4096];
				// Количество считанных данных из буфера
				int bits_size = 0;
				// Размер передаваемых в теле данных
				std::string len_data;
				// Выполняем чтение данных из сокета
				while(true){
					// Считываем данные из буфера
					size_t size = recv(socket, &buffer[bits_size], 256, 0);
					// Запоминаем количество считанных байтов
					bits_size += size;
					// Устанавливаем завершение строки
					buffer[bits_size] = '\0';
					// Выполняем поиск значения размера данных
					if(!len_data.length()){
						// Выполняем поиск размера передаваемых данных
						len_data = findHeaderParam("Content-length", buffer);
						// Если все данные переданы то выходим
						if(!len_data.length() && (strstr(buffer, "\r\n\r\n") != NULL)) break;
					// Сообщаем что размер найден
					} else if(strstr(buffer, "\r\n\r\n") != NULL) {
						// Запоминаем полученный буфер
						std::string str = buffer, result;
						// Позиция конца заголовков
						int pos = str.find("\r\n\r\n");
						// Выполняем чтение данных
						result = str.substr(pos, str.length() - pos);
						// Удаляем все переносы строк
						result.erase(std::remove(result.begin(), result.end(), '\r'), result.end());
						result.erase(std::remove(result.begin(), result.end(), '\n'), result.end());
						// Вырезаем пробелы
						trim(result);
						// Если все байты считаны
						if(result.length() >= atoi(len_data.c_str())){
							// Запоминаем данные тела запроса
							setVar(result.c_str(), entitybody);
							// Выходим из функции
							break;
						}
					}
				}
				// Выполняем парсинг заголовков http запроса
				parser(buffer);
				// Выполняем генерацию результирующего запроса
				createHead();
				// Удаляем буфер
				delete [] buffer;
			}
		}
	public:
		// Конструктор
		Http(int sock){
			// Выполняем чтение из сокета
			readSocket(sock);
		}
		// Деструктор
		~Http(){
			// Очищаем выделенную память под переменные
			delete [] query;
			delete [] command;
			delete [] method;
			delete [] host;
			delete [] port;
			delete [] path;
			delete [] protocol;
			delete [] version;
			delete [] connection;
			delete [] pconnection;
			delete [] useragent;
			delete [] auth;
			delete [] login;
			delete [] password;
			delete [] entitybody;
			delete [] head;
			// Очищаем память выделенную для вектора
			std::vector <std::string> ().swap(other);
			std::vector <std::string> ().swap(headers);
		}
		// Метод получения метода запроса
		const char * getMethod(){
			// Выводим значение переменной
			return (method != NULL ? method : "");
		}
		// Метод получения хоста запроса
		const char * getHost(){
			// Выводим значение переменной
			return (host != NULL ? host : "");
		}
		// Метод получения порта запроса
		const char * getPort(){
			// Выводим значение переменной
			return (port != NULL ? port : "");
		}
		// Метод получения пути запроса
		const char * getPath(){
			// Выводим значение переменной
			return (path != NULL ? path : "");
		}
		// Метод получения протокола запроса
		const char * getProtocol(){
			// Выводим значение переменной
			return (protocol != NULL ? protocol : "");
		}
		// Метод получения версии протокола запроса
		const char * getVersion(){
			// Выводим значение переменной
			return (version != NULL ? version : "");
		}
		// Метод получения метода авторизации запроса
		const char * getAuth(){
			// Выводим значение переменной
			return (auth != NULL ? auth : "");
		}
		// Метод получения логина авторизации запроса
		const char * getLogin(){
			// Выводим значение переменной
			return (login != NULL ? login : "");
		}
		// Метод получения пароля авторизации запроса
		const char * getPassword(){
			// Выводим значение переменной
			return (password != NULL ? password : "");
		}
		// Метод получения юзерагента запроса
		const char * getUseragent(){
			// Выводим значение переменной
			return (useragent != NULL ? useragent : "");
		}
		// Метод установки метода запроса
		void setMethod(const char * str){
			// Запоминаем данные
			setVar(str, method);
			// Выполняем генерацию результирующего запроса
			createHead();
		}
		// Метод установки хоста запроса
		void setHost(const char * str){
			// Запоминаем данные
			setVar(str, host);
			// Выполняем генерацию результирующего запроса
			createHead();
		}
		// Метод установки порта запроса
		void setPort(const char * str){
			// Запоминаем данные
			setVar(str, port);
			// Выполняем генерацию результирующего запроса
			createHead();
		}
		// Метод установки пути запроса
		void setPath(const char * str){
			// Запоминаем данные
			setVar(str, path);
			// Выполняем генерацию результирующего запроса
			createHead();
		}
		// Метод установки протокола запроса
		void setProtocol(const char * str){
			// Запоминаем данные
			setVar(str, protocol);
			// Выполняем генерацию результирующего запроса
			createHead();
		}
		// Метод установки версии протокола запроса
		void setVersion(const char * str){
			// Запоминаем данные
			setVar(str, version);
			// Выполняем генерацию результирующего запроса
			createHead();
		}
		// Метод установки метода авторизации запроса
		void setAuth(const char * str){
			// Запоминаем данные
			setVar(str, auth);
			// Выполняем генерацию результирующего запроса
			createHead();
		}
		// Метод установки юзерагента запроса
		void setUseragent(const char * str){
			// Запоминаем данные
			setVar(str, useragent);
			// Выполняем генерацию результирующего запроса
			createHead();
		}
		// Метод определения нужно ли держать соединение для прокси
		bool isAlive(){
			// Если это версия протокола 1.1 и подключение установлено постоянное для прокси
			if(!strcmp(version, "1.1") && (pconnection != NULL)
			&& !strcmp(toLowerCase(pconnection).c_str(), "keep-alive")) return true;
			else return false;
		}
};

TServer get_host_lock;	// Блокировки запроса данных с хоста
TClient client_lock;	// Событие подключения клиента

// Временный каталог для файлов
string nameSystem	= "anyksHttp";
string piddir		= "/var/run";

// Количество подключенных клиентов и максимально возможное количество подключений
uint32_t client_count = 0, max_clients = 100;

void sig_handler(int signum){
	std::cout << "signum" << signum << endl;
}

// Функция обработки сигнала о появившемся зомби процессе
void sigchld_handler(int signal) {
	while(waitpid(-1, NULL, WNOHANG) > 0);
}

// Функция обработки сигналов завершения процессов
void sigterm_handler(int signal){
	// close(client_sock); // Закрываем клиентский сокет
	// close(server_sock); // Закрываем серверный сокет
	exit(0); // Выходим
}

// Функция получения данных из сокета
int recv_sock(int sock, char * buffer, uint32_t size){
	int index = 0, ret;
	while(size){
		if((ret = recv(sock, &buffer[index], size, 0)) <= 0) return (!ret) ? index : -1;
		index += ret;
		size -= ret;
	}

	std::cout << " buffer read " << buffer << endl;

	return index;
}

// Функция отправки данных в сокет
int send_sock(int sock, const char * buffer, uint32_t size){
	int index = 0, ret;
	
	std::cout << " buffer write " << buffer << endl;

	while(size){
		if((ret = send(sock, &buffer[index], size, 0)) <= 0) return (!ret) ? index : -1;
		index += ret;
		size -= ret;
	}
	return index;
}

/*
void writeToclientSocket(const char* buff_to_server,int sockfd,int buff_length)
{

	string temp;

	temp.append(buff_to_server);

	int totalsent = 0;

	int senteach;

	while (totalsent < buff_length) {
		if ((senteach = send(sockfd, (void *) (buff_to_server + totalsent), buff_length - totalsent, 0)) < 0) {
			fprintf (stderr," Error in sending to server ! \n");
				exit (1);
		}
		totalsent += senteach;

	}

}
*/


// Функция обработки входящих данных с клиента
bool handle_handshake(int sock){

	Http * http = new Http(sock);

	// std::cout << ", method = " << http->getMethod() << ", path = " << http->getPath() << ", protocol = " << http->getProtocol() << ", version = " << http->getVersion() << ", host = " << http->getHost() << ", port = " << http->getPort() << endl;
	std::cout << "method = " << http->getMethod() << ", path = " << http->getPath() << ", protocol = " << http->getProtocol() << ", version = " << http->getVersion() << ", host = " << http->getHost() << ", port = " << http->getPort() << ", useragent = " << http->getUseragent() << ", auth = " << http->getAuth() << ", login = " << http->getLogin() << ", password = " << http->getPassword() << endl;

	std::cout << (http->isAlive() ? " Постоянное подключение к прокси " : " Запрос - ответа ") << endl;

	http->setUseragent("Test user agent");

	std::cout << "method = " << http->getMethod() << ", path = " << http->getPath() << ", protocol = " << http->getProtocol() << ", version = " << http->getVersion() << ", host = " << http->getHost() << ", port = " << http->getPort() << ", useragent = " << http->getUseragent() << ", auth = " << http->getAuth() << ", login = " << http->getLogin() << ", password = " << http->getPassword() << endl;

	// reinterpret_cast <const char *> (buffer)
	//const char * dd = "HTTP/1.1 200 Connection established\r\n\r\n";
	// writeToclientSocket(dd, sock, strlen(dd));

	delete http;

	return false;
	/*
	// Структура данных для чтения входящих параметров
	MethodIdentificationPacket packet;
	// Считываем данные из сокета
	int read_size = recv_sock(sock, (char *) &packet, sizeof(MethodIdentificationPacket));
	// Если считано данных не достаточно или версия пртокола не 5 то выходим
	if(read_size != sizeof(MethodIdentificationPacket) || packet.version != 5) return false;
	// Считываем данные в буфер, с указанным методом
	if(recv_sock(sock, buffer, packet.nmethods) != packet.nmethods) return false;
	// Создаем структуру для отправки клиенту
	MethodSelectionPacket response(METHOD_NOTAVAILABLE);
	// Устанавливаем тип авторизации
	for(unsigned i(0); i < packet.nmethods; ++i){
		// Сообщаем что авторизация не требуется
		if(buffer[i] == METHOD_NOAUTH) response.method = METHOD_NOAUTH;
		// Сообщаем что авторизация требуется
		// if(buffer[i] == METHOD_AUTH) response.method = METHOD_AUTH;
	}
	// Отправляем требование клиенту
	if(send_sock(sock, (const char *) &response, sizeof(MethodSelectionPacket)) != sizeof(MethodSelectionPacket) || response.method == METHOD_NOTAVAILABLE) return false;
	// Выполняем проверку авторизации
	return ((response.method == METHOD_AUTH) ? check_auth(sock) : true);
	*/
}

// Функция обработки данных из потока
void * handle_connection(void * arg){
	// Создаем сокет
	int sock = (uint64_t) arg;
	// Выполняем проверку авторизации и спрашиваем что надо клиенту, если удачно то делаем запрос на получение данных
	handle_handshake(sock);
	// if(handle_handshake(sock, buffer)) handle_request(sock, buffer);
	// Выключаем подключение
	shutdown(sock, SHUT_RDWR);
	// Закрываем сокет
	close(sock);
	// Блокируем поток клиента
	client_lock.lock();
	// Уменьшаем количество подключенных клиентов
	client_count--;
	// Если количество подключенных клиентов достигла предела то сообщаем об этом
	if(client_count == max_clients - 1) client_lock.signal();
	// Разблокируем клиента
	client_lock.unlock();
	// Сообщаем что все удачно
	return 0;
}

// Функция обработки потоков ядра
bool spawn_thread(pthread_t * thread, void * data){
	pthread_attr_t attr;
	pthread_attr_init(&attr);
	pthread_attr_setstacksize(&attr, 64 * 1024);
	pthread_attr_setdetachstate(&attr, PTHREAD_CREATE_DETACHED);
	return !pthread_create(thread, &attr, handle_connection, data);
}

/**
 * remove_pid Функция удаления pid файла
 * @param ext  тип ошибки
 */
void remove_pid(int ext){
	// Формируем имя файла
	string filename = piddir + string("/") + nameSystem + ".pid";
	// Удаляем файл
	remove(filename.c_str());
	// Выходим из приложения
	exit(ext);
}

/**
 * create_pid Функция записи pid файла
 * @param pid  идентификатор pid файла
 */
void create_pid(pid_t pid){
	// Формируем имя файла
	string filename = piddir + string("/") + nameSystem + ".pid";
	// Удаляем файл
	remove(filename.c_str());
	// Открываем файл на запись
	FILE * fh = fopen(filename.c_str(), "w");
	// Выводим номер процесса
	std::cout << pid << endl;
	// Записываем считанные данные в файл
	fprintf(fh, "%d", pid);
	// Закрываем файл
	fclose(fh);
}

int create_listen_socket(){
	// Сокет сервера
	int serversock;
	// Максимальное число клиентов
	int maxpending = 5;
	// Структура для сервера
	struct sockaddr_in echoserver;
	// Создаем сокет
	if((serversock = socket(PF_INET, SOCK_STREAM, IPPROTO_TCP)) < 0){
	// if((serversock = socket(PF_INET, SOCK_STREAM, IPPROTO_UDP)) < 0){
		std::cout << "[-] Could not create socket.\n";
		return -1;
	}
	/* Очищаем структуру */
	memset(&echoserver, 0, sizeof(echoserver));
	// Указываем что это сетевой протокол Internet/IP
	echoserver.sin_family = AF_INET;
	// Указываем адрес прокси сервера
	echoserver.sin_addr.s_addr = htonl(INADDR_ANY); // inet_addr("127.0.0.1");
	// Указываем порт сервера
	echoserver.sin_port = htons(5556); // htons(SERVER_PORT);
	// Выполняем биндинг сокета // ::bind (для мака)
	if(::bind(serversock, (struct sockaddr *) &echoserver, sizeof(echoserver)) < 0){
		std::cout << "[-] Bind error.\n";
		return -1;
	}
	// Выполняем чтение сокета
	if(listen(serversock, maxpending) < 0){
		std::cout << "[-] Listen error.\n";
		return -1;
	}
	return serversock;
}

// Функция установки количество разрешенных файловых дескрипторов
int set_fd_limit(int maxfd){
	// Структура для установки лимитов
	struct rlimit lim;
	// зададим текущий лимит на кол-во открытых дискриптеров
	lim.rlim_cur = maxfd;
	// зададим максимальный лимит на кол-во открытых дискриптеров
	lim.rlim_max = maxfd;
	// установим указанное кол-во
	return setrlimit(RLIMIT_NOFILE, &lim);
}

int main(int argc, char * argv[]){
	// Максимальное количество файловых дескрипторов
	int maxfd = 1024; // (по дефолту в системе 1024)
	// Максимальное количество воркеров
	const size_t max_works = 4;
	// Наши ID процесса и сессии
	pid_t pid[max_works], sid;
	// Структура для клиента
	struct sockaddr_in echoclient;
	// Время ожидания следующего запроса
	float ttl = 0.5; // 5;
	// Установим максимальное кол-во дискрипторов которое можно открыть
	set_fd_limit(maxfd);
	// Ответвляемся от родительского процесса
	// pid[0] = fork();
	// Если пид не создан тогда выходим
	// if(pid[0] < 0) remove_pid(EXIT_FAILURE);
	// Если с PID'ом все получилось, то родительский процесс можно завершить.
	// if(pid[0] > 0) remove_pid(EXIT_SUCCESS);
	// Изменяем файловую маску
	// umask(0);
	// Здесь можно открывать любые журналы
	// Создание нового SID для дочернего процесса
	sid = setsid();
	// Если идентификатор сессии дочернего процесса не существует
	if(sid < 0) remove_pid(EXIT_FAILURE);
	// Изменяем текущий рабочий каталог
	// if((chdir("/")) < 0) remove_pid(EXIT_FAILURE);
	// Закрываем стандартные файловые дескрипторы
	// close(STDIN_FILENO);
	// close(STDOUT_FILENO);
	// close(STDERR_FILENO);
	// Записываем пид процесса в файл
	create_pid(sid);
	// Определяем количество возможных передаваемых данных
	uint32_t clientlen = sizeof(echoclient);
	// Получаем сокет
	int socket = create_listen_socket();
	// Проверяем все ли удачно
	if(socket == -1){
		std::cout << "[-] Failed to create server\n";
		return 1;
	}
	// Устанавливаем сигнал установки подключения
	signal(SIGPIPE, sig_handler);		// Сигнал обрыва соединения во время записи
	signal(SIGCHLD, sigchld_handler);	// Дочерний процесс убит
	signal(SIGTERM, sigterm_handler);	// Процесс убит
	// Ключ по которому передаются данные между процессов
	key_t msgkey = ftok(".", getpid());
	// Получаем внешний идентификатор процесса
	int qid = msgget(msgkey, IPC_CREAT | 0660);
	// Буфер для передачи данных socket между процессами
	fmess fork_buf;
	// Получаем размер буфера
	int lenfork_buf = sizeof(fmess) - sizeof(long);
	// Освобождаем процесс
	msgctl(qid, IPC_RMID, 0);
	// Создаем дочерние потоки (от 1 потому что 0-й это этот же процесс)
	for(int i = 1; i < max_works; i++){
		// Создаем структуру для передачи сокета сервера
		fork_buf.type	= 1;		// Тип сообщения
		fork_buf.socket	= socket;	// Сокет сервера
		// Определяем тип потока
		switch(pid[i] = fork()){
			// Если поток не создан
			case -1:
				// Сообщаем что произошла ошибка потока
				perror("fork");
				// Выходим из потока
				exit(1);
			// Если это дочерний поток значит все нормально и продолжаем работу
			case 0:
				// Выполняем получение данных сокета от родителя
				msgrcv(qid, &fork_buf, lenfork_buf, 1, 0);

				std::cout << "pid = " << (int) getpid() << endl;

				// Бесконечный цикл
				while(true){
					// Создаем сокет клиента
					int clientsock;
					// Лочим клиента
					client_lock.lock();
					// Если количество клиентов превысило предел то ставим клиента в ожидание
					if(client_count == max_clients) client_lock.wait();
					// Разлочим клиента
					client_lock.unlock();
					// Проверяем разрешено ли подключение клиента
					if((clientsock = accept(fork_buf.socket, (struct sockaddr *) &echoclient, &clientlen)) > 0){
						// Выполняем блокировку клиента
						client_lock.lock();
						// Увеличиваем количество подключенных клиентов
						client_count++;
						// Создаем поток
						pthread_t thread;
						// Выполняем активацию потока
						spawn_thread(&thread, INTTOVOID_POINTER(clientsock));
						// Выполняем разблокирование клиента
						client_lock.unlock();
						// Засыпаем на указанное количество секунд
						sleep(ttl);
					}
				}
			break;
			// Если это родитель то отправляем потомку идентификатор сокета
			default: msgsnd(qid, &fork_buf, lenfork_buf, 0);
		}
	}
	// Ждем завершение работы потомка (от 1 потому что 0-й это этот же процесс а он не может ждать завершения самого себя)
	for(int i = 1; i < max_works; i++) waitpid(pid[i], NULL, 0);
	// Освобождаем процесс
	msgctl(qid, IPC_RMID, 0);
	// Выходим
	remove_pid(EXIT_SUCCESS);
	// Выходим
	return 0;
}