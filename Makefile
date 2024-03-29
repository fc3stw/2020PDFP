CC=g++
LDFLAGS=-std=c++11 -O3 -lm
SOURCES=src/module.cc src/main.cc src/routingFlow.cc src/globalRouter.cc src/routingGraph.cc src/placement.cc
OBJECTS=$(SOURCES:.c=.o)
EXECUTABLE=cell_move_router
INCLUDES=src/module.h src/routingFlow.h src/globalRouter.h src/routingGraph.h src/placement.h

all: $(SOURCES) bin/$(EXECUTABLE)

bin/$(EXECUTABLE): $(OBJECTS)
	$(CC) $(LDFLAGS) $(OBJECTS) -o $@

%.o:  %.c  ${INCLUDES}
	$(CC) $(CFLAGS) $< -o $@

clean:
	rm -rf *.o bin/$(EXECUTABLE)
