// t0.c
// Author: Kevin Greer

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdbool.h>

#define DEBUG 1

/*
  Symbols:
    - a sequence of non whitespace characters
    - program input is symbols divided by whitespace characters
    - are evaluated by evalSym() which tries to look them up in
      the "scope" symbol table, and if it doesn't find an exact
      match then it passes to the "???" function, also found in
      the symbol table
    - symbol dictionary entries point to Closures which execute directly,
      however, most symbols actually push/emit/compile code onto the "code" stack

  Closures:
    - a heap pointer to a function
    - followed by its arguments
    - without a ret, so only a single statement
    - created like: push2(heap, constant, value), where, in this case, "constant" is a function and "value" is a value that it remembers/uses

  Code:
    - a heap pointer to zero or more Closures
    - ends with a "ret", so can be many statements

  Activation Record
    - Allocated on heap.

    - Format:
      previous AR pointer called "fp" Frame-Pointer
      return address / continuation?
      space for arg1
      space for arg2
      ...
      space for argn

  Function Activation
    push current fp onto heap, update fp to this address
    push return address onto heap
    for each arg, push(heap, pop(stack))
    <code>
    set fp to previous
    set ip to ret

  Symbol Table
    - maps String keys to heap pointers to Closures
    - Closure are executed when a symbol is looked up
    - addCmd() adds a Closure directly (interpreted)
    - addFn() adds a Closure which will emit the supplied function as code when run (compiled)

  Issues:
    - malloc() is used in some places where the heap should be used instead so
      that memory can be GC'ed in the future
    - frame references could/should be reused
    - nested functions will write into each other, need to use scratch spaces then copy?
*/

void evalSym(char* sym);


typedef struct stack_ {
  long ptr;
  void* *arr;
} Stack; // Perhaps something like "Region" or "Space" would be a better name?


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


Scope* scope = NULL; // dictionary of words -> closures
Stack* stack = NULL; // used to pass arguments
Stack* heap  = NULL; // used for everything else, including frames
Stack* code  = NULL; // where code is generated to, shares memory with heap but has own pointer
long   ip    = 0;    // instruction pointer, code being run
long   fp    = 0;    // frame pointer, pointer on heap of current frame / activation-record
int    fd    = 0;    // frame depth


void* nextI() { return heap->arr[ip++]; } // next instruction or instruction argument


// Execute a single closure stored at the pointed to heap location. Not 'ret' terminated.
void callClosure(long ptr) {
  long ret = ip;
  ip = ptr;
    Fn fn = (Fn) nextI();
    (fn)();
  ip = ret;
}


Scope* createScope(char* key, long ptr) {
  Scope* node = (Scope*) malloc(sizeof(Scope));
  node->key   = (char*)  malloc(sizeof(key));
  node->ip    = ptr;
  node->left  = node->right = NULL;
  strcpy(node->key, key);
  return node;
}


