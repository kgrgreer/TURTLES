// t0.c
// Author: Kevin Greer

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdbool.h>
#include <sys/time.h>

#define DEBUG       1
#define PERFORMANCE 1

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

  Instructions:
    - a heap pointer to a function
    - followed by its arguments
    - without a ret, so only a single statement
    - created like: push2(heap, constant, value), where, in this case, "constant" is a function and "value" is a value that it remembers/uses

  Closures:
    - a heap pointer to the frame-pointer under which the function was defined (ie. its closure)
    - a pointer to the code to execute

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
    - Instructions are executed when a symbol is looked up
    - addCmd() adds a Closure directly (interpreted)
    - addFn() adds a Closure which will emit the supplied function as code when run (compiled)

  Issues:
    - malloc() is used in some places where the heap should be used instead so
      that memory can be GC'ed in the future
    - | could be replaced with |0 |1 |2 ...

  Todo:
    - better command dictionary which supports reverse lookup and argument information
    - support for emitting code comments in DEBUG mode or tagging non-code items like closures

  Ideas:
    - What if stack frames had their own heap? That would make it more likely
      that they could be unwound.

*/

void evalSym(char* sym);


typedef struct space_ {
  long ptr;
  void* *arr;
} Space;


Space* createSpace(long size) {
  Space* s = (Space*) malloc(sizeof(Space));
  s->arr = malloc(size * sizeof(void*));
  return s;
}


long push(Space* s, void* value) {
  long r = s->ptr;
  s->arr[s->ptr++] = value;
  return r;
}

long push2(Space* s, void* v1, void* v2) {
  long r = push(s, v1);
  push(s, v2);
  return r;
}

long push3(Space* s, void* v1, void* v2, void* v3) {
  long r = push2(s, v1, v2);
  push(s, v3);
  return r;
}


void* pop(Space* s) {
#ifdef DEBUG
  if ( s->ptr <= 0 ) printf("POP from empty stack.\n");
#endif
  return s->arr[--s->ptr];
}


typedef void (*Fn)();


typedef struct scope_ {
  char*          key;
  long           ip;
  struct scope_* left;
  struct scope_* right;
} Scope;


Scope* scope = NULL; // dictionary of words -> closures
Space* stack = NULL; // used to pass arguments
Space* heap  = NULL; // used for everything else, including frames
Space* code  = NULL; // where code is generated to, shares memory with heap but has own pointer
long   ip    = 0;    // instruction pointer, code being run
long   fp    = 0;    // frame pointer, pointer on heap of current frame / activation-record
int    fd    = 0;    // frame depth


void* nextI() { return heap->arr[ip++]; } // next instruction or instruction argument


// Execute a single instruction stored at the pointed to heap location.
// Not 'ret' terminated.
// Restores ip.
void callInstruction(long ptr) {
  long ret = ip;
  ip = ptr;
    Fn fn = (Fn) nextI();
    (fn)();
  ip = ret;
}


Scope* createScope(char* key /* copied */, long ptr) {
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

   // This is so the character isn't lost if the read command wants to consume input
   ungetc(c, stdin);

  return true;
}

void callClosure();
void callClosure0();

void createClosure() {
  void* fn = nextI();
  push(stack, (void*) push3(heap, callClosure, (void*) fp, fn));
}


void createClosure0() {
  void* fn = nextI();
  push(stack, (void*) push3(heap, callClosure0, (void*) fp, fn));
}


void ret() {
  // ip = (long) heap->arr[fp+1]; // jump to return address
  // fp = (long) heap->arr[fp];   // restore previous fp
  // printf("returning to %ld\n", ip);
}

void plus();
void print();
void createClosure();
void constant();
void call();
void callClosure0();
void frameReference();
void frameSetter();
void frameIncr();
void frameDecr();
void localVarSetup();
void now();
void defineAuto();
void define();
void forStatement();
void whileStatement();
void repeatStatement();

