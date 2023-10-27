#include "bptree.h"
#include <vector>
#include <sys/time.h>

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
        R->nkey = 1;
        R->isLeaf = false;
        n->parent = R;
        N_prime->parent = R;
        Root = R;
        return R;
    }

    NODE* P = n->parent;

    // Check if parent has space for new pointer
    if (P->nkey < N-1) {
        int pos;
        for (pos = 0; pos <= P->nkey; pos++) {
            if (P->chi[pos] == n) break;
        }
        for (int i = P->nkey; i > pos; i--) {
            P->key[i] = P->key[i-1];
            P->chi[i+1] = P->chi[i];
        }
        P->key[pos] = key;
        P->chi[pos+1] = N_prime;
        P->nkey++;
        N_prime->parent = P;
        return P;
    }

    // Handle the case where parent does not have space for new pointer
    TEMP T;
    for (int i = 0, j = 0; i < P->nkey; i++, j++) {
        if (j == N-1) j++;
        T.key[j] = P->key[i];
        T.chi[j] = P->chi[i];
    }
    T.chi[N-1] = P->chi[N-1];
    T.key[N-1] = key;
    T.chi[N+1] = N_prime;
    T.nkey = N;

    P->nkey = 0;
    int split_pos = (N + 1) / 2;
    for (int i = 0; i < split_pos; i++) {
        P->chi[i] = T.chi[i];
        P->key[i] = T.key[i];
        P->nkey++;
    }
    P->chi[split_pos] = T.chi[split_pos];

    NODE* P_prime = new NODE();
    for (int i = split_pos + 1, j = 0; i <= N; i++, j++) {
        P_prime->chi[j] = T.chi[i];
        P_prime->key[j] = T.key[i];
        P_prime->nkey++;
    }
    P_prime->chi[N-split_pos] = T.chi[N+1];

    for (int i = 0; i <= P_prime->nkey; i++) {
        P_prime->chi[i]->parent = P_prime;
    }

    return insert_in_parent(P, T.key[split_pos], P_prime);
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
	}
  else {
    leaf = find_leaf(Root, key);
  }
	if (leaf->nkey < (N-1)) {
		insert_in_leaf(leaf, key, data);
	}
	else { 
        NODE *new_leaf = alloc_leaf(leaf->parent);

        int temp_keys[N];
        NODE* temp_child[N+1];

        for (int i = 0; i < N - 1; i++) {
            temp_keys[i] = leaf->key[i];
            temp_child[i] = leaf->chi[i];
        }
        temp_child[N-1] = leaf->chi[N-1];

        int i, j;
        // 新しいキーが挿入されるべき位置を見つける
        for (i = 0; i < N-1 && key >= temp_keys[i]; i++);
        for (j = N-1; j > i; j--) {
            temp_keys[j] = temp_keys[j-1];
            temp_child[j+1] = temp_child[j];
        }
        temp_keys[i] = key;
        temp_child[i+1] = (NODE *)data;

        int split = (N+1)/2;
        leaf->nkey = split;
        for (i = 0; i < split; i++) {
            leaf->key[i] = temp_keys[i];
            leaf->chi[i] = temp_child[i];
        }
        leaf->chi[split] = temp_child[split];

        new_leaf->nkey = N - split;
        for (i = split, j = 0; i < N; i++, j++) {
            new_leaf->key[j] = temp_keys[i];
            new_leaf->chi[j] = temp_child[i];
        }
        new_leaf->chi[j] = temp_child[i];

        // Insert the middle key into the parent
        int middle_key = new_leaf->key[0];
        insert_in_parent(leaf->parent, middle_key, new_leaf);
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