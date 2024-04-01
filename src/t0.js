var stack = [], heap = [], heap2 = [], hp, __arrayStart__ = '__arrayStart__', outerCode;
function fn(f) { return code => code.push(f); }
function bfn(f) { return fn(() => { var b = stack.pop(), a = stack.pop(); stack.push(f(a, b)); }); }
var scope = {
  readChar: function() { return this.ip < this.input.length ? this.input.charAt(this.ip++) : undefined; },
  match: function(s) { return this.input.substring(this.ip, this.ip + s.length) == s; },
  readSym:  function() {
    var sym = '', c;
    while ( c = this.readChar() ) {
      if ( /\s/.test(c) ) { if ( sym ) break; else continue; }
      sym += c;
    }
    // console.log('sym: ', sym);
    return sym;
  },
  eval$: src => {
    var oldInput = scope.input, oldIp = scope.ip;
    scope.input = src;
    scope.ip    = 0;
    for ( var sym ; sym = scope.readSym() ; ) scope.evalSym(sym, { push: f => f() });
    scope.input = oldInput;
    scope.ip    = oldIp
  },
  eval: code => { code.push(() => scope.eval$(stack.pop())); },
  evalSym: function(line, code) {
    // console.log('********line', line);
    var sym = scope[line];
    if ( sym ) { sym(code); }
    else if ( line.startsWith('.') ) {
      var m = line.substring(1);
      code.push(function() {
        stack.push(stack[stack.length-1]);
        stack.push(stack[stack.length-1]);
        stack[stack.length-2] = m;
      });
      this.evalSym('()', code);
      this.evalSym('()', code);
    } else if ( line.startsWith('::') ) {
      var sym = line.substring(2);
      code.push(function() {
        var value = stack.pop();
        scope[sym] = code => { code.push(code => stack.push(value)); scope.evalSym('()', code); };
        scope['&' + sym] = code => { code.push(() => stack.push(value)); };
      });
    } else if ( line.startsWith(':') ) {
      var sym = line.substring(1);
      code.push(function() { var value = stack.pop(); scope[sym] = (code) => code.push(() => stack.push(value))});
    } else if ( line.endsWith(':') ) {
      var sym = line.substring(0, line.length-1);
      // console.log('***************************************************************', sym);
      code.push(() => stack.push(sym));
    } else if ( line.charAt(0) >= '0' && line.charAt(0) <= '9' || ( line.charAt(0) == '-' && line.length > 1 ) ) {
      code.push(() => stack.push(scope.parseFloat_(line)));
    } else if ( line.startsWith("'") ) {
      var s = line.substring(1);
      code.push(() => stack.push(s));
    } else {
      console.log('Warning: Unknown Symbol or Forward Reference "' + line + '" at:', scope.input.substring(scope.ip, scope.ip+40).replaceAll('\n', '\\n'), ' ...');
      if ( line === '' ) debugger;
      code.push(() => {
        if ( typeof scope[line] !== 'function' ) { console.error('Error, invalid symbol: ', line); debugger; }
        scope[line]({ push: f => f()})
      });
    }
  },
  '{': function(code) {
    var start = scope.ip, oldScope = scope, vars = [], fncode = [], paramCount, name = '';
    var curScope = scope = Object.create(scope);
    function countFrames() { var d = 0, s = scope; while ( s !== curScope ) { s = s.__proto__; d++; } return d; }
    function framesUp(d) { var p = hp; for ( var i = 0 ; i < d ; i++ ) p = heap[p]; return p; }
    function accessor(index, f) { return code => { var d = countFrames(); code.push(() => f(framesUp(d) + index)); } }
    function defineVar(v, index) {
      scope[v]        = accessor(index, i => stack.push(heap[i]));
      scope[':' + v]  = accessor(index, i => { heap2[i] = v; heap[i] = stack.pop(); });
      scope[v + '++'] = accessor(index, i => heap[i]++);
      scope[v + '--'] = accessor(index, i => heap[i]--);
    }
    while ( ( l = scope.readSym() ) != '|' && l != 'let' ) {
      if ( vars.length == 0 && l.startsWith(':') ) {
        name = l.substring(1);
        scope[name + '<-'] = accessor(0, i => { throw name; });
      } else {
        vars.push(l); // read var names
      }
    }
    for ( let i = 0 ; i < vars.length ; i++ ) {
      let index = vars.length-i;
      defineVar(vars[i], index);
    }
    paramCount = vars.length;
    if ( l === 'let' ) { // handle local variables
      outer: while ( l !== '|' ) {
        while ( ! ( l = scope.readSym() ).startsWith(':') ) {
          if ( l == '|' ) break outer;
          scope.evalSym(l, fncode);
        }
        var n = l.substring(1);
        vars.push(n);
        defineVar(n, vars.length);
        scope.evalSym(l, fncode);
      }
    }
    while ( ( l = scope.readSym() ) != '}' ) scope.evalSym(l, fncode);
    oldScope.ip = scope.ip;
    scope = oldScope;
    var src = scope.input.substring(start-2, scope.ip-1);
    code.push(function() {
      var p = hp;
      var f = function() {
        // console.log('executing: ', src);
        var old = hp;
        hp = heap.length;
        heap.push(p);
        for ( var i = 0 ; i < paramCount ; i++ ) heap.push(stack.pop());
        try {
          for ( var i = 0 ; i < fncode.length ; i++ ) fncode[i]();
        } catch (x) {
          if ( x !== name ) throw x;
        }
        hp = old;
      };
      f.toString = function() { return src; }
      stack.push(f);
    });
  },
  switch: function(code) {
    var options = [], l;
    while ( ( l = scope.readSym() ) != 'end' ) scope.evalSym(l, options);
    for ( var i = 0 ; i < options.length-1 ; i += 2 ) { options[i](); options[i] = stack.pop(); }
    code.push(function() {
      var value = stack.pop();
      for ( var i = 0 ; i < options.length ; i += 2 ) {
        if ( value === options[i] ) {
          options[i+1]();
          return;
        }
      }
      return options[options.length-1]();
    });
  },
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
  parseFloat_: Number.parseFloat,
  parseFloat: fn(() => stack.push(scope.parseFloat_(stack.pop()))),
  debugger:  fn(() => {debugger;}),
  print:     fn(() => console.log('' + stack.pop())),
  if:        fn(() => { var block = stack.pop(); var cond = stack.pop(); if ( cond ) block(); }),
  ifelse:    fn(() => { var fBlock = stack.pop(), tBlock = stack.pop(), cond = stack.pop(); (cond ? tBlock : fBlock)(); }),
  while:     fn(() => { var block = stack.pop(), cond = stack.pop(); while ( true ) { cond(); if ( ! stack.pop() ) break; block(); } }),
  const:     fn(() => { var sym = stack.pop(), value = stack.pop(); scope[sym] = fn(() => { stack.push(value); }); }),
  mod:       bfn((a, b) => a % b),
  pick:      fn(() => stack.push(stack[stack.length-stack.pop()-2])),
  charAt:    bfn((s, i) => i < s.length ? s.charAt(i) : null),
  charCode:  fn(() => stack.push(String.fromCharCode(stack.pop()))),
  indexOf:   bfn((s, p) => s.indexOf(p)),
  len:       fn(() => { stack.push(stack.pop().length); }),
  input_:    fn(() => { stack.push(scope.input); }),
  ip_:       fn(() => { stack.push(scope.ip); }),
  emit:      () => { var v = stack.pop(); outerCode.push(() => stack.push(v)); },
  '<-':      fn(() => { throw ''; }),
  'string?': fn(() => { stack.push(typeof stack.pop() === 'string'); }),
  'array?':  fn(() => { stack.push(Array.isArray(stack.pop())); }),
  'i[':      code => { outerCode = code; var s = '', c; while ( (c = scope.readChar()) != ']' ) s += c; scope.eval$(s); },
  '"':       code => { var s = '', c; while ( (c = scope.readChar()) != '"' ) s += c; code.push(() => stack.push(s)); },
  '"""':     code => { var s = ''; while ( ! scope.match('"""') ) s += scope.readChar(); code.push(() => stack.push(s)); scope.readChar(); scope.readChar(); scope.readChar(); },
  '//':      () => { while ( scope.readChar() != '\n' );},
  '/*':      () => { while ( scope.readSym() != '*/' );},
  '!':       fn(() => { stack.push( ! stack.pop()); }),
  '&':       bfn((a,b) => a && b),
  '|':       bfn((a,b) => a || b),
  '&&':      fn(() => { var aFn = stack.pop(); if ( ! stack.pop() ) stack.push(false); else aFn(); }),
  '||':      fn(() => { var aFn = stack.pop(); if (   stack.pop() ) stack.push(true);  else aFn(); }),
  '=':       bfn((a,b) => a === b),
  '!=':      bfn((a,b) => a !== b),
  '<':       bfn((a,b) => a < b),
  '<=':      bfn((a,b) => a <= b),
  '>':       bfn((a,b) => a > b),
  '>=':      bfn((a,b) => a >= b),
  '+':       bfn((a,b) => a + b), // Should be a different concat for strings
  '*':       bfn((a,b) => a * b),
  '-':       bfn((a,b) => a - b),
  '/':       bfn((a,b) => a / b),
  '^':       bfn((a,b) => Math.pow(a,b)),
  ';':       (code) => { code.push(function() {
    var value = stack.pop(), sym = stack.pop();
    scope[sym] = (code) => code.push(() => stack.push(value))}) },
  '%':       fn(() => stack.push(stack.pop() / 100)),
  '()':      fn(() => { var f = stack.pop();
     // console.log('running: ', f.toString());
    if ( typeof f !== 'function' )
      console.error('Error: "' + f + '" not a function.');
      f(); }),
  '??':      code => {
    var s = scope;
    code.push(() => {
      var oldScope = scope, key = stack.pop();
      if ( ! s[key] ) { console.log('Key not found in ??: ', key); debugger; }
      scope = s; s[key]({push: f => f()}); scope = oldScope;
    });
  },
  include: async function load(fn) {
    var code = await fetch(fn).then(response => response.text()).catch(x => { debugger; });
    this.eval$(code);
  }
};

