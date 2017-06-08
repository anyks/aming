# ПРАВИЛО СБОРКИ ПРОКСИ СЕРВЕРА ANYKS
#
#	автор:				Юрий Николаевич Лобарев
#	skype:				efrantick
#	телефон:			+7(920)672-33-22
#	авторские права:	Все права принадлежат автору © Юрий Лобарев, 2016
#
#   help: https://wiki.gentoo.org/wiki/GCC_optimization/ru

# Определяем тип операционной системы
OS := $(shell uname -s)

# Если это MacOS X (Сборка через make)
ifeq ($(OS), Darwin)
	# export MACOSX_DEPLOYMENT_TARGET="10.12";
	# Компилятор
	CC = export MACOSX_DEPLOYMENT_TARGET="10.12"; clang++

	# Сторонние модули
	LIBS =	-lz \
			-lldap \
			-lstdc++ \
			-lpthread \
			-I/usr/local/Cellar/openssl@1.1/1.1.0e/include \
			/usr/local/Cellar/openssl@1.1/1.1.0e/lib/libcrypto.a \
			/usr/local/opt/libevent/lib/libevent.a
endif

# Если это FreeBSD (Сборка через gmake)
ifeq ($(OS), FreeBSD)
	# Компилятор
	CC = clang++

	# Сторонние модули
	LIBS =	-lz \
			-lldap \
			-lstdc++ \
			-lcrypto \
			-lpthread \
			-DLDAP_DEPRECATED \
			-L/usr/local/lib \
			/usr/local/lib/libevent.a
endif

# Если это Linux
ifeq ($(OS), Linux)
	# Компилятор
	CC = gcc

	# Сторонние модули
	LIBS =	-lm \
			-lldap \
			-lcrypto \
			-pthread \
			/usr/lib/x86_64-linux-gnu/libevent.a \
			/usr/lib/gcc/x86_64-linux-gnu/5/libstdc++.a \
			/usr/lib/x86_64-linux-gnu/libz.a \
			/usr/lib/x86_64-linux-gnu/libm.a
endif

# Заголовочные файлы
INCLUDE =	-I/usr/include \
			-I/usr/local/include \
			-I./include \
			-I./app

# Код для сборки
SOURCES =	./anyks.cpp \
			$(shell find ./app/*.cpp) \
			$(shell find ./lib/*/*.cpp)

# Код для сборки Tools CCache
SOURCETCC = ./lib/config/conf.cpp \
			./lib/general/general.cpp \
			./lib/ini/ini.cpp \
			./tools/ccache.cpp

# Бинарный файл
BIN = ./bin

# Конфиг для стандартной сборки
CONFIG = -Wall -Wpedantic -Wstrict-aliasing -Werror=vla -Wno-unused-result -Wno-unused-value -Wno-deprecated -Wno-sign-compare -Wno-extra-tokens -g -std=c++11

# Правило сборки
all:
	# Выполняем сборку проекта
	mkdir -p $(BIN) && $(CC) $(CONFIG) -O2 -ggdb -pipe -fomit-frame-pointer -fstrict-aliasing -msse3 $(SOURCES) $(INCLUDE) $(LIBS) -o $(BIN)/http

# Правила сборки под Dev
dev:
	# Выполняем сборку проекта
	mkdir -p $(BIN) && $(CC) $(CONFIG) -O1 -ggdb $(SOURCES) $(INCLUDE) $(LIBS) -o $(BIN)/http

# Правила сборки под Dev Tools CCache
dtcc:
	mkdir -p $(BIN) && $(CC) $(CONFIG) -O1 -ggdb $(SOURCETCC) $(INCLUDE) -I./tools $(LIBS) -o $(BIN)/dtcc

# Правила сборки под Prod
prod:
	# Выполняем сборку проекта [-mbmi2]
	mkdir -p $(BIN) && $(CC) $(CONFIG) -O2 -pipe -fomit-frame-pointer -fstrict-aliasing -mavx2 -mfma -mrdrnd -mf16c $(SOURCES) $(INCLUDE) $(LIBS) -o $(BIN)/http

# Правила сборки под Prod Tools CCache [-mbmi2]
ptcc:
	mkdir -p $(BIN) && $(CC) $(CONFIG) -O2 -pipe -fomit-frame-pointer -fstrict-aliasing -mavx2 -mfma -mrdrnd -mf16c $(SOURCETCC) $(INCLUDE) -I./tools $(LIBS) -o $(BIN)/ptcc

# Правило запуска
start:
	$(BIN)/http -c ./config.ini

# Правило очистки
clean:
	rm -rf $(BIN)
