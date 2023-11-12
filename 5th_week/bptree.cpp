#include "bptree.h"
#include <vector>
#include <sys/time.h>
#include <algorithm>
using namespace std;

struct timeval cur_time(void) {
    struct timeval t;
    gettimeofday(&t, NULL);
    return t;
}

void print_tree_core(NODE *n) {
    printf("[");
    for (int i = 0; i < n->nkey; i++) {
        if (!n->isLeaf) print_tree_core(n->chi[i]);
        printf("%d", n->key[i]);
        if (i != n->nkey-1 && n->isLeaf) putchar(' ');
    }
    if (!n->isLeaf) print_tree_core(n->chi[n->nkey]);
    printf("]");
}

void print_tree(NODE *node) {
    print_tree_core(node);
    printf("\n"); fflush(stdout);
}

NODE *find_leaf(NODE *node, int key) {
    int kid;

    if (node->isLeaf) return node;
    for (kid = 0; kid < node->nkey; kid++) {
        if (key < node->key[kid]) break;
    }
    return find_leaf(node->chi[kid], key);
}

NODE *alloc_leaf(NODE *parent) {
    NODE *node;
    if (!(node = (NODE *)calloc(1, sizeof(NODE)))) ERR;
    node->isLeaf = true;
    node->parent = parent;
    node->nkey = 0;

    return node;
}

NODE *insert_in_leaf(NODE *leaf, int key, DATA *data) {
    int i;
    if (key < leaf->key[0]) {
        for (i = leaf->nkey; i > 0; i--) {
            leaf->chi[i] = leaf->chi[i - 1];
            leaf->key[i] = leaf->key[i - 1];
        }
        leaf->key[0] = key;
        leaf->chi[0] = (NODE *)data;
    } else {
        for (i = 0; i < leaf->nkey; i++) {
            if (key < leaf->key[i]) break;
        }
        for (int j = leaf->nkey; j > i; j--) {
            leaf->chi[j] = leaf->chi[j - 1];
            leaf->key[j] = leaf->key[j - 1];
        }
        leaf->key[i] = key;
        leaf->chi[i] = (NODE *)data; 
    }
    leaf->nkey++;

    return leaf;
}

NODE *insert_in_parent(NODE *n, int key, NODE *N_prime) {
    if (n == Root) {
        NODE *R = new NODE();
        R->key[0] = key;
        R->chi[0] = n;
        R->chi[1] = N_prime;
        R->nkey = 1;
        R->isLeaf = false;
        n->parent = R;
        N_prime->parent = R;
        Root = R;
        return R;
    }

    NODE *p = n->parent;
    int i, j;

    if (p->nkey < N - 1) {
        for (i = 0; i < p->nkey; i++) {
            if (key < p->key[i]) break;
        }
        for (j = p->nkey; j > i; j--) {
            p->key[j] = p->key[j - 1];
            p->chi[j + 1] = p->chi[j];
        }
        p->key[i] = key;
        p->chi[i+1] = N_prime;
        p->nkey++;
        return p;
    } else {
        TEMP temp;
        for (int i = 0; i < p->nkey; i++) {
            temp.key[i] = p->key[i];
            temp.chi[i] = p->chi[i];
        }
        temp.chi[p->nkey] = p->chi[p->nkey];
        temp.nkey = p->nkey;

        int pos;
        for (pos = 0; pos < temp.nkey; pos++) {
            if (key < temp.key[pos]) break;
        }
        for (int i = temp.nkey; i > pos; i--) {
            temp.key[i] = temp.key[i - 1];
            temp.chi[i + 1] = temp.chi[i];
        }
        temp.key[pos] = key;
        temp.chi[pos + 1] = N_prime;
        temp.nkey++;

        NODE *p_prime = new NODE();
        p_prime->isLeaf = false;

        int mid = N - (N/2) + 1;
        p->nkey = mid - 1;
        for (int i = 0; i < p->nkey; i++) {
            p->key[i] = temp.key[i];
            p->chi[i] = temp.chi[i];
        }
        p->chi[p->nkey] = temp.chi[p->nkey];

        int k_prime = temp.key[mid-1];

        p_prime->nkey = temp.nkey - mid;
        for (int j = 0; j < p_prime->nkey; j++) {
            p_prime->key[j] = temp.key[j + mid];
            p_prime->chi[j] = temp.chi[j + mid];
        }
        p_prime->chi[p_prime->nkey] = temp.chi[temp.nkey];

        for (int i = 0; i <= p_prime->nkey; i++) {
            if (p_prime->chi[i] != NULL) p_prime->chi[i]->parent = p_prime;
        }
        return insert_in_parent(p, k_prime, p_prime);
    }
}

void insert(int key, DATA *data) {
    NODE *leaf;

    if (Root == NULL) {
        leaf = alloc_leaf(NULL);
        Root = leaf;
    } else leaf = find_leaf(Root, key);

    if (leaf->nkey < (N-1)) insert_in_leaf(leaf, key, data);
    else {
        NODE *new_leaf = alloc_leaf(leaf->parent);

        NODE *temp = new NODE();
        temp->nkey = leaf->nkey;
        int i, j;

        for (int i = 0; i < N - 1; i++) {
            temp->key[i] = leaf->key[i];
            temp->chi[i] = leaf->chi[i];
        }

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

        for (int i = 0; i < N; i++) {
            leaf->key[i] = 0;
            leaf->chi[i] = NULL;
        }
        
        int split = N - (N/2);
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

        int smallest = new_leaf->key[0];
        insert_in_parent(leaf, smallest, new_leaf);
    }
}

void init_root(void) {
	Root = NULL;
}

int interactive() {
  int key;

  cout << "Key: ";
  cin >> key;

  return key;
}

int main(int argc, char *argv[]) {
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
