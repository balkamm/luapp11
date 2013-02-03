test: test.cpp $(shell find *.hpp)
	clang++ --std=c++11 test.cpp -o $@ -I/usr/include/luajit-2.0/ -I./
	$@