#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdbool.h>

typedef struct stack_node {
  int    ptr;
  void*  arr[10000];
} Stack;

/* Part of Heap where interactive commands are temporarily compiled to. */
#define SCRATCH (sizeof(Stack)-1000)

void push(Stack* stack, void* value) {
  stack->arr[stack->ptr++] = value;
}

void* pop(Stack* stack) {
  return stack->arr[--stack->ptr];
}

typedef void (*function_ptr)();

// An alternative to adding 'cl' would be to have custom tree_node stuctures
typedef struct tree_node {
  char*             key;
  function_ptr      value;
  long              cl;    // Heap pointer for closure-like extra data
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
  insert_node(
    strcmp(key,(*root)->key) < 0 ? &(*root)->left : &(*root)->right,
    key,
    value);
}


function_ptr search_node(TreeNode* root, char* key) {
  if ( root == NULL ) return NULL;

  int c = strcmp(key, root->key);
  if ( c == 0 ) return root->value;
  return search_node(c < 0 ? root->left : root->right, key);
}

/*
void free_tree(TreeNode* root) {
  if ( root == NULL ) return;
  free_tree(root->left);
  free_tree(root->right);
  free(root->key);
  free(root);
}
*/


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

  return true;
}

Stack*    stack = NULL;
Stack*    heap  = NULL;
TreeNode* scope = NULL;
long      ip    = 0;

void constant() {
  // Consume next constant value stored in the heap and push to stack
  push(stack, heap->arr[ip++]);
}

void define() {
  void* value = pop(stack);      // Definition Value
  char* sym   = heap->arr[ip++]; // Definition Key

//  insert_node(&scope, sym, ???); // needs to be an ip

}

void minusOne() { push(stack, (void*)  -1); }
void zero()     { push(stack, (void*)  0);  }
void one()      { push(stack, (void*)  1);  }
void two()      { push(stack, (void*)  2);  }
void ten()      { push(stack, (void*)  10); }


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


void eq() {
  long l2 = (long) pop(stack);
  long l1 = (long) pop(stack);
  bool ret = l1 == l2;
  push(stack, (void*) ret);
}


void not() {
  bool ret = ! (bool) pop(stack);
  push(stack, (void*) ret);
}


void gosub() {
  long addr = (long) pop(stack);
  push(stack, (void*) ip);
}


void ret() {
  ip = (long) pop(stack);
}


void print() {
  printf("%ld\n", (long) pop(stack));
}


void foo() { printf("foo\n"); }

void bar() { printf("bar\n"); }


void evalSym(char* sym) {
  function_ptr func = search_node(scope, sym);

  if ( func != NULL ) {
    func();
  } else if ( sym[0] == ':' ) {
    char* sym = strdup(sym+1);
    heap->arr[ip++] = define;
    heap->arr[ip++] = sym;
    /*
    var sym = line.substring(1);
    code.push(function() { var value = stack.pop(); scope[sym] = (code) => code.push(() => stack.push(value))});
    */

  } else if ( strcmp("//", sym) == 0 ) {
    // Ignore C++ style comments
    while ( getchar() != '\n' );
  } else if ( strcmp("/*", sym) == 0 ) {
    // Ignore C style comments
    int state = 0;
    char c;
    while ( ( c = getchar() ) ) {
      switch ( state ) {
        case 0: if ( c == '*' ) state = 1; break;
        case 1: if ( c == '/' ) return; state = c == '*' ? 1 : 0;
      }
    }
  } else if ( sym[0] >= '0' && sym[0] <= '9' ) {
    // Parse Integers
    heap->arr[ip++] = constant;
    heap->arr[ip++] = (void*) atol(sym);
  } else {
    printf("Unknown word: %s\n", sym);
  }
}

/** Execute code starting at ip until 0 found. **/
void execute(long ptr) {
  for ( ip = ptr ; heap->arr[ip] ; ip++ ) {
    ((function_ptr) heap->arr[ip++])();
  }
}


int main() {
  char c;
  char buf[256];

  stack = (Stack*) malloc(sizeof(Stack));
  heap  = (Stack*) malloc(sizeof(Stack));

  insert_node(&scope, "foo",   &foo);
  insert_node(&scope, "bar",   &bar);
  insert_node(&scope, "+",     &plus);
  insert_node(&scope, "-",     &minus);
  insert_node(&scope, "=",     &eq);
  insert_node(&scope, "!",     &not);
  insert_node(&scope, "print", &print);
  insert_node(&scope, ".",     &print); // like forth

  // These could be moved to t0 code.
  insert_node(&scope, "-1",    &minusOne);
  insert_node(&scope, "0",     &zero);
  insert_node(&scope, "1",     &one);
  insert_node(&scope, "2",     &two);
  insert_node(&scope, "10",    &ten);

  while ( readSym(buf, sizeof(buf)) ) {
    ip = SCRATCH;
    evalSym(buf);
    heap->arr[ip] = 0; // mark end of code
    execute(SCRATCH);
  }

  return 0;
}
