exports.CMDS = [
  [ 'call',   '()',       af('i',     'callI(i);') ],
  [ 'ret',    '<-',       '' ],
  [ 'plus',   '+',        sf('a,b',   'a+b')  ],
  [ 'minus',  '-',        sf('a,b',   'a-b')  ],
  [ 'mul',    '*',        sf('a,b',   'a*b')  ],
  [ 'divide', '/',        sf('a,b',   'a/b')  ],
  [ 'mod',    '%',        sf('a,b',   'a%b')  ],
  [ 'eq',     '=',        sf('a,b',   'a==b') ],
  [ 'neq',    '!=',       sf('a,b',   'a!=b') ],
  [ 'lt',     '<',        sf('a,b',   'a<b')  ],
  [ 'gt',     '!=',       sf('a,b',   'a>b')  ],
  [ 'lte',    '!=',       sf('a,b',   'a<=b') ],
  [ 'gte',    '!=',       sf('a,b',   'a>=b') ],
  [ 'not',    '!',        sf('a',     '!a')   ],
  [ 'and',    '&',        sf('a,b',   'a&&b') ],
  [ 'or',     '|',        sf('a,b',   'a||b') ],
  [ 'if_',    'if',       af('c,b',   'if ( c ) callI(b);') ],
  [ 'ifelse_','ifelse',   af('c,i,e', 'callI(c ? i : e);')  ],
  [ 'drop',   'drop',     'pop(stack);' ],
  [ 'andand', '&&',       'void* aFn = pop(stack); if ( ! pop(stack) ) { push(stack, (void*) 0); } else { push(stack, aFn); call(); }' ],
  [ 'oror',   '||',       'void* aFn = pop(stack); if ( pop(stack) ) { push(stack, (void*) 1); } else { push(stack, aFn); call(); }'   ],
  [ 'for_',   'for',      af('s,e,b', 'for ( long i = s ; i <= e ; i++ ) { push(stack, (void*) i); callI(b); }')   ],
  [ 'while_', 'while',    af('c,b',   'while ( true ) { callI(c); if ( ! pop(stack) ) break; callI(b); }') ],
  [ 'repeat', 'repeat',   af('b,t',   'for ( long i = 0 ; i <= t ; i++ ) callI(b);') ],
  [ 'now',    'now',      `struct timeval tp; gettimeofday(&tp, NULL); push(stack, (void*) (tp.tv_sec * 1000 + tp.tv_usec / 1000));` ],
  [ 'print',  '.',        af('a', `
    printf("\\n\\033[1;30m"); // Print in bold black
    printf("%ld", a);
    printf("\\033[0m");      // Revert colour code`)
  ],
  [ 'printStr', '.$',     af('a', `
    printf("\\n\\033[1;30m"); // Print in bold black
    printf("%s", (char *) a);
    printf("\\033[0m");      // Revert colour code`)
  ],
  [ 'arrayStart', '[', 'push(stack, &arrayStart);' ],
  [ 'arrayEnd',   ']', `
    long start = stack->ptr-1;
    for ( ; start && stack->arr[start] != &arrayStart ; start-- );
    int len = stack->ptr-start-1;
    long* a = (long*) malloc((len+1) * sizeof(long));
    a[0] = len;
    for ( int i = len-1 ; i >= 0 ; i-- ) a[i+1] = (long) pop(stack);
    pop(stack); // remove arrayStart
    push(stack, a);
  ` ],

  [ 'arrayPrint', '.[]', af('arr', `
    long* a = (long*) arr;
    int len = (int) a[0];
    printf("[ ");
    for ( int i = 1 ; i <= len ; i++ ) {
      if ( i > 1 ) printf(", ");
      printf("%ld", (long) a[i]);
    }
    printf(" ]");
  `) ],
    [ 'arrayAt',  '@',  sf('a,i',   '((long*)a)[i+1]') ],
    [ 'arraySet', ':@', af('v,a,i', '((long*)a)[i+1] = v;') ],
    [ 'arrayLen', '#',  sf('a',     '((long*)a)[0]') ],

//  [ '', '', f('', ``) ],

];

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
})
*/



exports.INSTRUCTIONS = [
  [ 'constant',       'void* v',               'push(stack, v)', true ],
  [ 'autoConstant',   'void* v',               'push(stack, v); call()', true ],
  [ 'varGet',         'int frame,long offset', 'push(stack, heap->arr[frameOffset(frame, offset)])', 'push2(code, (void*) (long) (fd-frame), (void*) offset)' ],
  [ 'varSet',         'int frame,long offset', 'heap->arr[frameOffset(frame, offset)] = pop(stack)', 'push2(code, (void*) (long) (fd-frame), (void*) offset)' ],
  [ 'varIncr',        'int frame,long offset', 'heap->arr[frameOffset(frame, offset)]++',            'push2(code, (void*) (long) (fd-frame), (void*) offset)' ],
  [ 'varDecr',        'int frame,long offset', 'heap->arr[frameOffset(frame, offset)]--',            'push2(code, (void*) (long) (fd-frame), (void*) offset)' ],
  [ 'callClosure',    'long pfp,long fn', `
    long ofp = fp;

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
  [ 'createClosure',  'void* fn',              'push(stack, (void*) push3(heap, callClosure, (void*) fp, fn))' ]
];