#ifdef DEBUG
void guru();
#endif

// TODO: temporary, use a better instruction dictionary
char* findKey(Scope* root, Fn fn) {
  if ( fn == &callClosure     ) return "callClosure";
  if ( fn == &createClosure   ) return "createClosure";
  if ( fn == &callClosure0    ) return "callClosure0";
  if ( fn == &createClosure0  ) return "createClosure0";
  if ( fn == &constant        ) return "constant";
  if ( fn == &plus            ) return "+";
  if ( fn == &print           ) return "print";
  if ( fn == &ret             ) return "<-";
  if ( fn == &call            ) return "()";
  if ( fn == &now             ) return "now";
  if ( fn == &frameReference  ) return "@";
  if ( fn == &frameSetter     ) return ":@";
  if ( fn == &frameIncr       ) return "++";
  if ( fn == &frameDecr       ) return "--";
  if ( fn == &localVarSetup   ) return "|";
  if ( fn == &define          ) return ":";
  if ( fn == &defineAuto      ) return "::";
  if ( fn == &forStatement    ) return "for";
  if ( fn == &whileStatement  ) return "while";
  if ( fn == &repeatStatement ) return "repeat";

#ifdef DEBUG
  if ( fn == &guru            ) return "guru";
#endif

  return "UNKNOWN";
}


/** Execute code starting at ip until 0 found. **/
void execute(long ptr) {
  long rip = ip;
  for ( ip = ptr ; ; ) {
    Fn fn = (Fn) nextI();
    // printf("executing: %ld %s\n", ip, findKey(scope, fn));
    if ( fn == ret ) break;
    fn();
  }
  ip = rip;
}


void callClosure() {
  long ofp = fp;
  long pfp = (long) nextI(); // parent fp
  long fn  = (long) nextI(); // fn ptr

  fp = push(heap, (void*) pfp); // previous FP
  long ohp = heap->ptr;
  // printf("calling closure at: %ld, fp: %ld, fn: %ld, from: %ld\n", closure, pfp, fn, ip);
  execute(fn);

  // Optimization, if nothing extra has been allocated on the heap, treat
  // it like a stack and revert back to position before pushing this frame.
  if ( heap->ptr == ohp ) heap->ptr = fp;

  fp = ofp;
}


void callClosure0() {
  long ofp = fp;
  long pfp = (long) nextI(); // parent fp
  long fn  = (long) nextI(); // fn ptr

  // printf("calling closure at: %ld, fp: %ld, fn: %ld, from: %ld\n", closure, pfp, fn, ip);
  fp = pfp;
  execute(fn);
  fp = ofp;
}


void call() {
  callInstruction((long) pop(stack));
}


void constant() {
  // Consume next constant value stored in the heap and push to stack
  long c = (long) nextI();
  // printf("constant %ld\n", c);
  push(stack, (void*) c);
}


void emitConstant() {
  // Consume next constant value stored in the heap and push to stack
  long c = (long) nextI();
  // printf("emit constant %ld\n", c);
  push2(code, constant, (void*) c);
}


void autoConstant() {
  // Consume next constant value stored in the heap and push to stack
  long c = (long) nextI();
  push(stack, (void*) c);
  call();
}


void emitAutoConstant() {
  // Consume next constant value stored in the heap and push to stack
  long c = (long) nextI();
  // printf("emit constant %ld\n", c);
  push2(code, autoConstant, (void*) c);
}


// TODO: cleanup frameOffset / emitter code

long frameOffset(long depth, long offset) {
  long f = fp;
  // printf("frame offset: depth: %ld offset: %ld fd: %d\n", depth, offset, fd);
  for ( int i = 0 ; i < depth ; i++ ) f = (long) heap->arr[f];
  return f+1+offset;
}


