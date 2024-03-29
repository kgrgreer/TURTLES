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


'Number " 0" ;;
'literalNumber " 0" ;;
'literal " 0" ;;
'primary " 0" ;;
'evaluation " 0" ;;
'result " a+b" ;;
'expression " a+b" ;;
'evaluation " a+b" ;;
'blockBodyReturn " ^ a+b" ;;
'blockBody " ^ a+b" ;;
'blockContents " | a b | ^ a + b" ;;
'methodBlock " ( | a b | ^ a + b )" ;;
'method " plus = ( | a b | ^ a + b )" ;;
'unaryPattern " foo" ;;
'binaryPattern " + argument" ;;
'keywordPattern " double: d" ;;
'keywordPattern " x: x y: y" ;;
'method " double: d = ( ^ d + d )" ;;
'method " x: x y: y = ( ^ x * y )" ;;
'assignation " x:=y:=0" ;;
'method " center = ( x := y := 0 )" ;;
'methodBlock " (x := y := 0)" ;;
'blockContents " x := y := 0" ;;
'blockBody " x := y := 0" ;;

'keywordMessage " or: true " ;;
'messages " or: true " ;;

'evaluation " true or: true " ;;

'expression " true or: true " ;;


'program " Ball = (
  | x y r |
  center = ( x := y := 0 )
  withoutLocalVars = ( a := 1 )
  withLocalVars = ( | a b c | a := 1 )
)
ColourBall = Ball (
  | colour |
  color = ( ^ color )
  color: c = ( color := c )
  r = ( ^ 'bar' )
  nativeMethod = primitive
  toString = ( ^ 'foo' )
  ----
  | a b c |
  aMethod = (^ 4 )
)
" ;;

'nestedBlock " []" ;;
'nestedBlock " [42]" ;;

'primary " [42]" ;;
'evaluation " [42]" ;;
'expression " [42]" ;;


'argument " a1" ;;
'blockArguments " :a1" ;;
'blockPattern " :a1|" ;;
'nestedBlock " [^42]" ;;
'nestedBlock " [:a1 :a2| ^42]" ;;
'nestedBlock " [ |l1 l2| ^42]" ;;
'nestedBlock " [ :a1 :a2 ||l1 l2| ^42]" ;;
*/


'program " Test1 = (
  | a b c |
  bar = ( ^ 2 )
  foo = (
      self ifTrue: [ ^ 42 ] ifElse: [ ^ 66 ]
  )
  foo2 = (
      self ifTrue: [ ^ 42 ]
  )
)
" ;;

'messages " ifTrue: [ ^ 42 ] ifElse: [ ^ 66 ]" ;;
'keywordMessage " ifTrue: [ ^ 42 ] ifElse: [ ^ 66 ]" ;;

'program " Test2 = (
  bar = ( ^ 2 )
  foo = (
      self ifTrue: [ ^ 42 ]
  )
)
" ;;

debugger

'program " Test3 = (

  arg1: a1 arg2: a2 = ( self foo. self bar. ^2 )

)
" ;;

'program " Boolean = (

    ifTrue: trueBlock ifFalse: falseBlock = (
        self ifTrue:  [ ^trueBlock value  ].
        self ifFalse: [ ^falseBlock value ]
    )

    || boolean = ( ^self or: boolean  )
    && boolean = ( ^self and: boolean )

)
" ;;

'unaryPattern " foo" ;;
'binaryPattern " + argument" ;;
'keywordPattern " double: d" ;;
'keywordPattern " double: d trouble: t" ;;

'messages " foo" ;;
'messages " double: d" ;;
'messages " + argument" ;;
'messages " double: d" ;;
'messages " double: d trouble: t" ;;
'unarySelector " double: " ;;

'keywordPattern " x: x y: y" ;;
'formula " a  y: 4" ;;
'binaryOperand " a  y: 4" ;;
'Identifier " a  y: 4" ;;


// Should return [ selector, values ]
'unaryMessage " foo" ;;
'binaryMessage " + 42" ;;
'keywordMessage " double: d" ;;
'keywordMessage " x: a  y: 4" ;;

'method " || boolean = ( ^self )" ;;
'method " || boolean = ( ^ 1 + 2 )" ;;
'pattern " arg1: a1 arg2: a2" ;;
'keywordPattern " arg1: a1 arg2: a2" ;;
'keyword " arg1:" ;;
'method " || boolean = ( ^ true or: false )" ;;

