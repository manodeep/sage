SRC := main.c core_read_parameter_file.c core_init.c core_io_tree.c \
       core_cool_func.c core_build_model.c core_save.c core_mymalloc.c \
       core_allvars.c model_infall.c model_cooling_heating.c model_starformation_and_feedback.c \
       model_disk_instability.c model_reincorporation.c model_mergers.c model_misc.c \
       io/tree_binary.c io/tree_hdf5.c
SRC  := $(addprefix src/, $(SRC))
OBJS := $(SRC:.c=.o)
INCL := core_allvars.h core_proto.h core_simulation.h io/tree_binary.h io/tree_hdf5.h Makefile
INCL := $(addprefix src/, $(INCL))

EXEC := sage 

# USE-MPI = yes  # set this if you want to run in embarrassingly parallel
USE-HDF5 = yes

LIBS =
CFLAGS =
OPTS =

ifdef USE-MPI
    OPT += -DMPI  #  This creates an MPI version that can be used to process files in parallel
    CC ?= mpicc  # sets the C-compiler
else
    CC ?= gcc  # sets the C-compiler
endif

ifdef USE-HDF5
    HDF5DIR := usr/local/x86_64/gnu/hdf5-1.8.17-openmpi-1.10.2-psm
    HDF5INCL := -I$(HDF5DIR)/include
    HDF5LIB := -L$(HDF5DIR)/lib -lhdf5 -Xlinker -rpath -Xlinker $(HDF5DIR)/lib

    OPT += -DHDF5
    LIBS += $(HDF5LIB)
    CFLAGS += $(HDF5INCL) 
endif

GITREF = -DGITREF_STR='"$(shell git show-ref --head | head -n 1 | cut -d " " -f 1)"'

# GSL automatic detection
GSL_FOUND := $(shell gsl-config --version 2>/dev/null)
ifndef GSL_FOUND
  $(warning GSL not found in path - please install GSL before installing SAGE (or, update the PATH environment variable such that "gsl-config" is found))
  # if the automatic detection fails, set GSL_DIR appropriately
  GSL_DIR := /opt/local
  GSL_INCL := -I$(GSL_DIR)/include  
  GSL_LIBDIR := $(GSL_DIR)/lib
  # since GSL is not in PATH, the runtime environment might not be setup correctly either
  # therefore, adding the compiletime library path is even more important (the -Xlinker bit)
  GSL_LIBS := -L$(GSL_LIBDIR) -lgsl -lgslcblas -Xlinker -rpath -Xlinker $(GSL_LIBDIR) 
else
  # GSL is probably configured correctly, pick up the locations automatically
  GSL_INCL := $(shell gsl-config --cflags)
  GSL_LIBDIR := $(shell gsl-config --prefix)/lib
  GSL_LIBS   := $(shell gsl-config --libs) -Xlinker -rpath -Xlinker $(GSL_LIBDIR)
endif

OPTIMIZE = -g -O3 -march=native -Wextra -Wshadow -Wall #-Wpadded # optimization and warning flags

LIBS   +=   -g -lm  $(GSL_LIBS) 
CFLAGS +=   $(OPTIONS) $(OPT) $(OPTIMIZE) $(GSL_INCL)


default: all

$(EXEC): $(OBJS) 
	$(CC) $(CFLAGS) $(OBJS) $(LIBS)   -o  $(EXEC)

%.o: %.c $(INCL)
	$(CC) $(CFLAGS) -c $< -o $@

.phony: clean celan celna clena

clean:
	rm -f $(OBJS) $(EXEC)

celan celna clena: clean

tidy:
	rm -f $(OBJS) ./$(EXEC)

all:  $(EXEC) 

