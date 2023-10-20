#include <iostream>
#include <vector>
#include <algorithm>
#include <random>
using namespace std;

const int DATA_SIZE = 1000 * 10000;
vector<int> Database(DATA_SIZE);

void init_data() {
    Database[0] = rand() % 10000000;
    for (int i = 1; i < DATA_SIZE; ++i) {
        Database[i] = (rand() % 10000000);
    }
    
    random_device rd;
    mt19937 g(rd());
    shuffle(Database.begin(), Database.end(), g);
}

void init() {
    init_data();
}

void count_sort() {
    int max = *max_element(Database.begin(), Database.end());
    vector<int>C(max, 0);
    vector<int>B(DATA_SIZE, 0);

    for (int j = 1; j <= DATA_SIZE; ++j) C[Database[j]]++;
    for (int i = 1; i <= max + 1; ++i) C[i] += C[i - 1];

    for (int j = DATA_SIZE; j > 0; --j) {
        B[C[Database[j]]] = Database[j];
        C[Database[j]]--;
    }

    for (int i = 0; i < DATA_SIZE; ++i) cout << B[i] << " ";
    cout << endl;
}

int main() {
    init();
    count_sort();

    
}