CXX := g++ 
CXXFLAGS := -Wall

all: main

main: main.o
	${CXX} ${CXXFLAGS} $^ -o $@

%.o: %.cpp
	${CXX} ${CXXFLAGS} -c $^ -o $@

.PHONY: clean test
clean:
	rm -rf main *.o *~

test: test_whois test_zipf

test_whois: main whois.txt
	./main whois.txt

test_zipf: main zipf.txt
	./main zipf.txt
