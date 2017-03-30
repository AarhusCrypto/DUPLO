



%{ /*** C/C++ Declarations ***/

#include <stdio.h>
#include <string>
#include <vector>

#include "ast.h"

%}

/*** yacc/bison Declarations ***/

/* Require bison 2.3 or later */
%require "2.3"

%expect 1

/* add debug output code to generated parser. disable this for release
 * versions. */
%debug

/* start symbol is named "start" */
%start start

/* write out a header file containing the token defines */
%defines

/* use newer C++ skeleton file */
%skeleton "lalr1.cc"

/* namespace to enclose parser in */
%name-prefix "compiler"

/* set the parser's class identifier */
%define "parser_class_name" "Parser"

/* keep track of the current position within the input */
%locations
%initial-action
{
    // initialize the initial location object
    @$.begin.filename = @$.end.filename = &driver.streamname;
};

/* The driver is passed by reference to the parser and to the scanner. This
 * provides a simple but effective pure interface, not relying on global
 * variables. */
%parse-param { class Driver& driver }

/* verbose error messages */
%error-verbose

 /*** BEGIN EXAMPLE - Change the example grammar's tokens below ***/

%union {
    int  			integerVal;
    double 			doubleVal;
    std::string*		stringVal;
    class Node*		node;
}

%token			END	     0	"end of file"
%token			EOL		"end of line"
%token <integerVal> 	INTEGER		"integer"
%token <doubleVal> 	DOUBLE		"double"
%token <stringVal> 	STRING		"STRING"




%token TERM "term"
%token NUMBER "number"

//%token INC_OP DEC_OP LEFT_SHIFT_OP RIGHT_SHIFT_OP LE_OP GE_OP EQ_OP NE_OP AND_OP OR_OP ROTATE_OP
//%token LPAREN RPAREN LBRACKET RBRACKET DOT COLON SEMICOLON COMMA NOT_OP MOD_OP PLUS_OP MINUS_OP MULT_OP DIVIDE_OP LBRACE RBRACE LESS_OP GREATER_OP BIT_OR_OP BIT_AND_OP BIT_XOR_OP EQ BIT_NOT_OP
//%token STRUCT_T INT_T TYPEDEF PDEFINE PINPUT PINCLUDE PPARTIES POUTPUT IF FOR ELSE RETURN FUNCTION


%token PROCBEGIN "+{"
%token PROCEND "}+"

%token LPAREN "("
%token RPAREN ")"
%token LBRACKET "["
%token RBRACKET "]"
%token DOT "."
%token COLON ":"
%token SEMICOLON ";"
%token COMMA ","
%token NOT_OP "!"
%token MOD_OP "%"
%token RE_MOD_OP "%%"
%token PLUS_OP "+"
%token MINUS_OP "-"
%token MULT_OP "*"
%token EX_MULT_OP "**"
%token DIVIDE_OP "/"
%token RE_DIVIDE_OP "//"
%token LBRACE "{"
%token RBRACE "}"
%token LESS_OP "<"
%token GREATER_OP ">"
%token BIT_OR_OP "|"
%token BIT_AND_OP "&"
%token BIT_XOR_OP "^"
%token EQ "="
%token BIT_NOT_OP "~"

%token INC_OP "++"
%token DEC_OP "--"
%token LEFT_SHIFT_OP "<<"
%token RIGHT_SHIFT_OP ">>"
%token LE_OP "<="
%token GE_OP ">="
%token EQ_OP "=="
%token NE_OP "!="
%token AND_OP "&&"
%token OR_OP "||"
%token ROTATE_OP "<<>"


%token STRUCT_T "struct_t"
%token INT_T "int_t"
%token UINT_T "uint_t"
%token TYPEDEF "typedef"
%token PDEFINE "#define"
%token PINPUT "#input"
%token PINCLUDE "#include"
%token PPARTIES "#parties"
%token POUTPUT "#output"
%token IF "if"
%token FOR "for"
%token ELSE "else"
%token RETURN "return"
%token FUNCTION "function"

