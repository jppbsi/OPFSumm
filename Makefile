LIB=./lib
INCLUDE=./include
SRC=./src
OBJ=./obj
UTIL=util

CC=gcc

FLAGS=  -O3 -Wall


INCFLAGS = -I$(INCLUDE) -I$(INCLUDE)/$(UTIL)

all: libOPF opfsumm

libOPF: libOPF-build
	echo "libOPF.a built..."

libOPF-build: \
util \
$(OBJ)/OPF.o \

	ar csr $(LIB)/libOPF.a \
$(OBJ)/common.o \
$(OBJ)/set.o \
$(OBJ)/gqueue.o \
$(OBJ)/realheap.o \
$(OBJ)/sgctree.o \
$(OBJ)/subgraph.o \
$(OBJ)/OPF.o \
$(OBJ)/summtools.o \
$(OBJ)/videosumm.o \

$(OBJ)/OPF.o: $(SRC)/OPF.c
	$(CC) $(FLAGS) -c $(SRC)/OPF.c $(INCFLAGS) \
	-o $(OBJ)/OPF.o

opfsumm: libOPF
	$(CC) $(FLAGS) $(INCFLAGS) src/opfsumm.c  -L./lib -o bin/opfsumm -lOPF -lm

util: $(SRC)/$(UTIL)/common.c $(SRC)/$(UTIL)/set.c $(SRC)/$(UTIL)/gqueue.c $(SRC)/$(UTIL)/realheap.c $(SRC)/$(UTIL)/sgctree.c $(SRC)/$(UTIL)/subgraph.c $(SRC)/$(UTIL)/summtools.c $(SRC)/$(UTIL)/videosumm.c
	$(CC) $(FLAGS) $(INCFLAGS) -c $(SRC)/$(UTIL)/common.c -o $(OBJ)/common.o
	$(CC) $(FLAGS) $(INCFLAGS) -c $(SRC)/$(UTIL)/set.c -o $(OBJ)/set.o
	$(CC) $(FLAGS) $(INCFLAGS) -c $(SRC)/$(UTIL)/gqueue.c -o $(OBJ)/gqueue.o
	$(CC) $(FLAGS) $(INCFLAGS) -c $(SRC)/$(UTIL)/realheap.c -o $(OBJ)/realheap.o
	$(CC) $(FLAGS) $(INCFLAGS) -c $(SRC)/$(UTIL)/sgctree.c -o $(OBJ)/sgctree.o
	$(CC) $(FLAGS) $(INCFLAGS) -c $(SRC)/$(UTIL)/subgraph.c -o $(OBJ)/subgraph.o
	$(CC) $(FLAGS) $(INCFLAGS) -c $(SRC)/$(UTIL)/summtools.c -o $(OBJ)/summtools.o
	$(CC) $(FLAGS) $(INCFLAGS) -c $(SRC)/$(UTIL)/videosumm.c -o $(OBJ)/videosumm.o


## Compiling LibOPF with LibIFT

opf-ift: libOPF-ift

libOPF-ift: libOPF-ift-build
	echo "libOPF.a built with IFT..."

libOPF-ift-build: \
OPF-ift.o \

	ar csr $(LIB)/libOPF.a \
$(OBJ)/OPF.o \

OPF-ift.o: $(SRC)/OPF.c
	$(CC) $(FLAGS) -c $(SRC)/OPF.c -I$(INCLUDE) -I$(IFT_DIR)/include \
	-o $(OBJ)/OPF.o

## Cleaning-up

clean:
	rm -f $(LIB)/lib*.a; rm -f $(OBJ)/*.o bin/opfsumm

#clean_results:
#	rm -f *.out *.opf *.acc *.time *.opf training.dat evaluating.dat testing.dat

#clean_results_in_examples:
#	rm -f examples/*.out examples/*.opf examples/*.acc examples/*.time examples/*.opf examples/training.dat examples/evaluating.dat examples/#testing.dat


