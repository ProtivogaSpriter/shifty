all: build

build:	source.cpp
	g++ -o ./bin/shifty source.cpp
