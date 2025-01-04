#ifndef BPTREEE_H
#define BPTREEE_H
#endif

#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <pthread.h>
#include <assert.h>
#include <strings.h>
#include <math.h>
#include "debug.h"
#include <iostream>
#include <vector>
using namespace std;
#define MAX_OBJ (1000*1000)
#define N 4

typedef struct _DATA {
	int key;
	int val;
	struct _DATA *next;
} DATA;

typedef struct _NODE {
	bool isLeaf;
	struct _NODE *chi[N];
	int key[N-1]; 
	int nkey;
	struct _NODE *parent;
} NODE;

typedef struct _TEMP {
	bool isLeaf;
	NODE *chi[N+1]; // for internal split (for leaf, only N is enough)
	int key[N]; // for leaf split
	int nkey;
} TEMP;

extern NODE *Root;
extern DATA Head;
extern DATA *Tail;

void init_root();
void insert(int key, DATA *data);
NODE* find_leaf(NODE *root, int key);
void print_tree(NODE *root);
vector<int> range_query(int start, int end);