%token POUND "#"
%token UPOUND "#u"

//%type <node> logical_or_expression logical_and_expression
%type <node> program_node  function_def preamble_rule expression struct_vardec struct_vardeclist struct_prim_var_dec struct_high_var_dec postdec defarg defarglist  compound_statement statement statement_list expression_statement primary_expression assignment_expression     inclusive_or_expression exclusive_or_expression and_expression equality_expression relational_expression shift_expression additive_expression multiplicative_expression cast_expression unary_expression argument_expression_list wire_expression postfix_expression for_statement var_dec_for_statement array_dec array_dec_list array_init array_init_list array_init_operand return_statement term_rule term_num_rule function_compound_statement start_function_statement_list end_function_statement_list term_expression
%type <node> var_dec_prim var_dec_right_side var_dec_statement if_statement return_type
%type <stringVal> TERM NUMBER


 /*** END EXAMPLE - Change the example grammar's tokens above ***/

%{

#include "parse_driver.h"
#include "scanner.h"

/* this "connects" the bison parser in the driver to the flex scanner class
 * object. it defines the yylex() function call to pull the next token from the
 * current lexer object of the driver context. */
#undef yylex
#define yylex driver.lexer->lex

#define GETLOC AstLocation::ConvertLocation(yyloc.begin.line,yyloc.end.line,yyloc.begin.column,yyloc.end.column);

%}

%% /*** Grammar Rules ***/

 /*** BEGIN EXAMPLE - Change the example grammar rules below ***/


primary_expression
: term_rule {$$ = $1;}
| term_num_rule {$$ = $1;}
| LPAREN expression RPAREN {$$ = new ParenExprNode($2); $$->nodeLocation = GETLOC;}
;

term_expression
: term_rule {$$ = $1;}
| term_num_rule {$$ = $1;}
;


postdec: array_dec_list {$$ = $1; $$->nodeLocation = GETLOC;}
;


struct_prim_var_dec
: term_rule postdec {$$ = new DeclarationNode($1,$2); $$->nodeLocation = GETLOC;}
| term_rule {$$ = new DeclarationNode($1); $$->nodeLocation = GETLOC;}
;

struct_high_var_dec 
: struct_prim_var_dec {$$ = new VarDeclarationCommaNode(); ((VarDeclarationCommaNode*)$$)->addNode($1); $$->nodeLocation = GETLOC;}
| struct_high_var_dec COMMA struct_prim_var_dec {$$ = $1; ((VarDeclarationCommaNode*)$1)->addNode($3);}
;

struct_vardec
: term_rule struct_high_var_dec SEMICOLON {$$ = new VarDeclarationNode($1, $2); $$->nodeLocation = GETLOC;}
;



struct_vardeclist
: struct_vardec {$$ = new VarDeclarationListNode(); ((VarDeclarationListNode*)$$)->addNode($1); }
| struct_vardeclist struct_vardec {$$ = $1; ((VarDeclarationListNode*)$1)->addNode($2);}
;



preamble_rule: 
PDEFINE term_rule expression {$$ = new DefineNode($2,$3); $$->nodeLocation = GETLOC;}
| PINCLUDE STRING {string s = *$2; s = s.substr(1,s.length()-2);$$ = new IncludeNode(new TermNode(s)); $$->nodeLocation = GETLOC;}
| POUTPUT term_num_rule term_rule {$$ = new OutputNode($2, $3); $$->nodeLocation = GETLOC;}
| PINPUT term_num_rule term_rule {$$ = new InputNode($2, $3); $$->nodeLocation = GETLOC;}
| PPARTIES term_num_rule {$$ = new PartiesNode($2); $$->nodeLocation = GETLOC;}
| TYPEDEF STRUCT_T term_rule LBRACE struct_vardeclist RBRACE {$$ = new StructTypedefNode($3,$5); $$->nodeLocation = GETLOC;}
| TYPEDEF INT_T expression term_rule {$$ = new IntTypedefNode($4,$3); $$->nodeLocation = GETLOC;}
| TYPEDEF UINT_T expression term_rule {$$ = new UIntTypedefNode($4,$3); $$->nodeLocation = GETLOC;}
;


