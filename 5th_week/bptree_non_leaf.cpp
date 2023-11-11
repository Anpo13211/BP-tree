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

NODE *insert_in_parent(NODE *n, int key, NODE* N_prime) {
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
            P->key[j] = P->key[j - 1];
            P->chi[j + 1] = P->chi[j];
        } 

        P->key[i] = key; // 適切な位置にキーを挿入し
        P->chi[i+1] = N_prime; // ポインターを更新
        P->nkey++;

        return P;
    } else {
        TEMP temp;
        // PのデータをTEMPにコピー
        for (int i = 0; i < P->nkey; i++) {
            temp.key[i] = P->key[i];
            temp.chi[i] = P->chi[i];
        }
        temp.chi[P->nkey] = P->chi[P->nkey];
        temp.nkey = P->nkey;

        // 新しいキーとポインタをTEMPに挿入
        int position;
        for (position = 0; position < temp.nkey; position++) {
            if (key < temp.key[position]) {
                break;
            }
        }
        for (int i = temp.nkey; i > position; i--) {
            temp.key[i] = temp.key[i - 1];
            temp.chi[i + 1] = temp.chi[i];
        }
        temp.key[position] = key;
        temp.chi[position + 1] = N_prime;
        temp.nkey++;

        // 新しいノードP'の作成
        NODE *P_prime = new NODE();
        P_prime->isLeaf = false;

        // キーとポインタをPとP'に再分配
        int mid = N - (N/2) + 1;
        P->nkey = mid - 1;
        for (int i = 0; i < P->nkey; i++) {
            P->key[i] = temp.key[i];
            P->chi[i] = temp.chi[i];
        }
        P->chi[P->nkey] = temp.chi[P->nkey];

        int k_prime = temp.key[mid-1];

        P_prime->nkey = temp.nkey - mid;
        for (int j = 0; j < P_prime->nkey; j++) {
            P_prime->key[j] = temp.key[j + mid];
            P_prime->chi[j] = temp.chi[j + mid];
        }
        P_prime->chi[P_prime->nkey] = temp.chi[temp.nkey];

        // P'の子ノードの親ポインタを更新
        for (int i = 0; i <= P_prime->nkey; i++) {
            if (P_prime->chi[i] != NULL) {
                P_prime->chi[i]->parent = P_prime;
            }
        }

        // 新しい親に挿入
        return insert_in_parent(P, k_prime, P_prime);
    }
}

void insert(int key, DATA *data) {
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