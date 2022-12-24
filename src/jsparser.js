scope.eval$(`

{ l op r |
  { o | [ l o .call [ op r o .call ] seq opt ] seq }
} ::binary // binary operator, ie. expr +/0 expr13

{ v |
  v 0 @
  v 1 @ { | "  " v 1 @ 1 @ "  " v 1 @ 0 @ + + + + } if
} :infix // convert an infix operator to postfix

{ o m super f | { ps | o m super () () f mapp ps .parse } } ::action


// https://developer.mozilla.org/en-US/docs/Web/JavaScript/Reference/Operators/Operator_Precedence

// Just a Parser, validates but has no semantic actions
{ let
  parse$: { s o | s 0 nil o .ignore PStream [ o .ignore opt o .start ] 1 seq1 () .value } ;
  call:   { m o | o m o () () } ;
  start:  { o |  [ o .stmts o .expr ] alt } ;
  stmt:   { o |
    [
      o .if
      o .while
      [ o .expr '; ] 0 seq1
      o .block
    ] alt
  } ;
  if: { o | [ 'if '( o .expr ') o .stmt [ 'else o .stmt ] 1 seq1 opt ] seq } ;
  while: { o | [ 'while '( o .expr ') o .stmt ] seq } ;
  block: { o | [ '{ o .stmts '} ] 1 seq1 } ;
  stmts: { o |  o .stmt '; lit delim } ;
  equality: [ '== '= litMap '!= ] alt ;
  inequality: [ '<= '< '>= '> ] alt ;
  and: '&& lit ;
  or: '|| lit ;
  ternary: { o | [ o .expr3 [ '? o .expr3 ': o .expr3 ] seq opt ] seq } ;
  assignment: { o | [ o .lhs '= o .expr ] seq } ;
  expr: { o | o .expr2 } ;
  expr2: { o | [ o .assignment o .ternary o .expr3 ] alt } ;
  expr3: 'expr4 or 'expr3 binary ;
  expr4: 'expr5 and 'expr4 binary ;
  expr5: { o | o .expr8 } ;
  expr8: 'expr9 equality 'expr8 binary ;
  expr9: 'expr10 inequality 'expr9 binary  ;
  expr10: { o | o .expr11 } ;
  expr11: 'expr12 '+- anyChar 'expr11 binary ;
  expr12: 'expr13 '*/%  anyChar 'expr12 binary ;
  expr13: 'expr14 '** '^ litMap 'expr13 binary ;
  expr14: { o | [ o .notPrefix o .iPrefix o .expr15 ] alt } ;
  expr15: { o | [ o .expr16 [ '++ '-- ] alt opt ] seq } ;
  expr16: { o | o .expr17 } ;
  expr17: { o | [ o .expr18 [ '[ o .expr '] ] 1 seq1 plus opt ] seq } ;
  expr18: { o | [ o .lhs o .number o .bool o .group o .array ] alt } ;
  notPrefix: { o | [ '! o .expr15 ] 1 seq1 } ;
  iPrefix: { o | [ [ '-- '++ ] alt o .expr15 ] seq } ;
  group: { o | [ '( o .expr ') ] 1 seq1 } ;
  number: { o | o .digit plus } ;
  digit: { o | '0 '9 range } ;
  bool: { o | [ 'true 'false ] alt } ;
  array: { o | [ '[ o .expr ', lit delim '] ] 1 seq1 } ;
  lhs: { o | [
    [ '_ 'a 'z' range 'A 'Z range ] alt
    [ '_ 'a 'z' range 'A 'Z range '0 '9 range ] alt
    star &join mapp
  ] seq &join mapp } ;
  space: { o | [ tab cr nl "  " ] alt plus } ;
  comment: { o | [ " //" nl notChars star nl ] seq } ;
  ignore: { o | [ o .space  o .comment ] alt plus tok } ;
  | { | ?? }
} :FormulaParser

// Add semantic actions to parser to create a JS to T0 compiler
{ | { let FormulaParser () :super |
  // TODO: factor out common actions
  { m | m switch
    'super      { m o | o m super () () () }
    'if         { | m super { a | [
      a 2 @ "  { | " a 4 @  "  }" a 5 @ { | "  { | " a 5 @ "  } ifelse " } { | "  if " } ifelse
    ] join } action }
    'while         { | m super { a | [
      "  { | " a 2 @ "  } { | " a 4 @  "  } while "
    ] join } action }
    'stmts      { | m super { a | a " " { i | "  " i + + } reduce } action } // TODO: fix add an extra space prefix
    'ternary    { | m super { a | a 1 @ { | [ a 0 @ "  { | " a 1 @ 1 @ "  } { | " a 1 @ 3 @ "  } ifelse" ] join } { | a 0  @ } ifelse } action }
    'assignment { | m super { a | a 2 @ "  dup :" a 0 @ + + } action }
    'expr3      { | m super { a | a 1 @ { | [ a 0 @ [ a 1 @ 0 @ " { | " a 1 @ 1 @ "  }" + + ] ] } { | a } ifelse  infix () } action }
    'expr4      { | m super { a | a 1 @ { | [ a 0 @ [ a 1 @ 0 @ " { | " a 1 @ 1 @ "  }" + + ] ] } { | a } ifelse  infix () } action }
    'expr8      { | m super infix action }
    'expr9      { | m super infix action }
    'expr11     { | m super infix action }
    'expr12     { | m super infix action }
    'expr13     { | m super infix action }
    'expr15     { | m super { a | a 0 @ a 1 @ { | "  " a 0 @ a 1 @ + + + } if } action }
    'expr17     { | m super { a | a 0 @ a 1 @ { | "  " + a 1 @ { e | e + "  @ " + } forEach } if } action }
    'notPrefix  { | m super { | "  !" + } action }
    'iPrefix    { | m super { a | a 1 @ a 0 @ + "  " a 1 @ + + } action }
    'number     { | m super &join action }
    'array      { | m super  { a | " [" a { e | "  " + e + } forEach "  ]" + } action }
    { | m super () () }
  end }
} () } ::FormulaCompiler

'startCommentTest print
"     // foo bar


// more to ignore
1+2
" 0 true { | } PStream FormulaCompiler .ignore () .value print
'endCommentTest print


{ code |
  { let code FormulaCompiler .parse$ :result |
    " " print
    " JS Code: " code   + print
    " T0 Code: " result + print
    { let result eval :v | " Result: " v + print v } ()
  } ()
} ::jsEval


" 1 +     2 *
3 "          jsEval
" 5*2**(2+3)+100 " jsEval
" 1<2 "            jsEval
" 1>2 "            jsEval
" 1>2||1<2 "       jsEval
" [1,[1,2],3][1][0] " jsEval
" answer=42 " jsEval
" answer=[[1,0],[0,1]][1][1]+((99<=99?1:0)+1)>2||1<2&&5==3&&!true " jsEval

{ let 1 :a 2 :b 3 :c | [ '****** a '- b '- c ] join print } ()
4 { z let 1 :a 2 :b 3 :c | [ '****** z a '- b '- c '- z ] join print } () // TODO: fix

/*
1 { i |
  i i++ print
  i++ i print
  " i=5  " jsEval
//   " i " jsEval // TODO: doesn't work
  " i+1 " jsEval
  // Next two compile correctly but don't run because there is no 'i' in their scope.
  // " ++i  " jsEval
  // " i++  " jsEval
  //
} ()
*/


"
if ( true ) 1+1;

" jsEval print

"
if ( 2 > 1 ) 1 else 2
" jsEval print

"
if ( 2 > 1 ) { 1+2 } else { 3*4 }
" jsEval print

"
while ( false ) { 1; 2 }
" jsEval print

`);

// TODO: recode in T0
scope['js{'] = code => {
  var start = scope.ip, end;
  while ( scope.readSym() != '}js' ) end = scope.ip;
  var s = scope.input.substring(start, end);
  stack.push(s);
  scope.eval$('FormulaCompiler .parse$ eval');
};

scope.eval$(`
" Embedded Javascript" section

{ let 3 :i |
  js{ //
    // this is a comment
    if ( true ) {
      1 +2*3**4;
      1+2
    }
  }js print
} ()
`);
