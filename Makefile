CC=g++
LDFLAGS=-std=c++11 -O3 -lm
SOURCES=src/main.cpp src/Parser.cpp src/algo.cpp src/util_bin.cpp src/util_cost.cpp src/util_export.cpp src/util_feasible.cpp src/util_init.cpp src/util_location.cpp src/util_slack.cpp
OBJECTS=$(SOURCES:.c=.o)
EXECUTABLE=cadb_0040_final
INCLUDES=src/module.h src/algo.h

all: $(SOURCES) $(EXECUTABLE)

$(EXECUTABLE): $(OBJECTS)
	$(CC) $(LDFLAGS) $(OBJECTS) -o $@

%.o:  %.c  ${INCLUDES}
	$(CC) $(CFLAGS) $< -o $@

clean:
	rm -rf *.o $(EXECUTABLE)
