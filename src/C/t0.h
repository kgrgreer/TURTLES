// t0.h

typedef struct space_ {
  long ptr;
  void* *arr;
} Space;


typedef void (*Fn)();


typedef struct scope_ {
  char*          key;
  long           ip;
  struct scope_* left;
  struct scope_* right;
} Scope;


long push(Space* s, void* value);

long push2(Space* s, void* v1, void* v2);

long push3(Space* s, void* v1, void* v2, void* v3);

void* pop(Space* s);

Scope* addFn(Scope* root, char* key, Fn fn);

void callInstruction(long ptr);

void* nextI();

long frameOffset(long depth, long offset);

void execute(long ptr);

void evalSym(char* sym);
