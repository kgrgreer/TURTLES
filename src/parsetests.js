scope.eval$(`
" Parser Tests" section

" thisthenthat0123 " 0 false false PStream :ps

" Literal Parser" section
ps 'this lit () .value print
'that print
ps 'that lit () print
ps 'this lit () .toString print

/*
" " 0 false false PStream :ps
ps "  " lit 1 repeat ()
print

"  " 0 false false PStream :ps
ps "  " lit () .toString
print
*/

" Seq Parser" section
[ 'this 'then 'that ] seq ::seqparser
ps seqparser print


" Alt Parser" section
[ 'think 'this ] alt :altparser
ps altparser () .toString print


" Range Parser" section
'0 '9 range :rangeparser
ps rangeparser () print
'a 'z range :rangeparser
ps rangeparser () .toString print


" Repeat Parser" section
'a 'z range 1 repeat :repeatparser
ps repeatparser () .toString print


" Optional Parser" section
'this print
ps 'this lit opt () .toString print


'that print
ps [ 'that lit opt 'this ] seq () .toString print


'thisthen print
ps [ 'this lit opt 'then ] seq () .toString print


" NotChars Parser" section
ps " 0123456789" notChars 0 repeat () .toString print
`);
