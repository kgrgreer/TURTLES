#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdbool.h>

#define DEBUG 1

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
#ifdef DEBUG
  if ( stack->ptr <= 0 ) printf("POP from empty stack.\n");
#endif
  return stack->arr[--stack->ptr];
}

typedef void (*Fn)();

typedef struct tree_node {
  char*             key;
  long              ip;
  struct tree_node* left;
  struct tree_node* right;
} SymNode;


Stack*   stack = NULL;
Stack*   calls = NULL; // call stack
Stack*   heap  = NULL;
SymNode* scope = NULL; // dictionary of words / closures
long     ip    = 0;    // instruction pointer
long     cp    = 0;    // code pointer, where code is being emitted to


void call(long ptr) {
  long ret = ip;
  ip = ptr;
  Fn* fn = (Fn*) &(heap->arr[ip++]);
  (*fn)();
  ip = ret;
}


SymNode* create_node(char* key, long ptr) {
  SymNode* node = (SymNode*) malloc(sizeof(SymNode));
  node->key = (char*) malloc(strlen(key) + 1);
  strcpy(node->key, key);
  node->ip    = ptr;
  node->left  = NULL;
  node->right = NULL;
  return node;
}


void insert_node(SymNode** root, char* key, long ptr) {
  if ( *root == NULL )  {
    *root = create_node(key, ptr);
  } else {
    insert_node(
      strcmp(key,(*root)->key) < 0 ? &(*root)->left : &(*root)->right,
      key,
      ptr);
  }
}


void emitFn() {
  heap->arr[cp++] = heap->arr[ip++];
}


void insertFn(SymNode** root, char* key, Fn fn) {
  long ptr = heap->ptr;
  push(heap, emitFn);
  push(heap, fn);
  insert_node(root, key, ptr);
}


void insertCmd(SymNode** root, char* key, Fn fn) {
  long ptr = heap->ptr;
  push(heap, fn);
  insert_node(root, key, ptr);
}


