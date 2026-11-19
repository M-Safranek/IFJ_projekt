all: compiler

compiler: compiler.c
	gcc -o compiler compiler.c codegen.c semantics.c parser.c Scanner.c symtable_stack.c symtable.c -Wall -Wextra