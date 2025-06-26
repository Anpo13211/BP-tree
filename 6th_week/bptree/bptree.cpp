#include "bptree.h"
#include <vector>
#include <sys/time.h>
#include <algorithm>
#include <queue>
#include <mutex>
#include <climits>
using namespace std;

mutex mtx;
NODE *Root = NULL;
DATA Head;
DATA *Tail = NULL;
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
        if (!current->isLeaf) print_tree_core_auto(current->child[i]);
        printf("%d", current->key[i]);
        if (i != current->nkey-1 && current->isLeaf) putchar(' ');
    }
    if (!current->isLeaf) print_tree_core_auto(current->child[current->nkey]);
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
                if (tNode->child[j] != NULL) q.push(tNode->child[j]);
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
    for (int i = 0; i < N; i++) node->child[i] = NULL;
    node->nkey = 0;
}

NODE *find_leaf(NODE *node, int key) {
    int kid;

    if (node->isLeaf) return node;
    for (kid = 0; kid < node->nkey; kid++) {
        if (key < node->key[kid]) break;
    }
    return find_leaf(node->child[kid], key);
}

// デバッグ用 find_leaf
// NODE* find_leaf(NODE* node, int key) {
//     int kid;
//     if (node->isLeaf) {
//         printf("Reached leaf: ");
//         for (int i = 0; i < node->nkey; i++) {
//             printf("%d ", node->key[i]);
//         }
//         printf("\n");
//         return node;
//     }

//     for (kid = 0; kid < node->nkey; kid++) {
//         if (key < node->key[kid]) break;
//     }

//     printf("Traversing to child[%d] for key=%d\n", kid, key);
//     return find_leaf(node->child[kid], key);
// }


