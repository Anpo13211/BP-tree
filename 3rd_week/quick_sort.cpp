#include <iostream>
#include <vector>
#include <algorithm>
#include <random>
using namespace std;

const int DATA_SIZE = 1000*10000;
vector<int> Database(DATA_SIZE);

void init_data() {
    Database[0] = rand() % 4;
    for (int i = 1; i < DATA_SIZE; ++i) {
        Database[i] = Database[i - 1] + (rand() % 4) + 1;
    }
    
    random_device rd;
    mt19937 g(rd());
    shuffle(Database.begin(), Database.end(), g);
}

void init() {
    init_data();
}

int partition(int p, int r) {
    int i, j, x, tmp;

    i = p - 1;
    x = Database[r];
    for (j = p; j < r; j++) {
        if (Database[j] <= x) {
            i++;

            tmp = Database[i];
            Database[i] = Database[j];
            Database[j] = tmp;
        }
    }
    tmp = Database[i + 1];
    Database[i + 1] = x;
    Database[r] = tmp;

    return i + 1;
}

void quick_sort(int p, int r) {
  int q;  // partition分割の基準となる要素のインデックス
  if (p < r) {    
    q = partition(p, r);
    quick_sort(p, q-1);
    quick_sort(q+1, r);
  }
}

int main() {
    init();
    quick_sort(0, DATA_SIZE - 1);
    for (int i = 0; i < DATA_SIZE; ++i) cout << Database[i] << " ";
    cout << endl;
}