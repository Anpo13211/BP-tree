#include <stdio.h>
#include <sys/time.h>
#include <iostream>
using namespace std;

/*****************************************/
#define DATA_SIZE (10)
#define HASH_BUCKET_SIZE (DATA_SIZE)
//#define HASH_BUCKET_SIZE (DATA_SIZE * 1)
/*****************************************/

//#define HASH_DELTA 3
#define HASH_DELTA (DATA_SIZE)
#define INVALID -9


int Database[DATA_SIZE];
int HashBucket[HASH_BUCKET_SIZE];

void init_data(void) {
  Database[0] = rand() % 8;
  for (int i = 1; i < DATA_SIZE; i++) { Database[i] = Database[i-1] + (rand() % 6) + 1; }

  for (int i = 0; i < DATA_SIZE; i++) { printf("%d ", Database[i]); }
  printf("\n");
}

void init_hash() {
  for (int i = 0; i < HASH_BUCKET_SIZE; i++) { HashBucket[i] = INVALID; }

  for (int i = 0; i < DATA_SIZE; i++) {
      int key = Database[i];
      int bid = key % HASH_BUCKET_SIZE;

      while (HashBucket[bid] != INVALID) {
        bid += HASH_DELTA;
        if (bid >= HASH_BUCKET_SIZE) {
	        bid -= HASH_BUCKET_SIZE;
	      }
        if (bid == key % HASH_BUCKET_SIZE) {
          cout << "Cannot insert." << endl;
          exit(1);
        }
      }
      HashBucket[bid] = key;
  }
}

void init() {
  init_data();
  init_hash();
}

void linear_search(const int key) {
  for (int i = 0; i < DATA_SIZE; i++) {
    if (Database[i] == key) {
      printf("[Linear] Found at %8d (Step = %8d)\n", i, i);
      return;
    }
  }
}

int binary_search(const int key, const int min, const int max) {
  int i, j, p;
  i = min;
  j = max;
  int step = 0;

  while (i <= j) {
    step++;
    p = (i + j) / 2;
    //printf("%d ", Database[p]);
    if (key < Database[p]) j = p-1;
    else if (key > Database[p]) i = p+1;
    else if (key == Database[p]) {
      //printf("\n");
      printf("[Binary] Found at %8d (Step = %8d)\n", p, step);
      return p;
    }
  }

  return -1;
}

void hash_search(const int key) {
  int bid = key % HASH_BUCKET_SIZE;
  int step = 0;
  while (true) {
    step++;
    if (HashBucket[bid] == INVALID) {
      break;
    }
    else if (HashBucket[bid] == key) {
      printf("[Hash  ] Found at %8d (Step = %8d)\n", bid, step);
      break;
    }
    else bid += HASH_DELTA;
    if (bid >= HASH_BUCKET_SIZE) bid -= HASH_BUCKET_SIZE;
  }
}

int main(void) {
  int key;

  init();
  printf("\n#Records: %d\n", DATA_SIZE);
  printf("Search Key? ");
  scanf("%d", &key);

  linear_search(key);
  binary_search(key, 0, DATA_SIZE-1);
  hash_search(key);

  return 0;
}
