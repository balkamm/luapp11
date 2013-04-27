test: test.cpp $(shell find . -name *.h) $(shell find . -name *.hpp)
	clang++ -g --std=c++11 test.cpp -o $@ -I/usr/include/luajit-2.0/ -I./ -L/usr//lib -lluajit-5.1
	$@