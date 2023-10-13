#include <iostream>
#include <vector>
#include <algorithm>
using namespace std;

const int DATA_SIZE = 1000*1000;

vector<int> Database(DATA_SIZE);

void init_data() {
    Database[0] = rand() % 2;
    for (int i = 1; i < DATA_SIZE; ++i) {
        Database[i] = Database[i-1] + (rand() % 3) + 1;
    }

    for (int i = 0; i < DATA_SIZE; ++i) cout << Database[i] << " ";
    cout << endl;
}

void init() {
    init_data();
}

int binary_search(const int key, const int min, const int max) {
    int i = min;
    int j = max;
    int step = 0;

    while (i <= j) {
        step++;
        int middle = (i + j) / 2;
        if (key < Database[middle]) j = middle-1;
        else if (key > Database[middle]) i = middle+1;
        else if (key == Database[middle]) {
            cout << "Found " << "(Step = " << step << ")\n";
            return middle;
        }
    }

    cout << "Not Found\n";
    return -1;
}

int main() {
    int K;

    init();
    cout << "Key? ";
    cin >> K;
    binary_search(K, 0, DATA_SIZE - 1);
    return 0;
}