long search_node(SymNode* root, char* key) {
  if ( root == NULL ) return -1;

  int c = strcmp(key, root->key);
  if ( c == 0 ) return root->ip;
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


void jump() {
  long ptr = (long) heap->arr[ip++];
  call(ptr);
}


/** Execute code starting at ip until 0 found. **/
void execute(long ptr) {
  for ( ip = ptr ; ; ) {
    // printf("executing: %ld %ld\n", ip, (long) heap->arr[ip]);
    Fn fn = (Fn) heap->arr[ip++];
    fn();
    // printf("executed %ld\n", ip);
    if ( ip == -1 ) return;
  }
}


void callFn() {
  // The () word which calls a function on the top of the stack
  long ptr = (long) pop(stack);
  push(calls, (void*) ip);

  printf("Calling function at: %ld from: %ld\n", ptr, ip);
  ip = ptr;
  execute(ip++);
  printf("Returned to: %ld\n", ip);
}


void constant() {
  // Consume next constant value stored in the heap and push to stack
  long c = (long) heap->arr[ip++];
  push(stack, (void*) c);
}


void autoConstant() {
  // Consume next constant value stored in the heap and push to stack
  long c = (long) heap->arr[ip++];
  push(stack, (void*) c);
  printf("autoConstant call %ld\n", c);
  push(calls, (void*) -1); // psedo return address causes stop to execution
  callFn(); // crashes after this
}


void ret() {
  ip = (long) pop(calls);
  // printf("returning to %ld\n", ip);
}


void define() {
  void* value = pop(stack);      // Definition Value
  char* sym   = heap->arr[ip++]; // Definition Key
  printf("define: %s %ld\n", sym, (long) value);

  long ptr = heap->ptr;
  push(heap, constant);
  push(heap, value);
  push(heap, ret);

  insert_node(&scope, sym, ptr);
}


void defineAuto() {
  // Define a function that automatically executes when accessed without requiring ()
  void* value = pop(stack);      // Definition Value
  char* sym   = heap->arr[ip++]; // Definition Key
  printf("defineAuto: %s %ld\n", sym, (long) value);

  long ptr = heap->ptr;
  push(heap, autoConstant);
  push(heap, value);
  push(heap, ret);
  insert_node(&scope, sym, ptr);

  char* sym2 = (char*) malloc(strlen(sym)+2);
  sym2[0] = '&';
  strcpy(sym2+1, sym);
  ptr = heap->ptr;
  push(heap, constant);
  push(heap, value);
  push(heap, ret);
  insert_node(&scope, sym2, ptr);
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


void multiply() {
  long l2 = (long) pop(stack);
  long l1 = (long) pop(stack);
  push(stack, (void*) (l1 * l2));
}


void divide() {
  long l2 = (long) pop(stack);
  long l1 = (long) pop(stack);
  push(stack, (void*) (l1 / l2));
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


void print() {
  printf("%ld\n", (long) pop(stack));
}


void evalSym(char* sym) {
  long ptr = search_node(scope, sym);

  // ???: If symbol not found then could fallback to 'unknownSymbol'
  // which would allow for extension through decoration.

  if ( ptr != -1 ) {
    // printf("evaled: %s\n", sym);
    call(ptr);
  } else if ( sym[0] == ':' ) {
    if ( sym[1] == ':' ) {
      // function definition appears as ::name
      char* s = strdup(sym+2);
      heap->arr[cp++] = defineAuto;
      heap->arr[cp++] = s;
    } else {
      // function definition appears as :name
      char* s = strdup(sym+1);
      heap->arr[cp++] = define;
      heap->arr[cp++] = s;
    }
  } else if ( sym[0] >= '0' && sym[0] <= '9' ) {
    // Parse Integers
    heap->arr[cp++] = constant;
    heap->arr[cp++] = (void*) atol(sym);
    printf("evaled number: %ld\n", (long) heap->arr[cp-1]);
  } else {
    printf("Unknown word: %s\n", sym);
  }
}


void foo() { printf("foo\n"); }

void bar() { printf("bar\n"); }


void printStack() {
  for ( long i = 0 ; i < stack->ptr ; i++ ) {
    printf("%ld ", (long) stack->arr[i]);
  }
}

/*
void callC() {
}
*/


void defun() {
  char buf[256];

  long ptr = heap->ptr;
  long ocp = cp;

  cp = heap->ptr;

  while ( readSym(buf, sizeof(buf)) ) {
    if ( strcmp(buf, "}") == 0 ) {
      printf("defun %ld bytes to %ld\n", cp-ptr, ptr);
      heap->arr[cp++] = ret;
      heap->ptr       = cp;
      cp              = ocp;

      heap->arr[cp++] = constant;
      heap->arr[cp++] = (void*) ptr;

      return;
    }
    evalSym(buf);
  }

  printf("Syntax Error: Unclosed function, missing }");
}


void cppComment() {
  // Ignore C++ // style comments
  while ( getchar() != '\n' );
}


void cComment() {
  // Ignore C style /* */ comments
  int state = 0;
  char c;
  while ( ( c = getchar() ) ) {
    switch ( state ) {
      case 0: if ( c == '*' ) state = 1; break;
      case 1: if ( c == '/' ) return; state = c == '*' ? 1 : 0;
    }
  }
}


int main() {
  char c;
  char buf[256];

  calls = (Stack*) malloc(sizeof(Stack)); // TODO: make smaller
  stack = (Stack*) malloc(sizeof(Stack));
  heap  = (Stack*) malloc(sizeof(Stack));

  insertCmd(&scope, "/*",   &cComment);
  insertCmd(&scope, "//",   &cppComment);
  insertCmd(&scope, "{",    &defun);

  insertFn(&scope, "foo",   &foo);
  insertFn(&scope, "bar",   &bar);
  insertFn(&scope, "+",     &plus);
  insertFn(&scope, "-",     &minus);
  insertFn(&scope, "*",     &multiply);
  insertFn(&scope, "/",     &divide);
  insertFn(&scope, "=",     &eq);
  insertFn(&scope, "!",     &not);
  insertFn(&scope, "print", &print);
  insertFn(&scope, ".",     &print); // like forth
  insertFn(&scope, "()",    &callFn);

  // These could be moved to t0 code.
  insertFn(&scope, "-1",    &minusOne);
  insertFn(&scope, "0",     &zero);
  insertFn(&scope, "1",     &one);
  insertFn(&scope, "2",     &two);
  insertFn(&scope, "10",    &ten);

  while ( true ) {
    printf("heap: %d, stack: ", heap->ptr); printStack(); printf("> ");
    if ( ! readSym(buf, sizeof(buf)) ) break;
    cp = SCRATCH;
    evalSym(buf);

    heap->arr[cp++] = ret;
    push(calls, (void*) -1); // psedo return address causes stop to execution
    printf("compiled %ld bytes\n", cp-SCRATCH);
    execute(SCRATCH);
  }

  printf("\n");

  return 0;
}