void frameReference() {
  int  frame  = (int)  nextI();
  long offset = (long) nextI();
  // printf("\nframe reference fp: %ld offset: %ld value: %ld\n", fp, offset, (long) heap->arr[2+fp+offset]);
  // Consume next constant value stored in the heap and push to stack
  push(stack, (void*) heap->arr[frameOffset(frame, offset)]);
}

void frameReferenceEmitter() {
  push3(code, frameReference, (void*) (long) (fd-(int) nextI()), nextI());
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
  push3(code, frameSetter, (void*) (long) (fd-(int) nextI()), nextI());
}

void frameIncr() {
  int  frame  = (int)  nextI();
  long offset = (long) nextI();
  heap->arr[frameOffset(frame, offset)]++;
}

void frameIncr00() { heap->arr[fp+1]++; }

void frameIncr10() { heap->arr[(long) heap->arr[fp]+1]++; }

void frameIncrEmitter() {
  int  frame  = fd-(int) nextI();
  long offset = (long) nextI();

#ifdef PERFORMANCE
  if ( frame == 0 && offset == 0 ) {
    push(code, frameIncr00);
  } else if ( frame == 1 && offset == 0 ) {
    push(code, frameIncr10);
  }
  else
#endif
  {
    push3(code, frameIncr, (void*) (long) frame, (void*) offset);
  }
}

void frameDecr() {
  int  frame  = (int)  nextI();
  long offset = (long) nextI();
  heap->arr[frameOffset(frame, offset)]--;
}

