// t0.h

typedef void (*Fn)();


typedef struct space_ {
  long ptr;
  void* *arr;
} Space;


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

Scope* addSym(Scope* root, char* key, long ptr);

long findSym(Scope* root, char* key);

bool readSym(char* buf, int bufSize);

void callI(long ptr);

void* nextI();

long frameOffset(long depth, long offset);

void execute(long ptr);

void evalSym(char* sym);

void execSym(char* sym);

int readChar();
