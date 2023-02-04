// list.c

#include <stdio.h>
#include "list.h"
#include "type.h"
#include "expression.h"

static struct LValue* parse_lvalue(struct Parser* parser) {
  struct LValue* lv = malloc(sizeof(*lv));

  EXPECT_TOKEN(parser, IDENTIFIER_LIT, EXPECTED_IDENTIFIER);
  lv->name = parser->previous.as.string;

  if(MATCH_TOKEN(parser, COLON))
    lv->type = parse_type(parser);
  else lv->type = NULL;

  return lv;
}


static struct VarDecl* parse_vardecl(struct Parser* parser) {
  struct VarDecl* var = malloc(sizeof(*var));

  var->lvalue = parse_lvalue(parser);
  var->rvalue = NULL;

  if(!MATCH_TOKEN(parser, ASSIGN))
    return var;

  if(!MATCH_TOKEN(parser, UNDEFINED))
    var->rvalue = parse_expression(parser);

  return var;
}


struct VarDeclList* parse_vardecls(struct Parser* parser) {
  struct VarDeclList* head = malloc(sizeof(*head));
  struct VarDeclList* tail = head;

  tail->current = parse_vardecl(parser);
  tail->next = NULL;

  while(MATCH_TOKEN(parser, COMMA)) {
    tail->next = malloc(sizeof(*(tail->next)));
    tail = tail->next;
    tail->next = NULL;

    tail->current = parse_vardecl(parser);
  }

  return head;
}


struct TypeList* parse_types(struct Parser* parser) {
  struct TypeList* head = malloc(sizeof(*head));
  struct TypeList* tail = head;

  tail->current = parse_type(parser);
  tail->next = NULL;

  while(MATCH_TOKEN(parser, COMMA)) {
    tail->next = malloc(sizeof(*(tail->next)));
    tail = tail->next;
    tail->next = NULL;

    tail->current = parse_type(parser);
  }

  return head;
}



// ### PRINT FUNCTIONS ### //


static void print_lvalue(const struct LValue* ast) {
  if(ast == NULL) { printf("(NULL)"); return; }

  printf("(%s ", ast->name);
  print_type(ast->type);
  printf(")");
}


static void print_vardecl(const struct VarDecl* ast) {
  if(ast == NULL) { printf("(NULL)"); return; }

  printf("(:= ");
  print_lvalue(ast->lvalue);
  printf(" ");
  print_expression(ast->rvalue);
  printf(")");
}


void print_vardecls(const struct VarDeclList* ast) {
  if(ast == NULL) { printf("(NULL)"); return; }

  print_vardecl(ast->current);
  if(ast->next) {
    printf(" ");
    print_vardecls(ast->next);
  }
}


void print_types(const struct TypeList* ast) {
  if(ast == NULL) { printf("(NULL)"); return; }

  print_type(ast->current);
  if(ast->next) {
    printf(" ");
    print_types(ast->next);
  }
}