void frameDecrEmitter() {
  push3(code, frameDecr, (void*) (long) (fd-(int) nextI()), nextI());
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

  scope = addSym(scope, sym, push2(heap, emitConstant, value));
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

  scope = addSym(scope, sym, push2(heap, emitAutoConstant, value));
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

void mod() {
  long l2 = (long) pop(stack);
  long l1 = (long) pop(stack);
  push(stack, (void*) (l1 % l2));
}

void eq() { push(stack, (void*) (long) (pop(stack) == pop(stack))); }

void neq() { push(stack, (void*) (long) (pop(stack) != pop(stack))); }

void lt() { push(stack, (void*) (long) (pop(stack) > pop(stack))); }

void gt() { push(stack, (void*) (long) (pop(stack) < pop(stack))); }

void lte() { push(stack, (void*) (long) (pop(stack) >= pop(stack))); }

void gte() { push(stack, (void*) (long) (pop(stack) <= pop(stack))); }

void not() { push(stack, (void*) (long) (! (bool) pop(stack))); }

void and() { push(stack, (void*) (long) ((bool) pop(stack) && (bool) pop(stack))); }

void or() { push(stack, (void*) (long) ((bool) pop(stack) || (bool) pop(stack))); }

void andand() { void* aFn = pop(stack); if ( ! pop(stack) ) { push(stack, (void*) 0); } else { push(stack, aFn); call(); } }

void oror() { void* aFn = pop(stack); if ( pop(stack) ) { push(stack, (void*) 1); } else { push(stack, aFn); call(); } }

// Could be defined as: { _ | }
void drop() { pop(stack); }

/*
'[]WithValue': fn(() => {
  var value = stack.pop(), length = stack.pop(), a = [];
  for ( var i = 0 ; i < length ; i++ ) a[i] = value;
  stack.push(a);
}),
'[]WithFn': fn(() => {
  var fn = stack.pop(), length = stack.pop(), a = [];
  for ( var i = 0 ; i < length ; i++ ) { stack.push(i); fn(); a[i] = stack.pop(); }
  stack.push(a);
}),
'@':  bfn((a, i) => a[i]),
':@': fn(() => { var i = stack.pop(), a = stack.pop(), v = stack.pop(); a[i] = v; }),
'[':  fn(() => stack.push(__arrayStart__)),
']':  fn(() => {
  var start = stack.length-1;
  for ( ; start && stack[start] !== __arrayStart__ ; start-- );
  var a = new Array(stack.length-start-1);
  for ( var i = a.length-1 ; i >= 0 ; i-- ) a[i] = stack.pop();
  stack.pop(); // remove arrayStart
  stack.push(a);
}),

*/

// Could be written in T0 as:
// { s e block | { | s e <= } { | s block () s++ } while } ::for
void forStatement() {
  long block = (long) pop(stack);
  long e     = (long) pop(stack);
  long s     = (long) pop(stack);
  for ( long i = s ; i <= e ; i++ ) {
    push(stack, (void*) i);
    callInstruction(block);
  }
}


void repeatStatement() {
  const long block = (long) pop(stack);
  const long times = (long) pop(stack);

  // Faster Version
  long ret = ip;
  ip = block;
  Fn fn = (Fn) nextI();
  if ( fn == callClosure0 && false ) {
    long ofp = fp;
    long pfp = (long) nextI(); // parent fp
    long fn  = (long) nextI(); // fn ptr

    // printf("calling closure at: %ld, fp: %ld, fn: %ld, from: %ld\n", closure, pfp, fn, ip);
    fp = pfp;
    ip = fn;
    const Fn f = (Fn) nextI();
    long iip = ip;
    for ( long i = 0 ; i < times ; i+=4 ) {
      /*
      (f)();
      (f)();
      (f)();
      (f)();
      */
      // faster: ???
      ip = iip; (f)();
      ip = iip; (f)();
      ip = iip; (f)();
      ip = iip; (f)();
    }
    fp = ofp;
    ip = ret;
  } else {
    long pp = ip;
    for ( long i = 0 ; i < times ; i++ ) {
      ip = pp;
      (fn)();
    }
    ip = ret;
  }
  /*
  // Simpler Version
  for ( long i = 0 ; i <= times ; i++ ) {
    callInstruction(block);
  }
  */
}


void whileStatement() {
  long block = (long) pop(stack);
  long cond  = (long) pop(stack);
  while ( true ) {
    callInstruction(cond);
    if ( ! pop(stack) ) break;
    callInstruction(block);
  }
}

void ifStatement() { long block = (long) pop(stack); if ( pop(stack) ) callInstruction(block); }

void ifElseStatement() { long fBlock = (long) pop(stack); long tBlock = (long) pop(stack); callInstruction(pop(stack) ? tBlock : fBlock); }

void print() {
  printf("\n\033[1;30m"); // Print in bold black
  printf("%ld", (long) pop(stack));
  printf("\033[0m");      // Revert colour code
}

void printStr() {
  printf("\n\033[1;30m"); // Print in bold black
  printf("%s", (char *) pop(stack));
  printf("\033[0m");      // Revert colour code
}

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
  } else if ( sym[0] == '\'' ) {
    char* s = strdup(sym+1);
    push2(code, constant, s);
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

  callInstruction(ptr);
}


