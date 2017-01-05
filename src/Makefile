# Define compiler
CXX=clang++

# Compiler flags
CXXFLAGS=-Wall -Werror=vla -O3 -pedantic -ggdb -g -std=c++11 -lz -lpthread /usr/local/opt/libevent/lib/libevent.a

# Build target executable
INCLUDES=/usr/local/include
TARGET=./proxy/http.cpp ./lib/http/http.cpp ./lib/base64/base64.cpp ./lib/log/log.cpp ./lib/osopt/osopt.cpp ./anyks.cpp

all: $(TARGET)

$(TARGET): $(CXX) $(CXXFLAGS) -I$(INCLUDES) -o $(TARGET)

clean: $(RM) $(TARGET)


#g++ -Wall -O3 -pedantic -ggdb -g -std=c++11 -Werror=vla -lz -lpthread -o ./bin/http ./proxy/http.cpp ./lib/http/http.cpp ./lib/base64/base64.cpp ./lib/log/log.cpp ./lib/osopt/osopt.cpp ./lib/ini/INIReader.cpp ./lib/ini/ini.cpp ./anyks.cpp -I/usr/local/include /usr/local/opt/libevent/lib/libevent.a
#clang++ -Wall -O3 -pedantic -ggdb -g -std=c++11 -Werror=vla -lz -lpthread -o ./bin/http ./proxy/http.cpp ./lib/http/http.cpp ./lib/base64/base64.cpp ./lib/log/log.cpp ./lib/osopt/osopt.cpp ./anyks.cpp -I/usr/local/include /usr/local/opt/libevent/lib/libevent.a