'blockBodyExpression " a" ;;
'blockBodyExpression " a . b" ;;
'expression " 1 + 2" ;;
'evaluation " 1 + 2" ;;
'messages " + 2" ;;
'method " arg1: a1 arg2: a2 = ( 42 . 56 . ^ 22 )" ;;
'method " arg1: a1 arg2: a2 = ( self foo. self bar. ^2 )" ;;
'method " arg1: a1 arg2: a2 = ( self foo. self bar. ^2 )" ;;
'method " arg1: a1 arg2: a2 = ( self foo )" ;;
'evaluation " self foo" ;;
'messages " foo" ;;
'unaryMessage " foo" ;;
'unaryMessage " foo" ;;


'method " arg1: a1 arg2: a2 = ( self foo. self bar. ^2 )" ;;
'blockBodyExpression " self plus: 7" ;;
'blockBodyExpression " self plus: 7. self foo. self bar. ^2" ;;

'blockBodyExpression " newEntry hash: 42" ;;
'blockBodyExpression " newEntry hash: (k hashcode)" ;;

'blockBodyExpression " self ifTrue: [ ^42 ] " ;;
'blockBodyExpression " self key ifTrue: [ ^42 ] " ;;
'blockBodyExpression " key = self  key  ifTrue: [ ^42 ] " ;;
'blockBodyExpression " key = self" ;;

'method """ getValue: key = (
        key = self key ifTrue: [ ^value ].
        next isNil ifTrue: [ ^nil ].
        ^next getValue: key.
    )
""" ;;


'program " Pair = (

    | key value |

    key = ( ^key )
    value = ( ^value )

    key: aKey = ( key := aKey )
    value: aValue = ( value := aValue )

    print = ( '[' print. key print. '=>' print. value print. ']' print )
    println = ( self print. '' println )

    ----

    withKey: aKey andValue: aValue = (
        | newPair |
        newPair := super new.
        newPair key: aKey.
        newPair value: aValue.
        ^newPair
    )
)
" ;;

'program """ HashEntry = (

    | key value next hash |

    key       = ( ^key )
    value     = ( ^value )
    next      = ( ^next )
    hash      = ( ^hash )

    key: k    = ( key := k )
    value: v  = ( value := v )
    next: n   = ( next := n )
    hash: h   = ( hash := h )

    setKey: key value: value = (
        key = self key
            ifTrue: [ self value: value. ^false. ]
            ifFalse: [
            next isNil
                ifTrue: [
                    self next: (HashEntry newKey: key value: value next: nil).
                    ^true. ]
                ifFalse: [
                    ^(self next setKey: key value: value) ] ].
    )

    getValue: key = (
        key = self key ifTrue: [ ^value ].
        next isNil ifTrue: [ ^nil ].
        ^next getValue: key.
    )

    containsKey: key = (
        key = self key ifTrue: [ ^true ].
        next isNil ifTrue: [ ^false ].
        ^next containsKey: key.
    )

    containsValue: value = (
        value = self value ifTrue: [ ^true ].
        next isNil ifTrue: [ ^false ].
        ^next containsValue: value.
    )
    keys = (
        next isNil
            ifTrue: [ ^Vector new append: key ]
            ifFalse: [ ^(next keys), key ]
    )

    values = (
        next isNil
            ifTrue: [ ^ Vector with: value ]
            ifFalse: [ ^(next values), value ]
    )

    ----

    newKey: k value: v next: n = (
        | newEntry |
        newEntry := super new.
        newEntry key: k.
        newEntry value: v.
        newEntry next: n.
        newEntry hash: (k hashcode).
        ^newEntry
    )
)
""" ;;

'method " foo = ( a := 1 )" ;;
'method " foo = ( a := b := 1 )" ;;
'method " foo = ( a := b := c:= 1 )" ;;

'program """
Fibonacci = (          "defines a subclass of Object"
    fib: n = (         "defines the fib method with the argument n"
        ^ n <= 1
            ifTrue:  1
            ifFalse: [ self fib: (n - 1) + (self fib: (n - 2)) ]
    )
)
""" ;;


'Done print
`);
