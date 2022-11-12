#include "ast.hpp"

unordered_map<string, int>global_init_table;

vector<unordered_map<string, id_attr> >idt;

unordered_map<int, vector<int> >aidt;

stack<pair<int, int> >loopstk;

set<pair<string, int> >has_ret;

unordered_map<int, tmpid>s2t[2];

int used_id = 0, label_id = 0;
bool in_global;

/*
 * 下面这几个函数用于优化块内的load/store数量
 */

void BaseAST::table_flush_and_clear(unordered_map<int, tmpid> &tb)
{
	bool is_global = &tb == &s2t[1];

	for(auto &i: tb) if(i.second.modified) {
		int value_rid = i.second.tid;
		if(i.second.is_compile_known_const) {// 为了使store指令的value不是常量（因为riscv不支持）
			astprintf("  %%%d = add 0, %d\n", ++used_id, i.second.tid);	
			value_rid = used_id;
		}
		if(is_global) {
			astprintf("  %%%d = getptr @S%d, 0\n", ++used_id, i.first);
			// 把加载global address的任务全部交给getptr
			astprintf("  store %%%d, %%%d\n", value_rid, used_id);
		}
		else astprintf("  store %%%d, @S%d\n", value_rid, i.first);
	}
	tb.clear();
}

void BaseAST::block_clear()
{
	for(int i = 0; i <= 1; i++)
		s2t[i].clear();
}

// 只对全局变量清空并输出store缓存
void BaseAST::block_flush_global() 
{
	table_flush_and_clear(s2t[1]);
}

// 对所有变量清空并输出store缓存
void BaseAST::block_flush_all() 
{
	for(int i = 0; i <= 1; i++)
		table_flush_and_clear(s2t[i]);
}

// 缓存store
void BaseAST::block_storeint(bool is_compile_known_const, int val, bool is_global, int sid)
{
	//astprintf("  store %s%d, @S%d\n", &"%"[is_compile_known_const], val, sid);
	s2t[is_global][sid] = (tmpid){val, is_compile_known_const, true};
}

// 查找load数据是否之前被读取过，若读取过，则返回之前的数据，否则做一次load
void BaseAST::block_loadint(bool &is_compile_known_const, int &tid, bool is_global, int sid)
{
	auto p = s2t[is_global].find(sid);
	if(p == s2t[is_global].end()) {
		tid = ++used_id;
		is_compile_known_const = 0;
		if(is_global) {
			astprintf("  %%%d = getptr @S%d, 0\n", ++used_id, sid);
			astprintf("  %%%d = load %%%d\n", tid, used_id);
		}
		else astprintf("  %%%d = load @S%d\n", tid, sid);
		s2t[is_global][sid] = (tmpid){tid, is_compile_known_const, false};
	} 
	else {
		tid = p->second.tid;
		is_compile_known_const = p->second.is_compile_known_const;
	}
}

/*
 * 以下均为koopa IR的辅助输出函数
 */

int BaseAST::cal_const(int x, int y, arithmetic_type type)
{
	switch(type) {
		case type_lor: return x || y;
		case type_land: return x && y;
		case type_ne: return x != y;
		case type_eq: return x == y;
		case type_lt: return x < y;
		case type_gt: return x > y;
		case type_le: return x <= y;
		case type_ge: return x >= y;
		case type_pls: return x + y;
		case type_mns: return x - y;
		case type_mlt: return x * y;
		case type_div: return x / y;
		case type_mod: return x % y;
	}
	assert(0);
}

// 查全局符号表
id_attr BaseAST::envfind(const string &s)
{
	for(auto i = idt.rbegin(); i != idt.rend(); i++)
	{
		auto p = i->find(s);
		if(p != i->end())
			return p->second;
	}
	return (id_attr){-1, false, false, false};
}

void BaseAST::printtype(vector<int>&vec, int i)
{
	if(i>=(int)vec.size()) {
		astprintf("i32");
		return;
	}
	if(i == 0 && vec[i] == -1) {
		astprintf("*");
		printtype(vec, i+1);
		return;
	}
	astprintf("[ ");
	printtype(vec, i+1);
	astprintf(" , %d ]", vec[i]);
}

