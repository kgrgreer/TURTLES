scope.eval$(`

'Grammar section ()

{ l op r |
  { o | [ l o .call [ op r o .call ] seq () opt () ] seq () }
} :bin // binary operator, ie. expr +/0 expr13

{ v |
  v 0 @
  v 1 @ { | "  " v 1 @ 1 @ "  " v 1 @ 0 @ + + + + } if
} :infix // convert an infix operator to postfix

{ o m super f | { ps | ps o m super () () f mapp () () } } :action


// https://developer.mozilla.org/en-US/docs/Web/JavaScript/Reference/Operators/Operator_Precedence

// Just a Parser, validates but has no semantic actions
{ let
  [ '== '= litMap () '!= ] alt ()                           :equality
  [ '<= '< '>= '> ] alt ()                                  :inequality
  '&& lit ()                                                :and
  '|| lit ()                                                :or
  { o | [ o .expr3 [ '? o .expr3 ': o .expr3 ] seq () opt () ] seq () } :ternary  // TODO: what should the second two expressions be?
  { o | [ o .lhs '= o .expr ] seq () }                      :assignment
  { o | [ o .assignment o .ternary o .expr3 ] alt () }      :expr2
  'expr4 or 'expr3  bin ()                                  :expr3
  'expr5 and 'expr4 bin ()                                  :expr4
  'expr9 equality 'expr8  bin ()                            :expr8
  'expr10 inequality 'expr9  bin ()                         :expr9
  'expr12 '+- anyChar () 'expr11  bin ()                    :expr11
  'expr13 '*/%  anyChar () 'expr12  bin ()                  :expr12
  'expr14 '** '^ litMap () 'expr13 bin ()                   :expr13 // TODO: fix, I think it should be right-associative
  { o | [
    o .notPrefix
    o .iPrefix
    o .expr15
  ] alt () }                                                :expr14
  { o | [ o .expr16 [ '++ '-- ] alt () opt () ] seq () }    :expr15
  { o | [ o .expr18 [ '[ o .expr '] ] 1 seq1 () 1 repeat () opt () ] seq () } :expr17
  { o | [ o .lhs o .number o .bool o .group o .array ] alt () }    :expr18
  { o | [ '! o .expr15 ] 1 seq1 () }                        :notPrefix
  { o | [ [ '-- '++ ] alt () o .expr15 ] seq () }           :iPrefix
  { o | [ '( o .expr ') ] 1 seq1 () }                       :group
  { o | o .digit 1 repeat () }                              :number
  { o | '0 '9 range () }                                    :digit
  { o | [ 'true 'false ] alt () }                           :bool
  { o | [ '[ o .expr ', lit () delim () '] ] 1 seq1 () }    :array
  { o | [
      [ '_ 'a 'z' range () 'A 'Z range () ] alt ()
      [ '_ 'a 'z' range () 'A 'Z range () '0 '9 range () ] alt () 0 repeat () join mapp ()
    ] seq () join mapp () }                                 :lhs
  |
  { m | m switch
    'parse$     { s o | s 0 nil PStream () o .start () .value }
    'call       { m o | o m o () () }
    'start      { o | o .expr }
    'expr       { o | o .expr2 }
    'expr2      expr2
    'expr3      expr3
    'expr4      expr4
    'expr5      { o | o .expr8 }
    'expr8      expr8
    'expr9      expr9
    'expr10     { o | o .expr11 }
    'expr11     expr11
    'expr12     expr12
    'expr13     expr13
    'expr14     expr14
    'expr15     expr15
    'expr16     { o | o .expr17 }
    'expr17     expr17
    'expr18     expr18
    'ternary    ternary
    'assignment assignment
    'lhs        lhs
    'notPrefix  notPrefix
    'iPrefix    iPrefix
    'group      group
    'number     number
    'array      array
    'bool       bool
    'digit      digit
    'inequality inequality
    'equality   equality
    // All of the above "'sym sym"'s could be eliminated if I had access to the compile-time scope at runtime
    { o | m print m ?? }
  end }
} :FormulaParser


// Add semantic actions to parser to create a JS to T0 compiler
{ | { let FormulaParser () :super |
  // TODO: factor out common actions
  { m | m switch
    'super      { m o | o m super () () () }
    'ternary    { | m super { a | a 1 @ { | [ a 0 @ "  { | " a 1 @ 1 @ "  } { | " a 1 @ 3 @ "  } ifelse" ] join () } { | a 0  @ } ifelse }  action () }
    'assignment { | m super { a | a 2 @ "  dup () :" a 0 @ + + } action () }
    'expr3      { | m super { a | a 1 @ { | [ a 0 @ [ a 1 @ 0 @ " { | " a 1 @ 1 @ "  }" + + ] ] } { | a } ifelse  infix () }  action () }
    'expr4      { | m super { a | a 1 @ { | [ a 0 @ [ a 1 @ 0 @ " { | " a 1 @ 1 @ "  }" + + ] ] } { | a } ifelse  infix () }  action () }
    'expr8      { | m super infix action () }
    'expr9      { | m super infix action () }
    'expr11     { | m super infix action () }
    'expr12     { | m super infix action () }
    'expr13     { | m super infix action () }
    'expr15     { | m super { a | a 0 @ a 1 @ { | "  " a 0 @ a 1 @ + + + } if } action () }
    'expr17     { | m super  { a | a 0 @ a 1 @ { | "  " + a 1 @ { e |  e + "  @ " + } forEach () } if } action () }
    'notPrefix  { | m super { | "  !" + } action () }
    'iPrefix { | m super { a | a 1 @ a 0 @ + "  " a 1 @ + + } action () }
    'number     { | m super join  action () }
    'array      { | m super  { a | " [" a { e | "  " + e + } forEach () "  ]" + } action () }
    { o | o m super () () }
  end }
} () } :FormulaCompiler


{ code |
  { let code FormulaCompiler () .parse$ :result |
    " " print
    " JS Code: " code   + print
    " T0 Code: " result + print
    { let result eval :v | " Result: " v + print v } ()
  } ()
} :jsEval


" 1+2*3 "          jsEval ()
" 5*2**(2+3)+100 " jsEval ()
" 1<2 "            jsEval ()
" 1>2 "            jsEval ()
" 1>2||1<2 "       jsEval ()
" [1,[1,2],3][1][0] " jsEval ()
" answer=42 " jsEval ()
" answer=[[1,0],[0,1]][1][1]+((99<=99?1:0)+1)>2||1<2&&5==3&&!true " jsEval ()

{ let 1 :a 2 :b 3 :c | [ '****** a '- b '- c ] join () print } ()
4 { z let 1 :a 2 :b 3 :c | [ '****** z a '- b '- c '- z ] join () print } () // TODO: fix

1 { i |
  i i++ print
  i++ i print
  " i=5  " jsEval ()
  " i    " jsEval ()
  " i+1  " jsEval ()
  /* Next two compile correctly but don't run because there is no 'i' in their scope.
  " ++i  " jsEval ()
  " i++  " jsEval ()
  */
} ()



/*
" 5*2^(2+3)+100 " 0 nil PStream () :ps

'a print
'b print
FormulaParser () :formulaparser
// ps formulaparser 'digit formulaparser ()  () print
'c print
// ps formulaparser.digit () print
'd print
// ps formulaparser.number () print
'e print
'b print
ps formulaparser.parse () .toString print

// ps 'test formulaparser.call () print
result.value
*/

`);

// TODO: recode in T0
scope['js{'] = code => {
  var start = scope.ip, end;
  while ( scope.readSym() != '}js' ) end = scope.ip;
  var s = scope.input.substring(start, end);
  stack.push(s);
  scope.eval$('FormulaCompiler () .parse$ eval');
};

scope.eval$(`
" Embedded Javascript" section ()

{ let 3 :i |
  js{ 1+2*3 }js print
} ()
`);
