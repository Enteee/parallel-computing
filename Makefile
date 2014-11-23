#
# Makefile
# vim: set noexpandtab list:
#

#Run configuration
RUN_COPIES=12
MODE=MST
SCENARIO=-1
ARGS=$(MODE) $(SCENARIO)

application := parallel
source_files := utils.cpp msg.cpp node.cpp main.cpp

CC=g++
DEBUG=gdb
TERMINAL_URXVT=$(shell which urxvt 2>/dev/null )
TERMINAL_KONSOLE=$(shell which konsole 2>/dev/null )
TERMINAL_XTERM=$(shell which xterm 2>/dev/null )
TARGET_DIR=target

#seed
SET_SEED=1

# MPI
MPI_COMPILE_FLAGS=$(shell mpic++ --showme:compile)
BASE_COMPILE_FLAGS=-fdiagnostics-color $(MPI_COMPILE_FLAGS) -Wall -std=c++11
COMPILE_FLAGS=$(BASE_COMPILE_FLAGS)
DEBUG_COMPILE_FLAGS=$(BASE_COMPILE_FLAGS) -O0 -DSET_SEED=$(SET_SEED) -g

MPI_LINK_FLAGS=$(shell mpic++ --showme:link)
BASE_LINK_FLAGS=$(MPI_LINK_FLAGS) -lboost_mpi -lboost_serialization -lSegFault
LINK_FLAGS=$(BASE_LINK_FLAGS)
DEBUG_LINK_FLAGS=$(BASE_LINK_FLAGS)

BASE_MPI_RUN_FLAGS=-n $(RUN_COPIES)
MPI_RUN_FLAGS=$(BASE_MPI_RUN_FLAGS)
DEBUG_MPI_RUN_FLAGS=$(BASE_MPI_RUN_FLAGS)

ifneq ($(TERMINAL_URXVT),)
BASE_MPI_RUN_TERMINAL=$(TERMINAL_URXVT) --hold -e
else ifneq ($(TERMINAL_KONSOLE),)
BASE_MPI_RUN_TERMINAL=$(TERMINAL_KONSOLE) --noclose -e
else ifneq ($(TERMINAL_XTERM),)
BASE_MPI_RUN_TERMINAL=$(TERMINAL_XTERM) -hold -e
else
BASE_MPI_RUN_TERMINAL=
endif
MPI_RUN_TERMINAL=$(BASE_MPI_RUN_TERMINAL)
DEBUG_MPI_RUN_TERMINAL=$(BASE_MPI_RUN_TERMINAL)

BASE_MPI_RUN_APPLICATION=./$(TARGET_DIR)/$(application)
MPI_RUN_APPLICATION=$(BASE_MPI_RUN_APPLICATION)
DEBUG_MPI_RUN_APPLICATION=$(DEBUG) -q -ex 'run $(ARGS)' $(BASE_MPI_RUN_APPLICATION)

BASE_MPI_RUN_ARGS=$(ARGS)
MPI_RUN_ARGS=$(BASE_MPI_RUN_ARGS)
DEBUG_MPI_RUN_ARGS=
HELP_MPI_RUN_ARGS= --help

.PHONY: debug run rundebug help

$(application): $(source_files)
	mkdir -p $(TARGET_DIR)
	$(CC) $(COMPILE_FLAGS) $(source_files) $(LINK_FLAGS) -o $(TARGET_DIR)/$(application)

run:
	mpirun $(MPI_RUN_FLAGS) $(MPI_RUN_TERMINAL) $(MPI_RUN_APPLICATION) $(MPI_RUN_ARGS)

debug: COMPILE_FLAGS=$(DEBUG_COMPILE_FLAGS)
debug: LINK_FLAGS=$(DEBUG_LINK_FLAGS)
debug: MPI_RUN_FLAGS=$(DEBUG_MPI_RUN_FLAGS)
debug: MPI_RUN_TERMINAL=$(DEBUG_MPI_RUN_TERMINAL)
debug: MPI_RUN_APPLICATION=$(DEBUG_MPI_RUN_APPLICATION)
debug: MPI_RUN_ARGS=$(DEBUG_MPI_RUN_ARGS)
debug: $(application)

rundebug: MPI_RUN_FLAGS=$(DEBUG_MPI_RUN_FLAGS)
rundebug: MPI_RUN_TERMINAL=$(DEBUG_MPI_RUN_TERMINAL)
rundebug: MPI_RUN_APPLICATION=$(DEBUG_MPI_RUN_APPLICATION)
rundebug: MPI_RUN_ARGS=$(DEBUG_MPI_RUN_ARGS)
rundebug: run

help: RUN_COPIES=1
help: MPI_RUN_ARGS=$(HELP_MPI_RUN_ARGS)
help: run

clean:
	rm -rf $(TARGET_DIR)

