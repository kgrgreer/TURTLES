// t0.c
// Author: Kevin Greer

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdbool.h>

#include "t0.h"
#include "cmds.h"

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
    - created like:
        push2(heap, constant, value),
        where, in this case, "constant" is a function and "value" is a value that it remembers/uses

  Closures:
    - a heap pointer to the frame-pointer under which the function was defined (ie. its closure)
    - a pointer to the code to execute

  Code:
    - a heap pointer to zero or more Closures
    - ends with a "ret", so can be many statements

  Activation Record
    - Allocated on heap

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
    - support for emitting code comments in DEBUG mode or tagging non-code items like closures
    - Add : :: :! words for setting values with String keys

  Ideas:
    - What if stack frames had their own heap? That would make it more likely
      that they could be unwound.

*/

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
void callI(long ptr) {
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


int readChar() {
  return getchar();
}


bool readSym(char* buf, int bufSize) {
  int c;
  int size = 0;

  /* Skip leading whitespace. */
  while ( isSpace(c = readChar()) );

  if ( c == EOF ) return false;

  buf[size++] = c;

  while ( (c = readChar()) != EOF && ! isSpace(c) && size < bufSize - 1 ) {
    buf[size++] = c;
  }
  buf[size] = '\0';

   // This is so the character isn't lost if the read command wants to consume input
   ungetc(c, stdin);

  return true;
}

void callClosure0();
void localVarSetup();


// createClosure0 and callClosure0 are more efficient verions for functions
// that don't take any arguments.
void createClosure0() {
  void* fn = nextI();
  push(stack, (void*) push3(heap, callClosure0, (void*) fp, fn));
}


#ifdef DEBUG
void guru();
#endif


// TODO: temporary, use a better instruction dictionary
char* findKey(Scope* root, Fn fn) {
  char* s = cmdToStr(fn);

  if ( s != NULL ) return s;

  if ( fn == &callClosure0    ) return "callClosure0";
  if ( fn == &createClosure0  ) return "createClosure0";
  if ( fn == &localVarSetup   ) return "|";

#ifdef DEBUG
  if ( fn == &guru            ) return "guru";
#endif

  return "UNKNOWN";
}


/** Execute code starting at ip until ret. **/
void execute(long ptr) {
  long rip = ip;
  for ( ip = ptr ; ; ) {
    Fn fn = (Fn) nextI();
    if ( fn == ret ) break;
    fn();
  }
  ip = rip;
}


void callClosure0() {
  long ofp = fp;
  long pfp = (long) nextI(); // parent fp
  long fn  = (long) nextI(); // fn ptr

  fp = pfp;
  execute(fn);
  fp = ofp;
}



long frameOffset(long depth, long offset) {
  long f = fp;
  for ( int i = 0 ; i < depth ; i++ ) f = (long) heap->arr[f];
  return f+1+offset; // TODO: remove need for +1
}


// Copy argument values from stack to the heap, as part of the activation record, where they can be accessed as frameReferences
void localVarSetup() {
  long numVars = (long) nextI();

  for ( long i = 0 ; i < numVars ; i++ ) {
    push(heap, pop(stack));
  }
}


char* strAdd(char* s1, char* s2) {
  int l1 = strlen(s1);
  char* s3 = (char*) malloc(l1 + sizeof(s2));

  strcpy(s3, s1);
  strcpy(s3+l1, s2);

  return s3;
}


void defineImmediate() {
  char* name = (char*) nextI();
  scope = addSym(scope, name, (long) pop(stack));
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
    } else if ( sym[1] == '!' ) {
      // function definition appears as :!name, an immediate variable
      char* s = strdup(sym+2);
      push2(code, defineImmediate, s);
    } else {
      // function definition appears as :name, a regular variable
      char* s = strdup(sym+1);
      push2(code, define, s);
    }
  } else if ( sym[0] >= '0' && sym[0] <= '9' || ( sym[0] == '-' && sym[1] >= '0' && sym[1] <= '9' ) ) {
    // Parse Integers
    push2(code, constant, (void*) atol(sym));
  } else if ( sym[0] == '\'' ) {
    char* s = strdup(sym+1);
    push2(code, constant, s);
  } else {
    int len = strlen(sym);

    if ( sym[len-1] == ':' ) {
      sym[len-1] = '\0';
      char* s = strdup(sym);
      push2(code, constant, s);
    } else {
      push2(code, forwardReference, strdup(sym));
    }
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

  callI(ptr);
}


