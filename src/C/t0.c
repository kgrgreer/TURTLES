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

#define MAX_FN_SIZE 10000

FILE* tin;

/*
  Symbols:
    - a sequence of non whitespace characters
    - read by readSym() which reads characters with 'key' function, from scope
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
    - a callClosure Instruction, with two arguments:
      - a heap pointer to the frame-pointer under which the function was defined (ie. its closure)
      - then a pointer to the code to execute

  Code:
    - a heap pointer to zero or more Instructions
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
    - maps String keys to heap pointers to Instructions
    - Instructions are executed when a symbol is looked up
    - addCmd() adds an Instruction directly (interpreted / immediate)
    - addFn() adds an Instruction which will emit the supplied function as code when run (compiled)
    - addSym() adds a pointer to an Instruction directly, called by addCmd() and addFn()

  Issues:
    - malloc() is used in some places where the stack or heap should be used instead so
      that memory can be GC'ed in the future
    - comments inside switch

  Todo:
    -  support for emitting code comments in DEBUG mode or tagging non-code items like closures
    -  Add : :: :! words for setting values with String keys
    -  Replace Stack.ptr with actual pointer instead of counter?
    -  Would it be possible to bootstrap with a simple defun() then upgrade in T0?
    -  Do we need two stacks: the call-stack and the argument stack?
    -  ? Replace . with >> for 'print'?
    -  Use ' for characters use " for front-quoted strings or #?
    -  | could be replaced with |0 |1 |2 ...
    -  Add split
    -  startsWith and endsWith
    -  extend ??? behaviour in prefix.t0

  Ideas:
    - What if stack frames had their own heap? That would make it more likely
      that they could be unwound.
    - should . replace .$ and .# replace .?
    - Replace @ with [ ] and [ ]:, could validate only one arg on stack between [ and ]
    - Add a prefix to strings which includes size and hash?
    - Internalize strings?
    - Use ( ) for comments? Along with (* *) (** **) (*** ***) (**** ****)
    - Do loops need break and continue?
    - maybe make : and :! read ahead a symbol? or have versions which read name from stack
    : define
    ! immediate
    % automatic
*/

Space* createSpace(long size) {
  Space* s = (Space*) malloc(sizeof(Space));
  s->size  = size;
  s->arr   = malloc(size * sizeof(void*));
  s->ptr   = 0;
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
  if ( s->ptr <= 0 ) {
    printf("POP from empty stack.\n");
    exit(1);
  }
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
  long pip = ip;
  ip = ptr;
    Fn fn = (Fn) nextI();
    (fn)();
  ip = pip;
}


Scope* createScope(char* key /* copied */, long ptr) {
  Scope* node = (Scope*) malloc(sizeof(Scope));
  node->key   = (char*)  malloc(strlen(key) + 1);
  node->ip    = ptr;
  node->left  = node->right = NULL;
  strcpy(node->key, key);
  return node;
}


/* Creates a new tree with added binding. Doesn't modify existing tree. */
// TODO: createScope is copying key, which it doesn't need to always do.
Scope* addSym(Scope* root, char* key /* copied */, long ptr) {
  if ( root == NULL ) return createScope(key, ptr);

  int c = strcmp(key, root->key);

  Scope* ret = createScope(root->key, root->ip);
  ret->left  = root->left;
  ret->right = root->right;

  if ( c == 0 ) {
    // Should we free key in this case?
    ret->ip = ptr;
  } else if ( c < 0 ) {
    ret->left  = addSym(ret->left, key, ptr);
  } else {
    ret->right = addSym(ret->right, key, ptr);
  }

  return ret;
}

void emitFn() { push(code, nextI()); } // Used by addFn()

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
  long ptr = findSym(scope, "_key_"); // TODO: use key != key

  if ( ptr == -1 ) return getc(tin);

  long prev = code->ptr;
  callI(ptr);
  push(code, ret);   // add a return statement
  execute(prev);     // execute any compiled code
  code->ptr = prev;

  return (int) (long) pop(stack);
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

//  printf("SYM: %s\n", buf);
  return true;
}

void localVarSetup();


// createClosure0 and callClosure0 are more efficient verions for functions
// that don't take any arguments. ??? Maybe clearer to rename createClosureNoVars
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