void BaseAST::printtype(ArraySizeAST* size)
{
	if(!size) {
		astprintf("i32");
		return;
	}
	size->dump();
	printtype(size->intvec, 0);
}

// aggregate类型格式化
ArrayItemAST* BaseAST::flatten(ArrayItemAST* init, vector<int>&size, size_t &pos, size_t level)
{

	ArrayItemAST* arr = new ArrayItemAST();

	for(int cpct = 0; pos < init->vec.size() && cpct < size[level]; cpct++) {
		if(init->vec[pos]->is_int) {
			if(level == size.size()-1) {
				arr->vec.push_back(init->vec[pos]);
				pos ++;
			}
			else {
				arr->vec.push_back(flatten(init, size, pos, level+1));
			}
		}
		else {
			if(level == size.size()-1) {
				break;
			}
			else{
				size_t son_pos = 0;
				arr->vec.push_back(flatten(init->vec[pos], size, son_pos, level+1));
				if(son_pos < init->vec[pos]->vec.size()) {
					fprintf(stderr, "error\n");
					exit(0);
				}
				pos++;
			}
		}
	}
	return arr;
}

// 输出格式化后的aggregate
void BaseAST::printlist(ArrayItemAST* init, vector<int>&size, int level)
{
	if(init->vec.size() == 0) {
		astprintf("zeroinit");
		return;
	}
	astprintf("{");
	
	for(size_t i = 0; i < init->vec.size(); i++) {
		auto p = init->vec[i];
		if(i) astprintf(", ");
		if(p->is_int) {
			p->exp->dump();
			assert(p->exp->is_compile_known_const);
			astprintf("%d", p->exp->ret);
		}
		else printlist(p, size, level+1);
	}
	for(size_t i = init->vec.size(); (int)i < size[level]; i++) {
		if(i) astprintf(", ");
		astprintf("zeroinit");
	}
	
	astprintf("}");
}

// 将用于初始化数组的aggregate类型格式化，具体来讲是补全大括号以及数组中不足的位置
void BaseAST::printinit(ArrayItemAST* init, vector<int>&size)
{
	size_t pos = 0;
	auto ptr = flatten(init, size, pos, 0);
	if(pos < init->vec.size()) {
		fprintf(stderr, "error\n");
		exit(0);
	}
	printlist(ptr, size, 0);
}

// 用数组名及多维下标，返回对应位置的指针
int BaseAST::getptr(int arr_id, vector<ExpAST*>&pos, vector<int>&size)
{
	int pre = arr_id;
	for(size_t i = 0; i < pos.size(); i++) {
		ExpAST *exp = pos[i];
		exp->dump();
		astout << exp->astout.str();
		exp->astout.str("");
		
		if(size[i]==-1) {
			bool unused;
			block_loadint(unused, pre, 0, pre);
			
			astprintf("  %%%d = getptr %%%d, %s%d\n", ++used_id, 
								pre, &"%"[exp->is_compile_known_const], exp->ret);
		}
		else {
			if(i == 0) {
				astprintf("  %%%d = getptr @S%d, 0\n", ++used_id, pre);
				pre = used_id;
			}
			astprintf("  %%%d = getelemptr %%%d, %s%d\n", ++used_id, pre, &"%"[exp->is_compile_known_const], exp->ret);
		}
		pre = used_id;
	}
	return pre;
}

void BaseAST::dump() {}

ExpAST::ExpAST() {// new
	found = 0;
}

ArraySizeAST::ArraySizeAST() {
	visit = 0;
}

void ArraySizeAST::dump() {
	if(visit) return;
	visit = 1;
	intvec.reserve(expvec.size());
	for(auto &exp: expvec) {
		exp->dump();
		assert(exp->is_compile_known_const);
		intvec.push_back(exp->ret);
	}
}

FParameterAST::FParameterAST(string *_name, BaseAST *_size)
: name(_name), size((ArraySizeAST*)_size)
{}

