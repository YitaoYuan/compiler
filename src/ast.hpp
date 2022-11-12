#ifndef __AST_HPP__
#define __AST_HPP__
#include<assert.h>
#include<vector>
#include<unordered_map>
#include<set>
#include<string>
#include<utility>
#include<stack>
#include<iostream>
#include<fstream>
#include<sstream>
using namespace std;

#define BUFSIZE 1024

#define myprintf(mystring, ...) (assert(snprintf(mybuf, BUFSIZE, ##__VA_ARGS__) < BUFSIZE), (mystring) << mybuf)
#define astprintf(...) myprintf(astout, ##__VA_ARGS__)
#define riscvprintf(...) myprintf(*riscvout, ##__VA_ARGS__)

class ArraySizeAST;
class ArrayItemAST;
class ExpAST;
struct id_attr;
struct tmpid;

extern unordered_map<string, int>global_init_table;// 存放全局变量的初始值

extern vector<unordered_map<string, id_attr> >idt;// 全局符号表

extern unordered_map<int, vector<int> >aidt;// idt的拓展，存放数组各维长度

extern stack<pair<int, int> >loopstk;// 一个栈，存放当前各层循环的出入口

extern set<pair<string, int> >has_ret;// 存放有返回值的函数，用于判断函数是否有返回值

extern unordered_map<int, tmpid>s2t[2];// 缓存块内load/store的值，确保块内不重复load/store，不优化数组存取，因为可能有别名

extern int used_id, label_id;
//		   变量编号  标签编号
extern bool in_global;
//			标记处理阶段，true表示处理全局变量，false表示处理函数

extern char mybuf[BUFSIZE];
// myprintf的buffer

const int short_circuit_threshold = 10;
// 超过该阈值的计算会优先使用短路

const char arithmetic_name[][4] = {
	"or", "and", "ne", "eq", "lt", "gt", 
	"le", "ge", "add", "sub", "mul", "div", "mod"
};


struct id_attr{ // 符号表项
	int ret;
	bool is_compile_known_const;
	bool is_array;
	bool is_global;
};

enum arithmetic_type : int{
	type_lor = 0, type_land, type_ne, type_eq, type_lt, type_gt, type_le, type_ge, 
	type_pls, type_mns, type_mlt, type_div, type_mod//ÎÞ"·Ç"
};
 
struct tmpid{ // 符号的局部值，例如%1 = load @S，则%1为@S的局部值
	int tid;
	bool is_compile_known_const;
	bool modified;
};


class BaseAST{
public:
	ostringstream astout;
	// 所有AST的dump结果存放在其astout.str()中

	virtual void dump();
	
	void block_clear();
	void table_flush_and_clear(unordered_map<int, tmpid> &tb);
	void block_flush_global();
	void block_flush_all();
	void block_storeint(bool is_compile_known_const, int val, bool is_global, int sid);
	void block_loadint(bool &is_compile_known_const, int &tid, bool is_global, int sid);

	static int cal_const(int, int, arithmetic_type);
	static id_attr envfind(const string &);
	void printtype(vector<int>&, int);
	void printtype(ArraySizeAST*);
	void printlist(ArrayItemAST*, vector<int>&, int);
	ArrayItemAST* flatten(ArrayItemAST*, vector<int>&, size_t&, size_t);
	void printinit(ArrayItemAST*, vector<int>&);

	int getptr(int, vector<ExpAST*>&, vector<int>&);
};

class StateAST: public BaseAST{// this is a tree node
public:
};

class ExpAST: public StateAST{
public:
	int ret;
	int treesize;
	bool has_side_effect;
	bool is_compile_known_const;
	bool is_bool;
	bool has_block;
	bool found;

	ExpAST();

	virtual void find_side_effect() = 0;
	virtual void analize() = 0;
};

class ArraySizeAST : public BaseAST{
public:
	vector<ExpAST*>expvec;
	vector<int>intvec;
	bool visit;
	
	ArraySizeAST();
	
