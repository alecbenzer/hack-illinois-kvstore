memory_map_test: memory_map_test.o
	clang++ -std=c++11 memory_map_test.o -lgtest -lgtest_main -pthread -o memory_map_test

memory_map_test.o: memory_map_test.cc
	clang++ -std=c++11 -c memory_map_test.cc
