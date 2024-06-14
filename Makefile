CFLAGS += -Wall -Wextra -Wpedantic
CFLAGS += -Wvla -Waggregate-return -Wfloat-equal -Wwrite-strings
CFLAGS += -std=c18

# Identifies all dependencies / macros/ and libraries required for building
# zergmap.
zergmap: zergmap.o graph.o map.o priority-queue.o set.o quadTree.o pathfinding.o zergmap-driver.o
zergmap: LDLIBS += -lm
zergmap.o: CPPFLAGS += -D_DEFAULT_SOURCE -D_POSIX_C_SOURCE=200809L
zergmap.o: zergmap_func.h

# Identifies all dependencies for creating graph.o.
graph.o: graph.h graph.c
graph.o: LDLIBS += -lm

map.o: map.h map.c

priority-queue.o: priority-queue.c priority-queue.h

set.o: set.h set.c

quadTree.o: quadTree.h quadTree.c

pathfinding.o: pathfinding.h pathfinding.c

.PHONY: debug
debug: CFLAGS += -g
debug: zergmap

# Runs automated unit tests.
.PHONY: check
check: test/test-run
	./test/test-run

# Runs automated integration tests against all PCAPs in easy_3n0r folder
# and evaluates zergmap return codes. 
.PHONY: check-test-1
check-test-1:
	bash -c "cd test; ./test-case-1-easy_3n0r.sh;"

# Runs zergmap against all PCAPs in easy_3n1r folder and evaluates zergmap
# return codes.
.PHONY: check-test-2
check-test-2:
	bash -c "cd test; ./test-case-2-easy_3n1r.sh;"

# Runs zergmap against all PCAPs in easy_5nxx folder and evaluates zergmap
# return codes.
.PHONY: check-test-3
check-test-3:
	bash -c "cd test; ./test-case-3-easy_5nxx.sh;"

# Runs zergmap against all PCAPs in easy_v6_2n0r folder and evaluates zergmap
# return codes.
.PHONY: check-test-4
check-test-4:
	bash -c "cd test; ./test-case-4-easy_v6_2n0r.sh;"

# Runs zergmap against all PCAPs in med_4n1r folder and evaluates zergmap
# return codes.
.PHONY: check-test-5
check-test-5:
	bash -c "cd test; ./test-case-5-med_4n1r.sh;"

# Runs zergmap against all PCAPs in hard_6n0r folder and evaluates zergmap
# return codes.
.PHONY: check-test-6
check-test-6:
	bash -c "cd test; ./test-case-6-hard_6n0r.sh;"

# Runs zergmap against all sample PCAPs individually and then runs
# zergmap -h 99 with every sample PCAP together, followed by evaluating
# zergmap return codes.
.PHONY: check-test-7
check-test-7:
	bash -c "cd test; ./test-case-7-all-pcaps.sh;"

# Runs valgrind against zergmap for all PCAP files and other cases
# e.g. no file, invalid file, invalid file extension and evaluates
# valgrind return codes. Also evaluates valgrind ran against test/test-run
# and evaluates valgrind return codes.
.PHONY: check-valgrind
check-valgrind: test/test-run
	bash -c "cd test; ./test-valgrind-zergmap.sh;"
	./test/test-run

.PHONY: profile
profile: CPPFLAGS += -pg
profile: LDLIBS += -pg
profile: zergmap

# Identifies all dependencies / macros/ and libraries required for building
# test/test-run.
test/test-run: LDLIBS += -lcheck -lm -lrt -lpthread -lsubunit
test/test-run: LDLIBS += -lcrypto
test/test-run: CFLAGS += -g
test/test-run: CPPFLAGS += -I.
test/test-run: test/test-run.o test/test-zergmap.o zergmap.o test/test-graph.o test/test-map.o test/test-pqueue.o graph.o pathfinding.o set.o map.o priority-queue.o test/test-quadTree.o quadTree.o
test/test-run: LDFLAGS += -L.
test/test-run: CPPFLAGS += -D_POSIX_C_SOURCE=200809L -D_DEFAULT_SOURCE

.PHONY: clean
clean:
	$(RM) zergmap test/test-run *.o test/*.o
