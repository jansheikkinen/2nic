# design

preliminary design document for the 2nic programming language

# goals
- minimalistic yet expressive
- fit for systems programming
- safer than c without holding your hand

# ideas
- types:
  - fixed-width types
  - `noreturn`, `any`
  - prefix notation
  - `mut`
- multiple return values
- tagged unions(?)
- everything is an expression
  - return from `break` like zig
- better function pointers
