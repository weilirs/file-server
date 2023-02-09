OS:=$(shell uname -s)
CC:=
CFLAGS:=-g -Wall -std=c++17 -Iapi -Iinclude -DDEBUG
POSTFIX:=



CC:=g++


CFLAGS+=-D_XOPEN_SOURCE
POSTFIX+=_macos



CC+=${CFLAGS}

FS_IMAGE:=

CXX = g++
CXXFLAGS = -g -std=c++11 -pedantic
# List of source files for your file server
FS_SOURCES=$(wildcard src/*.cpp)
TEST_SOURCES=$(wildcard test/*.cpp)
TEST_EXE=${TEST_SOURCES:test/%.cpp=build/%}

# Generate the names of the file server's object files
FS_OBJS=${FS_SOURCES:%.cpp=build/%.o}

.PHONY: all clean createfs showfs destroyfs fs test

all: fs test

# Compile the file server and tag this compilation
fs: build/fs


test%: build/test%
	

test: ${TEST_EXE}


build/fs: ${FS_OBJS} lib/libfs_server_macos.o
	${CC} -o $@ $^ -pthread -ldl

build/test%: test/test%.cpp lib/libfs_client_macos.o
	@mkdir -p $(@D)
	${CC} -o $@ $^
temp: test/temp.cpp lib/libfs_client_macos.o
	@mkdir -p $(@D)
	${CC} -o $@ $^

# Generic rules for compiling a source file to an object file
build/%.o: %.cpp
	@mkdir -p $(@D)
	${CC} -o $@ -c $<

build/%.o: %.cc
	@mkdir -p $(@D)
	${CC} -o $@ -c $<

createfs: utils/createfs_macos
	$^ ${FS_IMAGE}

showfs: utils/showfs_macos
	$^

%_masked.fs: %.fs
	sed -E "s/(.*)inode block [0-9]+(.*)/\1\2/g" < $< > $@
	sed -E "s/(.*)data disk blocks: [0-9]+(.*)/\1\2/g" < $@ > $@.temp
	sed -E "/^[ \t0-9]+$$/d" < $@.temp > $@
	rm -f $@.temp

clean:
	rm -rf build *.log *.fs
destroyfs: utils/disk.log
	rm -f $$(cat utils/disk.log) utils/disk.log

client: client.cpp
	$(CXX) $(CXXFLAGS) client.cpp -o client

server: src/main.cpp src/Server.cpp src/Request.cpp 
	${CC} -o $@ $^ -ldl
clean:
	rm -rf build