void FuncAST::dump() {
	
	idt.resize(idt.size()+1);
	
	
	astprintf("fun @%s(", name->c_str());
	
	if(has_ret_value)
		has_ret.insert(make_pair(*name, (int)params.size()));
	
	for(size_t i = 0; i < params.size(); i++) {
		astprintf("%s%%%d : ", i!=0?", ":"", ++used_id);
		printtype(params[i]->size);//dumped
	}
		
	astprintf(")%s {\n%%label%d:\n", has_ret_value?": i32":"", ++label_id);

	int endid = used_id + 1;
	for(size_t i = 0; i < params.size(); i++) {
		astprintf("  @S%d = alloc ", ++used_id);
		printtype(params[i]->size);
		astprintf("\n");

		block_storeint(0, (int)(endid - params.size() + i), 0, used_id);

		idt.back()[*params[i]->name] = (id_attr){used_id, false, params[i]->size? true : false, false};
		if(params[i]->size) aidt[used_id] = params[i]->size->intvec;
	}
		
	
	if(state) {
		state->dump();
		astout << state->astout.str();
		state->astout.str("");
	}


	block_flush_global();
	block_clear();
	astprintf("  ret%s\n", has_ret_value?" 0":"");
	astprintf("}\n\n");
	
	idt.resize(idt.size()-1);
}

ArithmeticAST::ArithmeticAST(BaseAST *_x, arithmetic_type _atype, BaseAST *_y)
: atype(_atype), x((ExpAST*)_x), y((ExpAST*)_y)
{}

void ArithmeticAST::find_side_effect() {
	if(found) return;
	found = 1;
	x->find_side_effect();
	y->find_side_effect();
	treesize = x->treesize + y->treesize + 1;
	has_side_effect = x->has_side_effect || y->has_side_effect;
}

void ArithmeticAST::analize() {
	is_compile_known_const = x->is_compile_known_const && y->is_compile_known_const;
	if(is_compile_known_const) 
		ret = cal_const(x->ret, y->ret, atype);
	// this can be modified
	
	is_bool = atype <= type_ge;
	has_block = x->has_block || y->has_block;
}

