scope.eval$(`

// A Parser Stream - used as input for parsers
{ str pos value |
  { m |
    m switch
      'parse  { parser this | this parser () }
      'pos    { this | pos }
      'head   { this | str pos charAt }
      'tail   { this | str pos 1 + this .head PStream }
      'value  { this | value }
      ':value { value this | str pos value PStream }
      'toString { this | " PStream: " pos " , '" value '' + + + + }
      { this | " PStream Unknown Method '" m + '' + print }
    end
  }
} ::PStream


{ delegate |
  { m |
    m switch
      'parse  { parser this | this parser () }
      'pos    { this | delegate .pos }
      'head   { this |
        [ " pos: " delegate .pos " , head-> " delegate .head ] join print
        delegate .head
      }
      'tail   { this | delegate .tail TracingPStream }
      'value  { this | delegate .value }
      ':value { value this | value delegate .:value TracingPStream }
      'toString { this | " TracingPStream " delegate .toString + }
      { this | " TracingPStream Unknown Method '" m + '' + print } // TODO: make generic
    end
  }
} ::TracingPStream


{ delegate ignore |
  { m |
    m switch
      'parse  { parser this | this parser () }
      'pos    { this | delegate .pos }
      'head   { this | delegate .head }
      'tail   { this | delegate .tail ignore IgnorePStream }
      'value  { this | delegate .value }
      ':value { value this | value delegate .:value ignore IgnorePStream }
      'toString { this | " IgnorePStream " delegate .toString + }
      { this | " IgnorePStream Unknown Method '" m + '' + print }
    end
  }
} ::IgnorePStream


{ str v | { ps let 0 :i |
  { | ps .head str i charAt = } { | ps .tail :ps  i++ } while
  str len i = { | v ps .:value } { | false } ifelse
} } ::litMap

{ str | str str litMap } ::lit

{ start end c | c start >=  c end <= & } ::inRange
{ start end | { ps |
  start end ps .head inRange { | ps .tail } { | false } ifelse
} } ::range

{ parsers | parsers { p | p string? { | p lit } { | p } ifelse } map } ::prepare

{ parsers | parsers prepare :parsers { ps let 0 :i |
  [ { | i parsers len < { | parsers i @ ps .parse :ps ps } && } { | i++ ps .value } while ]
  parsers len i = { a | a ps .:value } { _ | false } ifelse
} } ::seq

{ parsers i | parsers seq { a | a i @ } mapp } ::seq1

{ parsers | parsers prepare :parsers { ps let 0 :i false :ret |
  { | i parsers len < { | parsers i @ ps .parse :ret ret ! } && } { | i++ } while
  ret
} } ::alt

{ parser min | { ps let 0 :i false :ret |
  [ { | ps :ret  parser ps .parse :ps ps } { | i++ ps .value } while ]
  i min >=  { a | a ret .:value } { _ | false } ifelse
} } ::repeat

{ parser delim |
  [ [ parser delim ] 0 seq1 0 repeat parser opt ] seq
  { a | [ a 0 @  { e | e } forEach a 1 @ ] } mapp
} ::delim

{ parser | { ps let ps :ret |
  parser ps .parse :ret
  ret { | ret } { | false ps .:value } ifelse
} } ::opt

{ str | { ps | str ps .head indexOf -1 = { | ps .tail } { | false } ifelse } } ::notChars

{ str | { ps | str ps .head indexOf -1 > { | ps .tail } { | false } ifelse } } ::anyChar

{ p f | { ps | p ps .parse :ps ps { | ps .value f () ps .:value } { | false } ifelse } } ::mapp

`);

// Access to current input:
// ip_ print
// input_ print