	void dump();
};

class ArrayItemAST : public BaseAST{
public:
	bool is_int;
	vector<ArrayItemAST*>vec;
	ExpAST *exp;
};

class EmptyStateAST: public StateAST{
public:
};

class FParameterAST: public BaseAST{
public:
	string *name;
	ArraySizeAST *size;
	FParameterAST(string *_name, BaseAST *_size);
};

class FuncAST: public BaseAST{
public:
	string *name;
	vector<FParameterAST*> params;
	bool has_ret_value;
	StateAST *state;

	void dump();
};

class ArithmeticAST: public ExpAST{
public:
	arithmetic_type atype;
	ExpAST *x, *y;
	
	ArithmeticAST(BaseAST *_x, arithmetic_type _atype, BaseAST *_y);

	void find_side_effect();

	void analize();
	
	void dump();
};

class ConstNumberAST: public ExpAST{
public:
	
	ConstNumberAST(int _n);
	
	void find_side_effect();

	void analize();
	
	void dump();
};

class FuncCallAST: public ExpAST{
public:
	string *s;
	vector<ExpAST*>params;
	
	void find_side_effect();

	void analize();
	
	void dump();
};

class BlockAST: public StateAST{
public:
	StateAST *state;
	BlockAST(BaseAST *_state);
	
	void dump();
};

class IfAST: public StateAST{
public:
	ExpAST *exp;
	StateAST *condt, *condf;
	
	IfAST(BaseAST *_exp, BaseAST *_condt, BaseAST *_condf);
	
	void dump();
};

class WhileAST: public StateAST{
public:
	ExpAST *exp;
	StateAST *state;
	
	WhileAST(BaseAST *_exp, BaseAST *_state);
	
	void dump();
};

class BreakAST: public StateAST{
public:
	void dump();
};

class ContinueAST: public StateAST{
public:
	void dump();
};

class InodeAST: public StateAST{// inner node
public:
	StateAST *l, *r;// States / State
	
	InodeAST(BaseAST *_l, BaseAST *_r);
	
	void dump();
};

class RetAST: public StateAST{// leaf
public:
	ExpAST *exp;
	
	RetAST(BaseAST *_exp);
	
	void dump();
};

class DefIntAST: public StateAST{
public:
	string *name;
	ExpAST *exp;
	bool decl_const;
	bool decl_global;
	
	DefIntAST(string *_name, BaseAST *_exp, bool _decl_const, bool _decl_global);
	
	void dump();
};

class AssignIntAST: public StateAST{// leaf
public:
	string *name;
	ExpAST *exp;
	 
	AssignIntAST(string *_name, BaseAST *_exp);
	
	void find_side_effect();

	void dump();
};

class LvalExpAST: public ExpAST{// is this only be used as a Exp?? 
public:
	string *name;
	
	LvalExpAST(string *_name);

	void find_side_effect();
	
	void analize();

	void dump();
};

class DefArrayAST : public StateAST{// declare (+ assign)
public:
	string *name;
	ArraySizeAST *size;
	ArrayItemAST *item;
	bool decl_global;
	
	DefArrayAST(string *_name, BaseAST* _size, BaseAST* _item, bool _decl_global);
	
	void dump();
};

class AssignIntOfArrayAST: public StateAST{
public:
	string *name;
	ArraySizeAST *pos;
	ExpAST *exp;
	AssignIntOfArrayAST(string *_name, BaseAST* _pos, BaseAST* _exp);
	
	void dump();
};

class ArrayLvalExpAST: public ExpAST{// is this only be used as a Exp?? 
public:
	string *name;
	ArraySizeAST *pos;
	
	ArrayLvalExpAST(string *_name, BaseAST* _pos);
	
	void find_side_effect();
	
	void analize();
	
	void dump();
};

class GlobalAST:public BaseAST{
public:
	vector<FuncAST*> funcs;
	vector<BaseAST*> decls;
	
	void libdecl();
	
	void dump();
};

#endif