// Ex. { a b let 0 :i 1 :j | ... } is like 0 1 { a b i j | ... }
void defun() {
  char   buf[256];
  char*  vars[32];
  int    i = 0; // number of vars / arguments
  Scope* s = scope;
  Space* oldCode = code;
  Space  code2; // A temp code buffer to allow for reentrant function parsing
  void*  arr[1024];

  code2.ptr = 0;
  code2.arr = arr;
  code      = &code2;

  while ( true ) {
    if ( ! readSym(buf, sizeof(buf)) ) {
      printf("Syntax Error: Unclosed function, missing |\n");
      return;
    }

    if ( strcmp(buf, "|") == 0 ) break;

    if ( strcmp(buf, "let") == 0 ) {
      while ( true ) { // for each :<name>
        while ( true ) { // for each word before :<name>
          if ( ! readSym(buf, sizeof(buf)) ) {
            printf("Syntax Error: Unclosed function, missing |\n");
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
    scope = addSym(scope, varName,               push3(heap, emitVarGet,  (void*) (long) fd, k));
    scope = addSym(scope, strAdd(":", varName),  push3(heap, emitVarSet,  (void*) (long) fd, k));
    scope = addSym(scope, strAdd(varName, "++"), push3(heap, emitVarIncr, (void*) (long) fd, k));
    scope = addSym(scope, strAdd(varName, "--"), push3(heap, emitVarDecr, (void*) (long) fd, k));
  }

  if ( i > 0 ) push2(code, localVarSetup, (void*) (long) i);

  while ( true ) {
    if ( ! readSym(buf, sizeof(buf)) ) {
      printf("Syntax Error: Unclosed function, missing }\n");
      return;
    }

    if ( strcmp(buf, "}") == 0 ) break;

    evalSym(buf);
  }

  push(code, ret);
#ifdef DEBUG
  push(code, 0); // zero needed so dump() knows when the function is over
#endif

  long ptr = heap->ptr; // location where function will be copied to

  for ( int i = 0 ; i < code2.ptr ; i++ ) push(heap, arr[i]);

  code = oldCode;

  push2(code, i > 0 ? createClosure : createClosure0, (void*) ptr);

  scope = s; // revert to old scope, TODO: free dead scope

  if ( i > 0 ) fd--;
}


void switch_() {
  // Appears here instead of cmds.js because it is a command, not a function.
  // TODO: this leaves unused constant instructions before each constant value
  // which wastes memory. Add code to remove the constants and compact the
  // generated code
  char buf[256]; // Used to hold next read symbols
  long ptr = code->ptr;

  // The 0 is a placeholder and will be replaced at the end
  push2(code, switchI, (long*) 0l);

  for ( long i = 0 ; true ; i++ ) {
    if ( ! readSym(buf, sizeof(buf)) ) {
      printf("Syntax Error: Unclosed switch, missing end\n");
      return;
    }

    if ( strcmp(buf, "end") == 0 ) {
      code->arr[ptr+1] = (void*) (i/2);
      return;
    }

    evalSym(buf);
  }
}


// Ignore C++ style // comments
void cppComment() { while ( readChar() != '\n' ); }


// Ignore C style /* */ comments
void cComment() {
  for ( char c, prev ; ( c = readChar() ) ; prev = c )
    if ( prev == '*' && c == '/' ) return;
}


void strLiteral() {
  char buf[4096];
  readChar(); // remove whitespace after "
  int i = 0;

  while ( ( buf[i++] = readChar() ) != '"' );

  buf[i-1] = '\0';

  push2(code, constant, (void*) strdup(buf));
}


void str3Literal() {
  char buf[4096];
  readChar(); // remove whitespace after """
  int i = 0, quoteCount = 0;

  while ( ( buf[i++] = readChar() ) ) {
    if ( buf[i-1] == '"' ) {
      if ( ++quoteCount == 3 ) {
        buf[i-3] = '\0';
        push2(code, constant, (void*) strdup(buf));
        return;
      }
    } else {
      quoteCount = 0;
    }
  }
  // error
}


void eval() {

}


void immediate() { // i{
  long outerCode = code->ptr;
  char buf[4096];
  readChar(); // remove whitespace after """
  int i = 0, quoteCount = 0;

  while ( true ) {
    if ( ( buf[i++] = readChar() ) != '}' ) continue;
    if ( ( buf[i++] = readChar() ) != 'i' ) continue;
    break;
  }
  buf[i-3] = '\0';

  printf("immediate: %s\n", buf);
  // eval$(buf);
}

// 'i[':      code => { outerCode = code; var s = '', c; while ( (c = scope.readChar()) != ']' ) s += c; scope.eval$(s); },


// Clear (empty) the stack and the screen
void clearStack() { stack->ptr = 0; printf("\033c"); }


#ifdef DEBUG
#include "debug.c"
#endif


void nop() { }

/*
{ let 0 :c |
  { | :c } :switch
  { f | } :end
}
*/

void initScope() {
  scope = addCmd(scope, "???",    &unknownSymbol);
  scope = addCmd(scope, "/*",     &cComment);
  scope = addCmd(scope, "//",     &cppComment);
  scope = addCmd(scope, "{",      &defun);
  scope = addCmd(scope, "switch", &switch_);
  scope = addCmd(scope, "clear",  &clearStack);
  scope = addCmd(scope, "\"",     &strLiteral);
  scope = addCmd(scope, "\"\"\"", &str3Literal);
  scope = addCmd(scope, "i{",     &immediate);
  scope = addCmd(scope, "prompt", &nop);

  scope = addCmds(scope);

#ifdef DEBUG
  scope = addDebugCmds(scope);
#endif
}


void initSpace() {
  heap  = createSpace(500000000);
  stack = createSpace(50000);
  code  = createSpace(0);

  code->arr = heap->arr; // Code stack shares memory with heap, just has its own ptr
  heap->ptr = 1000;      // Make space for REPL scratch space
}


int main() {
  char buf[256]; // Used to hold next read symbols

  initSpace();
  initScope();

  while ( true ) {
    evalSym("prompt");

    if ( ! readSym(buf, sizeof(buf)) ) break;

    evalSym(buf);           // run symbol, which may compile to 'code'
    push(code, ret);        // add a return statement
    execute(code->ptr = 0); // execute any compiled code
  }

  printf("\n");

  return 0;
}
