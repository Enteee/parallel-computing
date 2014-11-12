#
# Makefile
# vim: set noexpandtab list:
#
application := parallel
source_files := utils.cpp msg.cpp node.cpp main.cpp

CC=g++
DEBUG=gdb
TERMINAL=urxvt
TARGET_DIR=target

#seed
SET_SEED=1

#Run
MODE="MST"

# MPI
MPI_COMPILE_FLAGS=$(shell mpic++ --showme:compile) -std=c++11 -O0
MPI_LINK_FLAGS=$(shell mpic++ --showme:link)
MPI_RUN_COPIES=3
MPI_RUN_FLAGS=-n $(MPI_RUN_COPIES)
MPI_RUN_TERMINAL=$(TERMINAL) --hold -e
MPI_RUN_DEBUG=$(MPI_RUN_TERMINAL) $(DEBUG) -q -ex run

COMPILE_FLAGS:=-fdiagnostics-color $(MPI_COMPILE_FLAGS) -Wall
DEBUG_COMPILE_FLAGS:=$(COMPILE_FLAGS) -DSET_SEED=$(SET_SEED) -g
LINK_FLAGS:=$(MPI_LINK_FLAGS) -lboost_mpi -lboost_serialization

.PHONY: run debug

$(application): $(source_files)
	mkdir -p $(TARGET_DIR)
	$(CC) $(COMPILE_FLAGS) $(source_files) $(LINK_FLAGS) -o $(TARGET_DIR)/$(application)

run: $(application)
	mpirun $(MPI_RUN_FLAGS) $(MPI_RUN_TERMINAL) ./$(TARGET_DIR)/$(application) $(MODE)

debug: COMPILE_FLAGS = $(DEBUG_COMPILE_FLAGS)
debug: $(application)
	mpirun $(MPI_RUN_FLAGS) $(MPI_RUN_DEBUG) ./$(TARGET_DIR)/$(application) $(MODE)

clean:
	rm -rf $(TARGET_DIR)

