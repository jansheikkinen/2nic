#pragma once

struct Parser;

// primitive | pointer | array | compound_type | optional
struct Type { };

struct Type* parse_type(struct Parser*);
