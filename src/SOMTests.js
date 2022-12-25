scope.eval$(`
" SOM Parser Tests" section

SOMParser :som

{ sym src
  let
    src "  " + 0 false false PStream :ps
    ps sym som .call ()       :result
  |
  [
    nl
    " Testing SOMParser Symbol: " sym nl
    " Input: " src nl
    " Output: " result { | result .toString } { | result } ifelse
  ] join print
} ::testSym


'Equal '= testSym

'Comment '""thisisacomment"" testSym

'OperatorSequence '~ testSym
'OperatorSequence '& testSym
'OperatorSequence '| testSym
'OperatorSequence '* testSym
'OperatorSequence '/ testSym
'OperatorSequence '! testSym

'STStringChar 'A testSym
'STStringChar '" testSym
'STStringChar '' testSym

'STString " 'a string'" testSym

'Num '0 testSym
'Num '9 testSym
'Num 'a testSym

'AlphaNum '0 testSym
'AlphaNum '9 testSym
'AlphaNum 'a testSym

'Identifier " identifier " testSym
'Identifier 'testing123 testSym

'Separator '--- testSym
'Separator '---- testSym
'Separator '----- testSym

'Number '1 testSym
'Number '10 testSym
'Number '10.2 testSym
'Number '-2 testSym

'Keyword " testing: " testSym

`);