defarg
: term_rule term_rule array_dec_list {$$ = new FunctionVarDeclarationNode($1, $2, $3); $$->nodeLocation = GETLOC;}
| term_rule term_rule {$$ = new FunctionVarDeclarationNode($1, $2); $$->nodeLocation = GETLOC;}
//| term_rule BIT_AND_OP term_rule array_dec_list {$$ = new FunctionVarDeclarationNode($1, $3, $4); ((FunctionVarDeclarationNode*)$$)->passByReference = true; $$->nodeLocation = GETLOC;}
//| term_rule BIT_AND_OP term_rule {$$ = new FunctionVarDeclarationNode($1, $3); ((FunctionVarDeclarationNode*)$$)->passByReference = true; $$->nodeLocation = GETLOC;}
;

defarglist
:defarglist COMMA defarg {$$ = $1; ((DeclarationArgCommaListNode*)$1)->addNode($3);}
| defarg {$$ = new DeclarationArgCommaListNode(); ((DeclarationArgCommaListNode*)$$)->addNode($1);}
;

assignment_expression
: inclusive_or_expression {$$ = $1; }
| unary_expression EQ assignment_expression {$$ = new AssignNode($1,$3); $$->nodeLocation = GETLOC;}
;

/*
logical_or_expression
: logical_and_expression {$$ = $1; }
| logical_or_expression OR_OP logical_and_expression {$$ = new ConditionalORNode($1,$3); $$->nodeLocation = GETLOC;}
;

logical_and_expression
: inclusive_or_expression {$$ = $1;}
| logical_and_expression AND_OP inclusive_or_expression {$$ = new ConditionalANDNode($1,$3); $$->nodeLocation = GETLOC;}
;
*/

inclusive_or_expression
: exclusive_or_expression {$$ = $1;}
| inclusive_or_expression BIT_OR_OP exclusive_or_expression {$$ = new BitwiseORNode($1, $3); $$->nodeLocation = GETLOC;}
;

exclusive_or_expression
: and_expression {$$ = $1;}
| exclusive_or_expression BIT_XOR_OP and_expression {$$ = new BitwiseXORNode($1, $3); $$->nodeLocation = GETLOC;}
;

and_expression
: equality_expression {$$ = $1;}
| and_expression BIT_AND_OP equality_expression {$$ = new BitwiseANDNode($1, $3); $$->nodeLocation = GETLOC;}
;

equality_expression
: relational_expression {$$ = $1;}
| equality_expression EQ_OP relational_expression {$$ = new ConditionalEqualNode($1, $3); $$->nodeLocation = GETLOC;}
| equality_expression NE_OP relational_expression {$$ = new ConditionalNotEqualNode($1, $3); $$->nodeLocation = GETLOC;}
;


relational_expression
: shift_expression {$$ = $1;}
| relational_expression LESS_OP shift_expression {$$ = new ConditionalLessNode($1, $3); $$->nodeLocation = GETLOC;}
| relational_expression GREATER_OP shift_expression {$$ = new ConditionalGreaterNode($1, $3); $$->nodeLocation = GETLOC;}
| relational_expression LE_OP shift_expression {$$ = new ConditionalLessEqualNode($1, $3); $$->nodeLocation = GETLOC;}
| relational_expression GE_OP shift_expression {$$ = new ConditionalGreaterEqualNode($1, $3); $$->nodeLocation = GETLOC;}
;

shift_expression
: additive_expression {$$ = $1;}
| shift_expression LEFT_SHIFT_OP additive_expression {$$ = new ShiftLeftNode($1, $3); $$->nodeLocation = GETLOC;}
| shift_expression RIGHT_SHIFT_OP additive_expression {$$ = new ShiftRightNode($1, $3); $$->nodeLocation = GETLOC;}
| shift_expression ROTATE_OP additive_expression {$$ = new RotateLeftNode($1, $3); $$->nodeLocation = GETLOC;}
;


