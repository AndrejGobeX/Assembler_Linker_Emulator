%{
#include<cerrno>
#include<climits>
#include<cstdlib>
#include<cstring> // strerror
#include<string>
#include"driver.hpp"
#include"parser.hpp"
%}

%option noyywrap nounput noinput batch debug

%{
    yy::parser::symbol_type
    make_NUMBER (const std::string &s, const yy::parser::location_type& loc);
    yy::parser::symbol_type
    make_HEX (const std::string &s, const yy::parser::location_type& loc);
%}

id    [a-zA-Z][a-zA-Z_0-9]*
int   [0-9]+
blank [ \t\r]
instr_0 (halt|iret|ret)
instr_jmp (call|jmp|jeq|jne|jgt)
instr_1 (int|push|pop|not)
instr_2 (xchg|add|sub|mul|div|cmp|and|or|xor|test|shl|shr)
instr_ld_st (ldr|str)
reg ("r"[0-7]|"pc"|"sp"|"psw")

%{
    #define YY_USER_ACTION  loc.columns(yyleng);
%}

%%

%{
    yy::location& loc = drv.location;
    loc.step ();
%}

{blank}+   loc.step ();
\n+        loc.lines (yyleng); loc.step ();
"#".*\n    loc.lines (1); loc.step ();

"-"        return yy::parser::make_MINUS  (loc);
"+"        return yy::parser::make_PLUS   (loc);
"*"        return yy::parser::make_STAR   (loc);
"/"        return yy::parser::make_SLASH  (loc);
"["        return yy::parser::make_LPAREN (loc);
"]"        return yy::parser::make_RPAREN (loc);
"%"        return yy::parser::make_PERCENT (loc);
","        return yy::parser::make_COMMA  (loc);
":"        return yy::parser::make_COLON  (loc);
"$"        return yy::parser::make_DOLLAR  (loc);

".global"  return yy::parser::make_GLOBAL (loc);
".extern"  return yy::parser::make_EXTERN (loc);
".section"  return yy::parser::make_SECTION (loc);
".word"  return yy::parser::make_WORD (loc);
".skip"  return yy::parser::make_SKIP (loc);
".equ"  return yy::parser::make_EQU (loc);
".end"  return yy::parser::make_END (loc);

{reg}   return yy::parser::make_REGISTER(yytext, loc);
{instr_0}  return yy::parser::make_INSTR_0 (yytext, loc);
{instr_1}  return yy::parser::make_INSTR_1 (yytext, loc);
{instr_2}  return yy::parser::make_INSTR_2 (yytext, loc);
{instr_ld_st} return yy::parser::make_INSTR_LD_ST (yytext, loc);
{instr_jmp}  return yy::parser::make_INSTR_JMP (yytext, loc);

{int}      return make_NUMBER (yytext, loc);
"0x"[0-9A-Fa-f]+    return make_HEX(yytext, loc);
{id}       return yy::parser::make_IDENTIFIER (yytext, loc);

.          {
                throw yy::parser::syntax_error
                (loc, "invalid character: " + std::string(yytext));
}

<<EOF>>    return yy::parser::make_END (loc);
%%

yy::parser::symbol_type
make_NUMBER (const std::string &s, const yy::parser::location_type& loc)
{
    errno = 0;
    long n = strtol (s.c_str(), NULL, 10);
    if (! (INT_MIN <= n && n <= INT_MAX && errno != ERANGE))
        throw yy::parser::syntax_error (loc, "integer is out of range: " + s);
    return yy::parser::make_NUMBER ((int) n, loc);
}

yy::parser::symbol_type
make_HEX (const std::string &s, const yy::parser::location_type& loc)
{
    errno = 0;
    long n = strtol (s.c_str(), NULL, 16);
    if (! (INT_MIN <= n && n <= INT_MAX && errno != ERANGE))
        throw yy::parser::syntax_error (loc, "integer is out of range: " + s);
    return yy::parser::make_HEX ((int) n, loc);
}

void driver::scan_begin()
{
    yy_flex_debug = trace_scanning;
    if (file.empty () || file == "-")
        yyin = stdin;
    else if (!(yyin = fopen (file.c_str (), "r")))
    {
        std::cerr << "cannot open " << file << ": " << strerror (errno) << '\n';
        exit (EXIT_FAILURE);
    }
}

void driver::scan_end()
{
    fclose (yyin);
}
