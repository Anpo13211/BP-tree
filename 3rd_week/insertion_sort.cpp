#include <iostream>
#include <vector>
#include <algorithm>
#include <random>
using namespace std;

const int DATA_SIZE = 1000*10000;
vector<int> Database(DATA_SIZE);

void init_data() {
    Database[0] = rand() % 2;
    for (int i = 1; i < DATA_SIZE; ++i) {
        Database[i] = Database[i - 1] + (rand() % 2) + 1;
    }
    
    random_device rd;
    mt19937 g(rd());
    shuffle(Database.begin(), Database.end(), g);
}

void init() {
    init_data();
}

void insertion_sort() {
    for (int j = 2; j < Database.size(); ++j) {
        int key = Database[j];
        int i = j - 1;
        while (i >= 0 && Database[i] > key) {
            Database[i + 1] = Database[i];
            i = i - 1;
        }
        Database[i + 1] = key;
    }
    for (int i = 0; i < DATA_SIZE; ++i) cout << Database[i] << " ";
    cout << endl;
}

int main() {
    init();
    insertion_sort();
}