additive_expression
: multiplicative_expression {$$ = $1;}
| additive_expression PLUS_OP multiplicative_expression {$$ = new ArithPlusNode($1, $3); $$->nodeLocation = GETLOC;}
| additive_expression MINUS_OP multiplicative_expression {$$ = new ArithMinusNode($1, $3); $$->nodeLocation = GETLOC;}
;


multiplicative_expression
: cast_expression {$$ = $1;}
| multiplicative_expression MULT_OP cast_expression {$$ = new ArithMultNode($1, $3); $$->nodeLocation = GETLOC;}
| multiplicative_expression DIVIDE_OP cast_expression {$$ = new ArithDivNode($1, $3); $$->nodeLocation = GETLOC;}
| multiplicative_expression MOD_OP cast_expression {$$ = new ArithModuloNode($1, $3); $$->nodeLocation = GETLOC;}
| multiplicative_expression EX_MULT_OP cast_expression {$$ = new ArithExMultNode($1, $3); $$->nodeLocation = GETLOC;}
| multiplicative_expression RE_DIVIDE_OP cast_expression {$$ = new ArithReDivNode($1, $3); $$->nodeLocation = GETLOC;}
| multiplicative_expression RE_MOD_OP cast_expression {$$ = new ArithReModuloNode($1, $3); $$->nodeLocation = GETLOC;}
;


cast_expression
: unary_expression {$$ = $1;}
;


unary_expression
: wire_expression {$$ = $1;}
| MINUS_OP unary_expression {$$ = new UnaryMinusNode($2); $$->nodeLocation = GETLOC;}
| NOT_OP unary_expression {$$ = new UnaryNOTNode($2); $$->nodeLocation = GETLOC;}
| BIT_NOT_OP unary_expression {$$ = new UnaryNOTNode($2); $$->nodeLocation = GETLOC;}
| INC_OP unary_expression {$$ = new UnaryPrePlusPlusNode($2); $$->nodeLocation = GETLOC;}
| DEC_OP unary_expression {$$ = new UnaryPreMinusMinusNode($2); $$->nodeLocation = GETLOC;}
;

argument_expression_list
: assignment_expression {$$ = new FunctionArgListNode(); ((FunctionArgListNode*)$$)->addNode($1);}
| argument_expression_list COMMA assignment_expression {$$ = $1; ((FunctionArgListNode*)$1)->addNode($3);}
;

wire_expression
: postfix_expression {$$ = $1;}
| postfix_expression LBRACE expression RBRACE {$$ = new WireAccessNode($1, $3); $$->nodeLocation = GETLOC;}
| postfix_expression LBRACE expression COLON expression RBRACE {$$ = new WireAccessNode($1, $3, $5); $$->nodeLocation = GETLOC;}
;

postfix_expression
: primary_expression {$$ = $1;}
| postfix_expression LBRACKET expression RBRACKET {$$ = new ArrayAccessNode($1, $3); $$->nodeLocation = GETLOC;}
| postfix_expression LPAREN RPAREN {$$ = new FunctionCallNode($1,NULL); $$->nodeLocation = GETLOC;}
| postfix_expression LPAREN argument_expression_list RPAREN {$$ = new FunctionCallNode($1,$3); $$->nodeLocation = GETLOC;}
| postfix_expression DOT term_rule {$$ = new DotOperatorNode($1,$3); $$->nodeLocation = GETLOC;}
| postfix_expression INC_OP {$$ = new UnaryPostPlusPlusNode($1); $$->nodeLocation = GETLOC;}
| postfix_expression DEC_OP {$$ = new UnaryPostMinusMinusNode($1); $$->nodeLocation = GETLOC;}
;






expression
: assignment_expression {$$ = $1;}
;

expression_statement
: expression SEMICOLON {$$ = new ExpressionStatementNode($1); $$->nodeLocation = GETLOC;}
| SEMICOLON {$$ = new ExpressionStatementNode(0);}
;