void ArithmeticAST::dump() {

	find_side_effect();

	
	bool need_bool = atype <= type_land;// need_bool 不一定等于 is_bool 
	bool is_short_circuit = need_bool;
	// need_bool 同时也表示是否为短路运算 
	
	
	if(is_short_circuit && 
	    (y->treesize > short_circuit_threshold || y->has_side_effect) ) {
		// y开销大或者调用了函数，做短路运算 

		has_block = 1;

		int T = ++used_id;
		astprintf("  %%T%d = alloc i32\n", T);
		
		x->astout.str("");
		x->dump();
		astout << x->astout.str();
		x->astout.str("");
		
		int res1 = ++used_id;
		if(x->is_compile_known_const) {
			astprintf("  %%%d = add 0, %d\n", res1, (int)(bool)x->ret);
		}
		else {
			astprintf("  %%%d = ne 0, %%%d\n", res1, x->ret);
		}
		
		/*
		int bool_id = used_id, br_id;
		if(atype == type_land) 
			astprintf("  %%%d = eq %s%d, 0\n", ++used_id, 
				&"%"[x->is_compile_known_const], x->ret);
		br_id = used_id;
		*/
		astprintf("  store %%%d, %%T%d\n", res1, T);// don't use block_storeint()

		block_flush_all();
		int Ly = ++label_id, Le = ++label_id;
		if(atype == type_land) 
			astprintf("  br %%%d, %%next%d, %%label%d\n", res1, Ly, Le);
		else
			astprintf("  br %%%d, %%label%d, %%next%d\n", res1, Le, Ly);

		astprintf("%%next%d:\n", Ly);
		
		y->astout.str("");
		y->dump();
		astout << y->astout.str();
		y->astout.str("");

		int res2 = ++used_id;
		if(y->is_compile_known_const) {
			astprintf("  %%%d = add 0, %d\n", res2, (int)(bool)y->ret);
		}
		else {
			astprintf("  %%%d = ne 0, %%%d\n", res2, y->ret);
		}
		
		
		astprintf("  store %%%d, %%T%d\n", res2, T);
		
		block_flush_all();
		astprintf("  jump %%label%d\n", Le);
		astprintf("%%label%d:\n", Le);
		
		astprintf("  %%%d = load %%T%d\n", ++used_id, T);// don't use block_loadint()
		
		ret = used_id;
		return;
	}

	x->dump();
	y->dump();
	analize();
	
	astout << x->astout.str();// 这里有可能跨块传变量
	x->astout.str("");
	
	int a = x->ret;
	if(need_bool && !x->is_bool) {
		if(x->is_compile_known_const) a = (bool)a;
		else {
			astprintf("  %%%d = ne 0, %%%d\n", ++used_id, a);
			a = used_id;
		}
	}

	if(!x->is_compile_known_const && y->has_block) {
		astprintf("  @S%d = alloc i32\n", ++used_id);
		astprintf("  store %%%d, @S%d\n", a, used_id);
		a = used_id;
	}

	astout << y->astout.str();
	y->astout.str("");

	int b = y->ret;
	if(need_bool && !y->is_bool) {
		if(y->is_compile_known_const) b = (bool)b;
		else {
			astprintf("  %%%d = ne 0, %%%d\n", ++used_id, b);
			b = used_id;
		}
	}
	
	is_compile_known_const = x->is_compile_known_const && y->is_compile_known_const;
	// we update this because a variable which is not a known-constant during its life 
	// may become a local-known-contant in some code segment

	if(is_compile_known_const) {
		ret = cal_const(a, b, atype);
		return;
	}
	
	if(!x->is_compile_known_const && y->has_block) {
		astprintf("  %%%d = load @S%d\n", ++used_id, a);
		a = used_id;
	}

	if(atype == type_gt || atype == type_le) {
		atype = atype == type_gt ? type_lt : type_ge;
		swap(x, y);
		swap(a, b);
	}				
	astprintf("  %%%d = %s %s%d, %s%d\n", ++used_id, arithmetic_name[atype], 
			&"%"[x->is_compile_known_const], a, &"%"[y->is_compile_known_const], b);
									
	ret = used_id;
}

ConstNumberAST::ConstNumberAST(int _n) 
{ ret = _n; }

void ConstNumberAST::find_side_effect() {
	if(found) return;
	found = 1;
	treesize = 0;
	has_side_effect = 0;
}

void ConstNumberAST::analize() {
	is_compile_known_const = 1;
	is_bool = (ret&~1) == 0;
	has_block = 0;
}

void ConstNumberAST::dump() { 
	find_side_effect();
	analize();
}

void FuncCallAST::find_side_effect() {
	if(found) return;
	found = 1;
	treesize = short_circuit_threshold + 1;
	has_side_effect = 1;
}

void FuncCallAST::analize() {
	is_compile_known_const = 0;// this can be modified
	is_bool = 0;
	has_block = 0;
}

void FuncCallAST::dump() {
	find_side_effect();

	analize();
	
	for(auto &exp: params) { // need to convert constant integer to register
		exp->dump();
		astout << exp->astout.str();
		exp->astout.str("");
		/*if(exp->is_compile_known_const) {
			astprintf("  %%%d = add 0, %d\n", ++used_id, exp->ret);
			exp->is_compile_known_const = 0;
			exp->ret = used_id;
		}*/
	}
	if(has_ret.find(make_pair(*s, (int)params.size())) != has_ret.end()) {
		ret = ++used_id;
		block_flush_global();
		astprintf("  %%%d = call @%s (", ret, s->c_str());
	}
	else {
		ret = -1;
		block_flush_global();
		astprintf("  call @%s (", s->c_str());
	}
		
	for(size_t i = 0; i < params.size(); i++) 
		astprintf("%s%s%d", i!=0?", ":"", &"%"[params[i]->is_compile_known_const], params[i]->ret);
	astprintf(")\n");
}

BlockAST::BlockAST(BaseAST *_state)
: state((StateAST*)_state) 
{}

