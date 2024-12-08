@echo off
if not exist obj mkdir obj
gcc -Wall -g -I./include -c src/project.c -o obj/project.o
gcc -Wall -g -I./include -c src/lval.c -o obj/lval.o
gcc -Wall -g -I./include -c src/lenv.c -o obj/lenv.o
gcc -Wall -g -I./include -c src/builtins.c -o obj/builtins.o
gcc -Wall -g -I./include -c src/mpc.c -o obj/mpc.o
gcc obj/project.o obj/lval.o obj/lenv.o obj/builtins.o obj/mpc.o -o daylo.exe