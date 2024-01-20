exports.CMDS = [
  [ 'call',   '()',       af('i',     'callInstruction(i);') ],
  [ 'ret',    '<-',       '' ],
  [ 'plus',   '+',        sf('a,b',   'a+b')  ],
  [ 'minus',  '-',        sf('a,b',   'a-b')  ],
  [ 'mul',    '*',        sf('a,b',   'a*b')  ],
  [ 'div',    '/',        sf('a,b',   'a/b')  ],
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
  [ 'if',     'if',       af('c,b',   'if ( c ) callInstruction(b);') ],
  [ 'ifelse', 'ifelse',   af('c,i,e', 'callInstruction(c ? i : e);')  ],
  [ 'drop',   'drop',     'pop(stack);' ],
  [ 'andand', '&&',       'void* aFn = pop(stack); if ( ! pop(stack) ) { push(stack, (void*) 0); } else { push(stack, aFn); call_(); }' ],
  [ 'oror',   '||',       'void* aFn = pop(stack); if ( pop(stack) ) { push(stack, (void*) 1); } else { push(stack, aFn); call_(); }'   ],
  [ 'for',    'for',      af('s,e,b', `  for ( long i = s ; i <= e ; i++ ) { push(stack, (void*) i); callInstruction(b); }`)   ],
  [ 'while',  'while',    af('c,b', `  while ( true ) { callInstruction(c); if ( ! pop(stack) ) break; callInstruction(b); }`) ],
  [ 'now',    'now',      `struct timeval tp; gettimeofday(&tp, NULL); push(stack, (void*) (tp.tv_sec * 1000 + tp.tv_usec / 1000));` ],
  [ 'print',  'print',    af('a', `
    printf("\\n\\033[1;30m"); // Print in bold black
    printf("%ld", a);
    printf("\\033[0m");      // Revert colour code`)
  ],
  [ 'printStr', 'print$', af('a', `
    printf("\\n\\033[1;30m"); // Print in bold black
    printf("%s", (char *) a);
    printf("\\033[0m");      // Revert colour code`)
  ]
];


exports.INSTRUCTIONS = [
  [ 'constant',       'void* v',               'push(stack, v)', true ],
  [ 'autoConstant',   'void* v',               'push(stack, v); call_()', true ],
  [ 'frameReference', 'int frame,long offset', 'push(stack, (void*) heap->arr[frameOffset(frame, offset)])' ],
  [ 'frameSetter',    'int frame,long offset', 'heap->arr[frameOffset(frame, offset)] = pop(stack)' ],
  [ 'frameIncr',      'int frame,long offset', 'heap->arr[frameOffset(frame, offset)]++' ],
  [ 'frameDecr',      'int frame,long offset', 'heap->arr[frameOffset(frame, offset)]--' ],
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