// scope.include('prefix.t0');
scope.eval$(`
1 1 = :true         // define true
1 2 = :false        // define false
{ | } :nil          // define 'nil', like doing nil = new Object() in Java/JS
{ n | 0 n - } ::neg // negate

9 charCode :tab  10 charCode :nl  13 charCode :cr

{ start end block | { | start end <= } { | start block () start++ } while } ::for

// Standard Forth-like Functions
{ v | v v } ::dup  { _ | } ::drop  { a b | b a } ::swap

// Standard High-Order Functions
{ a f | 0 a len 1 - { i | a i @ f () } for } ::do
{ a f | [ a f do ] } ::map
{ a v f | v a f do } ::reduce
{ a p | [  a { c | c p () { | c } if } do ] } ::filter
{ a | " " a { c | c + } do } ::join

// A helper function for displaying section titles
{ t | " " print t print } ::section
`);


/*
TODO:
  - ??? 'with' support, to be used with creating sub-contexts: with ( ctx ) { { let ... ?? } }
  - get rid of 'hp'? Put on end of heap.
  - fix 'nil to be falsey
  - make string function naming more consistent
  - have functions auto-call and use quoting to reference without calling?
  - return statement & recursive calls
  - optimize forward references
  - symbols
  - readSym() and readChar() should be callable from scripts
  - make eval() be the real method and eval$ call it
  - alloc?
  - don't put heap in an array to allow for JS GC?
  - Add in-line cache for method lookups, needs faster access to an object's class
  - Add a recursive array toString method
  - Fix associativeness of ** and ? operators
  - exceptions?
  - Don't use exceptions for returns
  - Make parsers be methods rather than functions, or, store 'this' on ps?

BUGS:
  - Can't update global variables inside of blocks

  { :outer a b c | ^outer ... }

  - ifelse is the same as ? ()

*/
