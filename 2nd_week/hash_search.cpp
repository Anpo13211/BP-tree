#include <iostream>
#include <vector>
#include <algorithm>
using namespace std;

const int HASH_DELTA = 2;
const int INVALID = -10;
const int DATA_SIZE = 1000*1000;
const int HASH_BUCKET_SIZE = DATA_SIZE * 10;

vector<int> Database(DATA_SIZE);
vector<int> HashBucket(HASH_BUCKET_SIZE);

void init_data() {
    Database[0] = rand() % 2;
    for (int i = 1; i < DATA_SIZE; ++i){
        Database[i] = Database[i - 1] + (rand() % 2) + 1;
    }
    for (int i = 0; i < DATA_SIZE; ++i) cout << Database[i] << " ";
    cout << endl;
}

void init_hash() {
    fill(HashBucket.begin(), HashBucket.end(), INVALID);
    for (int i = 0; i < DATA_SIZE; ++i) {
        int key = Database[i];
        int bid = key % HASH_BUCKET_SIZE;

        while (HashBucket[bid] != INVALID) {
            bid += HASH_DELTA;
            if (bid >= HASH_BUCKET_SIZE) bid -= HASH_BUCKET_SIZE;
        }
        HashBucket[bid] = key;
    }
}

void init() {
    init_data();
    init_hash();
}

int hash_search(const int key) {
    int bid = key % HASH_BUCKET_SIZE;
    int step = 0;
    while (true) {
        step++;
        if (HashBucket[bid] == INVALID) {
            cout << "Not Found\n";
            break;
        }
        else if (HashBucket[bid] == key) {
            cout << "Found " << "(Step = " << step << ")\n" << endl;
            break;
        }
        else {
            bid += HASH_DELTA;
        }
        if (bid >= HASH_BUCKET_SIZE) {
            bid -= HASH_BUCKET_SIZE; 
        }
    }
    return -1;
}

int main() {
    int K;
    init();

    cout << "Key? ";
    cin >> K;

    hash_search(K);
    return 0;
}