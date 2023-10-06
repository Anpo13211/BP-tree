#include <stdio.h>
#include <string.h>
#include <stdlib.h>

struct list {
   int data;
   struct list *next;
};
struct list *head = NULL;

int main(){
   struct list *current, *prev = NULL;
   for (int i = 1; i <= 100; ++i) {
      current = malloc(sizeof(struct list));
      if (!current) {
         printf("Memory allocation failed for %d\n", i);
         return 1;
      }
      current->data = i;
      current->next = NULL;

      if (prev == NULL) {
         head = current;
      } else {
         prev->next = current;
      }
      prev = current;
   }

   current = head;
   while (current) {
      printf("%d ", current->data);
      current = current->next;
   }

   while (head) {
      current = head;
      head = head->next;
      free(current);
   }
   return 0;
}