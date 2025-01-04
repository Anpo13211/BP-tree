#include "bptree.h"
#include <iostream>
#include <cassert>
#include <vector>
#include <climits>
using namespace std;

// サービステスト（複数のユニットを組み合わせて使ってるので）
void test_insert_and_search() {
    cout << "Running test_insert_and_search()..." << endl;

    init_root();

    int test_keys[] = {10, 20, 5, 6, 12, 30, 7, 17, 25, 40, 50, 60, 70, 80, 90, 100, 110, 120, 130, 140, 150};
    int num_keys = sizeof(test_keys) / sizeof(int);

    for (int i = 0; i < num_keys; i++) {
        insert(test_keys[i], NULL);
    }

    for (int i = 0; i < num_keys; i++) {
        NODE* leaf = find_leaf(Root, test_keys[i]);
        bool found = false;
        for (int j = 0; j < leaf->nkey + 1; j++) {
            if (leaf->key[j] == test_keys[i]) {
                found = true;
                break;
            }
        }
        assert(found && "Key not found after insertion");
    }

    cout << "test_insert_and_search() passed." << endl;
}

void debug_print_leaf_links(NODE* root) {
    NODE* node = find_leaf(root, INT_MIN); // 最も左のリーフを探す
    printf("Leaf links:\n");
    while (node != NULL) {
        for (int i = 0; i < node->nkey; i++) {
            printf("%d ", node->key[i]);
        }
        printf(" -> ");
        node = node->chi[N - 1]; // 次のリーフノード
    }
    printf("NULL\n");
}


void test_range_query() {
    init_root();
    insert(10, NULL);
    insert(20, NULL);
    insert(30, NULL);
    insert(40, NULL);
    insert(50, NULL);
    insert(100, NULL);

    debug_print_leaf_links(Root);

    vector<int> result = range_query(15, 45);
    for (int i = 0; i < result.size(); i++) {
        cout << result[i] << endl;
    }
    // assert(result.size() == 3);
    // assert(result[0] == 20);
    // assert(result[1] == 30);
}


#ifdef TEST
int main() {
    init_root();

    insert(10, NULL);
    insert(20, NULL);
    insert(30, NULL);
    insert(40, NULL);
    insert(50, NULL);
    insert(100, NULL);

    // デバッグ: リーフリンクの確認
    debug_print_leaf_links(Root);

    return 0;
}

#endif