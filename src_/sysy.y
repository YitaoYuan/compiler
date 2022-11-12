%code requires {
  #include <memory>
  #include <string>
  #include "ast.hpp"
}

%{

#include <iostream>
#include <memory>
#include <string>

// ���� lexer �����ʹ�������
#include "ast.hpp"

int yylex();
void yyerror(BaseAST *ast, const char *s);

using namespace std;



%}

// ���� parser �����ʹ��������ĸ��Ӳ���
// ������Ҫ����һ���ַ�����Ϊ AST, �������ǰѸ��Ӳ���������ַ���������ָ��
// ������ɺ�, ����Ҫ�ֶ��޸��������, �������óɽ����õ����ַ���
%parse-param { BaseAST * &ast }

// yylval �Ķ���, ���ǰ����������һ�������� (union)
// ��Ϊ token ��ֵ�е����ַ���ָ��, �е�������
// ֮ǰ������ lexer ���õ��� str_val �� int_val ���������ﱻ�����
// ����ΪʲôҪ���ַ���ָ�����ֱ���� string ���� unique_ptr<string>?
// ������ STFW �� union ��дһ��������������������ʲô���
%union {
  std::string *str_val;
  int int_val;
  BaseAST *ast_val; 
  StateAST *state_ast_val;
}

// lexer ���ص����� token ���������
// ע�� IDENT �� INT_CONST �᷵�� token ��ֵ, �ֱ��Ӧ str_val �� int_val
%token INT RETURN CONST IF ELSE WHILE VOID EQ NE LE GE LAND LOR BREAK CONTINUE
%token <str_val> IDENT
%token <int_val> INT_CONST 

// ���ս�������Ͷ���
%type <int_val> BType
%type <ast_val> FParameter
%type <ast_val> FuncDef Block Exp Number Lval LOrExp LAndExp  
%type <ast_val> EqExp RelExp AddExp MulExp UnaryExp PrimaryExp Stmts Stmt 
%type <ast_val> Assign 
%type <ast_val> While GlobalUnit FParameters FuncCall RParameters RParameter Return
%type <ast_val> NoIfStmt PairedStmt NotPairedStmt Break Continue
%type <ast_val> Decl ConstDecl ConstDefs ConstDef VarDecl VarDefs VarDef
%type <ast_val> GlobalDecl GlobalConstDecl GlobalConstDefs GlobalConstDef
%type <ast_val> GlobalVarDecl GlobalVarDefs GlobalVarDef
%type <ast_val> ArrayItem ArrayItems ArrayInit ArraySize
%%

// ��ʼ��, CompUnit ::= FuncDef, �����ź������˽�����ɺ� parser Ҫ��������
// ֮ǰ���Ƕ����� FuncDef �᷵��һ�� str_val, Ҳ�����ַ���ָ��
// �� parser һ�������� CompUnit, ��˵�����е� token ����������, ������������
// ��ʱ����Ӧ�ð� FuncDef ���صĽ���ռ�����, ��Ϊ AST �������� parser �ĺ���
// $1 ָ���������һ�����ŵķ���ֵ, Ҳ���� FuncDef �ķ���ֵ
CompUnit
  : GlobalUnit {
    ast = $1;
  }
  ;
  
  
GlobalUnit
  : %empty {
    $$ = new GlobalAST();
  }
  | GlobalUnit FuncDef {
    ((GlobalAST*)$$)->funcs.push_back((FuncAST*)$2);
  }
  | GlobalUnit GlobalDecl ';' {
    ((GlobalAST*)$$)->decls.push_back((StateAST*)$2);
  }
  ;

FuncDef
  : VOID IDENT '(' FParameters ')' '{' Stmts '}' {
    auto func = (FuncAST*)$4;
    func->name = $2;
    func->has_ret_value = 0;
    func->state = (StateAST*)$7;
    $$ = func;
  }
  | BType IDENT '(' FParameters ')' '{' Stmts '}' {
    auto func = (FuncAST*)$4;
    func->name = $2;
    func->has_ret_value = 1;
    func->state = (StateAST*)$7;
    $$ = func;
  }
  ;
  
FParameters
  : %empty {
    $$ = new FuncAST();
  }
  | FParameter {
    $$ = new FuncAST();
    ((FuncAST*)$$)->params.push_back((FParameterAST*)$1);
  }
  | FParameters ',' FParameter {
    ((FuncAST*)$$)->params.push_back((FParameterAST*)$3);
  }
  ;

FParameter
  : BType IDENT {
    $$ = new FParameterAST($2, NULL);
  }
  | BType IDENT '[' ']' {
    auto sz = new ArraySizeAST();
    sz->expvec.push_back(new ConstNumberAST(-1));
    $$ = new FParameterAST($2, sz);
  }
  | BType IDENT '[' ']' ArraySize {
    ((ArraySizeAST*)$5)->expvec.insert(((ArraySizeAST*)$5)->expvec.begin(), new ConstNumberAST(-1));
    $$ = new FParameterAST($2, $5);
  }
  ;

