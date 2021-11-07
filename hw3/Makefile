CC := g++
OBJ := merge.o
TARGET := a.out
CCFLAGS := -pthread

all: ${TARGET}

${TARGET}: ${OBJ}
	${CC} ${CCFLAGS} ${OBJ} -o ${TARGET}

%.o: %.cpp
	${CC} ${CCFLAGS} -c $^ -o $@

.PHONY:clean test
clean:
	rm ${OBJ} ${TARGET}

test: ${TARGET}
	./${TARGET}