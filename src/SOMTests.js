scope.eval$(`
" SOM Parser Tests" section

SOMParser :som

{ sym src
  let
    src 0 false false PStream :ps
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
`);