Block
  : '{' Stmts '}' {
    $$ = new BlockAST($2);// ��ʱ������ 
  }
  ;

Stmts // however, <Stmts>'s return type is StateAST*
  : %empty {
    $$ = NULL;
  }
  | Stmts Stmt {
    $$ = new InodeAST($1, $2);
  }
  ;

Stmt
  : PairedStmt
  | NotPairedStmt

NotPairedStmt  // virtual
  : IF '(' Exp ')' Stmt {
    $$ = new IfAST($3, $5, NULL);
  }
  | IF '(' Exp ')' PairedStmt ELSE NotPairedStmt {
    $$ = new IfAST($3, $5, $7);
  }
  ;

PairedStmt
  : IF '(' Exp ')' PairedStmt ELSE PairedStmt { // inner
    $$ = new IfAST($3, $5, $7);
  }
  | NoIfStmt
  ;

NoIfStmt
  : While
  | Block 
  | Decl ';'
  | Assign ';' 
  | Exp ';'
  | ';' {
    $$ = new EmptyStateAST();
  }
  | Return ';'
  | Break ';'
  | Continue ';'
  ;

Return 
  : RETURN Exp {
    $$ = new RetAST($2);
  }
  | RETURN {
  	$$ = new RetAST(NULL);
  }
  ;

While 
  : WHILE '(' Exp ')' Stmt {
    $$ = new WhileAST($3, $5);
  }
  ;

Break
  : BREAK {
    $$ = new BreakAST();
  }
  ;

Continue
  : CONTINUE {
    $$ = new ContinueAST();
  }
  ;
  
BType
  : INT { $$ = 1; }
  ;

ArraySize
  : '[' Exp ']' {// Exp �����ڱ���ʱ����ֵ 
    auto p = new ArraySizeAST();
    p->expvec.push_back((ExpAST*)$2);
	$$ = p; 
  }
  | ArraySize '[' Exp ']' {
    ((ArraySizeAST*)$$)->expvec.push_back((ExpAST*)$3);
  }
  ;
  
ArrayInit
  : '{' ArrayItems '}' {
    $$ = $2;
  }
  ;

ArrayItems
  : %empty {
    auto p = new ArrayItemAST();
    p->is_int = 0;
    $$ = p;
  }
  | ArrayItem {
    auto p = new ArrayItemAST();
    p->is_int = 0;
    p->vec.push_back((ArrayItemAST*)$1);
    $$ = p;
  }
  | ArrayItems ',' ArrayItem {
    ((ArrayItemAST*)$$)->vec.push_back((ArrayItemAST*)$3);
  }
  ;
  
ArrayItem
  : ArrayInit
  | Exp {
  	auto p = new ArrayItemAST();
  	p->is_int = 1;
  	p->exp = (ExpAST*)$1;
  	$$ = p;
  }
  ;

Decl
  : ConstDecl | VarDecl
  ;
  
ConstDecl
  : CONST BType ConstDefs {
    $$ = $3;
  }
  ;
  
ConstDefs
  : ConstDef
  | ConstDefs ',' ConstDef {
    $$ = new InodeAST($1, $3);
  }
  ;

ConstDef
  : IDENT '=' Exp {
    $$ = new DefIntAST($1, $3, 1, 0);/* important */
  }
  | IDENT ArraySize '=' ArrayInit {
    $$ = new DefArrayAST($1, $2, $4, 0);
  }
  ; 
  
VarDecl
  : BType VarDefs {
    $$ = $2;
  }
  ;
  
VarDefs
  : VarDef 
  | VarDefs ',' VarDef {
    $$ = new InodeAST($1, $3);
  }
  ;

VarDef
  : IDENT {
    $$ = new DefIntAST($1, NULL, 0, 0);
  }
  | IDENT ArraySize {
    $$ = new DefArrayAST($1, $2, NULL, 0);
  }
  | IDENT '=' Exp {
    $$ = new DefIntAST($1, $3, 0, 0);
  }
  | IDENT ArraySize '=' ArrayInit {
    $$ = new DefArrayAST($1, $2, $4, 0);
  }
  ;
  
GlobalDecl
  : GlobalConstDecl | GlobalVarDecl
  ;

GlobalConstDecl
  : CONST BType GlobalConstDefs {
    $$ = $3;
  }
  ;

GlobalConstDefs
  : GlobalConstDef
  | GlobalConstDefs ',' GlobalConstDef {
    $$ = new InodeAST($1, $3);
  }
  ;
  
