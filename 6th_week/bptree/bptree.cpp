#include "bptree.h"
#include <vector>
#include <sys/time.h>
#include <algorithm>
#include <queue>
using namespace std;

int DATA_SIZE = 1000*1000;
vector<int> Database(DATA_SIZE);

void init_data() {
    Database[0] = rand() % 2;
    for (int i = 1; i < DATA_SIZE; ++i) {
        Database[i] = Database[i - 1] + (rand() % 2) + 1;
    }
}

void init() {
    init_data();
}

struct timeval cur_time(void) {
    struct timeval t;
    gettimeofday(&t, NULL);
    return t;
}

// Print the tree
void print_tree_core_auto(NODE* current) {
    printf("[");
    for (int i = 0; i < current->nkey; i++) {
        if (!current->isLeaf) print_tree_core_auto(current->chi[i]);
        printf("%d", current->key[i]);
        if (i != current->nkey-1 && current->isLeaf) putchar(' ');
    }
    if (!current->isLeaf) print_tree_core_auto(current->chi[current->nkey]);
    printf("]");
}

void print_tree_core(NODE *current) {
    if (current == NULL) return;
    queue<NODE*> q;
    q.push(current);

    while (!q.empty()) {
        int l;
        l = q.size();
 
        for (int i = 0; i < l; i++) {
            NODE *tNode = q.front();
            q.pop();
 
            for (int j = 0; j < tNode->nkey; j++)
                if (tNode != NULL)
                    cout << tNode->key[j] << " ";
 
            for (int j = 0; j < tNode->nkey + 1; j++) {
                if (tNode->chi[j] != NULL) q.push(tNode->chi[j]);
            }
 
            cout << "\t";
        }
        cout << endl;
    }
}

void print_tree_auto(NODE *node) {
    print_tree_core_auto(node);
    printf("\n"); fflush(stdout);
}

void print_tree(NODE *node) {
    print_tree_core(node);
    printf("\n"); fflush(stdout);
}

void erase_entries(NODE *node) {
    for (int i = 0; i < N - 1; i++) node->key[i] = 0;
    for (int i = 0; i < N; i++) node->chi[i] = NULL;
    node->nkey = 0;
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
    if (!(node = (NODE *)calloc(1, sizeof(NODE)))) ERR; // メモリを確保することができるかどうか
    node->isLeaf = true;
    node->parent = parent;
    node->nkey = 0;

    return node;
}

NODE *alloc_internal(NODE *parent) {
    NODE *node;

    if (!(node = (NODE *)calloc(1, sizeof(NODE)))) ERR;
    node->isLeaf = false;
    node->parent = parent;
    node->nkey = 0;

    return node;
}

