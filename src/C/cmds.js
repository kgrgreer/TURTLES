// TODO: cond statement
// TODO: give af and sf better names

exports.CMDS = [
  [ 'call',   '()',       af('i',     'callI(i);') ],
  [ 'ret',    '<-',       'returnTo(-1);' ],
  [ 'plus',   '+',        sf('a,b',   'a+b')  ],
  [ 'minus',  '-',        sf('a,b',   'a-b')  ],
  [ 'mul',    '*',        sf('a,b',   'a*b')  ],
  [ 'divide', '/',        sf('a,b',   'a/b')  ],
  [ 'percent','%',        sf('a',     'a/100')],
  [ 'mod',    'mod',      sf('a,b',   'a%b')  ],
  [ 'eq',     '=',        sf('a,b',   'a==b') ],
  [ 'neq',    '!=',       sf('a,b',   'a!=b') ],
  [ 'lt',     '<',        sf('a,b',   'a<b')  ],
  [ 'gt',     '>',        sf('a,b',   'a>b')  ],
  [ 'lte',    '<=',       sf('a,b',   'a<=b') ],
  [ 'gte',    '>=',       sf('a,b',   'a>=b') ],
  [ 'pow_',   '^',        sf('a,b',   'pow(a,b)') ],
  [ 'not',    '!',        sf('a',     '!a')   ],
  [ 'and',    '&',        sf('a,b',   'a&&b') ],
  [ 'or',     '|',        sf('a,b',   'a||b') ],
  [ 'if_',    'if',       af('c,b',   'if ( c ) callI(b);') ],
  [ 'ifelse_','ifelse',   af('c,i,e', 'callI(c ? i : e);')  ],
  [ 'drop',   'drop',     'pop(stack);' ],
  [ 'andand', '&&',       'void* aFn = pop(stack); if ( ! pop(stack) ) { push(stack, (void*) 0); } else { push(stack, aFn); call(); }' ],
  [ 'oror',   '||',       'void* aFn = pop(stack); if (   pop(stack) ) { push(stack, (void*) 1); } else { push(stack, aFn); call(); }'   ],
  [ 'for_',   'for',      af('s,e,b', 'for ( long i = s ; i <= e ; i++ ) { push(stack, (void*) i); callI(b); }')   ],
  [ 'while_', 'while',    af('c,b',   'while ( true ) { callI(c); if ( ! pop(stack) ) break; callI(b); }') ],
  [ 'repeat', 'repeat',   af('b,t',   'for ( long i = 0 ; i < t ; i++ ) callI(b);') ],
  [ 'now',    'now',      'struct timeval tp; gettimeofday(&tp, NULL); push(stack, (void*) (tp.tv_sec * 1000 + tp.tv_usec / 1000));' ],
  [ 'ntoStr', '#>$',      af('n', `
    char str[64];
    sprintf(str, "%ld", (long) n);
    push(stack, strdup(str));
  `) ],
  [ 'toStr',     '>$', af('a', `
    // TODO: base on heap-> ptr instead of constant
    push(stack, (void*) a);
    if ( a < 10000000 ) {
      ntoStr();
    }
  `) ],
  [ 'strToChar', '$>c',   sf('s', '((char*) s)[0]') ],
  [ 'print',  '.',        af('a', `
//    printf("%ld", a);
if ( a < 1000000000 ) {
  printf("%ld", a);
} else {
  printf("%s", (char*) a);
}
  `)],
  [ 'printStr', '.$',     af('a', `
    printf("%s", (char *) a);
  `)],
  [ 'eqStr',     '=$', sf('a,b', '0 == strcmp((char*) a, (char*) b)') ],
  [ 'concatStr', '+$', sf('a,b', 'strAdd((char*) a, (char*) b)') ],
  [ 'arrayStart', '[', 'push(stack, &arrayStart);' ],
  [ 'arrayEnd',   ']', `
    long start = stack->ptr-1;
    for ( ; start && stack->arr[start] != &arrayStart ; start-- );
    int   len = stack->ptr-start-1;
    long* a   = (long*) malloc((len+1) * sizeof(long));
    a[0] = len;
    for ( int i = len-1 ; i >= 0 ; i-- ) a[i+1] = (long) pop(stack);
    pop(stack); // remove arrayStart
    push(stack, a);
  ` ],
  [ 'arrayPrint', '.[]', af('arr', `
    long* a   = (long*) arr;
    int   len = (int) a[0];
    printf("[ ");
    for ( int i = 1 ; i <= len ; i++ ) {
      if ( i > 1 ) printf(", ");
      printf("%ld", (long) a[i]);
    }
    printf(" ]");
  `) ],
    [ 'isFn',           'fn?',             sf('i',     'i > 0 && i < heap->size && ( heap->arr[i] == callClosure || heap->arr[i] == callClosure0 )') ],
    [ 'arrayAt',        '@',               sf('a,i',   '((long*)a)[i+1]') ],
    [ 'arraySet',       ':@',              af('v,a,i', '((long*)a)[i+1] = v;') ], // ???: rename?
    [ 'arrayLen',       '#',               sf('a',     '((long*)a)[0]') ],
    [ 'arrayWithValue', '[]WithValue',     af('len,val', `
      long* a = (long*) malloc((len+1) * sizeof(long));
      a[0] = len;
      for ( int i = 1 ; i <= len ; i++ ) a[i] = val;
      push(stack, a);
    `) ],
    [ 'arrayWithFn',    '[]WithFn',        af('len,fn', `
      long* a = (long*) malloc((len+1) * sizeof(long));
      a[0] = len;
      for ( int i = 1 ; i <= len ; i++ ) {
        push(stack, (void*) (long) i);
        callI(fn);
        a[i] = (long) pop(stack);
      }
      push(stack, a);
    `) ],
    [ 'charAt',   'charAt',   af('s,i', 'push(stack, (void*) (long) ((i < strlen((char*) s)) ? (long) ((char*) s)[i] : (long) NULL));') ],
    [ 'charCode', 'charCode', af('c', 'char* s = (char*) malloc(2); s[0] = c; s[1] = 0; push(stack, s);') ],
    [ 'indexOf',  'indexOf',  af('a,v', `
      printf("indexOf %ld\\n", v);
      long* b   = (long*) a;
      long  len = b[0];
      for ( int i = 1 ; i <= len ; i++ ) {
        if ( b[i] == v ) { // TODO: better equals
          push(stack, (void*) (long) (i-1));
          return;
        }
      }
      push(stack, (void*) -1);
    `) ],
    [ 'strIndexOf',  '$indexOf',  af('s,v', `
      char* p = strstr((char*) s, (char*) v);
      push(stack, p == NULL ? (void*) -1l : (void*) p-s);
    `) ],
    [ 'len',      'len',      sf('s', 'strlen((char*) s)') ],
    [ 'key',      'key',      sf('', 'getc(tin)') ],
    [ 'sym',      'sym',      `
      char buf[256];
      readSym(buf, sizeof(buf));
      push(stack, strdup(buf));
    ` ],
    [ 'eval_',    'eval_',    'eval__();' ],

    [ 'fopen__',  'fopen',    sf('s',  'fopen((char*) s, "r")') ],
    [ 'fgetc__',  'fgetc',    sf('fd', 'fgetc((FILE*) fd)') ],
    [ 'fclose__', 'fclose',   sf('fd', 'fclose((FILE*) fd)') ],
    [ 'require__', 'require', af('f', 'require((char*) f);') ],

    [ 'exit__',   'exit',     'exit(0);' ],
//  [ '', '', f('', ``) ],
];

