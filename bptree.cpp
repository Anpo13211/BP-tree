#include "bptree.h"
#include <vector>
#include <sys/time.h>
#include <algorithm>
using namespace std;

struct timeval
cur_time(void)
{
	struct timeval t;
	gettimeofday(&t, NULL);
	return t;
}

void
print_tree_core(NODE *n)
{
	printf("["); 
	for (int i = 0; i < n->nkey; i++) {
		if (!n->isLeaf) print_tree_core(n->chi[i]); 
		printf("%d", n->key[i]); 
		if (i != n->nkey-1 && n->isLeaf) putchar(' ');
	}
	if (!n->isLeaf) print_tree_core(n->chi[n->nkey]);
	printf("]");
}

void
print_tree(NODE *node)
{
	print_tree_core(node);
	printf("\n"); fflush(stdout);
}

NODE *
find_leaf(NODE *node, int key)
{
	int kid;

	if (node->isLeaf) return node;
	for (kid = 0; kid < node->nkey; kid++) {
		if (key < node->key[kid]) break;
	}

	return find_leaf(node->chi[kid], key);
}

NODE *
insert_in_leaf(NODE *leaf, int key, DATA *data)
{
	int i;
	if (key < leaf->key[0]) {
		for (i = leaf->nkey; i > 0; i--) {
			leaf->chi[i] = leaf->chi[i-1] ;
			leaf->key[i] = leaf->key[i-1] ;
		} 
		leaf->key[0] = key;
		leaf->chi[0] = (NODE *)data;
	} else {
		for (i = 0; i < leaf->nkey; i++) {
			if (key < leaf->key[i]) break;
		}
		for (int j = leaf->nkey; j > i; j--) {		
            // j - 1 のデータとキーが j の位置に移動されます(右シフトs)。
			leaf->chi[j] = leaf->chi[j-1] ; 
			leaf->key[j] = leaf->key[j-1] ;
		} 
        leaf->key[i] = key;
        leaf->chi[i] = (NODE *)data;
	}
	leaf->nkey++;

	return leaf;
}

NODE * 
insert_in_parent(NODE *n, int key, NODE* N_prime) 
{
    // Check if N is the root
    if (n == Root) {
        NODE* R = new NODE();
        R->key[0] = key;
        R->chi[0] = n;
        R->chi[1] = N_prime;
        R->nkey = 1;  // Only one key in the new root
        R->isLeaf = false;
        n->parent = R;
        N_prime->parent = R;
        Root = R;
        return R;
    }

    NODE *P = n->parent;
    int i, j;

    // Check if parent has space for new pointer
    if (P->nkey < N-1) {
        for (i = 0; i < P->nkey; i++) 
            if (key < P->key[i]) 
                break;

        for (j = P->nkey; j > i; j--) {		
            P->chi[j+1] = P->chi[j];
            P->key[j] = P->key[j-1];
        } 

        P->key[i] = key;
        P->chi[i+1] = N_prime;
        P->nkey++;

        return P;
    } else { 
        NODE *T = new NODE();
        j = 0;

        for (i = 0; i < P->nkey; i++, j++) {
            if (j == i && key < P->key[i]) {
                T->key[j] = key;
                T->chi[j+1] = N_prime;
                j++;
            }

            T->key[j] = P->key[i];
            T->chi[j+1] = P->chi[i+1];
        }

        if (j == P->nkey) {
            T->key[j] = key;
            T->chi[j+1] = N_prime;
        }

        NODE* P_prime = new NODE();
        int K_prime = T->key[(N+1)/2];

        P->nkey = (N-1)/2;
        P_prime->nkey = (N-1) - P->nkey;

        for (i = 0; i < P->nkey; i++) {
            P->key[i] = T->key[i];
            P->chi[i] = T->chi[i];
        }
        P->chi[P->nkey] = T->chi[P->nkey];

        for (i = 0, j = (N+1)/2; j < N; i++, j++) {
            P_prime->key[i] = T->key[j];
            P_prime->chi[i] = T->chi[j];
        }
        P_prime->chi[i] = T->chi[j];

        for (i = 0; i <= P_prime->nkey; i++) {
            P_prime->chi[i]->parent = P_prime;
        }

        delete T;

        return insert_in_parent(P, K_prime, P_prime);
    }
}

NODE *
alloc_leaf(NODE *parent)
{
	NODE *node;
	if (!(node = (NODE *)calloc(1, sizeof(NODE)))) ERR;
	node->isLeaf = true;
	node->parent = parent;
	node->nkey = 0;

	return node;
}

void 
insert(int key, DATA *data)
{
	NODE *leaf;

	if (Root == NULL) {
		leaf = alloc_leaf(NULL);
		Root = leaf;
	} else leaf = find_leaf(Root, key);
    
	if (leaf->nkey < (N-1)) insert_in_leaf(leaf, key, data); 
    else 
    { 
        NODE *new_leaf = alloc_leaf(leaf->parent);

        NODE *temp = new NODE();
        temp->nkey = leaf->nkey;
        int i, j;

        for (int i = 0; i < N - 1; i++) {
            temp->key[i] = leaf->key[i];
            temp->chi[i] = leaf->chi[i];
        }
        temp->chi[N-1] = leaf->chi[N-1];

        for (i = 0; i < N-1; i++) {
            if (temp->key[i] == 0 || key < temp->key[i]) break;
        }
        for (j = N-1; j > i; j--) {
            temp->key[j] = temp->key[j-1];
            temp->chi[j] = temp->chi[j-1];
        }
        temp->key[i] = key;
        if (i == N-1) temp->chi[N-1] = (NODE *)data;
        else temp->chi[i+1] = (NODE *)data;

        temp->chi[N-1] = leaf->chi[N-1];
        leaf->chi[N-1] = temp;
        
        int split = (N+1)/2;
        leaf->nkey = split;
        for (i = 0; i < split; i++) {
            leaf->key[i] = temp->key[i];
            leaf->chi[i] = temp->chi[i];
        }
        leaf->chi[split] = temp->chi[split];

        new_leaf->nkey = N - split;
        for (i = split, j = 0; i < N; i++, j++) {
            new_leaf->key[j] = temp->key[i];
            new_leaf->chi[j] = temp->chi[i];
        }
        new_leaf->chi[j] = temp->chi[i];

        int* ptr_to_smallest = min_element(new_leaf->key, new_leaf->key + new_leaf->nkey);
        int smallest = *ptr_to_smallest;
        insert_in_parent(leaf, smallest, new_leaf);
	}
}

void
init_root(void)
{
	Root = NULL;
}

int 
interactive()
{
  int key;

  std::cout << "Key: ";
  std::cin >> key;

  return key;
}

int
main(int argc, char *argv[])
{
  struct timeval begin, end;

	init_root();

	printf("-----Insert-----\n");
	begin = cur_time();
    while (true) {
		insert(interactive(), NULL);
    	print_tree(Root);
    }
	end = cur_time();

	return 0;
}