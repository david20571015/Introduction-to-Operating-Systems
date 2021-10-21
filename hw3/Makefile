CC := g++
OBJ := merge.o
TARGET := a.out
CCFLAGS := -pthread

all:${OBJ}
	${CC} ${CCFLAGS} ${OBJ} -o ${TARGET}
%.o:%.cpp
	${CC} ${CCFLAGS} -c $^ -o $@

.PHONY:clean
clean:
	rm ${OBJ} ${TARGET} 