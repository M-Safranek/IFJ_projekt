CC = gcc
CFLAGS = -std=c11 -O2 -Wall -Wextra
CPPFLAGS =
LDFLAGS =
LDLIBS =

TARGET = ifj26compiler

SOURCES = $(wildcard compiler.c parser.c Scanner.c scanner.c semantics.c codegen.c symtable.c symtable_stack.c)
OBJECTS = $(SOURCES:.c=.o)

.PHONY: all build clean rebuild
all: build
build: $(TARGET)

$(TARGET): $(OBJECTS)
	$(CC) $(LDFLAGS) -o $@ $(OBJECTS) $(LDLIBS)

%.o: %.c
	$(CC) $(CPPFLAGS) $(CFLAGS) -c $< -o $@

clean: rm -f $(OBJECTS) $(TARGET)

rebuild: clean build
