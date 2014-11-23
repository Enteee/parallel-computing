parallel-computing
==================

Parallel computing project for school

## Requirments 

* a compatible terminal emulator installed:
 * urxvt
 * konsole
 * xterm
* g++
* gdb
* mpi library installed
* boost installed

## Compile

### Normal

```bash
$ make
```

### Debug

```bash
$ make debug
```

## Run

### Normal

```bash
$ make run
```

### Debug

```bash
$ make rundebug
```

## Run Configuration

Change the options under '#Run configuration' in the Makefile.

```make
#Run configuration
RUN_COPIES=12
MODE=MST
SCENARIO=1
[...]
```

Use:
```bash
$ make help
```
to get an overview of possible options.

## BUGS