/* Immutable version of addSym. Creates a new tree with added binding. */
Scope* addSym(Scope* root, char* key, long ptr) {
  if ( root == NULL ) return createScope(key, ptr);

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

void emitFn() { push(code, nextI()); }

Scope* addFn(Scope* root, char* key, Fn fn)  { return addSym(root, key, push2(heap, emitFn, fn)); }

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


/** Execute code starting at ip until 0 found. **/
void execute(long ptr) {
  for ( ip = ptr ; ; ) {
    // printf("executing: %ld %ld\n", ip, (long) heap->arr[ip]);
    Fn fn = (Fn) nextI();
    fn();
    // printf("executed %ld\n", ip);
    if ( ip == -1 ) return;
  }
}


void closure() {
  // Consume next constant value stored in the heap and push to stack
  void* fn = nextI();
  push(stack, (void*) push2(heap, (void*) fp, fn));
}


// The "()" word which calls a function on the top of the stack
void call() {
  long closure = (long) pop(stack);
  long pfp     = (long) heap->arr[closure];   // previous fp
  long ptr     = (long) heap->arr[closure+1]; // fn ptr

  fp = push2(heap, (void*) pfp, (void*) ip); // previous FP, return address

  // printf("Calling function at: %ld from: %ld\n", ptr, ip);
  ip = ptr;
  execute(ip++);
  // fp = (long) heap->arr[fp];
  // printf("Returned to: %ld\n", ip);
}


void ret() {
  ip = (long) heap->arr[fp+1]; // jump to return address
  fp = (long) heap->arr[fp];   // restore previous fp
  // printf("returning to %ld\n", ip);
}


void constant() {
  // Consume next constant value stored in the heap and push to stack
  long c = (long) nextI();
  push(stack, (void*) c);
}


void autoConstant() {
  // Consume next constant value stored in the heap and push to stack
  long c = (long) nextI();
  push(stack, (void*) c);
  evalSym("()");
}


long frameOffset(long depth, long offset) {
  long f = fp;
  printf("frame offset: depth: %ld offset: %ld fd: %d\n", depth, offset, fd);
  for ( int i = 0 ; i < depth ; i++ ) f = (long) heap->arr[f];
  return f+2+offset;
}


void frameReference() {
  int  frame  = (int)  nextI();
  long offset = (long) nextI();
  // printf("frame reference fp: %ld offset: %ld value: %ld\n", fp, offset, (long) heap->arr[2+fp+offset]);
  // Consume next constant value stored in the heap and push to stack
  push(stack, (void*) heap->arr[frameOffset(frame, offset)]);
}

void frameReferenceEmitter() {
  // Consume next constant value stored in the heap and push to stack
  push3(code, frameReference, (void*) (long) (fd-(int)nextI()), nextI());
}

void frameSetter() {
  int  frame  = (int)  nextI();
  long offset = (long) nextI();
  void* value = pop(stack);
  // printf("frame setter fp: %ld offset: %ld = value: %ld\n", fp, offset, (long) value);
  // Consume next constant value stored in the heap and push to stack
  heap->arr[frameOffset(frame, offset)] = value;
}

void frameSetterEmitter() {
  push3(code, frameSetter, (void*) (long) (fd-(int)nextI()), nextI());
}

void frameIncr() {
  int  frame  = (int)  nextI();
  long offset = (long) nextI();
  heap->arr[frameOffset(frame, offset)]++;
}

void frameIncrEmitter() {
  push3(code, frameIncr, (void*) (long) (fd-(int)nextI()), nextI());
}

void frameDecr() {
  int  frame  = (int)  nextI();
  long offset = (long) nextI();
  heap->arr[frameOffset(frame, offset)]--;
}

void frameDecrEmitter() {
  push3(code, frameDecr, (void*) (long) (fd-(int)nextI()), nextI());
}


// Copy argument values from stack to the heap, as part of the activation record, where they can be accessed as frameReferences
void localVarSetup() {
  long numVars = (long) nextI();

  // printf("Copying %ld vars, fp: %ld\n", numVars, fp);
  for ( long i = 0 ; i < numVars ; i++ ) {
    // printf("%ld : %ld  @ %ld\n", i, (long) stack->arr[stack->ptr-1], heap->ptr);
    push(heap, pop(stack));
  }
}


// Define a top-level constant value, Ex. 42 :answer or { x | x 2 * } :double
void define() {
  void* value = pop(stack);  // Definition Value
  char* sym   = nextI();     // Definition Key
  // printf("define: %s %ld\n", sym, (long) value);

  scope = addSym(scope, sym, push2(heap, constant, value));
}



char* strAdd(char* s1, char* s2) {
  int l1 = strlen(s1);
  char* s3 = (char*) malloc(l1 + sizeof(s2));

  strcpy(s3, s1);
  strcpy(s3+l1, s2);

  return s3;
}


// Define a function that automatically executes when accessed without requiring ()
void defineAuto() {
  void* value = pop(stack);      // Definition Value
  char* sym   = nextI(); // Definition Key
  // printf("defineAuto: %s %ld\n", sym, (long) value);

  scope = addSym(scope, sym,              push2(heap, autoConstant, value));
  scope = addSym(scope, strAdd("&", sym), push2(heap, constant, value));
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

void print() { printf("%ld\n", (long) pop(stack)); }


/*
 * Function used by evalSym() if an exact match isn't found.
 * Used to handle higher-level constructs like numbers, strings and functions.
 */
void unknownSymbol() {
  char* sym = (char*) pop(stack);

  if ( sym[0] == ':' ) {
    if ( sym[1] == ':' ) {
      // function definition appears as ::name, an auto variable
      char* s = strdup(sym+2);
      push2(code, defineAuto, s);
    } else {
      // function definition appears as :name, a regular variable
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


void defineFn() {
  char buf[256];

  Scope* s    = scope;
  long   vars = push(heap, 0 /* # of vars */);
  int    i    = 0; // number of vars / arguments

  fd++;

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
    char* varName = heap->arr[vars+1+j];
    void* k       = (void*) (i-j-1);
    scope = addSym(scope, varName,               push3(heap, frameReferenceEmitter, (void*) (long) fd, k));
    scope = addSym(scope, strAdd(":", varName),  push3(heap, frameSetterEmitter,    (void*) (long) fd, k));
    scope = addSym(scope, strAdd(varName, "++"), push3(heap, frameIncrEmitter,      (void*) (long) fd, k));
    scope = addSym(scope, strAdd(varName, "--"), push3(heap, frameDecrEmitter,      (void*) (long) fd, k));
  }

  Stack* oldCode = code;
  Stack  code2; // A temp code buffer to allow for reentrant function parsing
  void*  arr[1024];

  code2.ptr = 0;
  code2.arr = arr;
  code      = &code2;

  push2(code, localVarSetup, (void*) (long) i);

  while ( true ) {
    if ( ! readSym(buf, sizeof(buf)) ) {
      printf("Syntax Error: Unclosed function, missing }");
      return;
    }

    if ( strcmp(buf, "}") == 0 ) break;

    evalSym(buf);
  }

  // printf("defineFn %ld bytes to %ld\n", code->ptr-ptr, ptr);
  push(code, ret);

  long ptr = heap->ptr; // location where function will be copied to

  for ( int i = 0 ; i < code2.ptr ; i++ ) push(heap, arr[i]);

  code = oldCode;

  push2(code, closure, (void*) ptr);
//   push2(code, constant, (void*) ptr);

  scope = s; // revert to old scope

  fd--;

  return;
}


// Ignore C++ style // comments
void cppComment() { while ( getchar() != '\n' ); }


// Ignore C style /* */ comments
void cComment() {
  for ( char c, prev ; ( c = getchar() ) ; prev = c )
    if ( prev == '*' && c == '/' ) return;
}


void printStack() {
  for ( long i = 0 ; i < stack->ptr ; i++ )
    printf("%ld ", (long) stack->arr[i]);
}


int main() {
  char buf[256]; // Used to hold next read symbols

  heap  = createStack(1000000);
  stack = createStack(16000);
  code  = createStack(0);

  // Code stack shares memory with heap, just has its own ptr
  code->arr = heap->arr;

  heap->ptr = 1000; // Make space for REPL scratch space

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

  // Create a top-level frame which points to itself as the previous frame
  // so it can be reused forever.
  push(code, (void*) 0);  // points to itself as previous frame
  push(code, (void*) -1); // psedo return address causes stop of execution

  while ( true ) {
    // printf("heap: %ld, stack: ", heap->ptr); printStack(); printf("> ");

    if ( ! readSym(buf, sizeof(buf)) ) break;

    code->ptr = 2;    // skip over frame info
    evalSym(buf);     // compile symbol
    push(code, ret);  // add a return statement
    execute(2);       // execute

    // printf("compiled %ld bytes\n", code->ptr-2);
  }

  printf("\n");

  return 0;
}