void BlockAST::dump() {
	idt.resize(idt.size()+1);
	if(state) {
		state->dump();
		astout << state->astout.str();
		state->astout.str("");
	}
	idt.resize(idt.size()-1);
}

IfAST::IfAST(BaseAST *_exp, BaseAST *_condt, BaseAST *_condf)
: exp((ExpAST*)_exp), condt((StateAST*)_condt), condf((StateAST*)_condf)
{}

void IfAST::dump() {
	exp->dump();
	astout << exp->astout.str();
	exp->astout.str("");
	
	if(exp->is_compile_known_const) {
		if(exp->ret) {
			// if(1)
			condt->dump();
			astout << condt->astout.str();
			condt->astout.str("");
		}		
		else if(condf) {
			// if(0)
			condf->dump();
			astout << condf->astout.str();
			condf->astout.str("");
		}	
		return;
	}

	int L1 = ++label_id, L2 = ++label_id, L3 = condf ? ++label_id : 0;
	block_flush_all();
	astprintf("  br %%%d, %%next%d, %%label%d\n", exp->ret, L1, L2); // if(exp)
	astprintf("%%next%d:\n", L1);

	condt->dump();
	astout << condt->astout.str();
	condt->astout.str("");

	if(condf) {
		block_flush_all();
		astprintf("  jump %%label%d\n", L3);
		astprintf("%%label%d:\n", L2);

		condf->dump();
		astout << condf->astout.str();
		condf->astout.str("");

		block_flush_all();
		astprintf("  jump %%label%d\n", L3);
		astprintf("%%label%d:\n", L3);
	}
	else {
		block_flush_all();
		astprintf("  jump %%label%d\n", L2);
		astprintf("%%label%d:\n", L2);
	}
}

WhileAST::WhileAST(BaseAST *_exp, BaseAST *_state)
: exp((ExpAST*)_exp), state((StateAST*)_state)
{}

void WhileAST::dump() {

	block_flush_all();
	exp->dump();

	if(exp->is_compile_known_const && exp->ret == 0) // while(0)
		return;

	int L1 = ++label_id, L2 = ++label_id;

	
	astprintf("  jump %%label%d\n", L1);
	astprintf("%%label%d:\n", L1);

	astout << exp->astout.str();
	exp->astout.str("");

	if(exp->is_compile_known_const) {			// while(1)

		assert(exp->ret != 0);

		loopstk.push(make_pair(L1, L2));
		state->dump();
		astout << state->astout.str();
		state->astout.str("");
		loopstk.pop();

		block_flush_all();
		astprintf("  jump %%label%d\n", L1);
		astprintf("%%label%d:\n", L2);
		return;
	}

	int L3 = ++label_id;						// while(exp)

	block_flush_all();
	astprintf("  br %%%d, %%next%d, %%label%d\n", exp->ret, L2, L3);
	astprintf("%%next%d:\n", L2);

	loopstk.push(make_pair(L1, L3));
	state->dump();
	astout << state->astout.str();
	state->astout.str("");
	loopstk.pop();

	block_flush_all();
	astprintf("  jump %%label%d\n", L1);
	astprintf("%%label%d:\n", L3);
}

void BreakAST::dump() {
	block_flush_all();
	astprintf("  jump %%label%d\n", loopstk.top().second);
	astprintf("%%label%d:\n", ++label_id);
}

void ContinueAST::dump() {
	block_flush_all();
	astprintf("  jump %%label%d\n", loopstk.top().first);
	astprintf("%%label%d:\n", ++label_id);
}

InodeAST::InodeAST(BaseAST *_l, BaseAST *_r) 
: l((StateAST*)_l), r((StateAST*)_r)
{}

void InodeAST::dump() {
	if(l) {
		l->dump();
		astout << l->astout.str();
		l->astout.str("");
	}
	if(r) {
		r->dump();
		astout << r->astout.str();
		r->astout.str("");
	}
}

RetAST::RetAST(BaseAST *_exp) 
: exp((ExpAST*)_exp)
{}

