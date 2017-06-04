/* УТИЛИТА ЧИСТКИ УСТАРЕВШИХ ФЙЛОВ КЭША ANYKS */
/*
*	автор:				Юрий Николаевич Лобарев
*	skype:				efrantick
*	телефон:			+7(920)672-33-22
*	авторские права:	Все права принадлежат автору © Юрий Лобарев, 2016 - 2017
*/
#include "ccache.h"

// Устанавливаем пространство имен
using namespace std;

/**
 * checkDomains Функция проверки кэша доменов
 * @param filename адрес файла записи
 * @param ctx      объект приложения
 */
void CCache::checkDomains(const string filename, void * ctx){
	cout << " +++++++++++ domain +++++++++++ " << filename << endl;
}
/**
 * checkCache Функция проверки кэша данных
 * @param filename адрес файла записи
 * @param ctx      объект приложения
 */
void CCache::checkCache(const string filename, void * ctx){
	cout << " +++++++++++ cache +++++++++++ " << filename << endl;
}
/**
 * infoPatch Функция определяющая тип адреса
 * @param  pathName  адрес файла или каталога
 * @return           результат проверки (-1 - Не определено, 0 - Файл, 1 - Каталог)
 */
const short CCache::infoPatch(const string pathName){
	// Результат проверки
	int result = -1;
	// Если файл определен
	if(!pathName.empty()){
		// Создаем структуру для информации о содержимом
		struct stat entryInfo;
		// Считываем информацию о содержимом
		if(lstat(pathName.c_str(), &entryInfo) == 0){
			// Если это каталог
			if(S_ISDIR(entryInfo.st_mode)){
				// Выводим название каталога в консоль
				printf("Catalog name: %s/\n", pathName.c_str());
				// Сообщаем что это каталог
				return 1;
			// Если это файл
			} else if(S_ISREG(entryInfo.st_mode)) {
				// Выводим название файла в консоль
				printf("\t%s has %lld bytes\n", pathName.c_str(), (long long) entryInfo.st_size);
				// Выводим результат
				return 0;
			// Если это символьная ссылка
			} else if(S_ISLNK(entryInfo.st_mode)) {
				// Копируем название файла на который ссылается ссылка
				char targetName[PATH_MAX + 1];
				// Считываем данные ссылки
				if(readlink(pathName.c_str(), targetName, PATH_MAX) != -1){
					// Выводим название файла в консоль
					printf("\t%s -> %s\n", pathName.c_str(), targetName);
					// Проверяем содержимое символьной ссылки
					return infoPatch(targetName);
				// Если ссылка битая
				} else printf("\t%s -> (invalid symbolic link!)\n", pathName.c_str());
			}
		// Если произошла ошибка чтения файла
		} else printf("Error statting %s: %s\n", pathName.c_str(), strerror(errno));
	}
	// Выводим результат
	return result;
}
/**
 * processDirectory Рекурсивная функция обхода содержимого каталога
 * @param  curDir адрес текущего каталога
 * @param  fn     функция обработчик содержимого файла
 * @return        результат работы
 */
const u_long CCache::processDirectory(const string curDir, handler fn){
	// Основные структуры
	struct dirent entry;
	struct dirent * entryPtr = NULL;
	// Количество найденных файлов
	u_long count = 0;
	// Адрес файла
	char pathName[PATH_MAX + 1];
	// Открываем текущий каталог
	DIR * dir = opendir(curDir.c_str());
	// Если каталог удачно открыт
	if(dir == NULL){
		// Выводим сообщение об ошибке
		printf("Error opening %s: %s", curDir.c_str(), strerror(errno));
		// Выходим и говорим что ничего не найдено
		return 0;
	}
	// Считываем содержимое каталога
	int retval = readdir_r(dir, &entry, &entryPtr);
	// Если указатель на каталог существует
	while(entryPtr != NULL){
		// Определяем внешний или текущий каталог
		if((strncmp(entry.d_name, ".", PATH_MAX) == 0)
		|| (strncmp(entry.d_name, "..", PATH_MAX) == 0)){
			/* Short-circuit the . and .. entries. */
			retval = readdir_r(dir, &entry, &entryPtr);
			continue;
		}
		// Копируем название текущего каталога в буфер
		(void) strncpy(pathName, curDir.c_str(), PATH_MAX);
		// Добавляем экранирование к текущему каталогу
		(void) strncat(pathName, "/", PATH_MAX);
		// Добавляем к пути название внутреннего файла или каталога
		(void) strncat(pathName, entry.d_name, PATH_MAX);
		// Определяем тип адреса
		int info = infoPatch(pathName);
		// Если путь определен
		if(info > -1){
			// Увеличиваем количество найденных файлов
			count++;
			// Проверяем что это за файл
			switch(info){
				// Обрабатываем полученный файл
				case 0: fn(pathName, this);							break;
				// Продолжаем работу, загружаем данные следующего каталога
				case 1: count += processDirectory(pathName, fn);	break;
			}
		}
		// Считываем количество файлов
		retval = readdir_r(dir, &entry, &entryPtr);
	}
	// Закрываем каталог
	(void) closedir(dir);
	// Возвращаем количество найденных файлов
	return count;
}
/**
 * CCache Конструктор
 * @param config адрес конфигурационного файла
 */
CCache::CCache(const string config){
	// Создаем объект конфигурации
	this->config = new Config(config);
	// Получаем каталог с адресом кэша доменов
	const string dns = addToPath(this->config->cache.dir, "dns");
	// Получаем каталог с адресом кэша данных
	const string cache = addToPath(this->config->cache.dir, "cache");
	// Выполняем проверку доменов
	processDirectory(dns, &CCache::checkDomains);
	// Выполняем проверку кэша
	processDirectory(cache, &CCache::checkCache);
}
/**
 * ~CCache Деструктор
 */
CCache::~CCache(){
	// Удаляем объект конфигурации
	if(this->config) delete this->config;
}
/**
 * main Главная функция приложения
 * @param  argc длина массива параметров
 * @param  argv массив параметров
 * @return      код выхода из приложения
 */
int main(int argc, char * argv[]){
	// Активируем локаль приложения
	// setlocale(LC_ALL, "");
	setlocale(LC_ALL, "en_US.UTF-8");
	// Адрес конфигурационного файла
	string configfile;
	// Определяем параметр запуска
	string param = (argc >= 2 ? argv[1] : "");
	// Если это параметр поиска конфигурационного файла
	if(param.compare("-c") == 0)
		// Ищем адрес конфигурационного файла
		configfile = (argc >= 3 ? argv[2] : "");
	// Если параметр конфигурационного файла найден
	else if(param.find("--config=") != string::npos)
		// Удаляем параметр из адреса файла
		configfile = param.replace(0, 9, "");
	// Создаем объект приложения
	CCache * app = new CCache(configfile);
	// Удаляем объект приложения
	if(app) delete app;
	// Выходим
	return 0;
}