long returnPtr;

/** Execute code starting at ip until ret. **/
void execute(long ptr) {
  long rip = ip;
  for ( ip = ptr ; ; ) {
    Fn fn = ((Fn) nextI());
// char* desc = findKey(scope, fn); printf("RUNNING: %s\n", desc);
    fn();
    if ( returnPtr ) {
      if ( returnPtr == -1 || returnPtr == ptr ) returnPtr = 0;
      break;
    }
  }
  ip = rip;
}

void returnTo(long ptr) {
  returnPtr = ptr;
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


// Copy argument values from stack to the heap, as part of the activation record,
// where they can be accessed as frameReferences. Also reserve space for local vars.
void localVarSetup() {
  long numParams = (long) nextI(), numVars = (long) nextI();
  long n         = numParams + numVars;

  for ( long i = numVars ; i < n ; i++ ) heap->arr[heap->ptr+n-i-1] = pop(stack);

  heap->ptr += n;
}


char* strAdd(char* s1, char* s2) {
  int   l1 = strlen(s1);
  char* s3 = (char*) malloc(l1 + strlen(s2)+1);

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
      // printf("DEFINE AUTO: %s\n", s);
      char* s = strdup(&sym[1]);
      s[0] = '&';
      push2(code, defineAuto, s);
    } else if ( sym[1] == '!' ) {
      // function definition appears as :!name, an immediate variable
      char* s = strdup(sym+2);
      // printf("DEFINE Immediate: %s\n", s);
      push2(code, defineImmediate, s);
    } else {
      // function definition appears as :name, a regular variable
      char* s = strdup(sym+1);
      // printf("DEFINE: %s\n", s);
      push2(code, define, s);
    }
  } else if ( ( sym[0] >= '0' && sym[0] <= '9' ) || ( sym[0] == '-' && sym[1] >= '0' && sym[1] <= '9' ) ) {
    // TODO: call $># ?
    // Parse Integers
    push2(code, constant, (void*) atol(sym));
  } else if ( sym[0] == '\'' ) {
    push2(code, constant, strdup(sym+1));
  } else if ( sym[0] == '`' && strlen(sym) == 2 ) {
    push2(code, constant, (void*) (long) sym[1]);
  } else if ( sym[0] == '.' ) {
    // TODO: save in scope to avoid recreating?
    push2(code, methodCall, strdup(sym+1));
  } else {
    int len = strlen(sym);

    if ( sym[len-1] == ':' ) {
      push2(code, constant, strndup(sym, len-1));
    } else {
      char* s = strdup(sym);
      printf("UNKNOWN/FUTURE SYMBOL: %s\n", s);
      push2(code, forwardReference, s);
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


void execSym(char* sym) {
  long prev = code->ptr;
  evalSym(sym);       // run symbol, which may compile to 'code'
  push(code, ret);    // add a return statement
  execute(prev);      // execute any compiled code
  code->ptr = prev;
}


void defineLocalVar(char* name, long i /* frame position */ ) {
  scope = addSym(scope, name,               push3(heap, emitVarGet,  (void*) (long) fd, (void*) (long) i));
  scope = addSym(scope, strAdd(":", name),  push3(heap, emitVarSet,  (void*) (long) fd, (void*) (long) i));
  scope = addSym(scope, strAdd(name, "++"), push3(heap, emitVarIncr, (void*) (long) fd, (void*) (long) i));
  scope = addSym(scope, strAdd(name, "--"), push3(heap, emitVarDecr, (void*) (long) fd, (void*) (long) i));
}


// Ex. { :name a b let 0 :i 1 :j | ... <- ... name<- } is like 0 1 { a b i j | ... }
// | ... is optional if there's no body
// :name is optional
// In 'let' variables can also use prefix naming like:
// { let a: 42 ;  b: 66 ; | ... }
void defun() {
  long   start = heap->ptr;
  char   buf[256];
  int    i = 0; // total number of vars: parameters + local
  Scope* s = scope;
  long   oldCode = code->ptr;
  char*  fnName  = 0;
  long   localVarSetupPos = -1;

  code->ptr += MAX_FN_SIZE;

  fd++;

  while ( true ) {
    if ( ! readSym(buf, sizeof(buf)) ) {
      printf("Syntax Error: Unclosed function, missing |\n");
      return;
    }

    // Named function so that name<- is defined
    // The difficulty with named returns is that we don't know the actual
    // return address until after the function is compiled so we need to
    // keep references to all uses so we can update them at the end when we
    // know.
    if ( i == 0 && buf[0] == ':' ) {
      fnName = strdup(buf+1);
      continue;
    }

    if ( strcmp(buf, "|") == 0 || strcmp(buf, "}") == 0 || strcmp(buf, "let") == 0 ) break;

    char* varName = strdup(buf); // TODO: free
    defineLocalVar(varName, i);
    i++;
  }

  int numParams = i;

  if ( i > 0 || strcmp(buf, "let") == 0 ) {
    // Second arg may need to be update after let
    localVarSetupPos = push3(code, localVarSetup, (void*) (long) numParams, (void*) (long) 0);
  }

  if ( strcmp(buf, "let") == 0 ) {
    while ( true ) { // for each :<name>
      char* varName;
      while ( true ) { // for each word before :<name> // TODO: make next 5 lines a MACRO
        if ( ! readSym(buf, sizeof(buf)) ) {
          printf("Syntax Error: Unclosed function, missing |\n");
          return;
        }

        if ( buf[0] == ':' ) {
          varName = strdup(buf+1); // TODO: free
          break;
        }

        int len = strlen(buf);
        if ( buf[len-1] == ':' ) {
          varName = strndup(buf, len-1); // TODO: free
          continue;
        }

        if ( strcmp(buf, ";") == 0 ) {
          buf[0] = ':';
          strcpy(&buf[1], varName);
          break;
        }

        // couldn't happen, :name would have to appear first
        if ( strcmp(buf, "|") == 0 ) goto body;
        // It wouldn't make sense to have a 'let' but no body
        if ( strcmp(buf, "}") == 0 ) goto body;

        evalSym(buf);
      } // while word before :<name>

      defineLocalVar(varName, i);
      i++;
      evalSym(buf);

      if ( strcmp(buf, "|") == 0 ) goto body;
      // It wouldn't make sense to have a 'let' but no body
      if ( strcmp(buf, "}") == 0 ) goto body;
    } // while :<name>
  }

  body:

  if ( i > numParams ) heap->arr[localVarSetupPos+2] = (void*) (long) i-numParams;

  if ( i == 0 ) fd--;

  // TODO: move above
  if ( fnName ) {
    char* sym = strAdd(fnName, "<-");
    // Correct address will replace 666 at end when it is known
    long ptr = push2(heap, emitLongRet, fnName);
    scope = addSym(scope, sym, ptr);
  }

  if ( strcmp(buf, "}") != 0 ) while ( true ) {
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

  // Address of function body
  long retAddr = heap->ptr;

  for ( int i = oldCode + MAX_FN_SIZE ; i < code->ptr ; i++ ) push(heap, code->arr[i]);

  if ( fnName ) {
    // Scan all generated code looking for longRet's that need rewriting
    for ( long i = start ; i < heap->ptr ; i++ ) {
      if ( heap->arr[i] == longRet && heap->arr[i+1] == fnName ) {
        heap->arr[i+1] = (void*) retAddr;
      }
    }
  }

  code->ptr = oldCode;

  push2(code, i > 0 ? createClosure : createClosure0, (void*) retAddr);

  scope = s; // revert to old scope

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

  for ( long i = 0 ; true ; ) {
    if ( ! readSym(buf, sizeof(buf)) ) {
      printf("Syntax Error: Unclosed switch, missing end\n");
      return;
    }

    if ( strcmp(buf, "end") == 0 ) {
      code->arr[ptr+1] = (void*) (i/2);
      return;
    }

    long prev = code->ptr;
    evalSym(buf);
    if ( code->arr[prev] == &constant || code->arr[prev] == &createClosure || code->arr[prev] == &createClosure0 ) {
      i++;
    } else {
      push(code, ret);
      execute(prev);
      code->ptr = prev;
    }
  }
  // TODO: copy values off of stack and make constants in
}


void scopeLookup() {
  push3(code, &scopeLookupI, scope, (void*) (long) fd);
}


void cond() {
  // Appears here instead of cmds.js because it is a command, not a function.
  char buf[256]; // Used to hold next read symbols
  long ptr = code->ptr;

  // The 0 is a placeholder and will be replaced at the end
  push2(code, condI, (long*) 0l);

  for ( long i = 0 ; true ; ) {
    if ( ! readSym(buf, sizeof(buf)) ) {
      printf("Syntax Error: Unclosed cond, missing end\n");
      return;
    }

    if ( strcmp(buf, "end") == 0 ) {
      code->arr[ptr+1] = (void*) (i/2);
      return;
    }

    long prev = code->ptr;
    evalSym(buf);
    if ( code->arr[prev] == &createClosure || code->arr[prev] == &createClosure0 ) {
      i++;
    } else {
      push(code, ret);
      execute(prev);
      code->ptr = prev;
    }
  }
}


// Ignore C++ style // comments
// void cppComment() { while ( readChar() != '\n' ); }

// Ignore C style /* */ comments
// void cComment() {
//   for ( char c, prev ; ( c = readChar() ) ; prev = c )
//     if ( prev == '*' && c == '/' ) return;
// }


void strLiteral() {
  char buf[4096];
  int  i = 0;

  while ( ( buf[i++] = readChar() ) != '"' );

  buf[i-1] = '\0';

  push2(code, constant, (void*) strdup(buf));
}

void str3Literal() {
  char buf[4096];
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

void emptyString() {
  push2(code, constant, (void*) "");
}


void immediate() { // i{
//   long outerCode = code->ptr;
  char buf[4096];
  int  i = 0;

  while ( true ) {
    if ( ( buf[i++] = readChar() ) != '}' ) continue;
    if ( ( buf[i++] = readChar() ) != 'i' ) continue;
    break;
  }
  buf[i-3] = '\0';

  push(stack, buf);
  execSym("eval");
}

// 'i[':      code => { outerCode = code; var s = '', c; while ( (c = scope.readChar()) != ']' ) s += c; scope.eval$(s); },


// Clear (empty) the stack and the screen
void clearStack() { stack->ptr = 0; printf("\033c"); }

void clearScreen() { printf("\033c"); }


#ifdef DEBUG
#include "debug.c"
#endif


void nop() { }


void initScope() {
  scope = addCmd(scope, "????",       &unknownSymbol);
  scope = addCmd(scope, "???",        &unknownSymbol);
  scope = addCmd(scope, "{",          &defun);
  scope = addCmd(scope, "switch",     &switch_);
  scope = addCmd(scope, "cond",       &cond);
  scope = addCmd(scope, "clearStack", &clearStack);
  scope = addCmd(scope, "cls",        &clearScreen);
  scope = addCmd(scope, "\"",         &strLiteral);
  scope = addCmd(scope, "\"\"\"",     &str3Literal);
  scope = addCmd(scope, "\\0",        &emptyString);
  scope = addCmd(scope, "??",         &scopeLookup);
  scope = addSym(scope, "i{",         push(heap, &immediate));
  scope = addCmd(scope, "prompt",     &nop);

  scope = addCmds(scope);

#ifdef DEBUG
  scope = addDebugCmds(scope);
#endif
}


void initSpace() {
  heap  = createSpace(5000000000);
  stack = createSpace(500000);
  code  = createSpace(0);

  code->arr = heap->arr; // Code stack shares memory with heap, just has its own ptr
  heap->ptr = 100000;    // Make space for REPL scratch space and for functions
}


void eval(bool showPrompt) {
  char buf[256]; // Used to hold next read symbols

  while ( true ) {
    if ( showPrompt ) execSym("prompt");

    if ( ! readSym(buf, sizeof(buf)) ) break;

    execSym(buf); // evalSym then execute any generated code
  }
}


void eval__() { eval(false); }


void require(char* filename) {
  FILE* old = tin;
  tin = fopen(filename, "r");
  eval(false);
  fclose(tin);
  tin = old;
}


int main() {
  initSpace();
  initScope();

  require("./lib/prefix.t0");

  tin = stdin;
  eval(true);

  printf("\n");

  return 0;
}
