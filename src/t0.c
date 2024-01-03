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
} Scope;


Stack* stack = NULL;
Stack* calls = NULL; // call stack
Stack* heap  = NULL;
Scope* scope = NULL; // dictionary of words / closures
long   ip    = 0;    // instruction pointer
long   cp    = 0;    // code pointer, where code is being emitted to


void call(long ptr) {
  long ret = ip;
  ip = ptr;
  Fn* fn = (Fn*) &(heap->arr[ip++]);
  (*fn)();
  ip = ret;
}


Scope* createNode(char* key, long ptr) {
  Scope* node = (Scope*) malloc(sizeof(Scope));
  node->key = (char*) malloc(strlen(key) + 1);
  strcpy(node->key, key);
  node->ip    = ptr;
  node->left  = NULL;
  node->right = NULL;
  return node;
}


/* Immutable version of addSym. Creates a new tree with added binding. */
Scope* addSym(Scope* root, char* key, long ptr) {
  if ( root == NULL )  return createNode(key, ptr);

  int cmp = strcmp(key, root->key);

  Scope* ret = createNode(root->key, root->ip);
  ret->left  = root->left;
  ret->right = root->right;

  if ( cmp == 0 ) {
    // Should we free key in this case?
    ret->ip = ptr;
  } else if ( cmp < 0 ) {
    ret->left  = addSym(ret->left, key, ptr);
  } else {
    ret->right = addSym(ret->right, key, ptr);
  }

  return ret;
}


void emitFn() {
  heap->arr[cp++] = heap->arr[ip++];
}


Scope* addFn(Scope* root, char* key, Fn fn) {
  long ptr = heap->ptr;
  push(heap, emitFn);
  push(heap, fn);
  return addSym(root, key, ptr);
}


Scope* addCmd(Scope* root, char* key, Fn fn) {
  long ptr = heap->ptr;
  push(heap, fn);
  return addSym(root, key, ptr);
}


long findSym(Scope* root, char* key) {
  if ( root == NULL ) return -1;

  int c = strcmp(key, root->key);
  if ( c == 0 ) return root->ip;
  return findSym(c < 0 ? root->left : root->right, key);
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

  scope = addSym(scope, sym, ptr);
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
  scope = addSym(scope, sym, ptr);

  char* sym2 = (char*) malloc(sizeof(sym)+1);
  sym2[0] = '&';
  strcpy(sym2+1, sym);
  ptr = heap->ptr;
  push(heap, constant);
  push(heap, value);
  push(heap, ret);
  scope = addSym(scope, sym2, ptr);
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


void unknownSymbol() {
  char* sym = (char*) pop(stack);

  if ( sym[0] == ':' ) {
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
    // printf("evaled number: %ld\n", (long) heap->arr[cp-1]);
  } else {
    printf("Unknown symbol: %s\n", sym);
  }
}


void evalSym(char* sym) {
  long ptr = findSym(scope, sym);

  if ( ptr == -1 ) {
    push(stack, sym);
    ptr = findSym(scope, "unknownSymbol");
  }

  call(ptr);
}


void printStack() {
  for ( long i = 0 ; i < stack->ptr ; i++ ) {
    printf("%ld ", (long) stack->arr[i]);
  }
}


void defineVar(int i, char* name) {
  printf("defineVar: %d %s\n", i, name);
}


void defineFn() {
  char buf[256];

  Scope* s    = scope;
  long   vars = heap->ptr;
  long   ocp  = cp;
  int    i    = 0;

  push(heap, 0); // number of vars

  while ( true ) {

    if ( ! readSym(buf, sizeof(buf)) ) {
      printf("Syntax Error: Unclosed function, missing |");
      return;
    }

    if ( strcmp(buf, "|") == 0 ) break;

    /*
    function countFrames() { var d = 0, s = scope; while ( s !== curScope ) { s = s.__proto__; d++; } return d; }
    function framesUp(d) { var p = hp; for ( var i = 0 ; i < d ; i++ ) p = heap[p]; return p; }
    function accessor(index, f) { return code => { var d = countFrames(); code.push(() => f(framesUp(d) + index)); } }
    function defineVar(v, index) {
      scope[v]        = accessor(index, i => stack.push(heap[i]));
      scope[':' + v]  = accessor(index, i => { heap2[i] = v; heap[i] = stack.pop(); });
      scope[v + '++'] = accessor(index, i => heap[i]++);
      scope[v + '--'] = accessor(index, i => heap[i]--);
    }
    */

    // Add var name to 'vars'
    push(heap, strdup(buf));
    heap->arr[vars]++;
    i++;
  }

  for ( int j = 0 ; j < i ; j++ ) {
    defineVar(j, heap->arr[vars+1+j]);
  }

  long ptr = cp = heap->ptr;

  while ( readSym(buf, sizeof(buf)) ) {
    if ( strcmp(buf, "}") == 0 ) {
      printf("defineFn %ld bytes to %ld\n", cp-ptr, ptr);
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

  scope = addCmd(scope, "unknownSymbol",   &unknownSymbol);
  scope = addCmd(scope, "/*",   &cComment);
  scope = addCmd(scope, "//",   &cppComment);
  scope = addCmd(scope, "{",    &defineFn);

  scope = addFn(scope, "+",     &plus);
  scope = addFn(scope, "-",     &minus);
  scope = addFn(scope, "*",     &multiply);
  scope = addFn(scope, "/",     &divide);
  scope = addFn(scope, "=",     &eq);
  scope = addFn(scope, "!",     &not);
  scope = addFn(scope, "print", &print);
  scope = addFn(scope, ".",     &print); // like forth
  scope = addFn(scope, "()",    &callFn);

  scope = addFn(scope, "-1",    &minusOne);
  scope = addFn(scope, "0",     &zero);
  scope = addFn(scope, "1",     &one);
  scope = addFn(scope, "2",     &two);
  scope = addFn(scope, "10",    &ten);

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
