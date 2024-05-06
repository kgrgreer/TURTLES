// debug.c -- debugging related code

// ???: could some of this code be rewritten in T0?

void printStack() {
  for ( long i = 0 ; i < stack->ptr ; i++ )
    printf("%ld ", (long) stack->arr[i]);
}


void printFrames(long fp) {
  for ( ; ; fp = (long) heap->arr[fp] ) {
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
  printf("heap:   %ld\n", heap->ptr);
  printf("code:   %ld\n", code->ptr);
  printf("frames: "); printFrames(fp); printf("\n");
  printf("stack:  [ "); printStack(); printf("]\n");
  printf("depth:  %d\n", fd);
  printf("------------------------------\n");
  printf("\033[0m");
}

// ???: Could this be written in t0?
void dump_(long ptr) {
  printf("\n");

  // Unknown count, assume we aren't dumping code if we don't recognize any
  // functions.
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


void dump() {
  dump_((long) pop(stack));
}


void dumpFrames() {
  long fp = (long) pop(stack);

  printFrames(fp);
}


// TODO: rewrite in t0
void debugPrompt() {
  printf("\033[0;32m"); // Print in green
  printf("\nheap: %ld, stack: [ ", heap->ptr); printStack(); printf("] > ");
  printf("\033[0m");    // Revert colour code
}


Scope* addDebugCmds(Scope* scope) {
  scope = addFn(scope, "debugPrompt", &debugPrompt);
  scope = addFn(scope, "prompt",      &debugPrompt);
  scope = addFn(scope, "guru",        &guru);
  scope = addFn(scope, "dump",        &dump);
  scope = addFn(scope, "dumpFrames",  &dumpFrames);

  return scope;
}
