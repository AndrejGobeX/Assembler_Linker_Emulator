%skeleton "lalr1.cc" // -*- C++ -*-
//%require "3.7.6"
%defines

%define api.token.raw

%define api.token.constructor
%define api.value.type variant
%define parse.assert

%code requires {
    #include<string>
    #include<sstream>
    class driver;
}

%param { driver& drv }

%locations

%define parse.trace
%define parse.error verbose //detailed
%define parse.lac full

%code {
#include"driver.hpp"
std::stringstream ss;
}

%define api.token.prefix {TOK_}
%token
  END     0
  MINUS   "-"
  PLUS    "+"
  STAR    "*"
  SLASH   "/"
  LPAREN  "["
  RPAREN  "]"
  DOLLAR  "$"
  PERCENT "%"
  COMMA   ","
  COLON   ":"
  GLOBAL
  EXTERN
  SECTION
  WORD
  SKIP
  EQU
;

%token <std::string> IDENTIFIER "identifier"
%token <std::string> REGISTER
%token <int> NUMBER "number"
%token <int> HEX "hex"
%token <std::string> INSTR_0
%token <std::string> INSTR_1
%token <std::string> INSTR_2
%token <std::string> INSTR_LD_ST
%token <std::string> INSTR_JMP
//%nterm <int> exp

//TODO: promeni u listu vektor nesto
%type <std::string> list_id
//TODO: promeni u nesto
%type <std::string> list_id_lit
//TODO: promeni u nesto
%type <std::string> literal
//TODO: global ubaci u tabelu simbol
//TODO: extern uvezi ...
//TODO: eol
//TODO: sve ovo lepo
%type <std::string> instruction
%type <std::string> operand
%type <std::string> operand_jmp

%printer { yyo << $$; } <*>;

%%
%start prog;
prog:       lines {}
  ;

lines:      %empty {}
  |         lines line {}
  ;

line:       directive
  |         instruction
  |         label line
  ;

label:      IDENTIFIER ":" { std::cout<<$1<<":\n"; }
  ;

directive:  GLOBAL list_id { std::cout<<"global "<<$2<<"\n"; }
  |         EXTERN list_id { std::cout<<"extern "<<$2<<"\n"; }
  |         SECTION IDENTIFIER { std::cout<<"section "<<$2<<"\n"; }
  |         WORD list_id_lit { std::cout<<"word "<<$2<<"\n"; }
  |         SKIP literal { std::cout<<"skip "<<$2<<"\n"; }
  |         EQU IDENTIFIER "," literal { std::cout<<$2<<" = "<<$4<<"\n"; }
  ;

list_id:    IDENTIFIER
  |         list_id "," IDENTIFIER { $$ = $1 + " " + $3; }
  ;

list_id_lit:  IDENTIFIER
  |           literal
  |           list_id_lit "," IDENTIFIER { $$ = $1 + " " + $3; }
  |           list_id_lit "," literal { $$ = $1 + " " + $3; }
  ;

literal:    NUMBER  { ss<<$1; ss>>$$; ss.clear(); }
  |         HEX { ss<<$1; ss>>$$; ss.clear(); }
  |         MINUS literal { $$ = "-" + $2; }
  ;

instruction:  INSTR_0 { std::cout<<$1<<"\n"; }
  |           INSTR_1 REGISTER { std::cout<<$1<<" "<<$2<<"\n"; }
  |           INSTR_2 REGISTER "," REGISTER { std::cout<<$1<<" "<<$2<<", "<<$4<<"\n"; }
  |           INSTR_JMP operand_jmp { std::cout<<$1<<" "<<$2<<"\n"; }
  |           INSTR_LD_ST REGISTER "," operand { std::cout<<$1<<" "<<$2<<", "<<$4<<"\n"; }
  ;

operand:    "$" literal { $$ = $2; }
  |         "$" IDENTIFIER { $$ = $2; }
  |         literal { $$ = "[" + $1 + "]"; }
  |         IDENTIFIER { $$ = "[" + $1 + "]"; }
  |         "%" IDENTIFIER { $$ = "PC" + $2; }
  |         REGISTER
  |         "[" REGISTER "]" { $$ = "[" + $2 + "]"; }
  |         "[" REGISTER PLUS literal "]" { $$ = "[" + $2 + "+" + $4 + "]"; }
  |         "[" REGISTER PLUS IDENTIFIER"]" { $$ = "[" + $2 + "+" + $4 + "]"; }
  ;

operand_jmp:  literal { $$ = $1; }
  |           IDENTIFIER { $$ = $1; }
  |           "%" IDENTIFIER { $$ = "PC" + $2; }
  |           "*" literal { $$ = "[" + $2 + "]"; }
  |           "*" IDENTIFIER { $$ = "[" + $2 + "]"; }
  |           "*" REGISTER { $$ = $2; }
  |           "*" "[" REGISTER "]" { $$ = "[" + $3 + "]"; }
  |           "*" "[" REGISTER PLUS literal "]" { $$ = "[" + $3 + "+" + $5 + "]"; }
  |           "*" "[" REGISTER PLUS IDENTIFIER"]" { $$ = "[" + $3 + "+" + $5 + "]"; }
  ;

%%

void
yy::parser::error (const location_type& l, const std::string& m)
{
  std::cerr << l << ": " << m << '\n';
}
