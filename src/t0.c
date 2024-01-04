#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdbool.h>

#define DEBUG 1

/*
  Closures:
    - a heap pointer to a function
    - followed by its arguments
    - without a ret, so only a single statement

  Code:
    - a heap pointer to zero or more Closures
    - with a ret, so can be many statements

  Activation Record
    - Allocated on heap.

    - Format:
      return address / continuation?
      previous AR pointer
      space for arg1
      space for arg2
      ...
      space for argn

  Function Activation
    push return address onto heap (caller?)
    push current AR onto heap
    set AR to heap ptr
    for each arg, push(heap, pop(stack))
    <code>
    set AR to previous
    set ip to ret

  Symbol Table
    - maps String keys to heap pointers to Closures
    - Closure are executed when a symbol is looked up
    - addCmd() adds a Closure directly (interpreted)
    - addFn() adds a Closure which will emit the supplied function as code when run (compiled)

  Issues:
*/

typedef struct stack_ {
  long ptr;
  void* *arr;
} Stack;

Stack* createStack(long size) {
  Stack* s = (Stack*) malloc(sizeof(Stack));
  s->arr = malloc(size * sizeof(void*));
  return s;
}

long push(Stack* stack, void* value) {
  long r = stack->ptr;
  stack->arr[stack->ptr++] = value;
  return r;
}

long push2(Stack* stack, void* v1, void* v2) {
  long r = push(stack, v1);
  push(stack, v2);
  return r;
}

long push3(Stack* stack, void* v1, void* v2, void* v3) {
  long r = push2(stack, v1, v2);
  push(stack, v3);
  return r;
}


void* pop(Stack* stack) {
#ifdef DEBUG
  if ( stack->ptr <= 0 ) printf("POP from empty stack.\n");
#endif
  return stack->arr[--stack->ptr];
}

typedef void (*Fn)();

typedef struct scope_ {
  char*          key;
  long           ip;
  struct scope_* left;
  struct scope_* right;
} Scope;


Stack* stack = NULL;
Stack* calls = NULL;
Stack* heap  = NULL;
Stack* code  = NULL;
Scope* scope = NULL; // dictionary of words / closures
long   ip    = 0;    // instruction pointer, code being run
long   fp    = 0;    // frame pointer, pointer on heap of current frame / activation-record ???: can replace 'calls'?


void callClosure(long ptr) {
  long ret = ip;
  ip = ptr;
  Fn* fn = (Fn*) &(heap->arr[ip++]);
  (*fn)();
  ip = ret;
}


Scope* createScope(char* key, long ptr) {
  Scope* node = (Scope*) malloc(sizeof(Scope));
  node->key = (char*) malloc(strlen(key) + 1);
  strcpy(node->key, key);
  node->ip    = ptr;
  node->left  = node->right = NULL;
  return node;
}


