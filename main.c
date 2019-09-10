#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include "mem_sim.h"

int main()
{
    sim_database *db=init_system("exec_file","swap_file",40,40);
    char a,b,c,d,e,f,g,z,h;
    printf("Adding some values to memory:\n");
    print_memory(db);
    store(db,40,'A');
    print_memory(db);
    store(db,44,'B');
    print_memory(db);
    store(db,48,'C');
    print_memory(db);
    store(db,52,'D');
    print_memory(db);
    store(db,56,'E');
    print_memory(db);
    store(db,60,'F');
    print_memory(db);
    store(db,64,'G');
    print_memory(db);
    store(db,68,'H');
    print_memory(db);
    printf("\n");
    
    a=load(db,40);
    b=load(db,44);
    c=load(db,48);
    d=load(db,56);
    e=load(db,48);
    f=load(db,68);
    g=load(db,60);
    z=load(db,19);
    h=load(db,23);
    print_memory(db);
    printf("\n");
    printf("\n");
    print_swap(db);
    printf("\n");
    print_page_table(db);
    printf("\n");
   
    printf("Loaded value is: %c\n", a);
    printf("Loaded value is: %c\n", b);
    printf("Loaded value is: %c\n", c);
    printf("Loaded value is: %c\n", d);
    printf("Loaded value is: %c\n", e);
    printf("Loaded value is: %c\n", f);
    printf("Loaded value is: %c\n", g);
    printf("Loaded value is: %c\n", z);
    printf("Loaded value is: %c\n", h);
    clear_system(db);
    return EXIT_SUCCESS;
}