NODE *alloc_leaf(NODE *parent) {
    NODE *node;
    if (!(node = (NODE *)calloc(1, sizeof(NODE)))) ERR; // メモリを確保することができるかどうか
    node->isLeaf = true;
    node->parent = parent;
    node->nkey = 0;

    for (int i = 0; i < N; i++) node->child[i] = NULL;
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

NODE *alloc_root(NODE *leaf, int key, NODE *leaf_prime) {
    NODE *node;

    if (!(node = (NODE *)calloc(1, sizeof(NODE)))) ERR;
    node->parent = NULL;
    node->isLeaf = false;
    node->key[0] = key;
    node->child[0] = leaf;
    node->child[1] = leaf_prime;
    node->nkey = 1;

    return node;
}

NODE *insert_in_leaf(NODE *leaf, int key, DATA *data) {
    int i;
    if (key < leaf->key[0]) {
        for (i = leaf->nkey; i > 0; i--) {
            leaf->child[i] = leaf->child[i - 1];
            leaf->key[i] = leaf->key[i - 1];
        }
        leaf->key[0] = key;
        leaf->child[0] = (NODE *)data;
    } else {
        for (i = 0; i < leaf->nkey; i++) {
            if (key < leaf->key[i]) break;
        }
        for (int j = leaf->nkey; j > i; j--) {
            leaf->child[j] = leaf->child[j - 1];
            leaf->key[j] = leaf->key[j - 1];
        }
        leaf->key[i] = key;
        leaf->child[i] = (NODE *)data; 
    }
    leaf->nkey++;

    return leaf;
}

void insert_into_temp(TEMP *temp, int key, void *data) {
    int i;
    if (key < temp->key[0]) {
            for (i = temp->nkey; i > 0; i--) {
                temp->key[i] = temp->key[i - 1];
                temp->child[i] = temp->child[i - 1];
            }
            temp->key[0] = key;
            temp->child[0] = (NODE *)data;
        } else {
            for (i = 0; i < temp->nkey; i++) {
                if (key < temp->key[i]) break;
            }
            for (int j = temp->nkey; j > i; j--) {
                temp->key[j] = temp->key[j - 1];
                temp->child[j] = temp->child[j - 1];
            }
            temp->key[i] = key;
            temp->child[i] = (NODE *)data; 
        }
        temp->nkey++;   
}

void Copy_leftnode_to_tempnode(TEMP *temp, NODE *left) {
    int i;
    bzero(temp, sizeof(TEMP));
    for (i = 0; i < (N - 1); i++) {
        temp->key[i] = left->key[i];
        temp->child[i] = left->child[i];
    }
    temp->nkey = N - 1;
    temp->child[i] = left->child[i];
}

void Let_temp_point_to_leafprime(TEMP *temp, NODE *leaf, int key, NODE *leaf_prime) {
    int leaf_temp_id = 0;
    int lp_temp_id = 0;
    int i;

    for (i = 0; i < temp->nkey + 1; i++) {
        if (temp->child[i] == leaf) {
            leaf_temp_id = i;
            lp_temp_id = leaf_temp_id + 1;
            break;
        }
    } 
    assert(i != temp->nkey + 1);

    for (i = temp->nkey; i > leaf_temp_id; i--) temp->key[i] = temp->key[i - 1]; // key の末尾から leaf_temp_id まで
    for (i = temp->nkey + 1; i > lp_temp_id; i--) temp->child[i] = temp->child[i - 1]; // child の末尾から lp_temp_id まで

    temp->key[leaf_temp_id] = key;
    temp->child[lp_temp_id] = leaf_prime;
    temp->nkey++;
}

void Copy_tempnode_to_parent(TEMP *temp, NODE *parent) {
    for (int j = 0; j < (int)ceil((N + 1) / 2); j++) {
        parent->key[j] = temp->key[j];
        parent->child[j] = temp->child[j];
        parent->nkey++;
    }
    parent->child[(int)ceil((N + 1) / 2)] = temp->child[(int)ceil((N + 1) / 2)];
}

void Copy_tempnode_to_parentprime(TEMP *temp, NODE *parent_prime) {
    int id;
    for (id = ((int)ceil((N + 1) / 2) + 1); id < N; id++) {
        parent_prime->key[id - ((int)ceil((N + 1) / 2) + 1)] = temp->key[id];
        parent_prime->child[id - ((int)ceil((N + 1) / 2) + 1)] = temp->child[id];
        parent_prime->nkey++;
    }
    parent_prime->child[id - ((int)ceil((N + 1) / 2) + 1)] = temp->child[id];
    for (int k = 0; k < parent_prime->nkey + 1; k++) parent_prime->child[k]->parent = parent_prime;
}

void insert_into_parent(NODE *leaf, int key, NODE *leaf_prime) {
    NODE *parent;
    NODE *parent_prime;

    if (leaf == Root) {
        Root = alloc_root(leaf, key, leaf_prime);
        leaf->parent = leaf_prime->parent = Root;
        return;
    }
    parent = leaf->parent;

    if (parent->nkey < (N - 1)) { // if p has less than n pointers 
        int left_child_id = 0;
        int right_child_id = 0;
        int i;

        for (i = 0; i < parent->nkey + 1; i++) {
            if (parent->child[i] == leaf) {
                left_child_id = i;
                right_child_id = left_child_id + 1;
                break;
            }
        }
        for (i = parent->nkey; i > left_child_id; i--) parent->key[i] = parent->key[i - 1];
        for (i = parent->nkey + 1; i > right_child_id; i--) parent->child[i] = parent->child[i - 1];

        parent->key[left_child_id] = key;
        parent->child[right_child_id] = leaf_prime;
        parent->nkey++;
    } else {
        TEMP temp;

        Copy_leftnode_to_tempnode(&temp, parent);
        Let_temp_point_to_leafprime(&temp, leaf, key, leaf_prime);
        
        erase_entries(parent);
        parent_prime = alloc_internal(parent->parent); // create node_prime
        Copy_tempnode_to_parent(&temp, parent);

        int pushup_key = temp.key[(int)ceil((double)N / (double)2)];

        Copy_tempnode_to_parentprime(&temp, parent_prime);
        insert_into_parent(parent, pushup_key, parent_prime);
    }
}

void debug_print_leaf_links(NODE* root) {
    NODE* node = find_leaf(root, INT_MIN); // 最も左のリーフを探す
    printf("Leaf links:\n");
    while (node != NULL) {
        for (int i = 0; i < node->nkey; i++) {
            printf("%d ", node->key[i]);
        }
        printf(" -> ");
        node = node->child[N - 1]; // 次のリーフノード
    }
    printf("NULL\n");
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
        NODE *leaf_prime = alloc_leaf(leaf->parent);
        TEMP temp;

        Copy_leftnode_to_tempnode(&temp, left);
        insert_into_temp(&temp, key, data);

        leaf_prime->child[N - 1] = left->child[N - 1];
        left->child[N - 1] = leaf_prime;
        erase_entries(left);

        // copy from temp to left
        for (int i = 0; i < (int)ceil((double)N / (double)2); i++) {
            left->key[i] = temp.key[i];
            left->child[i] = temp.child[i];
            left->nkey++;
        }

        // copy from temp to leaf_prime
        for (int i = (int)ceil((double)N / (double)2); i < N; i++) {
            leaf_prime->key[i - (int)ceil((double)N / (double)2)] = temp.key[i];
            leaf_prime->child[i - (int)ceil((double)N / (double)2)] = temp.child[i];
            leaf_prime->nkey++;
        }

        int key_for_parent = leaf_prime->key[0];
        insert_into_parent(left, key_for_parent, leaf_prime);

        debug_print_leaf_links(Root);
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

// リーフノードのリンクが正しくつながっていないかもしれない
vector<int> range_query(int start, int end) {
    vector<int> result;

    // 開始位置のリーフノードを見つける
    NODE *node = find_leaf(Root, start);

    // リーフノード間を順次たどる
    while (node != NULL) {
        for (int i = 0; i < node->nkey; i++) {
            if (node->key[i] >= start && node->key[i] <= end) {
                result.push_back(node->key[i]);  // 範囲内なら追加
            } else if (node->key[i] > end) {
                return result;  // 範囲を超えたら終了
            }
        }
        node = node->child[N - 1];  // 次のリーフノードへ移動
    }

    return result;
}

int interactive() {
  int key;

  cout << "Key: ";
  cin >> key;

  return key;
}

#ifndef TEST
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
        {
            lock_guard<mutex> guard(mtx); // スコープ内でロック
            bulk_insert(Root, Database);
            print_tree_auto(Root);
        }
        end = cur_time();
    } else if (mode == 2) {
        while (true) {
            int key;
            cout << "Enter a key to insert (or -1 to exit): ";
            cin >> key;
            if (key == -1) break;

            {
                lock_guard<mutex> guard(mtx); // スコープ内でロック
                insert(key, NULL);
                print_tree(Root);
            }
        }
    } else {
        cout << "Invalid input. Please enter 1 or 2." << std::endl;
        return 1; // 無効な入力の場合はプログラムを終了
    }

    printf("-----Search-----\n");
    int key;
    printf("The key you want to search: ");
    cin >> key;
    search(key);

	return 0;
}
#endif
