ex5: mem_sim.o main.o
	gcc -Wall mem_sim.o main.o -o ex5

mem_sim: mem_sim.c mem_sim.h
	gcc -Wall mem_sim.c -o mem_sim.o

main: main.c mem_sim.h
	gcc -Wall main.c -o main.o
	

