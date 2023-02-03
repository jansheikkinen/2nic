# grammar

preliminary grammar document for 2nic

```bnf

program = { include | declaration } EOF


include     = "include" "(" IDENTIFIER ")"
declaration = struct | union | function | variable

variable = "let"                      vardecl_list ";"
struct   = "struct"   IDENTIFIER? "{" vardecl_list "}"
union    = "union"    IDENTIFIER? "{" type_list    "}"
enum     = "enum"     IDENTIFIER? "{" assign_list  "}"
funcsig  = "function" IDENTIFIER? "(" [ vardecl_list | type_list ] ")" type_list
function = function_head block


block = "{" expression { ";" expression } "}"

expression = expr | stmtexpr

stmtexpr = if | while | for | block | variable | builtin | try | catch | orelse
builtin  = ("continue" | "return" | "break") [ "(" expression ")" ]
body     = "(" expression ")" expression [ "else" expression ]
if       = "if"    body
while    = "while" body
try      = "try" expression
catch    = expression "catch" expression
orelse   = expression "orelse" expression

expr    = assign
assign  = assign_list | log_or
cast    = log_or  [ "as" type ]
log_or  = log_and { "or"      log_and }
log_and = equal   { "and"     equal   }
equal   = compare { equal_op  compare }
compare = bitwise { cmp_op    bitwise }
bitwise = term    { bit_op    term    }
term    = factor  { term_op   factor  }
factor  = unary   { factor_op unary   }
unary   = unary_op unary | call
call    = primary { "(" expr_list? ")" | field | array_index }
primary = "(" expression ")" | stmtexpr | array_init
        | NUMBER | STRING | CHAR | BOOL

field       = ( "." | "->" ) IDENTIFIER
array_index = "[" expression "]"
array_init  = "[" expr_list  "]"
struct_init = "[" assign_list "]"
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
assign_arg  = call assign_op expression

vardecl_list = vardecl_arg { "," vardecl_arg }
assign_list  = assign_arg  { "," assign_arg  }
expr_list    = expression  { "," expression  }
type_list    = type        { "," type        }


type = primitive | pointer | array | compound | optional | result
result    = "!" type
optional  = "?" type
pointer   = "&" type
array     = "[" "]" type
compound  = struct | union | funcsig
primitive = "int8"   | "int16"   | "int32"   | "int64"   | "isize"
          | "uint8"  | "uint16"  | "uint32"  | "uint64"  | "usize"
          | "float8" | "float16" | "float32" | "float64" | "fsize"
          | "bool"   | "char"    | "type"    | "void"    | "noreturn"
          | "any"

```
