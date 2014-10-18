#
# Makefile
# vim: set noexpandtab list:
#
application := parallel
source_files := node.cpp main.cpp

CC := g++
DEBUG := gdb
TARGET_DIR := target

# MPI
MPI_COMPILE_FLAGS = $(shell mpic++ --showme:compile) -O0
MPI_LINK_FLAGS = $(shell mpic++ --showme:link)
MPI_RUN_COPIES=10
MPI_RUN_FLAGS =-n $(MPI_RUN_COPIES)
MPI_RUN_TERMINAL=urxvt -e
MPI_RUN_DEBUG=$(MPI_RUN_TERMINAL) gdb -ex run

COMPILE_FLAGS = -fdiagnostics-color $(MPI_COMPILE_FLAGS) -g -Wall
LINK_FLAGS = $(MPI_LINK_FLAGS) -lboost_mpi -lboost_serialization

.PHONY: run debug

$(application): $(source_files)
	mkdir -p $(TARGET_DIR)
	$(CC) $(COMPILE_FLAGS) $(source_files) $(LINK_FLAGS) -o $(TARGET_DIR)/$(application)

run: $(application)
	mpirun $(MPI_RUN_FLAGS) $(MPI_RUN_TERMINAL) ./$(TARGET_DIR)/$(application)

debug: $(application)
	mpirun $(MPI_RUN_FLAGS) $(MPI_RUN_DEBUG) ./$(TARGET_DIR)/$(application)

clean:
	rm -rf $(TARGET_DIR)

