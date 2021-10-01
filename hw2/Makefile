CC := g++
obj := matrix_mul.o
out := a.out

all:${obj}
	${CC} ${obj}
%.o:%.cpp
	${CC} -c $^ -o $@

.PHONY:clean
clean:
	rm ${obj} ${out} 