void RetAST::dump() {
	if(exp) {
		exp->dump();
		astout << exp->astout.str();
		exp->astout.str("");
		block_flush_global();
		astprintf("  ret %s%d\n", &"%"[exp->is_compile_known_const], exp->ret);
	}
	else {
		block_flush_global();
		astprintf("  ret\n");
	}
	astprintf("%%label%d:\n", ++label_id);
}

DefIntAST::DefIntAST(string *_name, BaseAST *_exp, bool _decl_const, bool _decl_global) 
: name(_name), exp((ExpAST*)_exp), decl_const(_decl_const), decl_global(_decl_global)
{}

void DefIntAST::dump()
{
	if(exp) {
		exp->dump();
		astout << exp->astout.str();
		exp->astout.str("");

		if(exp->is_compile_known_const && decl_const) {
			idt.back()[*name] = (id_attr){exp->ret, true, false, decl_global};
			// not necessary to add it to global table
			return;
		}
	}
	idt.back()[*name] = (id_attr){++used_id, false, false, decl_global};
	if(decl_global) {
		assert(!exp || exp->is_compile_known_const);
		
		int val = exp?exp->ret:0;
		if(!exp) {
			astprintf("global @S%d = alloc i32, zeroinit\n", used_id);
		}
		else astprintf("global @S%d = alloc i32, %d\n", used_id, val);
		global_init_table[*name] = val;
	}
	else {
		astprintf("  @S%d = alloc i32\n", used_id);
		if(exp) {
			block_storeint(exp->is_compile_known_const, exp->ret, 0, used_id);
			/*
			astprintf("  store %s%d, @S%d\n", 
						&"%"[exp->is_compile_known_const], exp->ret, used_id);
			*/
		}
	}
}

AssignIntAST::AssignIntAST(string *_name, BaseAST *_exp) 
: name(_name), exp((ExpAST*)_exp)
{}

void AssignIntAST::dump() {
	exp->dump();
	astout << exp->astout.str();
	exp->astout.str("");
	
	id_attr a = envfind(*name);
	block_storeint(exp->is_compile_known_const, exp->ret, a.is_global, a.ret);
	//astprintf("  store %s%d, @S%d\n", &"%"[exp->is_compile_known_const], exp->ret, a.ret);
}

LvalExpAST::LvalExpAST(string *_name) 
: name(_name)
{}

void LvalExpAST::find_side_effect() {
	if(found) return;
	found = 1;
	treesize = 1;
	has_side_effect = 0;
}

void LvalExpAST::analize() {
	id_attr ida = envfind(*name);
	
	is_compile_known_const = in_global ? true : ida.is_compile_known_const;
	if(in_global) {
		auto ptr = global_init_table.find(*name);
		if(ptr != global_init_table.end()) {
			ret = ptr->second;
		}
	}
	if(ida.is_compile_known_const) {
		ret = ida.ret;
	}
	is_bool = 0;
	has_block = 0;
}

void LvalExpAST::dump() {
	find_side_effect();
	analize();
	
	if(in_global) {
		auto ptr = global_init_table.find(*name);
		if(ptr != global_init_table.end()) {
			ret = ptr->second;
			return;
		}
	}
	
	id_attr ida = envfind(*name);
	if(ida.is_compile_known_const) {
		ret = ida.ret;
		is_compile_known_const = 1;
		return;
	}
	
	assert(!in_global);
	
	
	
	if(ida.is_array) {
		
		// 这里是精髓中的精髓，我会根据局部的赋值来确定这个左值在局部代码中是否可以看成const(即使它没有被声明为const)

		if(aidt[ida.ret][0]==-1) {
			block_loadint(is_compile_known_const, ret, 0, ida.ret);// here, is_compile_known_const may change
		}
		else {
			int mid = ++used_id;
			astprintf("  %%%d = getptr @S%d, 0\n", mid, ida.ret);
			astprintf("  %%%d = getelemptr %%%d, 0\n", ++used_id, mid);
			ret = used_id;
		}
		return;
	}
	
	block_loadint(is_compile_known_const, ret, ida.is_global, ida.ret);
	//astprintf("  %%%d = load @S%d\n", ret, ida.ret);
}

