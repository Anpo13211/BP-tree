#include "bptree.h"
#include <pthread.h>
#include <vector>
#include <sys/time.h>
#include <algorithm>
#include <queue>
#include <thread>
#include <mutex>
using namespace std;

mutex leaf_mtx;
int DATA_SIZE = 1000*1000;
vector<int> Database(DATA_SIZE);

void print_performance(struct timeval start, struct timeval end) {
    long long time_diff_microsec = (end.tv_sec - start.tv_sec) * 1000 * 1000 + (end.tv_usec - start.tv_usec);
    double time_diff_sec = time_diff_microsec / 1000000.0;
    double requests_per_sec = DATA_SIZE / time_diff_sec;

    printf("%9.2f req/sec (lat:%7lld usec)\n", requests_per_sec, time_diff_microsec);
}

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

NODE *find_leaf(NODE *node, int key) {
    int kid;

    if (node->isLeaf) {
        return node;
    }
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
    node->chi[0] = leaf;
    node->chi[1] = leaf_prime;
    node->nkey = 1;

    return node;
}

void erase_entries(NODE *node) {
    for (int i = 0; i < N - 1; i++) node->key[i] = 0;
    for (int i = 0; i < N; i++) node->chi[i] = NULL;
    node->nkey = 0;
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

void insert_in_parent(NODE *n, int key, NODE *N_prime) {
    if (n == Root) {
        Root = alloc_root(n, key, N_prime);
        n->parent = N_prime->parent = Root;
        return;    
    }

    NODE *p = n->parent;
    int i, j;

    if (p->nkey < N - 1) {
        for (i = 0; i < p->nkey; i++) {
            if (key < p->key[i]) break;
        }
        for (j = p->nkey; j > i; j--) p->key[j] = p->key[j - 1];
        for (j = p->nkey + 1; j > i + 1; j--) p->chi[j] = p->chi[j - 1];
        p->key[i] = key;
        p->chi[i+1] = N_prime;
        p->nkey++;
    } else {
        TEMP temp;
        int i;
        for (i = 0; i < N - 1; i++) {
            temp.key[i] = p->key[i];
            temp.chi[i] = p->chi[i];
        }
        temp.chi[i] = p->chi[i];
        temp.nkey = N - 1;

        int pos;
        for (pos = 0; pos < temp.nkey; pos++) {
            if (key < temp.key[pos]) break;
        }
        for (int i = temp.nkey; i > pos; i--) temp.key[i] = temp.key[i - 1];
        for (int i = temp.nkey + 1; i > pos + 1; i--) temp.chi[i] = temp.chi[i - 1];
        temp.key[pos] = key;
        temp.chi[pos + 1] = N_prime;
        temp.nkey++;

        erase_entries(p);

        NODE *p_prime = alloc_internal(p->parent);
        p_prime->isLeaf = false;

        for (int i = 0; i < p->nkey; i++) {
            p->key[i] = temp.key[i];
            p->chi[i] = temp.chi[i];
        }
        p->chi[p->nkey] = temp.chi[p->nkey];

        int k_prime = temp.key[(int)ceil((double)N / (double)2)];

        for (int i = 0; i < (int)ceil((N + 1) / 2); i++) {
            p->key[i] = temp.key[i];
            p->chi[i] = temp.chi[i];
            p->nkey++;
        }
        p->chi[(int)ceil((N + 1) / 2)] = temp.chi[(int)ceil((N + 1) / 2)];

        int id;
        for (id = ((int)ceil((N + 1) / 2) + 1); id < N; id++) {
            p_prime->key[id - ((int)ceil((N + 1) / 2) + 1)] = temp.key[id];
            p_prime->chi[id - ((int)ceil((N + 1) / 2) + 1)] = temp.chi[id];
            p_prime->nkey++;
        }
        p_prime->chi[id - ((int)ceil((N + 1) / 2) + 1)] = temp.chi[id];
        for (int k = 0; k < p_prime->nkey + 1; k++) p_prime->chi[k]->parent = p_prime;
        
        insert_in_parent(p, k_prime, p_prime);
    }
}

void insert(int key, DATA *data) {
    NODE *leaf;

    if (Root == NULL) {
        leaf = alloc_leaf(NULL);
        Root = leaf;
    } else {
        leaf = find_leaf(Root, key);
    }

    if (leaf->nkey < (N-1)) {
        insert_in_leaf(leaf, key, data);
    }
    else {
        NODE *left = leaf;

        NODE *new_leaf = alloc_leaf(leaf->parent);

        TEMP temp;
        int i, j;

        for (i = 0; i < N - 1; i++) {
            temp.key[i] = left->key[i];
            temp.chi[i] = left->chi[i];
        }
        temp.chi[i] = left->chi[i];
        temp.nkey = N - 1;

        int k;
        if (key < temp.key[0]) {
            for (k = temp.nkey; i > 0; i--) {
                temp.key[i] = temp.key[i - 1];
                temp.chi[i] = temp.chi[i - 1];
            }
            temp.key[0] = key;
            temp.chi[0] = (NODE *)data;
        } else {
            for (k = 0; k < temp.nkey; k++) {
                if (key < temp.key[k]) break;
            }
            for (int j = temp.nkey; j > k; j--) {
                temp.key[j] = temp.key[j - 1];
                temp.chi[j] = temp.chi[j - 1];
            }
            temp.key[k] = key;
            temp.chi[k] = (NODE *)data;
        }
        temp.nkey++;

        new_leaf->chi[N - 1] = left->chi[N - 1];
        left->chi[N - 1] = new_leaf;
        erase_entries(left);

        for (int i = 0; i < (int)ceil((double)N / (double)2); i++) {
            left->key[i] = temp.key[i];
            left->chi[i] = temp.chi[i];
            left->nkey++;
        }

        for (int i = (int)ceil((double)N / (double)2); i < N; i++) {
            new_leaf->key[i - (int)ceil((double)N / (double)2)] = temp.key[i];
            new_leaf->chi[i - (int)ceil((double)N / (double)2)] = temp.chi[i];
            new_leaf->nkey++;
        }

        int smallest = new_leaf->key[0];
        insert_in_parent(leaf, smallest, new_leaf);
    }
}

void bulk_insert(NODE *root, const vector<int> &data) {
    for (int value: data) {
        insert(value, NULL);
    }
}

void init_root(void) {
	Root = NULL;
}

void search(int key) {
    unique_lock<mutex> lock(leaf_mtx);

    NODE *n = find_leaf(Root, key);
    bool flag = false;
    for (int i = 0; i < n->nkey; i++) {
        if (n->key[i] == key) {
            flag = true;
            break;
        }
    }
    if (!flag){
        cout << "Key not found: " << key << endl;
    }
}

void search_single() {
    for (int i = 0; i < (int)Database.size(); i++) search(Database[i]);
}

void update(int key, DATA *new_data) {
    leaf_mtx.lock();

    NODE *leaf = find_leaf(Root, key);
    if (leaf == nullptr || !leaf->isLeaf) {
        cout << "Key [ " << key << " ] not found." << endl;
        leaf_mtx.unlock();
        return;
    }

    bool updated = false;
    for (int i = 0; i < leaf->nkey; i++) {
        if (leaf->key[i] == key) {
            
            DATA *old_data = (DATA *)leaf->chi[i];
            if (old_data != nullptr) {
                free(old_data);
            }

            leaf->chi[i] = (NODE *)new_data;
            updated = true;
            break;
        }
    }

    if (updated) {
        cout << "Key [ " << key << " ] updated successfully." << endl;
    } else {
        cout << "Key [ " << key << " ] not found in leaf node." << endl;
    }
    leaf_mtx.unlock();
}

struct ThreadData {
    int key;
    DATA *data;
};

void *thread_function(void *arg) {
    ThreadData *threadData = static_cast<ThreadData*>(arg);

    update(threadData->key, threadData->data);

    delete threadData;
    return nullptr;
}

void multithread_test() {
    const int num_threads = 10;
    pthread_t threads[num_threads];

    for (int i = 0; i < 10; ++i) {
        ThreadData *data = new ThreadData;
        data->key = i;
        data->data = new DATA;
        data->data->val = Database[i];

        pthread_create(&threads[i], nullptr, thread_function, data);
    }

    for (int i = 0; i < num_threads; ++i) {
        pthread_join(threads[i], nullptr);
    }
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
        // print_tree_auto(Root);
        end = cur_time();
        print_performance(begin, end);
    } else if (mode == 2) {
        while (true) {
            begin = cur_time();
            int key;
            cout << "Enter a key to insert (or -1 to exit): ";
            cin >> key;
            insert(key, NULL);
            print_tree(Root);
            end = cur_time();
            if (key == -1) {
                print_performance(begin, end);
                break;
            };
        }
    } else {
        cout << "Invalid input. Please enter 1 or 2." << endl;
        return 1; // 無効な入力の場合はプログラムを終了
    }

    int update_key;
    cout << "Enter a key to udpate: ";
    cin >> update_key;
    DATA *new_data = new DATA;
    new_data->key = update_key;
    new_data->val = 10;

    update(update_key, new_data);

    printf("-----Search-----\n");
    begin = cur_time();
    search_single();
    end = cur_time();
    print_performance(begin, end);

    multithread_test();

	return 0;
}