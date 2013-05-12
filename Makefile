INCLUDE = $(shell pkg-config --cflags luajit)
LIBS = $(shell pkg-config --libs luajit)

test: test.cpp $(shell find . -name *.h) $(shell find . -name *.hpp)
	clang++ -g --std=c++11 test.cpp -o $@ $(INCLUDE) -I./ $(LIBS)