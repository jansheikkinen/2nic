# grammar

preliminary grammar document for 2nic

```bnf

program = { include | declaration } EOF


include     = "include" STRING
declaration = struct | union | function | variable

variable = "let"                        vardecl_list ";"
struct   = "struct"   IDENTIFIER? [ "(" (vardecl_list | "void") ")" ]
union    = "union"    IDENTIFIER? [ "(" (type_list    | "void") ")" ]
funcsig  = "function" IDENTIFIER?
         "(" [ vardecl_list | "void" ] ")" ( type_list | "void" )
function = function_head block


block = "{" statement* expression? "}"
statement = expression? ";" | block | variable

expression = expr | stmtexpr

stmtexpr = if | while | for | block | builtin | try | catch | orelse
builtin  = ("continue" | "return" | "break") expression?
else     = "else" expression
body     = expression [ else ]
if       = "if"    "(" expression  ")" body
while    = "while" "(" expression  ")" body
for      = "for"   "(" variable? expression? ";" expression? ")" body

expr      = unary_low
unary_low = un_low_op unary_low | fallback
fallback  = assign      { fb_op     assign  }
assign    = cast        [ assign_op cast    ]
cast      = log_or      [ "as"      type    ]
log_or    = log_and     { "or"      log_and }
log_and   = equal       { "and"     equal   }
equal     = compare     { equal_op  compare }
compare   = bitwise     { cmp_op    bitwise }
bitwise   = term        { bit_op    term    }
term      = factor      { term_op   factor  }
factor    = unary       { factor_op unary   }
unary     = unary_op unary | call
call      = primary { "(" expr_list? ")" | field | array_index }
primary   = "(" expression ")" | stmtexpr | array_init
          | NUMBER | STRING | CHAR | BOOL

field       = ( "." | "->" ) IDENTIFIER
array_index = "[" expression "]"
array_init  = "[" expr_list  "]"
un_low_op   = "try"   | "return" | "continue" | "break"
fb_op       = "catch" | "orelse"
assign_op   =  "=" | "+=" | "-=" | "*=" | "+%=" | "-%=" | "*%=" | "/=" | "%="
            | "&=" | "|=" | "^=" | "<<=" | ">>="
equal_op    = "==" | "!="
cmp_op      = "<=" | ">=" |  "<" | ">"
bit_op      =  "&" |  "|" |  "^" | "<<" | ">>"
term_op     =  "-" | "-%" |  "+" | "+%"
factor_op   =  "*" | "*%" |  "/" | "%"
unary_op    =  "!" |  "-" |  "*" | "&" | "not"


lvalue      = IDENTIFIER [ ":" type ]
vardecl_arg = lvalue [ "=" ( expression | "undefined" ) ]

vardecl_list = vardecl_arg { "," vardecl_arg }
expr_list    = expression  { "," expression  }
type_list    = type        { "," type        }


type = "mut"? ( primitive | pointer | array | compound | optional | result )
result    = "!" type
optional  = "?" type
pointer   = "&" type
array     = "[" expression? "]" type
compound  = struct | union | funcsig
primitive = "int8"   | "int16"   | "int32"   | "int64"   | "isize"
          | "uint8"  | "uint16"  | "uint32"  | "uint64"  | "usize"
          | "float8" | "float16" | "float32" | "float64" | "fsize"
          | "bool"   | "char"    | "type"    | "void"    | "noreturn"
          | "any"

```
