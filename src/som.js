
scope.eval$(`
{ a | a ! { | " " <- } if a { i | i } filter { i | "  " i + } map join } ::joins // join, space separated, removing false values

// "translated from: https://github.com/SOM-st/SOM/blob/master/specification/SOM.g4"
{ let

  call:   { m o | o m o () () } ;

  program: { o | o .classdef plus } ;

  classdef: { o | [
    o .identifier '= o .superclass '(
      o .instanceFields
      o .method star
      [ o .Separator o .classFields o .method star ] seq opt
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

  blockBody: { o | [
    { | o .blockBodyReturn () }
    [
      { | o .expression () }
      [ '. { | o .blockBody () } opt ] seq opt
    ] seq
  ] alt } ;

  blockBodyReturn: { o |  [ '^ o .result ] 1 seq1 } ;

  result: { o | [ o .expression '. lit opt ] 0 seq1 } ;

  expression: { o | [ o .assignation o .evaluation ] alt } ;

  assignation: { o | [ o .assignments o .evaluation ] seq } ;

  assignments: { o | o .assignment plus } ;

  assignment: { o | [ o .variable ':= ] 0 seq1 } ;

  evaluation: { o | [ o .primary o .messages opt ] seq } ;

  primary: { o | [ o .variable { | o .nestedTerm () } { | o .nestedBlock () } { | o .literal () } ] alt } ;

  variable: { o | o .identifier } ;

  messages: { o | [
    [ o .unaryMessage  plus o .binaryMessage star &joins mapp o .keywordMessage opt ] seq &joins mapp
    [ o .binaryMessage plus &joins mapp o .keywordMessage opt ] seq &joins mapp
    o .keywordMessage
  ] alt } ;

  unaryMessage: { o | o .unarySelector } ;

  binaryMessage: { o | [ o .binarySelector o .binaryOperand ] seq } ;

  binaryOperand: { o | [ o .primary o .unaryMessage star ] seq } ;

  keywordMessage: { o | [ o .keyword o .formula ] seq &joins mapp plus } ;

  formula: { o | [ o .binaryOperand { | o .binaryMessage () } star ] seq } ;

  nestedTerm: { o | [ '( o .expression o ') ] 1 seq1 } ;

  literal: { o | [ o .literalArray o .literalSymbol  o .literalString  o .literalNumber ] alt } ;

  literalArray: { o | [ '# '( { | o .literal () } star ') ] 2 seq1 } ;

  literalNumber: { o | o .Number } ;

  literalSymbol: { o | [ '# [ o .string o .selector ] alt ] 1 seq1 tok } ;

  literalString: { o | o .STString } ;

  selector: { o | [ o .binarySelector o .keywordSelector  o .unarySelector ] alt } ;

  keywordSelector: { o | o .KeywordSequence } ;

  string: { o | o .STString } ;

  nestedBlock: { o | [ '[ { | o .blockPattern () } opt { | o .blockContents () } opt '] ] seq } ;

  blockPattern: { o | [ o .blockArguments '| ] 0 seq1 } ;

  blockArguments: { o | [ ': o .argument ] seq1 &join mapp plus } ;

  Number: { o | [ '- lit opt o .Num plus /* [ '. o .Num plus ] seq opt */ ] seq tok } ;

//  Number: { o | [ '- lit opt o .Num plus &join mapp [ '. o .Num plus &join mapp ] seq &join mapp opt ] seq { | { i | i } filter join } mapp tok } ;

  Alpha: { o | [ 'a 'z range 'A 'Z range ] alt } ;

  Num: { o | '0 '9 range } ;

  AlphaNum: { o | [ o .Alpha o .Num ] alt } ;

  Identifier: { o | [ o .Alpha o .AlphaNum star &join mapp ] seq &join mapp tok } ;

  STPrimitive: { o | 'primitive lit } ;

  Separator: { o | '- lit 4 repeat tok } ;

  Equal:    { o | '=  lit } ;
  Or:       { o | '|  lit } ;
  Comma:    { o | ',  lit } ;
  Minus:    { o | '-  lit } ;
  Not:      { o | '~  lit } ;
  And:      { o | '&  lit } ;
  Star:     { o | '*  lit } ;
  Div:      { o | '/  lit } ;
  Mod:      { o | '\  lit } ;
  Plus:     { o | '+  lit } ;
  More:     { o | '>  lit } ;
  Less:     { o | '<  lit } ;
  At:       { o | '@  lit } ;
  Per:      { o | '%  lit } ;

  OperatorSequence: { o | '~&|*/\+<>,@.-= anyChar plus } ;

  Keyword: { o | [ o .Identifier " :" ] seq &join mapp tok } ;

  KeywordSequence: { o | o .Keyword plus tok } ;

  // Javascript string escaping is messing this up
  STStringChar: { o | [ '\\b '\\n '\\r '\\f '\\0 '\\' '\\\\ '' notChars ] alt } ;

  STString: { o | [ '' o .STStringChar star '' ] 1 seq1 &join mapp } ;

  Comment: { o | [ '" '" notChars star '" ] 1 seq1 tok } ;

  Whitespace: { o | [ tab cr nl "  " ] alt plus tok } ;

   ignore: { o | [ o .Whitespace  o .Comment ] alt plus tok } ;

  | { m | m print m ?? }
} ::SOMParser


{ | { let SOMParser :super |
  { m | m switch
    'super      { m o | o m super () () () }
    //   method: { o | [ o .pattern '= [ o .STPrimitive o .methodBlock ] alt ] seq } ;
    'method { | m super { a |
      [
        "     '" a 0 @ "  { :--- "
        a 2 @ 0 @ joins
        "  | "
        a 2 @ 1 @
        "  } "
      ] join } action }
    'blockBodyReturn { | m super { a | a "  ---<-" + } action }
    'unaryMessage { | m super { a | "  " a + } action }
    'binaryMessage { | m super { a | a 1 @ "  " a 0 @ + + } action }
    'binaryOperand { | m super { a | a 0 @ a 1 @ joins + } action }
    'evaluation { | m super { a | a joins } action }

   // 'evaluation { | m super { a | a 0 @ "  " a 1 @ joins + + } action }
    'assignation { | m super { a | a 0 @ "  " a 1 @ joins + + } action }
    'assignment { | m super { a | "  :" a + } action }
    'messages { | m super { a | a } action }
    'STString { | m super { a | [ '" "  " a '" ] join } action }
    'superclass { | m super { a | a { | a } { | 'Object } ifelse } action }
    'classdef { | m super { a |
      [
        nl
        " { | { " nl
        "   " a 4 @ nl
        "   let " a 2 @ "  :super |" nl
        "   { m | m switch" nl
        "     'super { m o | o m super () () () }" nl
        a 5 @ 0 @ nl
        "     { | m super () () }" nl
        "   end }" nl
        " } () } :" a 0 @ nl
      ] join
    } action }
    /*
    'result { | m super { a |
      a debugger
    } action }
    */
    { | m super () () }
  end }
} () } ::SOMCompiler

`);