array_init_operand
: expression {$$ = $1; }
| array_init {$$ = $1; }
;

array_init_list
: array_init_operand {$$ = new ArrayInitListNode(); ((ArrayInitListNode*)$$)->addNode($1); }
| array_init_list COMMA array_init_operand {$$ = $1; ((ArrayInitListNode*)$1)->addNode($3);}
;

array_init
: LBRACE array_init_list RBRACE {$$ = new ArrayInitNode($2); $$->nodeLocation = GETLOC;}
;


array_dec
: LBRACKET expression RBRACKET {$$ = new ArrayDecNode($2); $$->nodeLocation = GETLOC;}
;

array_dec_list
: array_dec {$$ = new ArrayDeclarationListNode(); ((ArrayDeclarationListNode*)$$)->addNode($1);}
| array_dec_list array_dec {$$ = $1; ((ArrayDeclarationListNode*)$1)->addNode($2);}
;

var_dec_prim
: term_rule array_dec_list {$$ = new VarDeclarationPrimNode($1,$2, NULL); $$->nodeLocation = GETLOC;}
| term_rule array_dec_list EQ array_init {$$ = new VarDeclarationPrimNode($1,$2, $4); $$->nodeLocation = GETLOC;}
| term_rule {$$ = new VarDeclarationPrimNode($1,NULL, NULL); $$->nodeLocation = GETLOC;}
| term_rule EQ expression {$$ = new VarDeclarationPrimNode($1,NULL,$3); $$->nodeLocation = GETLOC;}
;

var_dec_right_side
: var_dec_prim {$$ = new DeclarationCommaListNode(); ((DeclarationCommaListNode*)$$)->addNode($1);}
| var_dec_right_side COMMA var_dec_prim {$$ = $1; ((DeclarationCommaListNode*)$1)->addNode($3);}
;

var_dec_statement
: term_rule var_dec_right_side SEMICOLON {$$ = new VarDeclarationNode($1, $2); $$->nodeLocation = GETLOC;}
;

var_dec_for_statement
: term_rule var_dec_right_side {$$ = new VarDeclarationForNode($1, $2); $$->nodeLocation = GETLOC;}
;

for_statement
: FOR LPAREN var_dec_for_statement SEMICOLON expression SEMICOLON expression  RPAREN statement {$$ = new ForNode($3,$5,$7,$9); $$->nodeLocation = GETLOC;}
;

if_statement
: IF LPAREN expression RPAREN statement ELSE statement {$$ = new IfNode($3,$5,$7); $$->nodeLocation = GETLOC;}
| IF LPAREN expression RPAREN statement {$$ = new IfNode($3,$5,NULL); $$->nodeLocation = GETLOC;}
;

return_statement
: RETURN expression SEMICOLON {$$ = new ReturnNode($2); $$->nodeLocation = GETLOC;}
| RETURN SEMICOLON {$$ = new ReturnNode(NULL); $$->nodeLocation = GETLOC;}
;

statement
: expression_statement {$$ = $1;}
| var_dec_statement {$$ = $1;}
| compound_statement {$$ = $1;}
| for_statement {$$ = $1;}
| if_statement {$$ = $1;}
//| return_statement {$$ = $1;}
;

statement_list
: statement {$$ = new StatementListNode(); ((StatementListNode*)$$)->addNode($1); }
| statement_list statement {$$ = $1; ((StatementListNode*)$1)->addNode($2);}
;

compound_statement
: LBRACE RBRACE  {$$ = new CompoundStatementNode(NULL,false); $$->nodeLocation = GETLOC;}
| LBRACE statement_list RBRACE {$$ = new CompoundStatementNode($2,false); $$->nodeLocation = GETLOC;}
| PROCBEGIN PROCEND  {$$ = new CompoundStatementNode(NULL,true); $$->nodeLocation = GETLOC;}
| PROCBEGIN statement_list PROCEND {$$ = new CompoundStatementNode($2,true); $$->nodeLocation = GETLOC;}
;

