STUDENT_ID := 0710734
MOUNT_POINT := tarfs

FUSE_SERVER := ${STUDENT_ID}.out

CXX := g++
CXXFLAGS := -Wall `pkg-config fuse --cflags --libs`

all: ${FUSE_SERVER}

${FUSE_SERVER}: ${STUDENT_ID}.cpp
	${CXX} $^ -o $@ ${CXXFLAGS}

.PHONY: clean test run install
clean:
	rm -rf ${FUSE_SERVER} test.tar output/* *~

test: demo.sh ${FUSE_SERVER}
	./demo.sh ${FUSE_SERVER}

run: ${FUSE_SERVER} ${MOUNT_POINT}
	./${FUSE_SERVER} -f ${MOUNT_POINT}

install:
	sudo apt install libfuse-dev

uninstall:
	sudo apt remove libfuse-dev