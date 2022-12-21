scope.eval$(`

// A Parser Stream - used as input for parsers
{ str pos value ignore let false :tail |
  { m |
    m switch
      'parse  { parser this | this parser () }
      'parseToken { parser this
        let this .ignoreOff parser () :result |
        result
          { | ignore result .ignoreOn }
          { | result }
        ifelse
      }
      'ignoreOff { this | str pos value false PStream }
      'ignoreOn { ignore this | str pos value ignore PStream }
      'pos    { this | pos }
      'head   { this |
//         [ " pos: " pos " , head-> " str pos charAt ] join print
        str pos charAt }
      'tail   { this |
        tail !
          { | str pos 1 + this .head ignore PStream ignore { | .maybeIgnore } if :tail }
        if
        tail
      }
      'maybeIgnore { this let this ignore () :ps |
        ps
          { | this .value ps .:value  }
          { | this }
        ifelse
      }
      'value  { this | value }
      ':value { v2 this | value v2 = { | this } { | str pos v2 ignore PStream } ifelse }
      'toString { this | " PStream: " pos " , '" value '' + + + + }
      { this | " PStream Unknown Method '" m + '' + print }
    end
  }
} ::PStream


{ p | { ps | p ps .parseToken } } ::tok

{ str v | { ps let 0 :i |
  { | i str len <  { | ps .head str i charAt = } && } { | ps .tail :ps  i++ } while
  str len i = { | v ps .:value } { | false } ifelse
} /* tok */ } ::litMap

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

{ parser | { ps let parser ps .parse :ret |
  ret { | ret } { | false ps .:value } ifelse
} } ::opt

{ str | { ps | str ps .head indexOf -1 = { | ps .tail } { | false } ifelse } } ::notChars

{ str | { ps | str ps .head indexOf -1 > { | ps .tail } { | false } ifelse } } ::anyChar

{ p f | { ps | p ps .parse :ps ps { | ps .value f () ps .:value } { | false } ifelse } } ::mapp
`);

// Access to current input:
// ip_ print
// input_ print
