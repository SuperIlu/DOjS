/*
 * NOT FINISHED AND NOT USED
 */
%{

#include <stdio.h>

%}

%union {
  char *sval;
  int   ival;
};

/* mkimp tokens
 */
%token Y_IF Y_IFDEF
%token <sval> Y_DEFINE Y_DEFINED Y_UNDEF Y_LVALUE Y_RVALUE
%token <sval> Y_STRING

%start Preprocess

%%

Preprocess  : define_stmt
            | if_stmt
            | ifdef_stmt
            | elif_stmt
            | else_stmt
            | undef_stmt
            | endif_stmt
            | include_stmt
            | pragma_stmt
            | error_stmt
            ;

define_stmt : Y_DEFINE Y_LVALUE
              {
                MacroAdd ($1, NULL);
              }
            | Y_DEFINE Y_LVALUE Y_RVALUE
              {
                MacroAdd ($1, $2);
              }
            ;

if_stmt     : Y_IF Y_STRING  { Debug ("#if: `%s'\n",$2); }
            ;

ifdef_stmt  : Y_IFDEF Y_STRING  { Debug ("#ifdef: `%s'\n",$2); }
            ;

elif_stmt   : Y_STRING  { Debug ("#elif: `%s'\n",$1); }
            ;

else_stmt   : Y_STRING  { Debug ("#else: `%s'\n",$1); }
            ;

undef_stmt  : Y_STRING  { Debug ("#undef: `%s'\n",$1); }
            ;

endif_stmt  : Y_STRING  { Debug ("#endif: `%s'\n",$1); }
            ;

include_stmt: Y_STRING  { Debug ("#include: `%s'\n",$1); }
            ;

pragma_stmt : Y_STRING  { Debug ("#pragma: `%s'\n",$1); }
            ;

error_stmt  : Y_STRING  { Debug ("#error: `%s'\n",$1); }
            ;

%%


