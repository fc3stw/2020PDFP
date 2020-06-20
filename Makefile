CC=g++
LDFLAGS=-std=c++11 -O3 -lm
SOURCES=src/module.cc src/main.cc src/routingFlow.cc src/globalRouter.cc
OBJECTS=$(SOURCES:.c=.o)
EXECUTABLE=APR
INCLUDES=src/module.h src/routingFlow.h src/globalRouter.h

all: $(SOURCES) bin/$(EXECUTABLE)

bin/$(EXECUTABLE): $(OBJECTS)
	$(CC) $(LDFLAGS) $(OBJECTS) -o $@

%.o:  %.c  ${INCLUDES}
	$(CC) $(CFLAGS) $< -o $@

clean:
	rm -rf *.o bin/$(EXECUTABLE)
