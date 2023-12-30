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

typedef void (*Fn)();

typedef struct tree_node {
  char*             key;
  Fn*               fn;
  struct tree_node* left;
  struct tree_node* right;
} SymNode;


Stack*   stack = NULL;
Stack*   heap  = NULL;
SymNode* scope = NULL;
long     ip    = 0;


void call(Fn* fn) {
  // TODO: pass ptr OR what if SymNode contained multiple inlined words?
  (*fn)();
}


SymNode* create_node(char* key, Fn* fn) {
  SymNode* node = (SymNode*) malloc(sizeof(SymNode));
  node->key = (char*) malloc(strlen(key) + 1);
  strcpy(node->key, key);
  node->fn = fn;
//   node->fn = (Fn*) &(heap->arr[heap->ptr]);
  push(heap, fn);
  node->left  = NULL;
  node->right = NULL;
  return node;
}


void insert_node(SymNode** root, char* key, Fn* fn) {
  if ( *root == NULL )  {
    *root = create_node(key, fn);
  } else {
    insert_node(
      strcmp(key,(*root)->key) < 0 ? &(*root)->left : &(*root)->right,
      key,
      fn);
  }
}

void insertFn(SymNode** root, char* key, Fn fn) {
  Fn* def = (Fn*) &(heap->arr[heap->ptr]);
  push(heap, fn);
  insert_node(root, key, def);
}


Fn* search_node(SymNode* root, char* key) {
  if ( root == NULL ) return NULL;

  int c = strcmp(key, root->key);
  if ( c == 0 ) return root->fn;
  return search_node(c < 0 ? root->left : root->right, key);
}

/*
void free_tree(SymNode* root) {
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


void constant() {
  // Consume next constant value stored in the heap and push to stack
  push(stack, heap->arr[ip++]);
}

void define() {
  void* value = pop(stack);      // Definition Value
  char* sym   = heap->arr[ip++]; // Definition Key
printf("define: %s %ld", sym, (long) value);
  Fn* def = (Fn*) &(heap->arr[heap->ptr]);
  push(heap, constant);
  push(heap, value);

  insert_node(&scope, sym, def);
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
  Fn* fn = search_node(scope, sym);

  if ( fn != NULL ) {
    call(fn);
  } else if ( sym[0] == ':' ) {
    char* s = strdup(sym+1);
    heap->arr[ip++] = define;
    heap->arr[ip++] = s;
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
    ((Fn) heap->arr[ip++])();
  }
}


int main() {
  char c;
  char buf[256];

  stack = (Stack*) malloc(sizeof(Stack));
  heap  = (Stack*) malloc(sizeof(Stack));

  insertFn(&scope, "foo",   &foo);
  insertFn(&scope, "bar",   &bar);
  insertFn(&scope, "+",     &plus);
  insertFn(&scope, "-",     &minus);
  insertFn(&scope, "=",     &eq);
  insertFn(&scope, "!",     &not);
  insertFn(&scope, "print", &print);
  insertFn(&scope, ".",     &print); // like forth

  // These could be moved to t0 code.
  insertFn(&scope, "-1",    &minusOne);
  insertFn(&scope, "0",     &zero);
  insertFn(&scope, "1",     &one);
  insertFn(&scope, "2",     &two);
  insertFn(&scope, "10",    &ten);

  while ( readSym(buf, sizeof(buf)) ) {
    ip = SCRATCH;
    evalSym(buf);
    heap->arr[ip] = 0; // mark end of code
    execute(SCRATCH);
  }

  return 0;
}
