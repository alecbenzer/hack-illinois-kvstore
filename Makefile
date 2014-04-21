CXX = clang++ -std=c++11
CXXFLAGS = -g

all: mmap_allocator_test server

mmap_allocator.o: mmap_allocator.cc mmap_allocator.h

mmap_allocator_test: mmap_allocator_test.o mmap_allocator.o
	clang++ -std=c++11 mmap_allocator_test.o mmap_allocator.o -lgtest -lgtest_main -pthread -o mmap_allocator_test

mmap_allocator_test.o: mmap_allocator_test.cc mmap_allocator.h

server.o: server.cc server.h kvstore.pb.h

main.o: main.cc

elements.o: elements.cc elements.h

kvstore.pb.cc kvstore.pb.h kvstore_pb2.py: kvstore.proto
	protoc --cpp_out=. --python_out=. kvstore.proto

kvstore.pb.o: kvstore.pb.cc kvstore.pb.h

server: server.o main.o elements.o mmap_allocator.o kvstore.pb.o
	clang++ -std=c++11 server.o main.o elements.o mmap_allocator.o kvstore.pb.o -o server -lprotobuf

clean:
	rm -f *.o server *_test
