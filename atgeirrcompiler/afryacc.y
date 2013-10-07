%token COLLECTION
%token OF
%token ON
%token SUBSET
%token SCALAR
%token VECTOR
%token BOOL
%token CELL
%token FACE
%token EDGE
%token VERTEX
%token FUNCTION
%token AND
%token OR
%token NOT
%token XOR
%token TRUE
%token FALSE
%token <str> BUILTIN
%token <str> ID
%token <str> INT
%token <str> FLOAT
%token <str> COMMENT
%token LEQ
%token GEQ
%token EQ
%token NEQ
%token RET
%token EOL

%type <node> program
%type <node> line
%type <node> statement
%type <node> declaration
%type <node> assignment
%type <node> comb_decl_assign
%type <node> expression
%type <node> type_expression
%type <node> basic_type
%type <node> f_decl_args
%type <node> number
%type <node> function_call
%type <node> f_call_args


%output "afryacc.cpp"
%defines "afryacc.hpp"

%start program
%error-verbose


%left OR
%left AND
%nonassoc XOR
%nonassoc EQ NEQ
%nonassoc LEQ GEQ '<' '>'
%left '+' '-'
%left '*'
%nonassoc '/'
%nonassoc '^'
%nonassoc NOT
%left UMINUS



%code requires{
#include "Parser.hpp"
}

%union{
    NodePtr node;
    std::string* str;
}


%%

program: program line           { $$ = new Node(); }
       |                        { $$ = new Node(); }
       ;

line: statement EOL             { $$ = new Node(); }
    | statement COMMENT EOL     { $$ = new Node(); }
    | COMMENT EOL               { $$ = new Node(); }
    | EOL                       { $$ = new Node(); }
    ;

statement: declaration          { $$ = new Node(); }
         | assignment           { $$ = new Node(); }
         | comb_decl_assign     { $$ = new Node(); }
         | expression           { $$ = new Node(); }
;

declaration: ID ':' type_expression  { $$ = new Node(); }

assignment: ID '=' expression   { $$ = new Node(); }

comb_decl_assign: ID ':' type_expression '=' expression  { $$ = new Node(); }

expression: number              { $$ = new Node(); }
          | function_call       { $$ = new Node(); }
          ;

type_expression: basic_type     { $$ = new Node(); }
               | COLLECTION OF basic_type     { $$ = new Node(); }
               | COLLECTION OF basic_type ON expression { $$ = new Node(); }
               | COLLECTION OF basic_type SUBSET OF expression { $$ = new Node(); }
               | COLLECTION OF basic_type ON expression SUBSET OF expression { $$ = new Node(); }
               | FUNCTION '(' f_decl_args ')' RET type_expression    { $$ = new Node(); }
               ;

basic_type: SCALAR  { $$ = new Node(); }
          | VECTOR  { $$ = new Node(); }
          | BOOL    { $$ = new Node(); }
          | CELL    { $$ = new Node(); }
          | FACE    { $$ = new Node(); }
          | EDGE    { $$ = new Node(); }
          | VERTEX  { $$ = new Node(); }
          ;

f_decl_args: f_decl_args ',' declaration { $$ = new Node(); }
           | declaration                 { $$ = new Node(); }
           |                             { $$ = new Node(); }
           ;

number: INT                     { $$ = createNumber(numFromString(*($1))); delete $1; }
      | FLOAT                   { $$ = createNumber(numFromString(*($1))); delete $1; }
      ;

function_call: BUILTIN '(' f_call_args ')'  { $$ = new Node(); }
             | ID '(' f_call_args ')'       { $$ = new Node(); }
             ;

f_call_args: expression                     { $$ = new Node(); }

%%

void yyerror(const char* err)
{
    std::cerr << "Parser error on line " << yylineno << ": " << err << std::endl;
}
