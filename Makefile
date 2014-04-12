server.o: server.cc
	clang++ -std=c++11 -stdlib=libc++ -c server.cc server.h

main.o: main.cc
	clang++ -std=c++11 -stdlib=libc++ -c main.cc

clean:
	-rm *.o
