#include <stdio.h>
#include <stdlib.h>

typedef struct CooccurranceTable {
  unsigned int length;
  unsigned int capacity;
  int *target;
  int *context;
  double *cooccurrence;
} CooccurranceTable;

CooccurranceTable new_table(unsigned int initial_capacity) {
  CooccurranceTable res;
  res.length = 0;
  res.capacity = initial_capacity;
  res.target = (int *)calloc(initial_capacity, sizeof(int));
  res.context = (int *)calloc(initial_capacity, sizeof(int));
  res.cooccurrence = (double *)calloc(initial_capacity, sizeof(double));
  for (unsigned int i = 0; i < initial_capacity; i++) {
    // target = -1 means empty field.
    res.target[i] = -1;
    // initialise cooccurrence to zero
    res.cooccurrence[i] = 0;
  }
  return res;
}

void free_table(CooccurranceTable table) {
  free(table.target);
  free(table.context);
  free(table.cooccurrence);
}

int hash_tuple(int target, int context) {
  // Tuple hashing implementation, stolen from Java
  int res = 1;
  res = 31 * res + target;
  res = 31 * res + context;
  return res;
}

int increase_field(CooccurranceTable *table, int target, int context,
                   double amount) {
  // Increases a given field in the cooccurrence table with given amount
  // returns the index at which the element is stored.
  // Returns -1 if the table is out of capacity.
  if (table->length == table->capacity) {
    return -1;
  }
  int index = hash_tuple(target, context) % table->capacity;
  while (!(((table->target[index] == target) &&
            (table->context[index] == context)) ||
           (table->target[index] == -1))) {
    if (index == (table->capacity - 1)) {
      index = 0;
    }
    index++;
  }
  if (table->target[index] == -1) {
    table->length++;
    table->target[index] = target;
    table->context[index] = context;
  }
  table->cooccurrence[index] += amount;
  return index;
}

typedef struct TokenizedCorpus {
  unsigned int n_works;
  unsigned int length;
  int *tokens;
  unsigned int *lengths;
} TokenizedCorpus;

void count_cooccurrences(TokenizedCorpus corpus, CooccurranceTable *table,
                         unsigned int window, unsigned int target_lower,
                         unsigned int target_upper) {
  unsigned int i_doc = 0;
  unsigned int i_context_start = 0;
  unsigned int i_context_end = i_context_start + window * 2 + 1;
  unsigned int i_target = i_context_start + window + 1;
  unsigned int offset = 0;
  while (i_context_end < corpus.length) {
    while (i_context_end < (offset + corpus.lengths[i_doc])) {
      for (unsigned int i_context = i_context_start; i_context < i_context_end;
           i_context++) {
        if (i_target != i_context) {
          int target = corpus.tokens[i_target];
          int context = corpus.tokens[i_context];
          int distance = abs(i_target - i_context);
          double amount = 1 / (double)distance;
          if ((target >= target_lower) && (target < target_upper)) {
            int idx = increase_field(table, target, context, amount);
            if (idx == -1) {
              printf("Warning: Table is full\n");
            }
          }
        }
      }
      i_context_start++;
      i_context_end++;
      i_target++;
    }
    offset += corpus.lengths[i_doc];
    i_context_start = offset;
    i_context_end = i_context_start + window * 2 + 1;
    i_target = i_context_start + window + 1;
    i_doc += 1;
  }
}

void print_cooccurrence(CooccurranceTable table) {
  for (unsigned int i = 0; i < table.capacity; i++) {
    int target = table.target[i];
    int context = table.context[i];
    double amount = table.cooccurrence[i];
    printf("Target: %d, Context: %d - %f\n", target, context, amount);
  }
}

int main(void) {
  printf("Creating tokens.\n");
  int tokens[] = {
      // Doc 1 (11)
      1,
      1,
      5,
      6,
      3,
      3,
      1,
      8,
      3,
      1,
      5,
      // Doc 2 (9)
      3,
      5,
      5,
      3,
      6,
      1,
      4,
      1,
      5,
      // Doc 3 (4)
      3,
      5,
      5,
      3,
  };
  unsigned int lengths[] = {11, 9, 4};
  TokenizedCorpus corpus;
  corpus.lengths = lengths;
  corpus.length = 11 + 9 + 4;
  corpus.n_works = 3;
  corpus.tokens = tokens;

  printf("Creating table.\n");
  CooccurranceTable table = new_table(30);
  printf("Counting cooccurrences.\n");
  count_cooccurrences(corpus, &table, 3, 0, 10);
  printf("Cooccurrences:\n");
  printf("Capacity %d\n", table.capacity);
  print_cooccurrence(table);
  free_table(table);
}
