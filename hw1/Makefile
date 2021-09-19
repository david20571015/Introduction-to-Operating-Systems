CC := gcc
obj := shell.o
out := a.out

all:${obj}
	${CC} ${obj}
%.o:%.c
	${CC} -c $^ -o $@

.PHONY:clean
clean:
	rm ${obj} ${out} 