function_compound_statement
: LBRACE RBRACE  {$$ = new CompoundStatementNode(NULL,false); $$->nodeLocation = GETLOC;}
| LBRACE start_function_statement_list RBRACE {$$ = new CompoundStatementNode($2,false); $$->nodeLocation = GETLOC;}
;



end_function_statement_list
: statement {$$ = new StatementListNode(); ((StatementListNode*)$$)->addNode($1); }
| end_function_statement_list statement {$$ = $1; ((StatementListNode*)$1)->addNode($2);}
;

start_function_statement_list
: return_statement {$$ = new StatementListNode(); ((StatementListNode*)$$)->addNode($1); }
| statement {$$ = new StatementListNode(); ((StatementListNode*)$$)->addNode($1); }
| end_function_statement_list return_statement {$$ = $1; ((StatementListNode*)$1)->addNode($2);}
| end_function_statement_list statement {$$ = $1; ((StatementListNode*)$1)->addNode($2);}
;


return_type
: term_rule array_dec_list {$$ = new FunctionReturnTypeNode($1, $2); $$->nodeLocation = GETLOC;}
| term_rule {$$ = new FunctionReturnTypeNode($1, 0); $$->nodeLocation = GETLOC;}
;

function_def
: FUNCTION return_type term_rule LPAREN defarglist RPAREN function_compound_statement {$$ = new FunctionDeclarationNode($3,$5,$7,$2); $$->nodeLocation = GETLOC;}
| FUNCTION return_type term_rule LPAREN RPAREN function_compound_statement {$$ = new FunctionDeclarationNode($3,NULL,$6,$2); $$->nodeLocation = GETLOC;}
;

program_node : program_node function_def {$$ = $1; ((ProgramListNode*)$1)->addNode($2);}
| program_node preamble_rule {$$ = $1; ((ProgramListNode*)$1)->addNode($2);}
| program_node var_dec_statement {$$ = $1; ((ProgramListNode*)$1)->addNode($2);}
| preamble_rule {$$ = new ProgramListNode(); ((ProgramListNode*)$$)->addNode($1); }
| function_def {$$ = new ProgramListNode(); ((ProgramListNode*)$$)->addNode($1);}
//| var_dec_statement {$$ = new ProgramListNode(); ((ProgramListNode*)$$)->addNode($1);}
;

term_rule
: TERM {AstLocation * loc = AstLocation::ConvertLocation(yyloc.begin.line,yyloc.end.line,yyloc.begin.column,yyloc.end.column);$$ = new TermNode(*$1,loc);}
;

term_num_rule
: NUMBER {AstLocation * loc = AstLocation::ConvertLocation(yyloc.begin.line,yyloc.end.line,yyloc.begin.column,yyloc.end.column);$$ = new TermNode(*$1,1,loc);}
| NUMBER POUND term_expression {AstLocation * loc = AstLocation::ConvertLocation(yyloc.begin.line,yyloc.end.line,yyloc.begin.column,yyloc.end.column);$$ = new TermNode(*$1,1,loc); ((TermNode*)$$)->bitwidthnode = $3; $3->parent = $$; ((TermNode*)$$)->isSigned=true;}
| NUMBER UPOUND term_expression {AstLocation * loc = AstLocation::ConvertLocation(yyloc.begin.line,yyloc.end.line,yyloc.begin.column,yyloc.end.column);$$ = new TermNode(*$1,1,loc); ((TermNode*)$$)->bitwidthnode = $3; $3->parent = $$; ((TermNode*)$$)->isSigned=false; }
;

start	: /* empty */
| program_node END {driver.prog.topLevelNodes.push_back($1);}
| program_node EOL {driver.prog.topLevelNodes.push_back($1);}
;

 /*** END EXAMPLE - Change the example grammar rules above ***/

%% /*** Additional Code ***/

void compiler::Parser::error(const Parser::location_type& l,
			    const std::string& m)
{
    driver.error(l, m);
exit(0);
}
