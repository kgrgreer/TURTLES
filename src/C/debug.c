// debug.c -- debugging related code

// ???: could some of this code be rewritten in T0?

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


void dump_(long ptr) {
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


void dump() {
  dump_((long) pop(stack));
}


void dumpFrames() {
  long fp = (long) pop(stack);

  printFrames(fp);
}


void debugPrompt() {
  printf("\033[0;32m"); // Print in blue
  printf("\nheap: %ld, stack: [ ", heap->ptr); printStack(); printf("] > ");
  printf("\033[0m");    // Revert colour code
}


Scope* addDebugCmds(Scope* scope) {
  scope = addCmd(scope, "prompt",     &debugPrompt);
  scope = addFn(scope,  "guru",       &guru);
  scope = addFn(scope,  "dump",       &dump);
  scope = addFn(scope,  "dumpFrames", &dumpFrames);

  return scope;
}
