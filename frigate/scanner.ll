/* $Id: scanner.ll 44 2008-10-23 09:03:19Z tb $ -*- mode: c++ -*- */
/** \file scanner.ll Define the example Flex lexical scanner */


D			[0-9]
L			[a-zA-Z_]
H			[a-fA-F0-9]
E			[Ee][+-]?{D}+
FS			(f|F|l|L)
IS			(u|U|l|L)*

%{ /*** C/C++ Declarations ***/

#include <string>

#include "scanner.h"

/* import the parser's token type into a local typedef */
typedef compiler::Parser::token token;
typedef compiler::Parser::token_type token_type;

/* By default yylex returns int, we use token_type. Unfortunately yyterminate
 * by default returns 0, which is not of token_type. */
#define yyterminate() return token::END

/* This disables inclusion of unistd.h, which is not available under Visual C++
 * on Win32. The C++ scanner uses STL streams instead. */
#define YY_NO_UNISTD_H
    

    

%}

%x C_COMMENT

/*** Flex Declarations and Options ***/

/* enable c++ scanner class generation */
%option c++

/* change the name of the scanner class. results in "ExampleFlexLexer" */
%option prefix="Example"

/* the manual says "somewhat more optimized" */
%option batch

/* enable scanner to generate debug output. disable this for release
 * versions. */
%option debug

/* no support for include files is planned */
%option yywrap nounput 

/* enables the use of start condition stacks */
%option stack

%option yylineno

/* The following paragraph suffices to track locations accurately. Each time
 * yylex is invoked, the begin position is moved onto the end position. */
%{
    /* handle locations */
    int yycolumn = 1;
    
#define YY_USER_ACTION yylloc->begin.line = yylloc->end.line = yylineno; \
yylloc->begin.column = yycolumn; yylloc->end.column = yycolumn+yyleng-1; \
yycolumn += yyleng;
%}



%% /*** Regular Expressions Part ***/

 /* code to place at the beginning of yylex() */
%{

    
    // reset location
    yylloc->step();
%}

 /*** BEGIN EXAMPLE - Change the example lexer rules below ***/


"<<>"		{ return(token::ROTATE_OP); }
">>"		{ return(token::RIGHT_SHIFT_OP); }
"<<"		{ return(token::LEFT_SHIFT_OP); }
"++"		{ return(token::INC_OP); }
"--"		{ return(token::DEC_OP); }
"&&"		{ return(token::AND_OP); }
"||"		{ return(token::OR_OP); }
"<="		{ return(token::LE_OP); }
">="		{ return(token::GE_OP); }
"=="		{ return(token::EQ_OP); }
"!="        { return(token::NE_OP); }
"+{"         { return(token::PROCBEGIN); }
"}+"         { return(token::PROCEND); }
";"         { return(token::SEMICOLON); }
"{"         { return(token::LBRACE); }
"}"         { return(token::RBRACE); }
","			{ return(token::COMMA); }
":"			{ return(token::COLON); }
"="			{ return(token::EQ); }
"("			{ return(token::LPAREN); }
")"			{ return(token::RPAREN); }
"["         { return(token::LBRACKET); }
"]"         { return(token::RBRACKET); }
"."			{ return(token::DOT); }
"&"			{ return(token::BIT_AND_OP); }
"!"			{ return(token::NOT_OP); }
"~"			{ return(token::BIT_NOT_OP); }
"-"			{ return(token::MINUS_OP); }
"+"			{ return(token::PLUS_OP); }
"**"		{ return(token::EX_MULT_OP); }
"*"			{ return(token::MULT_OP); }
"//"		{ return(token::RE_DIVIDE_OP); }
"/"			{ return(token::DIVIDE_OP); }
"%%"		{ return(token::RE_MOD_OP); }
"%"			{ return(token::MOD_OP); }
"<"			{ return(token::LESS_OP); }
">"			{ return(token::GREATER_OP); }
"^"			{ return(token::BIT_XOR_OP); }
"|"			{ return(token::BIT_OR_OP); }


"struct_t"	{ return(token::STRUCT_T); }
"int_t"		{ return(token::INT_T); }
"uint_t"	{ return(token::UINT_T); }
"typedef"	{ return(token::TYPEDEF); }
"#define"	{ return(token::PDEFINE); }
"#parties"	{ return(token::PPARTIES); }
"#input"	{ return(token::PINPUT); }
"#output"	{ return(token::POUTPUT); }
"#include"	{ return(token::PINCLUDE); }
"if"		{ return(token::IF); }
"else"		{ return(token::ELSE); }
"for"		{ return(token::FOR); }
"return"    { return(token::RETURN); }
"function"    { return(token::FUNCTION); }

"##"		{ return(token::UPOUND); }
"#"			{ return(token::POUND); }


"\""[A-Za-z][A-Za-z0-9_.]*"\"" {
    yylval->stringVal = new std::string(yytext, yyleng);
    return token::STRING;
}


[0-9]+ {
    yylval->stringVal = new std::string(yytext, yyleng);
    return token::NUMBER;
}


[A-Za-z_][A-Za-z0-9_]* {
    yylval->stringVal = new std::string(yytext, yyleng);
    return token::TERM;
}

 /* gobble up white-spaces */
[ \t\r]+ {
    yylloc->step();
}

[\n]+ {
    //yylloc->step();
    //yylineno++;
    yycolumn=1;
}



"/*"            { BEGIN(C_COMMENT); }
<C_COMMENT>"*/" { BEGIN(INITIAL); }
<C_COMMENT>"\n" { }
<C_COMMENT>.    { }



 /* pass all other characters up to bison */
. {
    return static_cast<token_type>(*yytext);
}

 /*** END EXAMPLE - Change the example lexer rules above ***/

%% /*** Additional Code ***/

namespace compiler {

Scanner::Scanner(std::istream* in,
		 std::ostream* out)
    : ExampleFlexLexer(in, out)
{
}

Scanner::~Scanner()
{
}

void Scanner::set_debug(bool b)
{
    yy_flex_debug = b;
}

}

/* This implementation of ExampleFlexLexer::yylex() is required to fill the
 * vtable of the class ExampleFlexLexer. We define the scanner's main yylex
 * function via YY_DECL to reside in the Scanner class instead. */

#ifdef yylex
#undef yylex
#endif

int ExampleFlexLexer::yylex()
{
    std::cerr << "in ExampleFlexLexer::yylex() !" << std::endl;
    return 0;
}

/* When the scanner receives an end-of-file indication from YY_INPUT, it then
 * checks the yywrap() function. If yywrap() returns false (zero), then it is
 * assumed that the function has gone ahead and set up `yyin' to point to
 * another input file, and scanning continues. If it returns true (non-zero),
 * then the scanner terminates, returning 0 to its caller. */

int ExampleFlexLexer::yywrap()
{
    return 1;
}
