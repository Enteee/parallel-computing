#
# Makefile
# vim: set noexpandtab list:
#
application := parallel
source_files := main.cpp

CC := g++
TARGET_DIR := target
MPI_COMPILE_FLAGS = $(shell mpic++ --showme:compile)
MPI_LINK_FLAGS = $(shell mpic++ --showme:link)

MPI_RUN_COPIES=6

.PHONY: run

$(application): $(source_files)
	mkdir -p $(TARGET_DIR)
	$(CC) $(MPI_COMPILE_FLAGS) $(source_files)  $(MPI_LINK_FLAGS) -o $(TARGET_DIR)/$(application)

run: $(application)
	mpirun -n $(MPI_RUN_COPIES) ./$(TARGET_DIR)/$(application)

clean:
	rm -rf $(TARGET_DIR)