NODE *alloc_root(NODE *left, int key, NODE *right) {
    NODE *node;

    if (!(node = (NODE *)calloc(1, sizeof(NODE)))) ERR;
    node->parent = NULL;
    node->isLeaf = false;
    node->key[0] = key;
    node->chi[0] = left;
    node->chi[1] = right;
    node->nkey = 1;

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

void insert_in_temp(TEMP *temp, int key, void *data) {
    int i;
    if (key < temp->key[0]) {
            for (i = temp->nkey; i > 0; i--) {
                temp->key[i] = temp->key[i - 1];
                temp->chi[i] = temp->chi[i - 1];
            }
            temp->key[0] = key;
            temp->chi[0] = (NODE *)data;
        } else {
            for (i = 0; i < temp->nkey; i++) {
                if (key < temp->key[i]) break;
            }
            for (int j = temp->nkey; j > i; j--) {
                temp->key[j] = temp->key[j - 1];
                temp->chi[j] = temp->chi[j - 1];
            }
            temp->key[i] = key;
            temp->chi[i] = (NODE *)data; 
        }
        temp->nkey++;   
}

void copy_from_left_to_temp(TEMP *temp, NODE *left) {
    int i;
    bzero(temp, sizeof(TEMP));
    for (i = 0; i < (N - 1); i++) {
        temp->key[i] = left->key[i];
        temp->chi[i] = left->chi[i];
    }
    temp->nkey = N - 1;
    temp->chi[i] = left->chi[i];
}

void insert_temp_after_left_child(TEMP *temp, NODE *left, int key, NODE *right) {
    int left_temp_id = 0;
    int right_temp_id = 0;
    int i;

    for (i = 0; i < temp->nkey + 1; i++) {
        if (temp->chi[i] == left) {
            left_temp_id = i;
            right_temp_id = left_temp_id + 1;
            break;
        }
    } 
    assert(i != temp->nkey + 1);

    for (i = temp->nkey; i > left_temp_id; i--) temp->key[i] = temp->key[i - 1];
    for (i = temp->nkey + 1; i > right_temp_id; i--) temp->chi[i] = temp->chi[i - 1];

    temp->key[left_temp_id] = key;
    temp->chi[right_temp_id] = right;
    temp->nkey++;
}

void copy_from_temp_to_left_parent(TEMP *temp, NODE *left_parent) {
    for (int j = 0; j < (int)ceil((N + 1) / 2); j++) {
        left_parent->key[j] = temp->key[j];
        left_parent->chi[j] = temp->chi[j];
        left_parent->nkey++;
    }
    left_parent->chi[(int)ceil((N + 1) / 2)] = temp->chi[(int)ceil((N + 1) / 2)];
}

void copy_from_temp_to_right_parent(TEMP *temp, NODE *right_parent) {
    int id;
    for (id = ((int)ceil((N + 1) / 2) + 1); id < N; id++) {
        right_parent->key[id - ((int)ceil((N + 1) / 2) + 1)] = temp->key[id];
        right_parent->chi[id - ((int)ceil((N + 1) / 2) + 1)] = temp->chi[id];
        right_parent->nkey++;
    }
    right_parent->chi[id - ((int)ceil((N + 1) / 2) + 1)] = temp->chi[id];
    for (int k = 0; k < right_parent->nkey + 1; k++) right_parent->chi[k]->parent = right_parent;
}

void insert_in_parent(NODE *left, int key, NODE *right) {
    NODE *left_parent;
    NODE *right_parent;

    if (left == Root) {
        Root = alloc_root(left, key, right);
        left->parent = right->parent = Root;
        return;
    }
    left_parent = left->parent;

    if (left_parent->nkey < (N - 1)) { // if p has less than n pointers 
        int left_child_id = 0;
        int right_child_id = 0;
        int i;

        for (i = 0; i < left_parent->nkey + 1; i++) {
            if (left_parent->chi[i] == left) {
                left_child_id = i;
                right_child_id = left_child_id + 1;
                break;
            }
        }
        for (i = left_parent->nkey; i > left_child_id; i--) left_parent->key[i] = left_parent->key[i - 1];
        for (i = left_parent->nkey + 1; i > right_child_id; i--) left_parent->chi[i] = left_parent->chi[i - 1];

        left_parent->key[left_child_id] = key;
        left_parent->chi[right_child_id] = right;
        left_parent->nkey++;
    } else {
        TEMP temp;

        copy_from_left_to_temp(&temp, left_parent);

        insert_temp_after_left_child(&temp, left, key, right);
        
        erase_entries(left_parent);
        right_parent = alloc_internal(left_parent->parent); // create node_prime
        copy_from_temp_to_left_parent(&temp, left_parent);

        int key_parent = temp.key[(int)ceil((double)N / (double)2)];

        // copy from temp to right_parent
        copy_from_temp_to_right_parent(&temp, right_parent);
        insert_in_parent(left_parent, key_parent, right_parent);
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
        NODE *left = leaf;
        NODE *right = alloc_leaf(leaf->parent);
        TEMP temp;

        copy_from_left_to_temp(&temp, left);
        insert_in_temp(&temp, key, data);

        right->chi[N - 1] = left->chi[N - 1];
        left->chi[N - 1] = right;
        erase_entries(left);

        // copy from temp to left
        for (int i = 0; i < (int)ceil((double)N / (double)2); i++) {
            left->key[i] = temp.key[i];
            left->chi[i] = temp.chi[i];
            left->nkey++;
        }

        // copy from temp to right
        for (int i = (int)ceil((double)N / (double)2); i < N; i++) {
            right->key[i - (int)ceil((double)N / (double)2)] = temp.key[i];
            right->chi[i - (int)ceil((double)N / (double)2)] = temp.chi[i];
            right->nkey++;
        }

        int smallest_key = right->key[0];
        insert_in_parent(left, smallest_key, right);
    }
}

void bulk_insert(NODE *root, const vector<int> &data) {
    for (int value : data) {
        insert(value, NULL);
    }
}

void init_root(void) {
	Root = NULL;
}

void search(int key) {
    NODE *n = find_leaf(Root, key);
    bool flag = false;
    for (int i = 0; i < n->nkey + 1; i++) {
        if (n->key[i] == key) {
            flag = true;
            break;
        }
    }
    if (flag == true) {
        cout << "Key [ " << key << " ] has found." << endl;
    } else cout << "There is no this key" << endl;
}

int interactive() {
  int key;

  cout << "Key: ";
  cin >> key;

  return key;
}

int main(int argc, char *argv[]) {
    init();
    init_root();
    struct timeval begin, end;

    printf("-----Insert-----\n");
    int mode, stop;
    printf("Choose the mode: 1 is auto insert, 2 is manual: ");
    cin >> mode;
    if (mode == 1) {
        begin = cur_time();
        bulk_insert(Root, Database);
        print_tree_auto(Root);
        end = cur_time();
    } else if (mode == 2) {
        while (true) {
		    insert(interactive(), NULL);
    	    print_tree(Root);
        }
    } else {
        cout << "Please enter 1 or 2" << endl;
    }
	
    

    printf("-----Search-----\n");
    int key;
    printf("The key you want to search: ");
    cin >> key;
    search(key);

	return 0;
}