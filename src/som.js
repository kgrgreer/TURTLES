
scope.eval$(`
{ a w let true :first | a ! { | " " <- } if a { i | i } filter { i | first { | false :first i } { | w i + } ifelse } map join } ::joinWith // join, space separated, removing false values
{ a | a "  " joinWith } ::joins // join, space separated, removing false values

// "translated from: https://github.com/SOM-st/SOM/blob/master/specification/SOM.g4"
{ let
  call:   { m o | o m o () () } ;
  program: { o | o .classdef plus } ;
  classdef: { o | [
    o .identifier '= o .superclass '(
      o .instanceFields
      o .method star
      [ o .Separator o .classFields opt o .method star ] seq opt
    ')
  ] seq } ;
  superclass: { o | o .Identifier opt } ;
  instanceFields: { o | o .fields } ;
  classFields: { o | o .fields } ;
  fields: { o | [ '| o .variable star '| ] 1 seq1 opt } ;
  method: { o | [ o .pattern '= [ o .STPrimitive o .methodBlock ] alt ] seq } ;
  pattern: { o | [ o .keywordPattern o .binaryPattern o .unaryPattern ] alt } ;
  unaryPattern: { o | o .unarySelector } ;
  binaryPattern: { o | [ o .binarySelector o .argument ] seq } ;
  keywordPattern: { o | [ o .keyword o .argument ] seq plus } ;
  methodBlock: { o | [ '( o .blockContents opt ') ] 1 seq1 } ;
  unarySelector: { o | o .identifier } ;
  binarySelector: { o | o .OperatorSequence } ;
  identifier: { o | [ o .STPrimitive { | o .Identifier () } ] alt } ;
  keyword: { o | o .Keyword } ;
  argument: { o | o .variable } ;
  blockContents: { o | [ [ '| o .localDefs '| ] 1 seq1 opt { | o .blockBody () } ] seq } ;
  localDefs: { o | o .variable star } ;
  blockBody: { o | [ { | o .blockBodyReturn () } { | o .blockBodyExpression () } ] alt } ;
  blockBodyReturn: { o |  [ '^ o .result ] 1 seq1 } ;
  blockBodyExpression: { o | [ { | o .expression () } [ '. { | o .blockBody () } opt ] 1 seq1 opt ] seq } ;
  result: { o | [ o .expression '. lit opt ] 0 seq1 } ;
  expression: { o | [ o .assignation o .evaluation ] alt } ;
  assignation: { o | [ o .assignments o .evaluation ] seq } ;
  assignments: { o | o .assignment plus } ;
  assignment: { o | [ o .variable ':= ] 0 seq1 } ;
  evaluation: { o | [ o .primary o .messages opt ] seq } ;
  primary: { o | [ o .variable { | o .nestedTerm () } { | o .nestedBlock () } { | o .literal () } ] alt } ;
  variable: { o | o .identifier } ;
  messages: { o | [
    [ " " [ ] litMap dup o .keywordMessage ] seq
    [ " " [ ] litMap o .binaryMessage plus o .keywordMessage opt ] seq
    [ o .unaryMessage2 plus o .binaryMessage star o .keywordMessage opt ] seq
  ] alt } ;
  unaryMessage2: { o | [ o .unaryMessage o .Whitespace ] 0 seq1 tok } ;
  unaryMessage: { o | { | o .unarySelector () } } ;
  binaryMessage: { o | [ o .binarySelector o .binaryOperand ] seq } ;
  binaryOperand: { o | [ o .primary o .unaryMessage star ] seq } ;
  keywordMessage: { o | [ o .keyword o .formula ] seq plus } ;
  formula: { o | [ o .binaryOperand { | o .binaryMessage () } star ] seq } ;
  nestedTerm: { o | [ '( o .expression ') ] 1 seq1 } ;
  literal: { o | [ o .literalArray o .literalSymbol  o .literalString  o .literalNumber ] alt } ;
  literalArray: { o | [ '# '( { | o .literal () } star ') ] 2 seq1 } ;
  literalNumber: { o | o .Number } ;
  literalSymbol: { o | [ '# [ o .string o .selector ] alt ] 1 seq1 tok } ;
  literalString: { o | o .STString } ;
  selector: { o | [ o .binarySelector o .keywordSelector  o .unarySelector ] alt } ;
  keywordSelector: { o | o .KeywordSequence } ;
  string: { o | o .STString } ;
  nestedBlock: { o | [ '[  { | o .blockPattern () } opt { | o .blockContents () } opt '] ] seq } ;
  blockPattern: { o | [ o .blockArguments '| ] 0 seq1 } ;
  blockArguments: { o | [ " :" o .argument ] seq &join mapp plus } ;
//  Number: { o | [ '- lit opt o .Num plus [ '. o .Num plus ] seq opt ] seq tok } ;
  Number: { o | [ '- lit opt o .Num plus &join mapp [ '. o .Num plus &join mapp ] seq &join mapp opt ] seq { | { i | i } filter join } mapp tok } ;
  Alpha: { o | [ 'a 'z range 'A 'Z range ] alt } ;
  Num: { o | '0 '9 range } ;
  AlphaNum: { o | [ o .Alpha o .Num ] alt } ;
  Identifier: { o | [ o .Alpha o .AlphaNum star &join mapp ] seq &join mapp tok } ;
  STPrimitive: { o | 'primitive lit } ;
  Separator: { o | '- lit 4 repeat tok } ;
  OperatorSequence: { o | '~&|*/\+<>,%@-= anyChar plus &join mapp } ;
  Keyword: { o | [ o .Identifier " :" ] seq &join mapp tok } ;
  KeywordSequence: { o | o .Keyword plus tok } ;
  // Javascript string escaping is messing this up
  STStringChar: { o | [ '\\b '\\n '\\r '\\f '\\0 '\\' '\\\\ '' notChars ] alt } ;
  STString: { o | [ '' o .STStringChar star '' ] 1 seq1 &join mapp } ;
  Comment: { o | [ '" '" notChars star '" ] 1 seq1 tok } ;
  Whitespace: { o | [ tab cr nl "  " ] alt plus tok } ;
  ignore: { o | [ o .Whitespace  o .Comment ] alt plus tok } ;
  | { m | m ?? }
} ::SOMParser


{ | { let SOMParser :super |
  { m | m switch
    'super      { m o | o m super () () () }
    'method { | m super { a | [
      "     '" a 0 @ 0 @ "  { :--- " a 0 @ 1 @ "  | "
      a 2 @
      "  } "
    ] join } action }
    'STPrimitive { | m super { a | "  { | }" } action } // TODO: lookup primitive implementation
    'blockContents { | m super { a | a 0 @ { | [ " { let" a 0 @ { i | " 0 :" i + } do " |" a 1 @ " } ()" ] joins } { | a 1 @ } ifelse } action }
    'blockBodyReturn { | m super { a | a "  ---<-" + } action }
    'blockBodyExpression { | m super { a | a joins } action }
    'formula { | m super { a | a 0 @ a 1 @ joins + } action }
    'keywordMessage { | m super { a | a [ a { i | i 0 @ } map join a { i | i 1 @ } map joins ] }  action }
    'binaryOperand { | m super { a | a 0 @ a 1 @ joins + } action }
    'evaluation { | m super { a | a joins } action }
    'unaryPattern { | m super { a | [ a " " ] } action }
    'keywordPattern { | m super { a | [ a { i | i 0 @ } map join  a { i | i 1 @ } map joins ] } action }
    'assignation { | m super { a | [ a 1 @  a 0 @ { i | "  " } map "  dup " joinWith a 0 @ joins ] joins } action }
    'messages { | m super { a | [
      a 0 @ { i | '. i + } map joins
      a 1 @ { i | i 1 @ " swap ." i 0 @ +  } do
      a 2 @ { i | '>s2 i 1 @  's2> '. i 0 @ +  } do
    ] joins } action }
    'fields { | m super { a | a { | a joins } { | " " } ifelse } action }
    'STString { | m super { a | [ '" "  " a '" ] join } action }
    'superclass { | m super { a | a { | a } { | 'Object } ifelse } action }
    'program { | m super { a | a nl joinWith } action }
    'assignment { | m super { a | "  :" a + } action }
    'nestedBlock { | m super { a | [ '{ a 1 @ joins '| a 2 @ '} ] joins } action }
    'classdef { | m super { a let
        name_:           a 0 @ ;
        superName_:      a 2 @ ;
        instanceFields_: a 4 @ ;
        methods_:        a 5 @ ;
        classFields_:    a 6 @ { | a 6 @ 1 @ } { | " " } ifelse ;
        classMethods_:   a 6 @ { | a 6 @ 2 @ } { | [ ] } ifelse ;
      | [
        nl " { | { "
        "   " instanceFields_ nl
        "   let " superName_ " _ :super |" nl
        "   { m | m switch" nl
        "     'super { m o | o m super () () () }" nl
        methods_ nl joinWith
        "     { | m super () () }" nl
        "   end }" nl
        " } () } ::" name_ '_ nl
        nl
        " { "
        classFields_ nl
        "   let Class :super |" nl
        "   { m | m switch" nl
        "     'new_ { | " name_ " _ }" nl
        "     'name { | '" name_ "  }" nl
        classMethods_ nl joinWith
        "     { | m super () () }" nl
        "   end }" nl
        " } :" name_ nl
      ] join
    } action }
    { | m super () () }
  end }
} () } ::SOMCompiler
`);
