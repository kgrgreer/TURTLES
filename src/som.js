
scope.eval$(`
  // "translated from: https://github.com/SOM-st/SOM/blob/master/specification/SOM.g4"
  { let
  program: { o | o .classdef plus } ;

  classdef: { o | [
    o .identifier '= o .superclass
    o .instanceFields o .method star
    [ o .Separator o .classFields o .method star ] seq opt
    o .EndTerm
  ] seq } ;

  superclass: { o | [ o .Identifier opt o .NewTerm ] seq } ;

  instanceFields: { o | o .fields } ;

  classFields: { o | o .fields } ;

  fields: { o | [ o .Or o .variable star o .Or ] seq opt } ;

  method: { o | [ o .pattern o .Equal [ o .StPrimitive o .methodBlock ] alt ] seq } ;

  pattern: { o | [ o .keywordPattern o .binaryPattern o .unaryPattern ] alt } ;

  unaryPattern: { o | o .unarySelector } ;

  binaryPattern: { o | [ o .binarySelector o .argument ] seq } ;

  keywordPattern: { o | [ o .keyword o .argument ] seq plus } ;

  methodBlock: { o | [ o .NewTerm o .blockContents opt o .EndTerm ) seq } ;

  unarySelector: { o | o .identifier } ;

  binarySelector: { o | [
    o .Or  o .Comma o .Minus o .Equal o .Not o .And o .Star o .Div
    o .Mod o .Plus  o .More  o .Less  o .At  o .Per o .OperatorSequence
  ] } ;

  // checked until here

  identifier: { o | [ o .STPrimitive o .Identifier ] alt } ;

  keyword: { o |  o .Keyword } ;

  argument: { o | o .variable } ;

  blockContents: { o | [ [ o .Or o .localDefs o .Or ] seq opt ] o .blockBody ] seq } ;

  localDefs: { o | o .variable star } ;

  blockBody: { o | [
    [ o .Exit o .result ] seq
    [ [ o .expression [ o.Period o. blockBody ] seq opt ] seq opt
  ] alt } ;

  result: { o | [ o .expression o .Period ] seq opt } ;

  expression: { o | [ o .assignation o evaluation ] alt } ;

  assignation: { o | [ o .assignments o evaluation ] seq } ;

  assignments: { o | o .assignment plus } ;

  assignment: { o | o .variable o .Assign } ;

  evaluation: { o | [ o .primary o messages opt ] seq } ;

  primary: { o | [ o .variable o .nestedTerm o .literal ] alt } ;

  variable: { o | o .identifier } ;

  messages: { o | [
    [ o .unaryMessage plus o .binaryMessage star o .keywordMessage opt ] seq
    [ o .binaryMessage plus o .keywordMessaget opt ] seq
    o .keywordMessage
  ] alt } ;

  unaryMessage: { o | o .unarySelector } ;

  binaryMessage: { o | [ o .binarySelector o .binaryOperand ] seq } ;

  binaryOperand: { o | [ o .primary o .unaryMessage star ] seq } ;

  keywordMessage: { o | [ o .keyword o .formula ] seq plus } ;

  formula: { o | [ o .binaryOperand o .binaryMessage star ] seq } ;

  nestedTerm: { o | [ o .NewTerm o .expression o .EndTerm ] seq } ;

  literal: { o | [ o .literalArray o .literalSymbol o .literalString o .literalNumber ] alt } ;

  literalArray: { o | [ o .Pound o .NewTerm o .literal star o .EndTerm ] seq } ;

  literalNumber: { o | o .Number } ;

  literalSymbol: { o | [ o .Pound [ o .string o .selector ] alt ] seq } ;

  literalString: { o | o .STString } ;

  selector: { o | [ o .binarySelector o .keywordSelector o .unarySelector ] alt } ;

  keywordSelector: { o | [ o .Keyword o .KeywordSequence ] alt } ;

  string: { o | o .STString } ;

  nestedBlock: { o | [ o .NewBlock o .blockPattern opt o.blockContents opt o .EndBlock ] seq } ;

  blockPattern: { o | [ o .CblockArguments o .Or ] seq } ;

  blockArguments: { o | [ o .Colon o .argument ] seq plus } ;

  Number: { o | [ '- lit opt o .Num plus [ '. o .Num plus ] seq opt ] seq tok } ;

  Alpha: [ 'a 'z range 'A 'Z range ] alt ;

  Num: '0 '9 range ;

  AlphaNum: { o | [ o .Alpha o .Num ] Alt } ;

  Identifier: { o | [ o .Alpha o .AlphaNum star ] seq tok } ;

  STPrimitive: " primitive lit ;

  Separator: '- lit 4 repeat tok ;

  Equal:    '=  lit ;
  NewTerm:  '(  lit ;
  EndTerm:  ')  lit ;
  Or:       '|  lit ;
  Comma:    ',  lit ;
  Minus:    '-  lit ;
  Not:      '~  lit ;
  And:      '&  lit ;
  Star:     '*  lit ;
  Div:      '/  lit ;
  Mod:      '\  lit ;
  Plus:     '+  lit ;
  More:     '>  lit ;
  Less:     '<  lit ;
  At:       '@  lit ;
  Per:      '%  lit ;
  Colon:    ':  lit ;
  NewBlock: '[  lit ;
  EndBlock: ']  lit ;
  Pound:    '#  lit ;
  Exit:     '^  lit ;
  Period:   '.  lit ;
  Assign:   ':= lit ;

  OperatorSequence: { o | [
    o .Not  o .And  o .Or    o .Star o .Div o .Mod   o .Plus
    o .More o .Less o .Comma o .At   o .Per o .Minus o .Equal
  ] alt } ;

  Keyword: { o | [ o .Identifier o .Colon ] seq tok } ;

  KeywordSequence: { o | o .Keyword plus } ;

  STStringChar: [ '\b '\n '\r '\f '\0 '\' '\\ '\' notChars ] alt ;

  STString: { o | [ '' o .STStringChar star '' ] seq } ;

  Comment: [ '" '" notChars star '" ] seq ;

  Whitespace: [ tab cr nl "  " ] alt plus ;

  ignore: { o | [ o .space  o .comment ] alt plus tok } ;

  | { | ?? }
} :SOMParser
`);
