%skeleton "lalr1.cc" // -*- C++ -*-
//%require "3.7.6"
%defines

%define api.token.raw

%define api.token.constructor
%define api.value.type variant
//%define parse.assert

%code requires {
    #include<string>
    #include<vector>
    #include"line.hpp"
    #include"symbol_table.hpp"
    #include"arg.hpp"
    class driver;
    class compiler;
}

%param { driver& drv }

%locations

%define parse.trace
%define parse.error verbose //detailed
//%define parse.lac full

%code {
#include"driver.hpp"
#include"compiler.hpp"

compiler * le_compiler = compiler::get_compiler();
symbol_table & sym_tab = le_compiler->get_symbol_table();
line temp_line("");
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
%type <std::vector<std::string>*> list_id
%type <std::vector<arg>*> list_id_lit
%type <int> literal

%type <arg> operand
%type <arg> operand_jmp
%type <std::string> instruction

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

label:      IDENTIFIER ":" { 
                    sym_tab.add($1,
                      sym_tab.make_entry(0, le_compiler->get_section(),
                      false, false, le_compiler->get_lc())
                    ); 
            }
  ;

directive:  GLOBAL list_id {
                    for(std::string _name : *($2))
                    {
                      sym_tab.add(_name,
                        sym_tab.make_entry(0, "",
                        false, true, -2)
                      );
                    }
                    delete $2;
                    $2 = nullptr;
            }
  |         EXTERN list_id {
                    for(std::string _name : *($2))
                    {
                      sym_tab.add(_name,
                        sym_tab.make_entry(0, "",
                        false, true, -1)
                      );
                    }
                    delete $2;
                    $2 = nullptr;
            }
  |         SECTION IDENTIFIER { 
                    le_compiler->set_section($2);
                    sym_tab.add($2,
                      sym_tab.make_entry(0, le_compiler->get_section(),
                      false, false, 0)
                    );
                    temp_line.set_name(".section");
                    temp_line.add_arg($2);
                    le_compiler->add_line(temp_line);
                    temp_line.clear();
            }
  |         WORD list_id_lit {
                    temp_line.set_name(".word");
                    for(arg _arg : *($2))
                    {
                      temp_line.add_arg(_arg);
                    }
                    le_compiler->add_line(temp_line);
                    temp_line.clear();
                    delete $2;
                    $2 = nullptr;
            }
  |         SKIP literal {
                    temp_line.set_name(".skip");
                    temp_line.add_arg($2);
                    le_compiler->add_line(temp_line);
                    temp_line.clear();
            }
  |         EQU IDENTIFIER "," literal { 
                    sym_tab.add($2,
                      sym_tab.make_entry(0, "ABS",
                      true, false, $4)
                    );
            }
  ;

list_id:    IDENTIFIER { $$ = new std::vector<std::string>(); $$->push_back($1); }
  |         list_id "," IDENTIFIER { $$ = $1; $$->push_back($3); }
  ;

list_id_lit:  IDENTIFIER { $$ = new std::vector<arg>(); $$->push_back($1); }
  |           literal { $$ = new std::vector<arg>(); $$->push_back($1); }
  |           list_id_lit "," IDENTIFIER { $$ = $1; $$->push_back($3); }
  |           list_id_lit "," literal { $$ = $1; $$->push_back($3); }
  ;

literal:    NUMBER
  |         HEX
  |         MINUS literal { $$ = -$2; }
  ;

instruction:  INSTR_0 { temp_line.set_name($1); le_compiler->add_line(temp_line); }
  |           INSTR_1 REGISTER { temp_line.set_name($1); temp_line.add_arg($2);
                      le_compiler->add_line(temp_line);
                      temp_line.clear();
              }
  |           INSTR_2 REGISTER "," REGISTER {
                      temp_line.set_name($1); temp_line.add_arg($2); temp_line.add_arg($4);
                      le_compiler->add_line(temp_line);
                      temp_line.clear();
              }
  |           INSTR_JMP operand_jmp {
                      temp_line.set_name($1); temp_line.add_arg($2); temp_line.set_jmp();
                      le_compiler->add_line(temp_line);
                      temp_line.clear();
              }
  |           INSTR_LD_ST REGISTER "," operand {
                      temp_line.set_name($1); temp_line.add_arg($2); temp_line.add_arg($4);
                      le_compiler->add_line(temp_line);
                      temp_line.clear();
              }
  ;

operand:    "$" literal                     { $$ = arg::make_arg($2, 0); }
  |         "$" IDENTIFIER                  { $$ = arg::make_arg($2, 0); }
  |         literal                         { $$ = arg::make_arg($1, 4); }
  |         IDENTIFIER                      { $$ = arg::make_arg($1, 4); }
  |         "%" IDENTIFIER                  { $$ = arg::make_arg("pc", 5); $$.add_offset(-2); $$.get_offset().add_offset($2); }
  |         REGISTER                        { $$ = arg::make_arg($1, 1); }
  |         "[" REGISTER "]"                { $$ = arg::make_arg($2, 2); }
  |         "[" REGISTER PLUS literal "]"   { $$ = arg::make_arg($2, 3); $$.add_offset($4); }
  |         "[" REGISTER PLUS IDENTIFIER"]" { $$ = arg::make_arg($2, 3); $$.add_offset($4); }
  ;

operand_jmp:  literal                             { $$ = arg::make_arg($1, 0); }
  |           IDENTIFIER                          { $$ = arg::make_arg($1, 0); }
  |           "%" IDENTIFIER                      { $$ = arg::make_arg("pc", 5); $$.add_offset(-2); $$.get_offset().add_offset($2); }
  |           "*" literal                         { $$ = arg::make_arg($2, 4); }
  |           "*" IDENTIFIER                      { $$ = arg::make_arg($2, 4); }
  |           "*" REGISTER                        { $$ = arg::make_arg($2, 1); }
  |           "*" "[" REGISTER "]"                { $$ = arg::make_arg($3, 2); }
  |           "*" "[" REGISTER PLUS literal "]"   { $$ = arg::make_arg($3, 3); $$.add_offset($5); }
  |           "*" "[" REGISTER PLUS IDENTIFIER"]" { $$ = arg::make_arg($3, 3); $$.add_offset($5); }
  ;

%%

void
yy::parser::error (const location_type& l, const std::string& m)
{
  std::cerr << l << ": " << m << '\n';
}