GlobalConstDef
  : IDENT '=' Exp {
    $$ = new DefIntAST($1, $3, 1, 1);// decl_const, decl_global
  }
  | IDENT ArraySize '=' ArrayInit {
    $$ = new DefArrayAST($1, $2, $4, 1);
  }
  ;
  
GlobalVarDecl
  : BType GlobalVarDefs {
    $$ = $2;
  }
  ;
  
GlobalVarDefs
  : GlobalVarDef 
  | GlobalVarDefs ',' GlobalVarDef {
    $$ = new InodeAST($1, $3);
  }
  ;

GlobalVarDef
  : IDENT {
    $$ = new DefIntAST($1, NULL, 0, 1);
  }
  | IDENT ArraySize {
    $$ = new DefArrayAST($1, $2, NULL, 1);
  }
  | IDENT '=' Exp {
    $$ = new DefIntAST($1, $3, 0, 1);
  }
  | IDENT ArraySize '=' ArrayInit {
    $$ = new DefArrayAST($1, $2, $4, 1);
  }
  ;
  
Assign
  : IDENT '=' Exp {
    $$ = new AssignIntAST($1, $3);
  }
  | IDENT ArraySize '=' Exp {
    $$ = new AssignIntOfArrayAST($1, $2, $4);//
  }
  ;


Exp
  : LOrExp 
  ;
  
LOrExp
  : LAndExp 
  | LOrExp LOR LAndExp {// ||
    $$ = new ArithmeticAST($1, type_lor, $3);
  }
  ;

LAndExp
  : EqExp 
  | LAndExp LAND EqExp {// &&
    $$ = new ArithmeticAST($1, type_land, $3);
  }
  ;

EqExp
  : RelExp 
  | EqExp EQ RelExp {// ==
    $$ = new ArithmeticAST($1, type_eq, $3);
  } 
  | EqExp NE RelExp {// !=
    $$ = new ArithmeticAST($1, type_ne, $3);
  }
  ;

RelExp
  : AddExp 
  | RelExp '>' AddExp {
    $$ = new ArithmeticAST($1, type_gt, $3);
  }
  | RelExp '<' AddExp {
    $$ = new ArithmeticAST($1, type_lt, $3);
  }
  | RelExp GE AddExp {// >=
    $$ = new ArithmeticAST($1, type_ge, $3);
  }
  | RelExp LE AddExp {// <=
    $$ = new ArithmeticAST($1, type_le, $3);
  }
  ;
  
AddExp
  : MulExp  
  | AddExp '+' MulExp {
    $$ = new ArithmeticAST($1, type_pls, $3);
  } 
  | AddExp '-' MulExp {
    $$ = new ArithmeticAST($1, type_mns, $3);
  }
  ;

MulExp
  : UnaryExp 
  | MulExp '*' UnaryExp {
    $$ = new ArithmeticAST($1, type_mlt, $3);
  } 
  | MulExp '/' UnaryExp {
    $$ = new ArithmeticAST($1, type_div, $3);
  }
  | MulExp '%' UnaryExp {
    $$ = new ArithmeticAST($1, type_mod, $3);
  }
  ;
  
UnaryExp
  : PrimaryExp 
  | '+' UnaryExp {
    $$ = $2;
  }
  | '-' UnaryExp {
    $$ = new ArithmeticAST(new ConstNumberAST(0), type_mns, $2);
  } 
  | '!' UnaryExp {
    $$ = new ArithmeticAST(new ConstNumberAST(0), type_eq, $2);
  }
  ;

PrimaryExp
  : '(' Exp ')' { $$ = $2; }
  | Number 
  | Lval
  | FuncCall
  ;
  
Number
  : INT_CONST {
    $$ = new ConstNumberAST($1);
  }
  ;

Lval// this is only used as an expression
  : IDENT {
    $$ = new LvalExpAST($1);
  }
  | IDENT ArraySize {
    $$ = new ArrayLvalExpAST($1, $2); 
  }
  ;

FuncCall
  : IDENT '(' RParameters ')' {
    ((FuncCallAST*)$3)->s = $1;
    $$ = $3;
  }
  ;

RParameters
  : %empty {
    $$ = new FuncCallAST();
  }
  | RParameter {
    $$ = new FuncCallAST();
    ((FuncCallAST*)$$)->params.push_back((ExpAST*)$1);
  }
  | RParameters ',' RParameter {
    ((FuncCallAST*)$$)->params.push_back((ExpAST*)$3);
  }
  ;
  
RParameter
  : Exp
  ;
%%

// �����������, ���еڶ��������Ǵ�����Ϣ
// parser ����������� (��������ĳ���������﷨����), �ͻ�����������
void yyerror(BaseAST *ast, const char *s) {
  ast = ast+1-1;// shut up !
  cerr << "error: " << s << endl;
  //ast->dump();
}