// Ex. { a b let 0 :i 1 :j | ... } is like 0 1 { a b i j | ... }
void defun() {
  char  buf[256];
  char* vars[32];
  int   i = 0; // number of vars / arguments
  Scope* s = scope;

  Space* oldCode = code;
  Space  code2; // A temp code buffer to allow for reentrant function parsing
  void*  arr[1024];

  code2.ptr = 0;
  code2.arr = arr;
  code      = &code2;

  while ( true ) {
    if ( ! readSym(buf, sizeof(buf)) ) {
      printf("Syntax Error: Unclosed function, missing |");
      return;
    }

    if ( strcmp(buf, "|") == 0 ) break;

    if ( strcmp(buf, "let") == 0 ) {
      while ( true ) {
        while ( true ) {
          if ( ! readSym(buf, sizeof(buf)) ) {
            printf("Syntax Error: Unclosed function, missing |");
            return;
          }

          if ( buf[0] == ':' ) break;

          if ( strcmp(buf, "|") == 0 ) goto outer;

          evalSym(buf); // TODO: is done too soon because code2 hasn't been setup yet
        }

        // Add var name after the : to 'vars'
        vars[i++] = strdup(buf+1); // TODO free()

        if ( strcmp(buf, "|") == 0 ) goto outer;
      }
    }

    // Add var name to 'vars'
    vars[i++] = strdup(buf);
  }

  outer:

  if ( i > 0 ) fd++;

  // ???: does this need to be delayed or can we just execute directly above?
  for ( long j = 0 ; j < i ; j++ ) {
    char* varName = vars[j];
    void* k       = (void*) (i-j-1);
    scope = addSym(scope, varName,               push3(heap, frameReferenceEmitter, (void*) (long) fd, k));
    scope = addSym(scope, strAdd(":", varName),  push3(heap, frameSetterEmitter,    (void*) (long) fd, k));
    scope = addSym(scope, strAdd(varName, "++"), push3(heap, frameIncrEmitter,      (void*) (long) fd, k));
    scope = addSym(scope, strAdd(varName, "--"), push3(heap, frameDecrEmitter,      (void*) (long) fd, k));
  }

  if ( i > 0 ) {
    push2(code, localVarSetup, (void*) (long) i);
  } else {
    // fd--;
  }

  while ( true ) {
    if ( ! readSym(buf, sizeof(buf)) ) {
      printf("Syntax Error: Unclosed function, missing }");
      return;
    }

    if ( strcmp(buf, "}") == 0 ) break;

    // printf("evalSym %s\n", buf);
    evalSym(buf);
  }

  // printf("defun %ld bytes to %ld\n", code->ptr, heap->ptr);
  push(code, ret);
#ifdef DEBUG
  push(code, 0); // zero needed so dump() knows when the function is over
#endif

  long ptr = heap->ptr; // location where function will be copied to

  for ( int i = 0 ; i < code2.ptr ; i++ ) push(heap, arr[i]);

  code = oldCode;

  if ( i > 0 ) {
    push2(code, createClosure, (void*) ptr);
  } else {
    push2(code, createClosure0, (void*) ptr);
  }

  scope = s; // revert to old scope, TODO: free dead scope

  if ( i > 0 ) fd--;

  return;
}


// Ignore C++ style // comments
void cppComment() { while ( getchar() != '\n' ); }


// Ignore C style /* */ comments
void cComment() {
  for ( char c, prev ; ( c = getchar() ) ; prev = c )
    if ( prev == '*' && c == '/' ) return;
}


void strLiteral() {
  char buf[4096];
  getchar(); // remove whitespace after "
  int i = 0;

  while ( ( buf[i++] = getchar() ) != '"' );

  buf[i-1] = '\0';

  push2(code, constant, (void*) strdup(buf));
}


// Clear (empty) the stack and the screen
void clearStack() { stack->ptr = 0; printf("\033c"); }


void now() {
	struct timeval tp;

	gettimeofday(&tp, NULL);
	push(stack, (void*) (tp.tv_sec * 1000 + tp.tv_usec / 1000));
}

#ifdef DEBUG

void printStack() {
  for ( long i = 0 ; i < stack->ptr ; i++ )
    printf("%ld ", (long) stack->arr[i]);
}

void printFrames(long fp) {
  for ( long f = fp ; ; fp = (long) heap->arr[fp] ) {
    printf("%ld", (long) fp);
    if ( ! fp ) break;
    printf(" -> ");
  }
}

// Display Guru Medidation Information
void guru() {
  printf("\n\033[1;31m");
  printf("-------------------------------\n");
  printf("       Guru Meditation\n");
  printf("-------------------------------\n");
  printf("ip:     %ld\n", ip);
  printf("frames: "); printFrames(fp); printf("\n");
  printf("stack:  [ "); printStack(); printf("]\n");
//  printf("depth: %d\n", fd);
  printf("------------------------------\n");
  printf("\033[0m");
}


