all: mmap_allocator_test memory_map_test

memory_map_test: memory_map_test.o
	clang++ -std=c++11 memory_map_test.o -lgtest -lgtest_main -pthread -o memory_map_test

memory_map_test.o: memory_map_test.cc
	clang++ -std=c++11 -c memory_map_test.cc

mmap_allocator_test: mmap_allocator_test.o
	clang++ -std=c++11 mmap_allocator_test.o -lgtest -lgtest_main -pthread -o mmap_allocator_test

mmap_allocator_test.o: mmap_allocator_test.cc mmap_allocator.h
	clang++ -std=c++11 -c mmap_allocator_test.cc

server.o: server.cc server.h
	clang++ -std=c++11 -c server.cc

mmap_allocator.o: mmap_allocator.h mmap_allocator.cc
	clang++ -std=c++11 -c  mmap_allocator.cc

main.o: main.cc
	clang++ -std=c++11 -c main.cc

elements.o: elements.cc elements.h
	clang++ -std=c++11 -c elements.cc

server: server.o main.o elements.o mmap_allocator.o
	clang++ -std=c++11 server.o main.o elements.o mmap_allocator.o -o server

clean:
	rm -f *.o server *_test
