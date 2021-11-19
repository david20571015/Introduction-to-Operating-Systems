CC := gcc 
CCFLAGS := -Wall

all: main bf.so ff.so

main: main.o
	${CC} ${CCFLAGS} main.o -o main

%.o: %.c
	${CC} ${CCFLAGS} -c $^ -o $@

%.so: %.c
	${CC} ${CCFLAGS} -shared -fPIC $^ -o $@

.PHONY:clean test test_bf test_ff
clean:
	rm -rf main *.o *.so *~

test: main
	./main

test_bf: main bf.so 
	LD_PRELOAD=./bf.so ./main

test_ff: main ff.so 
	LD_PRELOAD=./ff.so ./main