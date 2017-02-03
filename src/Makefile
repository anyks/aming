# ПРАВИЛО СБОРКИ ПРОКСИ СЕРВЕРА ANYKS
#
#	автор:				Юрий Николаевич Лобарев
#	skype:				efrantick
#	телефон:			+7(920)672-33-22
#	авторские права:	Все права принадлежат автору © Юрий Лобарев, 2016
#

# Определяем тип операционной системы
OS := $(shell uname -s)

# Если это MacOS X (Сборка через make)
ifeq ($(OS), Darwin)
	# Компилятор
	CC = clang++

	# Сторонние модули
	LIBS =	-lz \
			-lpthread \
			/usr/local/opt/libevent/lib/libevent.a
endif

# Если это FreeBSD (Сборка через gmake)
ifeq ($(OS), FreeBSD)
	# Компилятор
	CC = clang++

	# Сторонние модули
	LIBS =	-lz \
			-lpthread \
			/usr/local/lib/libevent.a
endif

# Если это Linux
ifeq ($(OS), Linux)
	# Компилятор
	CC = gcc

	# Сторонние модули
	LIBS =	-lm \
			-pthread \
			/usr/lib/x86_64-linux-gnu/libevent.a \
			/usr/lib/gcc/x86_64-linux-gnu/5/libstdc++.a \
			/usr/lib/x86_64-linux-gnu/libz.a \
			/usr/lib/x86_64-linux-gnu/libm.a
endif

# Конфигурация сборки
# -Wextra (Варнинги по мелочи)
CONFIG = -Wall -Wpedantic -Werror=vla -Wno-unused-result -O2 -ggdb -g -std=c++11

# Заголовочные файлы
INCLUDE =	-I/usr/include \
			-I/usr/local/include \
			-I./include \
			-I./app

# Код для сборки
SOURCES =	./anyks.cpp \
			$(shell find ./app/*.cpp) \
			$(shell find ./lib/*/*.cpp)

# Бинарный файл
BIN = ./bin/http

# Правило сборки
all:
	mkdir -p ./bin && $(CC) $(CONFIG) $(SOURCES) $(MODULES) $(INCLUDE) $(LIBS) -o $(BIN)

# Правило запуска
start:
	./bin/http -c ./config.ini

# Правило очистки
clean:
	rm -rf ./bin
