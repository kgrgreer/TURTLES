scope.eval$(`
" SOM Parser Tests" section

SOMParser :som

{ sym src
  let
    som                                    :parser
    src "  " + 0 false som .ignore PStream :ps
    ps sym parser .call ()                 :result
  |
  [
    nl
    " Testing SOMParser Symbol: " sym nl
    " Input: " src nl
    " Output: " result { | result .toString } { | result } ifelse
  ] join print
} ::;;


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
`);