DefArrayAST::DefArrayAST(string *_name, BaseAST* _size, BaseAST* _item, bool _decl_global)
: name(_name), size((ArraySizeAST*)_size), item((ArrayItemAST*)_item), decl_global(_decl_global)
{}

void DefArrayAST::dump() {
	
	size->dump();
	
	int ptr = ++used_id;
	idt.back()[*name] = (id_attr){ptr, false, true, decl_global};
	aidt[ptr] = size->intvec;
	
	if(decl_global) {
		astprintf("global @S%d = alloc ", ptr);
		printtype(size);
		astprintf(", ");
		if(item) printinit(item, size->intvec);
		else astprintf("zeroinit");
		astprintf("\n");
	}
	else {
		astprintf("  @S%d = alloc ", ptr);
		printtype(size);
		astprintf("\n");
		
		
		if(item) {
			astprintf("  store ");
			printinit(item, size->intvec);
			astprintf(", @S%d\n", ptr);
		}
	}
}

AssignIntOfArrayAST::AssignIntOfArrayAST(string *_name, BaseAST* _pos, BaseAST* _exp)
: name(_name), pos((ArraySizeAST*)_pos), exp((ExpAST*)_exp)
{}

void AssignIntOfArrayAST::dump() {
	id_attr ida = envfind(*name);
	vector<int> &size = aidt[ida.ret];
	
	exp->dump();
	astout << exp->astout.str();
	exp->astout.str("");
	
	assert(pos->expvec.size() == size.size());
	
	int ptr_id = getptr(ida.ret, pos->expvec, size);

	if(exp->is_compile_known_const) {
		astprintf("  %%%d = add 0, %d\n", ++used_id, exp->ret);
		astprintf("  store %%%d, %%%d\n", used_id, ptr_id);
	}
	else
		astprintf("  store %%%d, %%%d\n", exp->ret, ptr_id);
}

ArrayLvalExpAST::ArrayLvalExpAST(string *_name, BaseAST* _pos) 
: name(_name), pos((ArraySizeAST*)_pos)
{}

void ArrayLvalExpAST::find_side_effect() {
	if(found) return;
	found = 1;
	treesize = pos->expvec.size();
	has_side_effect = 0;
}

void ArrayLvalExpAST::analize() {
	is_compile_known_const = 0;
	is_bool = 0;
	has_block = 0;
}

void ArrayLvalExpAST::dump() {
	find_side_effect();
	analize();
	
	assert(!in_global);// "Lval" such as "A[i]" is regarded unkown during compiling 
	
	id_attr ida = envfind(*name);
	vector<int> &size = aidt[ida.ret];
	
	int ptr_id = getptr(ida.ret, pos->expvec, size);
	
	bool is_int = pos->expvec.size()==size.size();
	if(is_int) astprintf("  %%%d = load %%%d\n", ++used_id, ptr_id);
	else astprintf("  %%%d = getelemptr %%%d, 0\n", ++used_id, ptr_id);

	ret = used_id;
}

void GlobalAST::libdecl() {
	astprintf(
R"(decl @getint(): i32
decl @getch(): i32
decl @getarray(*i32): i32
decl @putint(i32)
decl @putch(i32)
decl @putarray(i32, *i32)
decl @starttime()
decl @stoptime()

)"
	);
	has_ret.insert(make_pair("getint", 0));
	has_ret.insert(make_pair("getch", 0));
	has_ret.insert(make_pair("getarray", 1));
}

void GlobalAST::dump() {
	libdecl();
	
	idt.resize(idt.size()+1);
	in_global = 1;
	for(auto &decl: decls) {
		decl->dump();
		astout << decl->astout.str();
		decl->astout.str("");
	}
	in_global = 0;
	astprintf("\n");
	for(auto &func: funcs) {
		func->dump();
		astout << func->astout.str();
		func->astout.str("");
	}
	idt.resize(idt.size()-1);
}