// Used below for var Get/Set/Incr/Decr
const EMIT_VAR = 'push2(code, (void*) (long) (fd-frame), (void*) offset)';

// ???: Should all instruction names end with a common suffix like 'I'?
exports.INSTRUCTIONS = [
//  name,               args,                    code,                      emit (String | Boolean)
  // TODO: add PIC to methodCall
  [ 'methodCall',       'char* name',            'push2(stack, name, stack->arr[stack->ptr-1]); call(); call();' ],
  [ 'longRet',          'long offset',           'returnTo(offset)',        true ],
  [ 'constant',         'void* v',               'push(stack, v)',          true ],
  [ 'autoConstant',     'void* v',               'push(stack, v); call()',  true ],
  [ 'varGet',           'int frame,long offset', 'push(stack, heap->arr[frameOffset(frame, offset)])', EMIT_VAR ],
  [ 'varSet',           'int frame,long offset', 'heap->arr[frameOffset(frame, offset)] = pop(stack)', EMIT_VAR ],
  [ 'varIncr',          'int frame,long offset', 'heap->arr[frameOffset(frame, offset)]++',            EMIT_VAR ],
  [ 'varDecr',          'int frame,long offset', 'heap->arr[frameOffset(frame, offset)]--',            EMIT_VAR ],
  [ 'callClosure',      'long pfp,long fn', `
    long ofp = fp;

    // Frame Pointer is start of where local variables will be allocated
    // First value in the frame is pointer to the previous frame
    fp = push(heap, (void*) pfp); // previous FP
    long ohp = heap->ptr;
    // printf("calling closure at: %ld, fp: %ld, fn: %ld, from: %ld\\n", closure, pfp, fn, ip);
    execute(fn);

    // Optimization, if nothing extra has been allocated on the heap, treat
    // it like a stack and revert back to position before pushing this frame.
    if ( heap->ptr == ohp ) heap->ptr = fp;

    fp = ofp;
    `
  ],
  [ 'createClosure',    'void* fn',  'push(stack, (void*) push3(heap, callClosure, (void*) fp, fn))' ],
  [ 'define',           'char* sym', 'scope = addSym(scope, sym, push2(heap, emitConstant, pop(stack)));' ],
  [ 'defineAuto',       'char* sym', `
    void* fn = pop(stack);
    scope = addSym(scope, sym, push2(heap, emitAutoConstant,  fn));

    sym[-1] = '&';
    scope = addSym(scope, &sym[-1], push2(heap, emitConstant, fn));
  ` ],
  [ 'forwardReference', 'char* sym', `
    long ptr = findSym(scope, sym);
    if ( ptr == -1 ) {
      printf("Unresolved reference: %s\\n", sym);
    } else {
      // TODO: this works for now because define and defineAuto are the same size
      // as forwardReference (2 words) but that could change (Ex. if common constant
      // values get their own instructions and become length 1.
      // Length 1 commands can be easily fixed by adding a NOP instruction,
      // but longer instructions will require either a jump or ...

      // printf("Resolving reference: %s\\n", sym);
      long oldPtr = code->ptr;
      code->ptr = ip-2; // back-up over [forwardReference, sym], two spaces
      callI(ptr);
      code->ptr = oldPtr;
      ip -= 2; // back-up again so we re-run the new definition
    }
  `],
  [ 'condI', 'int count', `
    for ( long i = 0 ; i < count ; i++ ) {
      ((Fn) nextI())();
      call();
      long value = (long) pop(stack);
      if ( value ) {
        ((Fn) nextI())();
        // Advance over unused constants in the switch statement
        ip += ( count - i - 1 ) * 4;
        return;
      }

      // skip over unused value
      ip += 2;
    }
  `, true ],
  [ 'switchI', 'int count', `
    char* c = (char*) pop(stack);
    for ( long i = 0 ; i < count ; i++ ) {
      nextI(); // skip over 'constant' instruction
      char* value = (char*) nextI();
      if ( strcmp(c, value) == 0 ) {
        ((Fn) nextI())();
        // Advance over unused constants in the switch statement
        ip += ( count - i - 1 ) * 4 + 2;
        return;
      }

      // skip over unused value
      ip += 2;
    }
    ((Fn) nextI())();
  `, true ]
];
