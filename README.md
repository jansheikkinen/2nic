# 2nic

a simple systems programming language. its mostly c-like with a few extra
features, like expression-orientation and lack of NULL.

```
include "std.io";

function main(argc: isize, argv: &char) isize {
  printf("Hello, world!\n");
  0
}
```
