#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdbool.h>

typedef struct stack_node {
  int    ptr;
  void*  arr[10000];
} Stack;


void push(Stack* stack, void* value) {
  stack->arr[stack->ptr++] = value;
}

void* pop(Stack* stack) {
  return stack->arr[--stack->ptr];
}

typedef void (*function_ptr)();

typedef struct tree_node {
  char*             key;
  function_ptr      value;
  struct tree_node* left;
  struct tree_node* right;
} TreeNode;


TreeNode* create_node(char* key, function_ptr value) {
  TreeNode* node = (TreeNode*) malloc(sizeof(TreeNode));
  node->key = (char*) malloc(strlen(key) + 1);
  strcpy(node->key, key);
  node->value = value;
  node->left  = NULL;
  node->right = NULL;
  return node;
}


void insert_node(TreeNode** root, char* key, function_ptr value) {
  if ( *root == NULL )  {
    *root = create_node(key, value);
    return;
  }
  if ( strcmp(key, (*root)->key) < 0 ) {
    insert_node(&(*root)->left, key, value);
  } else {
    insert_node(&(*root)->right, key, value);
  }
}


function_ptr search_node(TreeNode* root, char* key) {
  if ( root == NULL ) return NULL;

  if ( strcmp(key, root->key) == 0 )
    return root->value;

  if ( strcmp(key, root->key) < 0 )
    return search_node(root->left, key);

  return search_node(root->right, key);
}


void free_tree(TreeNode* root) {
  if ( root == NULL ) return;
  free_tree(root->left);
  free_tree(root->right);
  free(root->key);
  free(root);
}


bool isSpace(char c) {
  return c == ' ' || c == '\t' || c == '\n';
}


bool readSym(char* buffer, int buffer_size) {
  char c;
  int size = 0;

  /* Skip leading whitespace. */
  while ( isSpace(c = getchar()) );
  if ( c == EOF ) return false;
  buffer[size++] = c;

  while ( (c = getchar()) != EOF && ! isSpace(c) && size < buffer_size - 1 ) {
    buffer[size++] = c;
  }
  buffer[size] = '\0';
  return size > 0;
}

Stack*    stack = NULL;
Stack*    heap  = NULL;
TreeNode* scope = NULL;

void constant(void* obj) { push(stack, obj); }


void one() { constant((void*) 1); }


void two() { constant((void*) 2); }


void plus() {
  long l2 = (long) pop(stack);
  long l1 = (long) pop(stack);
  push(stack, (void*) l1 + l2);
}


void minus() {
  long l2 = (long) pop(stack);
  long l1 = (long) pop(stack);
  push(stack, (void*) l1 - l2);
}


void print() {
  printf("%ld\n", (long) pop(stack));
}


void foo() { printf("foo\n"); }


void bar() { printf("bar\n"); }


void compile(function_ptr fn) {
  push(heap, (void*) fn);
}

void immediate(function_ptr fn) {
  fn();
}


void evalSym(char* sym, function_ptr evalulator) {
  function_ptr func = search_node(scope, sym);

  if ( func != NULL ) {
    evalulator(func);
  } else if ( sym[0] >= '0' && sym[0] <= '9' ) {
    constant((void*) atol(sym));
  } else {
    printf("Unknown word: %s\n", sym);
  }
}


int main() {
  char c;
  char buf[256];

  stack = (Stack*) malloc(sizeof(Stack));
  heap  = (Stack*) malloc(sizeof(Stack));

  insert_node(&scope, "foo",   &foo);
  insert_node(&scope, "bar",   &bar);
  insert_node(&scope, "1",     &one);
  insert_node(&scope, "2",     &two);
  insert_node(&scope, "+",     &plus);
  insert_node(&scope, "-",     &minus);
  insert_node(&scope, "print", &print);

  while ( readSym(buf, sizeof(buf)) ) {
    evalSym(buf, &immediate);
  }

  return 0;
}
