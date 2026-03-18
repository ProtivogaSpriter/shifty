all: build

bindir:
	mkdir bin

build:	source.cpp bindir
	g++ -o ./bin/shifty source.cpp

clean:
	rm -r ./bin
