
scope.eval$(`
  // "translated from: https://github.com/SOM-st/SOM/blob/master/specification/SOM.g4"
  { let
    { o | o .classdef plus }
    :program

    { o | [
      o .identifier '= o .superclass
      o .instanceFields o .method star
      [ o .Separator o .classFields o .method star ] seq opt
      o .EndTerm
    ] seq }
    :classdef

    { o | [ o .Identifier opt o .NewTerm ] seq }
    :superclass

    { o | o .fields }
    :instanceFields

    { o | o .fields }
    :classFields

    { o | [ o .Or o .variable star o .Or ] seq opt }
    :fields

    { o | [ o .pattern o .Equal [ o .StPrimitive o .methodBlock ] alt ] seq }
    :method

    { o | [ o .keywordPattern o .binaryPattern o .unaryPattern ] alt }
    :pattern

    { o | o .unarySelector }
    :unaryPattern

    { o | [ o .binarySelector o .argument ] seq }
    :binaryPattern

    { o | [ o .keyword o .argument ] seq plus }
    :keywordPattern

    { o | [ o .NewTerm o .blockContents opt o .EndTerm ) seq }
    :methodBlock

    { o | o .identifier }
    :unarySelector

    { o | [
      o .Or   o .Comma o .Minus o .Equal o .Not o .And o .Star o .Div o .Mod
      o .Plus o .More  o .Less  o .At    o .Per o .OperatorSequence
    ] }
    :binarySelector

    // checked until here

    { o | [ o .STPrimitive o .Identifier ] alt }
    :identifier

    { o |  o .Keyword }
    :keyword

    { o | o .variable }
    :argument

    { o | [ [ o .Or o .localDefs o .Or ] seq opt ] o .blockBody ] seq }
    :blockContents

    { o | o .variable star }
    :localDefs

    { o | [
      [ o .Exit o .result ] seq
      [ [ o .expression [ o.Period o. blockBody ] seq opt ] seq opt
    ] alt }
    :blockBody

    { o | [ o .expression o .Period ] seq opt }
    :result

    { o | [ o .assignation o evaluation ] alt }
    :expression

    { o | [ o .assignments o evaluation ] seq }
    :assignation

    { o | o .assignment plus }
    :assignments

    { o | o .variable o .Assign }
    :assignment

    { o | [ o .primary o messages opt ] seq }
    :evaluation

    { o | [
      o .variable
      o .nestedTerm
      o .literal
    ] alt }
    :primary

    { o | o .identifier }
    :variable

    { o | [
      [ o .unaryMessage plus o .binaryMessage star o .keywordMessage opt ] seq
      [ o .binaryMessage plus o .keywordMessaget opt ] seq
      o .keywordMessage
    ] alt }
    :messages

    { o | o .unarySelector }
    :unaryMessage

    { o | [ o .binarySelector o .binaryOperand ] seq }
    :binaryMessage

    { o | [ o .primary o .unaryMessage star ] seq }
    :binaryOperand

    { o | [ o .keyword o .formula ] seq plus }
    :keywordMessage

    { o | [ o .binaryOperand o .binaryMessage star ] seq }
    :formula

    { o | [ o .NewTerm o .expression o .EndTerm ] seq }
    :nestedTerm

    { o | [ o .literalArray o .literalSymbol o .literalString o .literalNumber ] alt }
    :literal

    { o | [ o .Pound o .NewTerm o .literal star o .EndTerm ] seq }
    :literalArray

    { o | o .Number }
    :literalNumber

    { o | [ o .Pound [ o .string o .selector ] alt ] seq }
    :literalSymbol

    { o | o .STString }
    :literalString

    { o | [ o .binarySelector o .keywordSelector o .unarySelector ] alt }
    :selector

    { o | [ o .Keyword o .KeywordSequence ] alt }
    :keywordSelector

    { o | o .STString }
    :string

    { o | [ o .NewBlock o .blockPattern opt o.blockContents opt o .EndBlock ] seq }
    :nestedBlock

    { o | [ o .CblockArguments o .Or ] seq }
    :blockPattern

    { o | [ o .Colon o .argument ] seq plus }
    :blockArguments

    { o | [ '- lit opt o .Num plus [ '. o .Num plus ] seq opt ] seq tok }
    :Number

    { o | [ 'a 'z range 'A 'Z range ] alt }
    :Alpha

    { o | '0 '9 range }
    :Num

    { o | [ o .Alpha o .Num ] Alt }
    :AlphaNum

    { o | [ o .Alpha o .AlphaNum star ] seq tok }
    :Identifier

    { o | " primitive lit }
    :STPrimitive

    { o | '- lit 4 repeat tok }
    :Separator

    '= lit :Equal
    '( lit :NewTerm
    ') lit :EndTerm
    '| lit :Or
    ', lit :Comma
    '- lit :Minus
    '~ lit :Not
    '& lit :And
    '* lit :Star
    '/ lit :Div
    '\ lit :Mod
    '+ lit :Plus
    '> lit :More
    '< lit :Less
    '@ lit :At
    '% lit :Per
    ': lit :Colon
    '[ lit :NewBlock
    '] lit :EndBlock
    '# lit :Pound
    '^ lit :Exit
    '. lit :Period
    " :=" lit :Assign

    { o | [
      o .Not  o .And  o .Or    o .Star o .Div o .Mod   o .Plus
      o .More o .Less o .Comma o .At   o .Per o .Minus o .Equal
    ] alt }
    :OperatorSequence

    { o | [ o .Identifier o .Colon ] seq tok }
    :Keyword

    { o | o .Keyword plus }
    :KeywordSequence

    { o | [
      '\b
      '\n
      '\r
      '\f
      '\0
      '\'
      '\\
      '\' notChars
    ] alt }
    :STStringChar

    { o | [ '' o .STStringChar star '' ] seq }
    :STString

    { o | [ '" '" notChars star '" ] seq }
    :Comment

    { o | [ tab cr nl "  " ] alt plus }
    :Whitespace

    { o | [ o .space  o .comment ] alt plus tok }
    :ignore

  | { | ?? }
} :SOMParser
`);
