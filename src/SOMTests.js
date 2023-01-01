scope.eval$(`
" SOM Parser Tests" section

// SOMParser :som
SOMCompiler :som

{ sym src
  let
    som                                    :parser
    src "  " + 0 false som .ignore PStream :ps
    ps sym parser .call ()                 :result
  |
  nl print
  " Symbol: " sym + print
  " Input:  " src + print
  " Output: " result { | result .toString } { | false } ifelse + print
} ::;;

/*
'Equal '= ;;

'Comment '""thisisacomment"" ;;

'OperatorSequence '~ ;;
'OperatorSequence '& ;;
'OperatorSequence '| ;;
'OperatorSequence '* ;;
'OperatorSequence '/ ;;
'OperatorSequence '! ;;

'STStringChar 'A ;;
'STStringChar '" ;;
'STStringChar '' ;;

'STString " 'a string'" ;;

'Num '0 ;;
'Num '9 ;;
'Num 'a ;;

'AlphaNum '0 ;;
'AlphaNum '9 ;;
'AlphaNum 'a ;;

'Identifier " identifier " ;;
'Identifier 'testing123 ;;

'Separator '--- ;;
'Separator '---- ;;
'Separator '----- ;;

'Number '1 ;;
'Number '10 ;;
'Number '10.2 ;;
'Number '-2 ;;

'Keyword " testing: " ;;

'identifier 'identifier ;;

'unarySelector   'identifier ;;
'binarySelector  '= ;;
'KeywordSequence " keyword1:keyword2: " ;;
'keywordSelector " keyword1:keyword2: " ;;
'unarySelector   " symbol " ;;
'selector        " symbol " ;;

'literalSymbol '#symbol ;;
'literalNumber '1 ;;
'literal '1 ;;

'literalArray " #(12)" ;;
'literalArray " #(1 2)" ;;
'literal " #( 1 2 )" ;;
'literal " #( 1 2 'string' #(#a #b))" ;;
'literal '1 ;;

'variable 'a ;;

'literal '1 ;;
'literal '2 ;;
'primary '10 ;;

'unarySelector 'foo ;;
'unaryMessage 'foo ''

'unaryPattern 'foo ;;
'unaryMessage 'foo ;;

'primary '10  ;;
'binaryOperand '10ab  ;;
'binaryOperand '10  ;;

'formula '10+2 ;;

'result '5 ;;
'blockBody " ^5" ;;
'blockContents " ^5" ;;
'methodBlock " () " ;;
'method " foobar = ( )" ;;

*/

'result " a+b" ;;
'expression " a+b" ;;
'evaluation " a+b" ;;
'blockBodyReturn " ^ a+b" ;;
'blockBody " ^ a+b" ;;
'blockContents " | a b | ^ a + b" ;;
'methodBlock " ( | a b | ^ a + b )" ;;
'method " plus = ( | a b | ^ a + b )" ;;
'program " Ball = ( | x y r | )
ColourBall = Ball
  (
    | colour |
    r = ( ^ 'bar' )
    toString = ( ^ 'foo' )
    ----
    | a b c |
    aMethod = (^ 4 )
  )
" ;;


'Done print
`);