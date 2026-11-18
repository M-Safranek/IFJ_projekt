#include "symtable.h"
#include <stdio.h>

int main() {
  table_symbol *top_of_tree = 0;
  printf("%p\n", top_of_tree);
  top_of_tree = insert(top_of_tree, "test_func", true, true);
  printf("%p\n", top_of_tree);
  top_of_tree = insert(top_of_tree, "x", true, false);
  top_of_tree = insert(top_of_tree, "y", true, false);
  top_of_tree = insert(top_of_tree, "another_function", true, true);
  top_of_tree = insert(top_of_tree, "abcd", true, false);
  printf("%s\n", search("abcd", top_of_tree)->name);
  table_symbol *search_result = search("y", top_of_tree);
  search_result->modifiable = true;
  printf("%i\n", search("y", top_of_tree)->modifiable);
  delete_tree(top_of_tree);
}