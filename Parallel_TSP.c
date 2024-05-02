//Muhammad Shayaan Nofil
//i21-0450
//Assignment #2

#include <stdio.h>
#include <stdlib.h>
#include <time.h>
#include <mpi.h>
#include <math.h>

#define NUM_VERTICES 12

struct node {
    int data;
    char name;
    struct node* next;
};

struct node* createNode(int dt, char nam){
    struct node* temp = malloc(sizeof(struct node));
    temp->data = dt;
    temp->name = nam;
    temp->next = NULL;

    return temp;
}

struct list {
    struct node* head;
    int num;
};

void addnode(struct list* lt, int dt, char nam){

    if (lt->num == 0){
        lt->head = createNode(dt, nam);
        lt->num += 1;
    }
    else{
        struct node* temp = lt->head;

        while (temp->next != NULL){
            temp = temp->next;
        }

        temp->next = createNode(dt, nam);
        lt->num += 1;
    }    
}

void addnode2(struct list* lt, struct node* nd){

    if (lt->num == 0){
        lt->head = nd;
        lt->num += 1;
    }
    else{
        struct node* temp = lt->head;

        while (temp->next != NULL){
            temp = temp->next;
        }

        temp->next = nd;
        lt->num += 1;
    }    
}

void popnode(struct list* lt){
    if (lt->num > 0){
        struct node* temp = lt->head;
        struct node* last = malloc(sizeof(struct node));
        
        while (temp->next != NULL){
            last = temp;
            temp = temp->next;
        }
        last->next = NULL;
        lt->num -= 1;
        free(temp);
    }
}

void pophead(struct list* lt){
    struct node* temp = lt->head;

    if (lt->num > 1){
        if (temp->next != NULL){
            temp = temp->next;
        }
        if (temp->next != NULL){
            struct node* temp2 = temp->next;
            free(temp);
            lt->head->next = temp2;
        }
    }
}

int searchlist(struct list* lt, char nam){
    struct node* temp = lt->head;

    while (temp->next != NULL){
        if (temp->name == nam){
            return 1;
        }
        temp = temp->next;
    }
    if (temp->name == nam){
        return 1;
    }
    return 0;
}

int getlistlength(struct list* lt){
    struct node* temp = lt->head;
    int i = 0;

    while (temp->next != NULL){
        temp = temp->next;
        i += 1;
    }

    return i + 1;
}

struct list* copylist(struct list* lt1, struct list* lt2){
    struct node* temp = lt2->head;
    free(lt1);
    lt1 = malloc(sizeof(struct list));

    while (temp->next != NULL){
        addnode(lt1, temp->data, temp->name);
        temp = temp->next;
    }
    addnode(lt1, temp->data, temp->name);

    return lt1;
}

void printlist (struct list* lt){

    if (lt->num > 0){
        struct node* temp = lt->head;

        while (temp->next != NULL){
            printf("(%c,%d), ", temp->name, temp->data);
            temp = temp->next;
        }
        printf("(%c,%d)\n", temp->name, temp->data);
    }
}

struct graph{
    struct list* lt;
};

struct graph PopulateGraph(){
    struct graph gr;

    gr.lt = malloc(sizeof(struct list) * NUM_VERTICES);

    for (int i = 0; i < NUM_VERTICES; i++){
        addnode(&gr.lt[i], 0, i+65);
        for (int j = 0; j < NUM_VERTICES; j++){
            if (j != i){
                addnode(&gr.lt[i], (rand()%20 + 1), j+65);
            }
        }
        //printlist(&gr.lt[i]);
    }

    return gr;
}

void printgraph (struct graph gr){
    for (int i = 0; i < NUM_VERTICES; i++){
        printlist(&gr.lt[i]);
    }
}


int searchvert (struct graph gr, char vert){
    for (int i = 0; i < NUM_VERTICES; i++){
        if (gr.lt[i].head->name == vert){
            return i;
        }
    }
    return -1;
}

void TSP_serial(struct graph gr, struct node* nd, int am, struct list* visited, int* min, struct list* minlist){
    addnode(visited, nd->data, nd->name);

    struct node* temp = gr.lt[searchvert(gr, nd->name)].head;

    while (temp->next != NULL){
        if (temp->name != nd->name && searchlist(visited, temp->name) == 0){
            TSP_serial(gr, temp, (am + temp->data), visited, min, minlist);
        }
        temp = temp->next;
    }
    if (temp->name != nd->name && searchlist(visited, temp->name) == 0){
        TSP_serial(gr, temp, (am + temp->data), visited, min, minlist);
    }


    if (getlistlength(visited) == NUM_VERTICES){
        temp = gr.lt[searchvert(gr, nd->name)].head;

        while (temp->next != NULL){
            if (temp->name == visited->head->name){
                addnode(visited, temp->data, temp->name);
                am += temp->data;
            }
            temp = temp->next;
        }

        if (am <= *min){
            *min = am;
            minlist = copylist(minlist, visited);
        }
        popnode(visited);
    }
    popnode(visited);
}


int main(int argc,char*argv[]){
    srand(time(0));

    struct graph gr = PopulateGraph();

    struct list *visited, *minimum = malloc(sizeof(struct list));
    visited = malloc(sizeof(struct list));
    int *min = malloc(sizeof(int));
    *min = 100000;

    int i, rank, size;
	MPI_Status status;
	int root = 0;
	MPI_Init(&argc, &argv) ;
	MPI_Comm_size (MPI_COMM_WORLD,&size);
	MPI_Comm_rank (MPI_COMM_WORLD,&rank);

    clock_t begin = clock();

    int endnum = NUM_VERTICES/size;
    if (NUM_VERTICES%size != 0){
        endnum += 1;
    }
    struct node* templist1 = gr.lt[0].head;
    struct list templist;

    addnode(&templist, templist1->data, templist1->name);
    if (templist1->next != NULL){
        templist1 = templist1->next;
    }
    struct node* last = malloc(sizeof(struct node));
    for (int i = 1; i <= NUM_VERTICES - 1; i++){
        if (rank == i/endnum){
            if (templist1->name != last->name){
                addnode(&templist, templist1->data, templist1->name);
            }
        }
        if (templist1->next != NULL){
            last = templist1;
            templist1 = templist1->next;
        }
    }

    gr.lt[0] = templist;
    // printf("%d\n", rank);
    // printlist(&gr.lt[0]);
    
    TSP_serial(gr, gr.lt[0].head, 0, visited, min, minimum);

    int recmin = 0;
    MPI_Reduce(min, &recmin, 1, MPI_INT, MPI_MIN, 0, MPI_COMM_WORLD);
    MPI_Bcast(&recmin, 1, MPI_INT, 0, MPI_COMM_WORLD);


    if (recmin == *min){
        printf("Minimum path: ");
        printlist(minimum);
        printf("Minimum cost: %d\n", *min);

        clock_t end = clock();
        double time_spent = (double)(end - begin) / CLOCKS_PER_SEC;
        printf("Time spent: %f\n", time_spent);
    }

    MPI_Finalize();
}