void dump() {
  long ptr = (long) pop(stack);

  printf("\n");

  int uc = 0;
  for ( int i = 0 ; i < 100 ; i++, ptr++ ) {
    Fn fn = (Fn) heap->arr[ptr];
    char* desc = findKey(scope, fn);
    if ( strcmp(desc, "UNKNOWN") == 0 ) {
      if ( uc++ == 4 ) return;
      printf("%ld : %ld\n", ptr, (long) fn);
    } else {
      uc = 0;
      printf("%ld : %s\n", ptr, desc);
    }
    if ( fn == ret && heap->arr[ptr+1] == 0 ) return;
  }
}


void dumpFrames() {
  long fp = (long) pop(stack);

  printFrames(fp);
}

#endif



int main() {
  char buf[256]; // Used to hold next read symbols

  heap  = createSpace(500000000);
  stack = createSpace(50000);
  code  = createSpace(0);

  // Code stack shares memory with heap, just has its own ptr
  code->arr = heap->arr;

  heap->ptr = 1000; // Make space for REPL scratch space

  scope = addCmd(scope, "???",       &unknownSymbol);
  scope = addCmd(scope, "/*",        &cComment);
  scope = addCmd(scope, "//",        &cppComment);
  scope = addCmd(scope, "{",         &defun);
  scope = addCmd(scope, "clear",     &clearStack);
  scope = addCmd(scope, "\"",        &strLiteral);

  scope = addFn(scope, "+",          &plus);
  scope = addFn(scope, "-",          &minus);
  scope = addFn(scope, "*",          &multiply);
  scope = addFn(scope, "/",          &divide);
  scope = addFn(scope, "%",          &mod);
  scope = addFn(scope, "=",          &eq);
  scope = addFn(scope, "!=",         &neq);
  scope = addFn(scope, "<",          &lt);
  scope = addFn(scope, ">",          &gt);
  scope = addFn(scope, "<=",         &lte);
  scope = addFn(scope, ">=",         &gte);
  scope = addFn(scope, "!",          &not);
  scope = addFn(scope, "&",          &and);
  scope = addFn(scope, "|",          &or);
  scope = addFn(scope, "&&",         &andand);
  scope = addFn(scope, "||",         &oror);
  scope = addFn(scope, "drop",       &drop);
  scope = addFn(scope, "if",         &ifStatement);
  scope = addFn(scope, "ifelse",     &ifElseStatement);
  scope = addFn(scope, "while",      &whileStatement);
  scope = addFn(scope, "for",        &forStatement);
  scope = addFn(scope, "repeat",     &repeatStatement);
  scope = addFn(scope, "print",      &print);
  scope = addFn(scope, "print$",     &printStr);
  scope = addFn(scope, ".",          &print); // like forth
  scope = addFn(scope, "()",         &call);
  scope = addFn(scope, "now",        &now);
  scope = addFn(scope, "<-",         &ret); // TODO: end functions with a 0 so dump() can work

#ifdef DEBUG
  scope = addFn(scope, "guru",       &guru);
  scope = addFn(scope, "dump",       &dump);
  scope = addFn(scope, "dumpFrames", &dumpFrames);
#endif

  // Create a top-level frame which points to itself as the previous frame
  // so it can be reused forever.
  push(code, (void*) 0);  // points to itself as previous frame
  push(code, (void*) -1); // psedo return address causes stop of execution

  while ( true ) {
#ifdef DEBUG
    printf("\033[0;34m"); // Print in blue
    printf("\nheap: %ld, stack: [ ", heap->ptr); printStack(); printf("] > ");
    printf("\033[0m");      // Revert colour code
#endif

    if ( ! readSym(buf, sizeof(buf)) ) break;

    code->ptr = 2;    // skip over frame info
    evalSym(buf);     // compile symbol
    push(code, ret);  // add a return statement
    execute(2);       // execute compiled code

    // printf("compiled %ld bytes\n", code->ptr-2);
  }

  printf("\n");

  return 0;
}