/* Immutable version of addSym. Creates a new tree with added binding. */
Scope* addSym(Scope* root, char* key, long ptr) {
  if ( root == NULL )  return createScope(key, ptr);

  int cmp = strcmp(key, root->key);

  Scope* ret = createScope(root->key, root->ip);
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


void emitFn() { push(code, heap->arr[ip++]); }

long emitFnClosure(Fn fn) { return push2(heap, emitFn, fn); }


Scope* addFn(Scope* root, char* key, Fn fn) { return addSym(root, key, emitFnClosure(fn)); }

Scope* addCmd(Scope* root, char* key, Fn fn) { return addSym(root, key, push(heap, fn)); }


long findSym(Scope* root, char* key) {
  if ( root == NULL ) return -1;

  int c = strcmp(key, root->key);
  if ( c == 0 ) return root->ip;
  return findSym(c < 0 ? root->left : root->right, key);
}


bool isSpace(char c) {
  return c == ' ' || c == '\t' || c == '\n';
}


bool readSym(char* buf, int bufSize) {
  char c;
  int size = 0;

  /* Skip leading whitespace. */
  while ( isSpace(c = getchar()) );

  if ( c == EOF ) return false;

  buf[size++] = c;

  while ( (c = getchar()) != EOF && ! isSpace(c) && size < bufSize - 1 ) {
    buf[size++] = c;
  }
  buf[size] = '\0';

  return true;
}

/*
void jump() {
  long ptr = (long) heap->arr[ip++];
  call(ptr);
}
*/


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


/* The () word which calls a function on the top of the stack */
void call() {
  long ptr = (long) pop(stack);
  push(calls, (void*) ip);

  printf("Calling function at: %ld from: %ld\n", ptr, ip);
  ip = ptr;
  execute(ip++);
  printf("Returned to: %ld\n", ip);
}


void ret() {
  ip = (long) pop(calls);
  // printf("returning to %ld\n", ip);
}


void constant() {
  // Consume next constant value stored in the heap and push to stack
  long c = (long) heap->arr[ip++];
  push(stack, (void*) c);
}

long constantClosure(void* value) { return push2(heap, constant, value); }


void frameReference() {
  // Consume next constant value stored in the heap and push to stack
  long offset = (long) heap->arr[ip++];
  // push2At(heap, cp, constant, offset);
//  push(stack, (void*) offset);
}

long frameReferenceClosure(long offset) { return push2(heap, constant, (void*) offset); }


void evalSym(char* sym);

void autoConstant() {
  // Consume next constant value stored in the heap and push to stack
  long c = (long) heap->arr[ip++];
  push(stack, (void*) c);
  printf("autoConstant call %ld\n", c);
  push(calls, (void*) -1); // psedo return address causes stop to execution
  evalSym("()");
}

long autoConstantClosure(void* value) { return push2(heap, autoConstant, value); }


void define() {
  void* value = pop(stack);      // Definition Value
  char* sym   = heap->arr[ip++]; // Definition Key
  printf("define: %s %ld\n", sym, (long) value);

  scope = addSym(scope, sym, constantClosure(value));
}

/* Define a function that automatically executes when accessed without requiring () */
void defineAuto() {
  void* value = pop(stack);      // Definition Value
  char* sym   = heap->arr[ip++]; // Definition Key
  printf("defineAuto: %s %ld\n", sym, (long) value);

  scope = addSym(scope, sym, autoConstantClosure(value));

  char* sym2 = (char*) malloc(sizeof(sym)+1);
  sym2[0] = '&';
  strcpy(sym2+1, sym);
  scope = addSym(scope, sym2, constantClosure(value));
}


void plus() { push(stack, (void*) (long) pop(stack) + (long) pop(stack)); }

void minus() {
  long l2 = (long) pop(stack);
  long l1 = (long) pop(stack);
  push(stack, (void*) l1 - l2);
}

void multiply() { push(stack, (void*) ((long) pop(stack) * (long) pop(stack))); }

void divide() {
  long l2 = (long) pop(stack);
  long l1 = (long) pop(stack);
  push(stack, (void*) (l1 / l2));
}

void eq() { push(stack, (void*) (long) (pop(stack) == pop(stack))); }

void not() { push(stack, (void*) (long) (! (bool) pop(stack))); }

void and() { push(stack, (void*) (long) ((bool) pop(stack) && (bool) pop(stack))); }

void or() { push(stack, (void*) (long) ((bool) pop(stack) || (bool) pop(stack))); }

/*
void gosub() {
  long addr = (long) pop(stack);
  push(stack, (void*) ip);
}*/


void print() { printf("%ld\n", (long) pop(stack)); }


/*
 * Function used by evalSym() if an exact match isn't found.
 * Used to handle higher-level constructs like numbers, strings and functions.
 */
void unknownSymbol() {
  char* sym = (char*) pop(stack);

  if ( sym[0] == ':' ) {
    if ( sym[1] == ':' ) {
      // function definition appears as ::name
      char* s = strdup(sym+2);
      push2(code, defineAuto, s);
    } else {
      // function definition appears as :name
      char* s = strdup(sym+1);
      push2(code, define, s);
    }
  } else if ( sym[0] >= '0' && sym[0] <= '9' || ( sym[0] == '-' && sym[1] >= '0' && sym[1] <= '9' ) ) {
    // Parse Integers
    push2(code, constant, (void*) atol(sym));
    // printf("evaled number: %ld\n", (long) heap->arr[cp-1]);
  } else {
    printf("Unknown symbol: %s\n", sym);
  }
}


void evalSym(char* sym) {
  long ptr = findSym(scope, sym);

  if ( ptr == -1 ) {
    push(stack, sym);
    ptr = findSym(scope, "???");
    // ???: Could unknownSymbol be built into findSym()
    // Would allow for context inheritance
  }

  callClosure(ptr);
}


void defineVar(int i, char* name) {
  printf("defineVar: %d %s\n", i, name);
}


void defineFn() {
  char buf[256];

  Scope* s    = scope;
  long   vars = push(heap, 0 /* # of vars */);
  long   ocp  = code->ptr;
  int    i    = 0;

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

  for ( long j = 0 ; j < i ; j++ ) {
    defineVar(j, heap->arr[vars+1+j]);
    scope = addSym(scope, heap->arr[vars+1+j], frameReferenceClosure(j+100));
  }

  long ptr = code->ptr = heap->ptr;

  while ( readSym(buf, sizeof(buf)) ) {
    if ( strcmp(buf, "}") == 0 ) {
      printf("defineFn %ld bytes to %ld\n", code->ptr-ptr, ptr);
      push(code, ret);
      heap->ptr = code->ptr;
      code->ptr = ocp;

      push2(code, constant, (void*) ptr);

      return;
    }
    evalSym(buf);
  }

  scope = s; // revert to old scope

  printf("Syntax Error: Unclosed function, missing }");
}


void cppComment() {
  // Ignore C++ // style comments
  while ( getchar() != '\n' );
}


void cComment() {
  // Ignore C style /* */ comments
  for ( char c, prev ; ( c = getchar() ) ; prev = c )
    if ( prev == '*' && c == '/' ) return;
}


void printStack() {
  for ( long i = 0 ; i < stack->ptr ; i++ ) {
    printf("%ld ", (long) stack->arr[i]);
  }
}


int main() {
  char buf[256];

  heap  = createStack(1000000);
  stack = createStack(16000);
  calls = createStack(4000); // call stack
  code  = createStack(0);

  code->arr = heap->arr;

  heap->ptr = 1000; // Make space for REPL scratch

  scope = addCmd(scope, "???",  &unknownSymbol);
  scope = addCmd(scope, "/*",   &cComment);
  scope = addCmd(scope, "//",   &cppComment);
  scope = addCmd(scope, "{",    &defineFn);

  scope = addFn(scope, "+",     &plus);
  scope = addFn(scope, "-",     &minus);
  scope = addFn(scope, "*",     &multiply);
  scope = addFn(scope, "/",     &divide);
  scope = addFn(scope, "=",     &eq);
  scope = addFn(scope, "!",     &not);
  scope = addFn(scope, "&&",    &and);
  scope = addFn(scope, "||",    &or);
  scope = addFn(scope, "print", &print);
  scope = addFn(scope, ".",     &print); // like forth
  scope = addFn(scope, "()",    &call);

  while ( true ) {
    printf("heap: %ld, stack: ", heap->ptr); printStack(); printf("> ");

    if ( ! readSym(buf, sizeof(buf)) ) break;

    code->ptr = 0;
    evalSym(buf);

    push(code, ret);
    push(calls, (void*) -1); // psedo return address causes stop of execution

    printf("compiled %ld bytes\n", code->ptr);

    execute(0);
  }

  printf("\n");

  return 0;
}
