#include <cassert>
#include <cstdio>
#include <iostream>
#include <memory>
#include <string>
#include <unordered_map>
#include <algorithm>
#include <string.h>
#include "ast.hpp"
#include "mykoopa.h"
using namespace std;

typedef unordered_map<void*,int>  Table;

extern FILE *yyin;
extern int yyparse(BaseAST *&ast);

char mybuf[BUFSIZE];

ostream* output;

static ostream *riscvout;

#ifndef INT_MAX
const int INT_MAX = (1u<<31)-1;
#endif
// note that n_t != 7, because we use "t6" to convert big int
const int n_s = 12, n_t = 6, n_a = 8;


vector<string> global_link_funcs = {"main", "getch", "getarray", "putint", "putch", "putarray", "starttime", "stoptime"};

/*
 * 以下函数处理全局变量的定义
 */

int getarrsize(vector<int>&size, koopa_raw_type_t arr)
{
	int pi = 1;
	while(arr->tag == KOOPA_RTT_ARRAY) {
		int len = arr->data.array.len;
		pi *= len;
		size.push_back(len);
		arr = arr->data.array.base;
	}
	return pi;
}

void printarr(vector<int>&size, koopa_raw_value_t value, int offset, int level, int &preoffset)
{
	if(value->kind.tag == KOOPA_RVT_ZERO_INIT)
		return;
		
	if(value->ty->tag == KOOPA_RTT_INT32) {
		int offsetd = offset - preoffset;
		preoffset = offset + 1;
		if(offsetd) riscvprintf("    .zero   %d\n", 4*offsetd);
		riscvprintf("    .word   %d\n", value->kind.data.integer.value);
		return;
	}
	
	koopa_raw_slice_t list = value->kind.data.aggregate.elems;
	for(size_t i = 0; i < list.len; i++) 
		printarr(size, (koopa_raw_value_t)list.buffer[i], offset*size[level]+i, level+1, preoffset);
}

void solve_global_decl(koopa_raw_value_t decl)
{
	riscvprintf("%s:\n", decl->name+1); 
	koopa_raw_value_t value = decl->kind.data.global_alloc.init;
	if(value->ty->tag == KOOPA_RTT_INT32) {
		if(value->kind.tag == KOOPA_RVT_ZERO_INIT)
			riscvprintf("    .word   0\n");
		else
			riscvprintf("    .word   %d\n", value->kind.data.integer.value);
		return;
	}
	if(value->ty->tag == KOOPA_RTT_ARRAY) {
		vector<int>size;
		int totsize = getarrsize(size, value->ty);
		int preoffset = 0;
		printarr(size, value, 0, 0, preoffset);
		int offsetd = totsize - preoffset;
		if(offsetd) riscvprintf("    .zero   %d\n", 4*offsetd);
	}
}

void solve_global_decls(koopa_raw_slice_t &decls)
{
	riscvprintf("  .data\n");
	for(size_t i = 0; i < decls.len; i++)
		solve_global_decl((koopa_raw_value_t)decls.buffer[i]);
}

/*
 * 以下函数用于公共子表达式消除
 */

bool value_eq(koopa_raw_value_t vi, koopa_raw_value_t vj)
{
	return 	vi == vj ||
		(	vi->kind.tag == KOOPA_RVT_INTEGER &&
			vj->kind.tag == KOOPA_RVT_INTEGER &&
			vi->kind.data.integer.value == vj->kind.data.integer.value
		);
}

// 判断两个指令的返回值等价，用于做公共子表达式的消除
bool equal_inst(koopa_raw_value_kind_t &kindi, koopa_raw_value_kind_t &kindj)
{
	koopa_raw_value_tag_t tagi = kindi.tag, tagj = kindj.tag;

	if(tagi != tagj) return 0;
	if(tagi == KOOPA_RVT_GET_PTR) {
		koopa_raw_get_ptr_t &getptri = kindi.data.get_ptr, &getptrj = kindj.data.get_ptr;
		if(! (value_eq(getptri.src, getptrj.src) && value_eq(getptri.index, getptrj.index))	) 
			return 0;
	}
	else if(tagi == KOOPA_RVT_GET_ELEM_PTR) {
		koopa_raw_get_elem_ptr_t &getelemptri = kindi.data.get_elem_ptr, &getelemptrj = kindj.data.get_elem_ptr;
		if(! (value_eq(getelemptri.src, getelemptrj.src) && value_eq(getelemptri.index, getelemptrj.index))	) 
			return 0;
	}
	else if(tagi == KOOPA_RVT_BINARY) { // KOOPA_RVT_BINARY
		koopa_raw_binary_t &bi = kindi.data.binary, &bj= kindj.data.binary;
		if(bi.op != bj.op) return 0;
		if(bi.op == KOOPA_RBO_NOT_EQ || bi.op == KOOPA_RBO_EQ
			 || bi.op == KOOPA_RBO_ADD || bi.op == KOOPA_RBO_MUL
			 || bi.op == KOOPA_RBO_AND || bi.op == KOOPA_RBO_OR) {
			if( ! ((value_eq(bi.lhs, bj.lhs) && value_eq(bi.rhs, bj.rhs)) || (value_eq(bi.lhs, bj.rhs) && value_eq(bi.rhs, bj.lhs))) )
				return 0;
		}
		else {
			if( ! (value_eq(bi.lhs, bj.lhs) && value_eq(bi.rhs, bj.rhs)) )
				return 0;
		}
	}
	else assert(0);

	return 1;
}

// 替换等价的表达式的返回值
void replace(koopa_raw_value_t instold, koopa_raw_value_t instnew)
{
	for(size_t i = 0; i < instold->used_by.len; i++) {
		koopa_raw_value_t inst = (koopa_raw_value_t)instold->used_by.buffer[i];
		koopa_raw_value_kind_t &kind = inst->kind;
		koopa_raw_value_tag_t tag = kind.tag;

		if(tag == KOOPA_RVT_LOAD) {
			koopa_raw_load_t &ld = kind.data.load;
			if(ld.src == instold)
				ld.src = instnew;
			else assert(0);
		}
		else if(tag == KOOPA_RVT_STORE) {
			koopa_raw_store_t &st = kind.data.store;
			if(st.value == instold)
				st.value = instnew;
			else if(st.dest == instold)
				st.dest = instnew;
			else assert(0);
		}
		else if(tag == KOOPA_RVT_GET_PTR) {
			koopa_raw_get_ptr_t &gp = kind.data.get_ptr;
			if(gp.src == instold)
				gp.src = instnew;
			else if(gp.index == instold)
				gp.index = instnew;
			else assert(0);
		}
		else if(tag == KOOPA_RVT_GET_ELEM_PTR) {
			koopa_raw_get_elem_ptr_t &gep = kind.data.get_elem_ptr;
			if(gep.src == instold)
				gep.src = instnew;
			else if(gep.index == instold)
				gep.index = instnew;
			else assert(0);
		}
		else if(tag == KOOPA_RVT_BINARY) {
			koopa_raw_binary_t &bin = kind.data.binary;
			bool has = 0;
			if(bin.lhs == instold)
				bin.lhs = instnew, has = 1;
			if(bin.rhs == instold) // don't use "else" 
				bin.rhs = instnew, has = 1;
			assert(has);
		}
		else if(tag == KOOPA_RVT_BRANCH) {
			koopa_raw_branch_t &br = kind.data.branch;
			if(br.cond == instold)
				br.cond = instnew;
			else assert(0);
		}
		else if(tag == KOOPA_RVT_CALL) {
			koopa_raw_call_t &cll = kind.data.call;
			bool has = 0;
			koopa_raw_slice_t &slice = cll.args;
			for(size_t l = 0; l < slice.len; l++) 
				if(slice.buffer[l] == instold)
					slice.buffer[l] = instnew, has = 1;
			assert(has);
		}
		else if(tag == KOOPA_RVT_RETURN) {
			koopa_raw_return_t &ret = kind.data.ret;
			if(ret.value == instold)
				ret.value = instnew;
			else assert(0);
		}
		else assert(0);
	}
}

// 块内的公共子表达式消除，仅针对getptr,getelemptr,binrary，不包含load是因为之前在koopa消减了重复load
void shrink_block(koopa_raw_basic_block_t bp)
{
	koopa_raw_slice_t &insts = bp->insts;

	bool *del = new bool[insts.len];
	memset(del, 0, insts.len);

	for(size_t i = 0; i < insts.len; i++) {

		koopa_raw_value_t instpi = (koopa_raw_value_t)insts.buffer[i];
		koopa_raw_value_kind_t &kindi = instpi->kind;
		koopa_raw_value_tag_t tagi = kindi.tag;

		if(tagi == KOOPA_RVT_ALLOC)
			del[i] = 1;

		if(del[i]) continue;

		// load/store has been optimized in our design of AST.
		if(tagi != KOOPA_RVT_GET_PTR && tagi != KOOPA_RVT_GET_ELEM_PTR && tagi != KOOPA_RVT_BINARY) 
				continue;

		for(size_t j = i+1; j < insts.len; j++) {
			
			if(del[j]) continue;

			// if inst[i] != inst[j] then continue
			koopa_raw_value_t instpj = (koopa_raw_value_t)insts.buffer[j];
			koopa_raw_value_kind_t &kindj = instpj->kind;

			if(!equal_inst(kindi, kindj)) continue;
			// now these two expressions are equal

			del[j] = 1;
			// replace instj with insti
			replace(instpj, instpi);			
		}
	}

	// note that we didn't modify field 'used_by' in 'koopa_raw_value_t', so don't use it anymore
	int new_len = 0;
	for(size_t i = 0; i < bp->insts.len; i++) 
		if(!del[i]) insts.buffer[new_len++] = insts.buffer[i];
	// we will lose some memory, however we don't care that.
	insts.len = new_len;
	delete [] del;	
}

/*
 * 以下函数用于生成目标代码
 */

struct reg{
	int nxt_use;
	bool used;
	bool locked;
	char type;
	int subid;
	void *value;
	string name() {
		string ret;
		ret += type;
		if(subid < 10) ret += '0'+subid;
		else ret += '1', ret += '0'+(subid-10);
		return ret;
	}
};

string i2s(int x) {
	static char buf[20];
	sprintf(buf, "%d", x);
	return string(buf);
}

// unused
void type_def_str(koopa_raw_type_t type, string &ret) 
{
	if(type->tag == KOOPA_RTT_INT32) {
		ret += string("I");
	}
	else if(type->tag == KOOPA_RTT_POINTER) {
		ret += "Pof";
		type_def_str(type->data.pointer.base, ret);
	}
	else if(type->tag == KOOPA_RTT_ARRAY) {
		ret += "A" + i2s(type->data.array.len) + "of";
		type_def_str(type->data.array.base, ret);
	}
	else assert(0);
}

string func_def_str(koopa_raw_function_t funcp)
{
	string ret;
	ret += funcp->name + 1;
	return ret;

	// unused
	koopa_raw_slice_t &params = funcp->ty->data.function.params;
	for(size_t i = 0; i < params.len; i++) {
		ret += "_";
		string type;
		type_def_str((koopa_raw_type_t)params.buffer[i], type);
		ret += type;
	}
	return ret;
}

// 查询一个KOOPA IR变量下一次被使用时所在的KOOPA指令编号
int time_of_next_use(koopa_raw_value_t value, koopa_raw_slice_t &insts, int start)
{
	for(size_t pos = start; pos < insts.len; pos ++) {
		koopa_raw_value_t inst = (koopa_raw_value_t)insts.buffer[pos];
		koopa_raw_value_kind_t &kind = inst->kind;
		koopa_raw_value_tag_t tag = kind.tag;
		switch(tag) {
			case KOOPA_RVT_ALLOC:
				break;
			case KOOPA_RVT_LOAD:
				if(kind.data.load.src == value)
					return pos;
				break;
			case KOOPA_RVT_STORE:
				if(kind.data.store.value == value || kind.data.store.dest == value)
					return pos;
				break;
			case KOOPA_RVT_GET_PTR:
				if(kind.data.get_ptr.src == value || kind.data.get_ptr.index == value)
					return pos;
				break;
			case KOOPA_RVT_GET_ELEM_PTR:
				if(kind.data.get_elem_ptr.src == value || kind.data.get_elem_ptr.index == value)
					return pos;
				break;
			case KOOPA_RVT_BINARY:
				if(kind.data.binary.lhs == value || kind.data.binary.rhs == value)
					return pos;
				break;
			case KOOPA_RVT_BRANCH:
				if(kind.data.branch.cond == value)
					return pos;
				break;
			case KOOPA_RVT_JUMP:
				break;
			case KOOPA_RVT_CALL:
				for(size_t j = 0; j < kind.data.call.args.len; j++) 
					if(kind.data.call.args.buffer[j] == value)
						return pos;
				break;
			case KOOPA_RVT_RETURN:
				if(inst->kind.data.ret.value == value)// this can be NULL
					return pos;
				break;
			default:
				assert(0);
		}
	}
	return -1;
}

// 生成初始化局部数组的目标代码
void array_init(koopa_raw_value_t agg, size_t pos, vector<int>&size, 
				size_t level, size_t offset, const string &r, vector<vector<string> > &block_insts)
{
	if(agg->kind.tag == KOOPA_RVT_AGGREGATE) {
		koopa_raw_slice_t &slice = agg->kind.data.aggregate.elems;
		assert(level < size.size() && (size_t)size[level] == slice.len);
		for(int i = 0; i < size[level]; i++) {
			array_init((koopa_raw_value_t)slice.buffer[i], pos, size, level+1, offset*size[level]+i, r, block_insts);
		}
	}
	else if(agg->kind.tag == KOOPA_RVT_ZERO_INIT) {
		assert(level <= size.size());
		if(level == size.size()) {
			// 0
			block_insts.push_back({"sw", "zero", i2s(pos+4*offset)+"(fp)"});
		}
		else {
			for(int i = 0; i < size[level]; i++) {
				array_init(agg, pos, size, level+1, offset*size[level]+i, r, block_insts);
			}
		}
	}
	else if(agg->kind.tag == KOOPA_RVT_INTEGER) {
		assert(level == size.size());
		block_insts.push_back({"li", r, i2s(agg->kind.data.integer.value)});
		block_insts.push_back({"sw", r, i2s(pos+4*offset)+"(fp)"});
	}
}

/*
 * 这个函数的语义如下：
 *     查找KOOPA IR的变量value当前的位置，若位于寄存器中，返回对应寄存器；
 *     若不在寄存器，而在内存中，加载至寄存器，并返回；
 *     若均不在，则说明变量value是第一次出现，申请一个寄存器，返回。
 *     注意如果需要使用被占用的寄存器，那么会首先将当前该寄存器的值压栈。
 *     如果value为NULL，说明不是KOOPA的变量，而是生成目标代码需要一个临时寄存器。
 *     若pos_spec!=-1，那么如果变量不在寄存器中，加载至寄存器时就会优先选用reg_set[pos_spec]。
 *     若pos_spec==-1，那么如果变量不在寄存器中，加载至寄存器时会优先选择编号小且未使用的
 *     寄存器，若寄存器均被占用，则会选择下一次使用最晚的寄存器并将其压栈。
 */
reg* get_reg(koopa_raw_value_t			value,
			int  						pos_spec,
			koopa_raw_slice_t 			&insts,
			vector<reg> 				&reg_set, 
			int 						tm, 
			int							sp_base,
			vector<int>					&stk_usage,
			Table 						&addr_of_v,
			vector<vector<string> >		&block_insts)
{
	// find an register to use
	if(value && value->kind.tag == KOOPA_RVT_INTEGER)
		value = NULL;

	int ret = -1, type = 1;
	int unuse_id = -1, max_nxt_id = -1;
	for(size_t i = 0; i < reg_set.size(); i++) {
		if(value && reg_set[i].value == value) {
			ret = i;
			break;
		}
		if(reg_set[i].locked)
			continue;
		int nxt_use = reg_set[i].nxt_use;
		if(unuse_id == -1 && nxt_use < tm) {
			unuse_id = i;
		}
		if(max_nxt_id == -1 || nxt_use > reg_set[max_nxt_id].nxt_use)
			max_nxt_id = i;
	}
	if(ret == -1 && pos_spec != -1) {
		ret = pos_spec;
		type = reg_set[pos_spec].nxt_use < tm ? 2 : 3;
	}
	if(ret == -1) ret = unuse_id, type = 2;
	if(ret == -1) ret = max_nxt_id, type = 3;
	assert(ret != -1);
	
		

	reg *retp = &reg_set[ret];

	if(type == 3 && retp->value && addr_of_v.find(retp->value) == addr_of_v.end()) {
		// find a place in stack to put the old register
		size_t pos;
		for(pos = 0; pos < stk_usage.size(); pos++)
			if(stk_usage[pos] < tm)
				break;
		if(pos == stk_usage.size()) 
			stk_usage.push_back(0);
		stk_usage[pos] = retp->nxt_use; // pos is stack_id

		int offset = sp_base - (pos + 1) * 4; // calculate real address

		addr_of_v[retp->value] = offset;

		block_insts.push_back({"sw", retp->name(), i2s(offset)+"(fp)"});
	}

	int nxt_time = value ? time_of_next_use(value, insts, tm+1) : -1; // if cannot find next time, return tm

	auto p = addr_of_v.find(value);
	if(p != addr_of_v.end()) {
		int stack_id = (sp_base - p->second) / 4 - 1;

		// stack_id < 0 means it's a parameter
		if(stack_id >= 0) stk_usage[stack_id] = nxt_time;// update time stamp in stack

		if(type != 1) // need to load value from memory
			block_insts.push_back({"lw", retp->name(), i2s(p->second)+"(fp)"});
	}

	// update "value" in register to replace "reg_set[ret].value"
	retp->nxt_use = nxt_time;
	retp->used = 1;
	retp->locked = 1;
	retp->value = value; 

	return retp;
}

// 生成块的目标代码
int gen_riscv(	koopa_raw_basic_block_t 	bp, 
				vector<reg>					&reg_set, 
				int  						pos_a0,
				Table 						&local_const_addr, 
				Table 						&addr_of_v,
				int  						sp_base,
				vector<vector<string> >		&block_insts)
{

	//指令从1开始编号，覆盖编号i表示覆盖[i,i+1]指令区间
	koopa_raw_slice_t &insts = bp->insts;
	vector<int>stk_usage; // "int" means the time out of date
	block_insts.reserve(2*insts.len);
	int call_stk_use = 0;
	for(size_t i = 0; i < insts.len; i++) {
		koopa_raw_value_t inst = (koopa_raw_value_t)insts.buffer[i];
		koopa_raw_value_kind_t &kind = inst->kind;
		koopa_raw_value_tag_t tag = kind.tag;
	
		koopa_raw_value_t ptr1, ptr2;
		bool load1, load2, loadt;
		string instname, s1, s2, t, addition_instname;
		reg *ps1, *ps2, *pt;

		load1 = load2 = loadt = false;
		ps1 = ps2 = pt = NULL;

		for(auto &rg: reg_set) rg.locked = 0; // unlock

		switch(tag) {
			case KOOPA_RVT_ALLOC:
				break;
			case KOOPA_RVT_LOAD:// lw
				
				ptr1 = kind.data.load.src;
				assert(ptr1->kind.tag != KOOPA_RVT_GLOBAL_ALLOC);

				instname = "lw";

				pt = get_reg(inst, -1, insts, reg_set, i, sp_base, stk_usage, addr_of_v, block_insts);

				if(ptr1->kind.tag == KOOPA_RVT_ALLOC) {
					s1 = i2s(local_const_addr[ptr1])+"(fp)";
				}
				else {
					ps1 = get_reg(ptr1, -1, insts, reg_set, i, sp_base, stk_usage, addr_of_v, block_insts);

					s1 = string("0(") + ps1->name() + ")";

					ps1 = NULL;
				}
				break;
			case KOOPA_RVT_STORE:// sw 可能有常量，可能有数组整个赋值

				ptr1 = kind.data.store.value;
				ptr2 = kind.data.store.dest;
				assert(ptr1->kind.tag != KOOPA_RVT_INTEGER);

				if(ptr1->kind.tag == KOOPA_RVT_ZERO_INIT || ptr1->kind.tag == KOOPA_RVT_AGGREGATE) {// TODO
					assert(ptr2->kind.tag == KOOPA_RVT_ALLOC);
					vector<int>size;
					reg *pr = get_reg(NULL, -1, insts, reg_set, i, sp_base, stk_usage, addr_of_v, block_insts);
					// TODO may need 2 regs
					getarrsize(size, ptr2->ty->data.pointer.base);
					array_init(ptr1, local_const_addr[ptr2], size, 0, 0, pr->name(), block_insts);
				}
				else if(ptr2->kind.tag == KOOPA_RVT_ALLOC) {
					instname = "sw";

					ps1 = get_reg(ptr1, -1, insts, reg_set, i, sp_base, stk_usage, addr_of_v, block_insts);

					s2 = i2s(local_const_addr[ptr2])+"(fp)";
				}
				else {
					
					assert(ptr2->kind.tag != KOOPA_RVT_GLOBAL_ALLOC && ptr2->kind.tag != KOOPA_RVT_ALLOC);

					instname = "sw"; 

					ps1 = get_reg(ptr1, -1, insts, reg_set, i, sp_base, stk_usage, addr_of_v, block_insts);

					ps2 = get_reg(ptr2, -1, insts, reg_set, i, sp_base, stk_usage, addr_of_v, block_insts);
					s2 = string("0(") + ps2->name() + ")";

					ps2 = NULL;
				} 
				break;
			case KOOPA_RVT_GET_PTR:// add

				pt = get_reg(inst, -1, insts, reg_set, i, sp_base, stk_usage, addr_of_v, block_insts);

				ptr1 = kind.data.get_ptr.src;
				ptr2 = kind.data.get_ptr.index;

				if(ptr1->kind.tag == KOOPA_RVT_GLOBAL_ALLOC) {		
					assert(ptr2->kind.tag == KOOPA_RVT_INTEGER);
					assert(ptr2->kind.data.integer.value == 0);

					instname = "la";

					s1 = ptr1->name + 1;
				}
				else if(ptr1->kind.tag == KOOPA_RVT_ALLOC) {
					assert(ptr2->kind.tag == KOOPA_RVT_INTEGER);
					assert(ptr2->kind.data.integer.value == 0);
					
					instname = "addi";

					s1 = "fp";

					int offset = local_const_addr[ptr1];
					s2 = i2s(offset);
				}
				else if(ptr2->kind.tag == KOOPA_RVT_INTEGER) {
					instname = "addi";
					
					ps1 = get_reg(ptr1, -1, insts, reg_set, i, sp_base, stk_usage, addr_of_v, block_insts);

					vector<int>_;
					int offset = ptr2->kind.data.integer.value * 4 * getarrsize(_, ptr1->ty->data.pointer.base);

					s2 = i2s(offset);
				}
				else {
					instname = "add";
					
					ps2 = get_reg(ptr2, -1, insts, reg_set, i, sp_base, stk_usage, addr_of_v, block_insts);

					vector<int>_;
					int size = 4 * getarrsize(_, ptr1->ty->data.pointer.base);

					block_insts.push_back({"li", pt->name(), i2s(size)});

					block_insts.push_back({"mul", pt->name(), pt->name(), ps2->name()});

					ps1 = get_reg(ptr1, -1, insts, reg_set, i, sp_base, stk_usage, addr_of_v, block_insts);

					instname = "add";

					ps2 = pt;
				}
				break;
			case KOOPA_RVT_GET_ELEM_PTR:

				pt = get_reg(inst, -1, insts, reg_set, i, sp_base, stk_usage, addr_of_v, block_insts);

				ptr1 = kind.data.get_elem_ptr.src;
				ptr2 = kind.data.get_elem_ptr.index;

				if(ptr2->kind.tag == KOOPA_RVT_INTEGER) {

					instname = "addi";
					
					ps1 = get_reg(ptr1, -1, insts, reg_set, i, sp_base, stk_usage, addr_of_v, block_insts);

					vector<int>_;
					int offset = ptr2->kind.data.integer.value * 4 * 
									getarrsize(_, ptr1->ty->data.pointer.base->data.array.base);

					s2 = i2s(offset);
				}
				else {
					instname = "add";
					
					ps2 = get_reg(ptr2, -1, insts, reg_set, i, sp_base, stk_usage, addr_of_v, block_insts);

					vector<int>_;
					int size = 4 * getarrsize(_, ptr1->ty->data.pointer.base->data.array.base);

					block_insts.push_back({"li", pt->name(), i2s(size)});

					block_insts.push_back({"mul", pt->name(), pt->name(), ps2->name()});

					ps1 = get_reg(ptr1, -1, insts, reg_set, i, sp_base, stk_usage, addr_of_v, block_insts);

					instname = "add";

					ps2 = pt;
				}
				break;
			case KOOPA_RVT_BINARY:// bin
			{
				pt = get_reg(inst, -1, insts, reg_set, i, sp_base, stk_usage, addr_of_v, block_insts);

				ptr1 = kind.data.binary.lhs;
				ptr2 = kind.data.binary.rhs;
				int type = (int)(ptr1->kind.tag == KOOPA_RVT_INTEGER) | (int)(ptr2->kind.tag == KOOPA_RVT_INTEGER) << 1;
				if(type == 3) {
					assert(kind.data.binary.op == KOOPA_RBO_ADD);
					int res = ptr1->kind.data.integer.value + ptr2->kind.data.integer.value;
					instname = "li";
					s1 = i2s(res);
					break;
				}
				int intv = type == 1 ? ptr1->kind.data.integer.value : 
							type == 2 ? ptr2->kind.data.integer.value : 0;
				
				int exchangeable = 0;
				switch(inst->kind.data.binary.op) {
					case KOOPA_RBO_ADD:
					case KOOPA_RBO_AND:
					case KOOPA_RBO_OR:
					case KOOPA_RBO_NOT_EQ:
					case KOOPA_RBO_EQ:
					case KOOPA_RBO_MUL:
						if(type == 1) {
							swap(ptr1, ptr2);
							swap(kind.data.binary.lhs, kind.data.binary.rhs);
							type = 2;
						}
						exchangeable = 1;
				}

				if(exchangeable) {
					switch(kind.data.binary.op) {
						case KOOPA_RBO_ADD:
							instname = "add";
							break;
						case KOOPA_RBO_AND:
							instname = "and";
							break;
						case KOOPA_RBO_OR:
							instname = "or";
							break;
						case KOOPA_RBO_NOT_EQ:// xor/xori + snez
							instname = "xor";
							addition_instname = "snez";
							break;
						case KOOPA_RBO_EQ:// xor/xori + seqz
							instname = "xor";
							addition_instname = "seqz";
							break;
						case KOOPA_RBO_MUL:
							instname = "mul";
							break;
					}

					ps1 = get_reg(ptr1, -1, insts, reg_set, i, sp_base, stk_usage, addr_of_v, block_insts);

					if(type == 0)
						ps2 = get_reg(ptr2, -1, insts, reg_set, i, sp_base, stk_usage, addr_of_v, block_insts);
					else {
						s2 = i2s(intv);
						if(kind.data.binary.op == KOOPA_RBO_MUL) {
							ps2 = pt;
							load2 = 1;
						}
						else instname += 'i';
					}
				}
				else {
					switch(kind.data.binary.op) {
						case KOOPA_RBO_GE:
							addition_instname = "xori";
						case KOOPA_RBO_LT:
							
							if(type == 0) {
								instname = "slt";
								ps1 = get_reg(ptr1, -1, insts, reg_set, i, sp_base, stk_usage, addr_of_v, block_insts);
								ps2 = get_reg(ptr2, -1, insts, reg_set, i, sp_base, stk_usage, addr_of_v, block_insts);
							}
							else if(type == 1) {
								instname = "slt";
								load1 = 1;
								s1 = i2s(intv);
								ps1 = get_reg(ptr1, -1, insts, reg_set, i, sp_base, stk_usage, addr_of_v, block_insts);
								ps2 = get_reg(ptr2, -1, insts, reg_set, i, sp_base, stk_usage, addr_of_v, block_insts);
							}
							else {// type == 2
								instname = "slti";
								ps1 = get_reg(ptr1, -1, insts, reg_set, i, sp_base, stk_usage, addr_of_v, block_insts);
								s2 = i2s(intv);
							}
							break;
			
						case KOOPA_RBO_SUB:
							if(type == 0) {
								instname = "sub";
								ps1 = get_reg(ptr1, -1, insts, reg_set, i, sp_base, stk_usage, addr_of_v, block_insts);
								ps2 = get_reg(ptr2, -1, insts, reg_set, i, sp_base, stk_usage, addr_of_v, block_insts);
							}
							else if(type == 1) {
								loadt = 1;
								t = i2s(intv);

								instname = "sub";
								ps1 = pt;
								ps2 = get_reg(ptr2, -1, insts, reg_set, i, sp_base, stk_usage, addr_of_v, block_insts);
							}
							else {
								instname = "addi";
								ps1 = get_reg(ptr1, -1, insts, reg_set, i, sp_base, stk_usage, addr_of_v, block_insts);
								s2 = i2s(-intv);
							}
							// s1 - s2, sub t, s1, s2;
							// 3 - s, li t, 3; sub t, t, s;
							// s - 1, addi t, s, -1
							break;
						case KOOPA_RBO_DIV:// li + div
						case KOOPA_RBO_MOD:
							instname = kind.data.binary.op == KOOPA_RBO_DIV ? "div" : "rem";
							ps1 = get_reg(ptr1, -1, insts, reg_set, i, sp_base, stk_usage, addr_of_v, block_insts);
							ps2 = get_reg(ptr2, -1, insts, reg_set, i, sp_base, stk_usage, addr_of_v, block_insts);
							if(type & 1) s1 = i2s(intv), load1 = 1;
							if(type & 2) s2 = i2s(intv), load2 = 1;
							break;
						
						default:
							assert(0);
							break;
					}
				}
				break;
			}
			case KOOPA_RVT_BRANCH:// 
			{
				koopa_raw_branch_t &br = kind.data.branch;

				instname.resize(0);

				ptr1 = br.cond;
				reg *pr = get_reg(ptr1, -1, insts, reg_set, i, sp_base, stk_usage, addr_of_v, block_insts);
				if(br.true_bb->name[1] == 'l') { // bnez

					/*instname = "bnez";
					s2 = br.true_bb->name;
					s2[0] = '.';*/
					block_insts.push_back({"bnez", pr->name(), string(".")+(br.true_bb->name+1)});
					block_insts.push_back({"j", string(".")+(br.false_bb->name+1)});
				}
				else {// beqz
					/*instname = "beqz";
					s2 = br.false_bb->name;
					s2[0] = '.';*/
					block_insts.push_back({"beqz", pr->name(), string(".")+(br.false_bb->name+1)});
					block_insts.push_back({"j", string(".")+(br.true_bb->name+1)});
				}
				// beqz/bnez value, label
				// 用 label的名字来判断那个是next吧

				break;
			}
			case KOOPA_RVT_JUMP:
				// j label
				instname = "j";
				s1 = kind.data.jump.target->name;
				s1[0] = '.';
				break;
			case KOOPA_RVT_CALL:
			{	
				instname.resize(0);

				koopa_raw_call_t &cll = kind.data.call;
				koopa_raw_function_t callee = cll.callee;
				bool has_ret_val = callee->ty->data.function.ret->tag == KOOPA_RTT_INT32;

				for(size_t narg = 0; narg < cll.args.len && narg < n_a; narg++) {
					koopa_raw_value_t ptr = (koopa_raw_value_t)cll.args.buffer[narg];
					int pos_ai = pos_a0 - narg;
					reg *arg_s = get_reg(ptr, pos_ai, insts, reg_set, i, sp_base, stk_usage, addr_of_v, block_insts);

					if(ptr->kind.tag == KOOPA_RVT_INTEGER) {
						assert(arg_s == &reg_set[pos_ai]);
						block_insts.push_back({"li", arg_s->name(), i2s(ptr->kind.data.integer.value)});
					}

					if(arg_s == &reg_set[pos_ai]) {
						//riscvprintf("match %d\n", narg);
						//if(narg == 1)
						//	riscvprintf("%s\n", cll.args.buffer[1]==cll.args.buffer[0] ? "eq":"ne");
						continue;
					}

					reg *arg_t = get_reg(NULL, pos_ai, insts, reg_set, i, sp_base, stk_usage, addr_of_v, block_insts);
					
					assert(arg_t == &reg_set[pos_ai]);

					block_insts.push_back({"mv", arg_t->name(), arg_s->name()});

					if(arg_s - &reg_set[0] > pos_a0 || arg_s - &reg_set[0] < pos_ai) 
						arg_s->locked = 0;
					// lock所有会用到的ai寄存器，其余unlock
				}
				// 栈中参数
				for(size_t narg = n_a; narg < cll.args.len; narg++) {
					koopa_raw_value_t ptr = (koopa_raw_value_t)cll.args.buffer[narg];
					reg *arg_s = get_reg(ptr, -1, insts, reg_set, i, sp_base, stk_usage, addr_of_v, block_insts);

					if(ptr->kind.tag == KOOPA_RVT_INTEGER) {
						block_insts.push_back({"li", arg_s->name(), i2s(ptr->kind.data.integer.value)});
						//arg_s->locked = 0;
					}

					block_insts.push_back({"sw", arg_s->name(), i2s(4*(narg-n_a))+"(sp)"});

					arg_s->locked = 0;
				}

				// 取消所有调用者保存寄存器的使用，push调用者保存的寄存器
				for(size_t r_id = 0; r_id < reg_set.size(); r_id ++) {
					if(reg_set[r_id].type != 's' && (size_t)reg_set[r_id].nxt_use > i) {
						reg *pr = get_reg(NULL, r_id, insts, reg_set, i, sp_base, stk_usage, addr_of_v, block_insts);
						pr->locked = 0;
					}
				}
				// 将a0的所有者设为此函数返回值
				if(has_ret_val) get_reg(inst, pos_a0, insts, reg_set, i, sp_base, stk_usage, addr_of_v, block_insts);

				block_insts.push_back({"call", func_def_str(callee)});

				int stk_use = cll.args.len - n_a;
				if(stk_use > call_stk_use) call_stk_use = stk_use;

				break;
			}	
			case KOOPA_RVT_RETURN:
				instname = "ret";

				ptr1 = kind.data.ret.value;
				if(ptr1) {
					if(ptr1->kind.tag == KOOPA_RVT_INTEGER) {
						reg *pr = get_reg(NULL, pos_a0, insts, reg_set, i, sp_base, stk_usage, addr_of_v, block_insts);
						assert(pr == &reg_set[pos_a0]);
						block_insts.push_back({"li", "a0", i2s(ptr1->kind.data.integer.value)});
					}
					else {
						reg *pr = get_reg(ptr1, pos_a0, insts, reg_set, i, sp_base, stk_usage, addr_of_v, block_insts);
						if(ps1 != &reg_set[pos_a0]) 
							block_insts.push_back({"mv", "a0", pr->name()});
					}
				}

				// this is NULL or in register a0
				break;
			default:
				assert(0);
		}
		if(instname.empty()) continue;
		// 如果上面的instname不为空，则说明需要下面的代码辅助输出
		// 若为空，则说明上面已经处理完毕，不需要下面的代码
		// 主要是二元运算在用下面的代码

		if(loadt) block_insts.push_back({"li", pt->name(), t});
		if(load1) block_insts.push_back({"li", ps1->name(), s1});
		if(load2) block_insts.push_back({"li", ps2->name(), s2});

		vector<string>inststr;
		inststr.push_back(instname);
		if(pt) inststr.push_back(pt->name());
		if(ps1) inststr.push_back(ps1->name());
		else if(!s1.empty()) inststr.push_back(s1);
		if(ps2) inststr.push_back(ps2->name());
		else if(!s2.empty()) inststr.push_back(s2);

		block_insts.push_back(inststr);

		if(!addition_instname.empty()) {
			if(addition_instname == "xori")
				block_insts.push_back({addition_instname, pt->name(), pt->name(), "1"});
			else 
				block_insts.push_back({addition_instname, pt->name(), pt->name()});
		}
	} 

	return sp_base - 4 * (stk_usage.size() + call_stk_use);
}



void solve_func(koopa_raw_function_t func)
{
	bool has_call = 0;

	// alloc address for alloced variable in stack
	int sp = 0;
	Table local_const_addr;

	if(!func->bbs.len) {
		//fprintf(stderr, "skip\n");
		return;
	}
	riscvprintf("%s:\n", func_def_str(func).c_str());
	

	// 分配koopa IR中alloc变量的栈地址

	for(size_t i = 0; i < func->bbs.len; i++) {
		koopa_raw_basic_block_t bp = (koopa_raw_basic_block_t)func->bbs.buffer[i];
		koopa_raw_slice_t &insts = bp->insts;
		for(size_t j = 0; j < insts.len; j++) {
			koopa_raw_value_t instp = (koopa_raw_value_t)insts.buffer[j];
			if(instp->kind.tag == KOOPA_RVT_ALLOC) {
				vector<int>unused;
				int size = 4*getarrsize(unused, instp->ty->data.pointer.base);
				sp -= size;
				local_const_addr[instp] = sp;
			}
			if(instp->kind.tag == KOOPA_RVT_CALL)
				has_call = 1;
		}
	}
	
	// 初始化寄存器集合

	vector<reg>reg_set, tmp;
	// prefer to use registers on the left
	int pos_a0;

	for(int i = 0; i < n_s; i++)
		tmp.push_back((reg){-1, false, false, 's', i, NULL});
	for(int i = 0; i < n_t; i++)
		reg_set.push_back((reg){-1, false, false, 't', i, NULL});
	for(int i = n_a-1; i >= 0; i--)
		reg_set.push_back((reg){-1, false, false, 'a', i, NULL});
	if(has_call) {// s t a 
		reg_set.insert(reg_set.begin(), tmp.begin(), tmp.end());
		pos_a0 = reg_set.size()-1;
	}
	else {// t a s
		pos_a0 = reg_set.size()-1;
		reg_set.insert(reg_set.end(), tmp.begin(), tmp.end());
	}

	// 对于第一个块，初始化函数参数，包括寄存器和栈中的参数

	Table addr_of_v;
	int n_stk_params = 0;
	for(size_t i = 0; i < func->params.len; i++) {
		koopa_raw_value_t param = (koopa_raw_value_t)func->params.buffer[i];
		if(i < n_a) {
			koopa_raw_basic_block_t b0p = (koopa_raw_basic_block_t)func->bbs.buffer[0];
			reg *rp = &reg_set[pos_a0 - i];
			rp->nxt_use = time_of_next_use(param, b0p->insts, 0);
			rp->value = param;

			assert(reg_set[pos_a0 - i].nxt_use != -1);// because we store all parameters
		}
		else {
			addr_of_v[param] = n_stk_params++ * 4;
		}
	}

	// 预生成汇编

	vector<vector<vector<string> > >func_insts;
	func_insts.resize(func->bbs.len + 1);

	int min_sp = 0;
	// min_sp计算了alloc变量和运行时栈上分配的地址，未计算函数传参以及被调用者保存寄存器的地址

	for(size_t i = 0; i < func->bbs.len; i++) {
		
		koopa_raw_basic_block_t bp = (koopa_raw_basic_block_t)func->bbs.buffer[i];
		
		shrink_block(bp);

		func_insts[i].push_back({string(".")+(bp->name+1)});
											//变量是常量地址				//变量的内容存放在对应地址
		int new_sp = gen_riscv(bp, reg_set, pos_a0, local_const_addr, addr_of_v, sp, func_insts[i]);

		if(new_sp < min_sp) min_sp = new_sp;

		for(auto &r: reg_set) {
			r.nxt_use = -1;
			r.value = NULL;
		}
		addr_of_v.clear();

		// delete additional "jump" at the end of the block
		if(i < func->bbs.len - 1 && 
			!func_insts[i].empty() &&
			func_insts[i].back()[0] == "j" &&
			string(".") + (((koopa_raw_basic_block_t)func->bbs.buffer[i+1])->name + 1) == func_insts[i].back()[1]
		)	
			func_insts[i].pop_back();
	}


	// 添加被调用者保存寄存器的保存和恢复，统一return，计算变量在栈中的实际地址偏移量
	static int ret_label = 0;
	ret_label ++;

	vector<vector<string> >func_begin;

	int saved_reg_addr = 0, saved_reg_addr_end = 0;
	
	if(has_call) saved_reg_addr_end -= 4;
	for(auto &r: reg_set) 
		if(r.used && r.type == 's') 
			saved_reg_addr_end -= 4;

	int tot_abs_offset = - saved_reg_addr_end - min_sp;
	
	func_begin.push_back({"addi", "sp", "sp", i2s(-tot_abs_offset)}); // sp总偏移

	if(has_call) {
		saved_reg_addr -= 4;
		func_begin.push_back({"sw", "ra", i2s(saved_reg_addr + tot_abs_offset) + "(sp)"});
	}
	for(auto &r: reg_set) 
		if(r.used && r.type == 's') {					// push被调用者保存寄存器
			saved_reg_addr -= 4;
			func_begin.push_back({"sw", r.name(), i2s(saved_reg_addr + tot_abs_offset) + "(sp)"});
		}
	

	// 计算预生成汇编中的实际地址偏移量

	func_insts[0].insert(func_insts[0].begin(), func_begin.begin(), func_begin.end());
	
	// my_params |(fp) saved_regs(incl "ra")| alloced_variables | run_time_saved_regs | func_call_params |(sp)

	//			 |  len = -saved_reg_addr   |                        len = -min_sp                       |

	// high address -> -> -> -> -> -> -> -> -> -> -> -> -> -> -> -> -> -> -> -> -> -> -> -> low address

	for(size_t i = 0; i < func->bbs.len; i++) {

		vector<vector<string> > &block_insts = func_insts[i];

		for(auto &inst: block_insts) {
			if(inst[0] == "addi" && inst[2] == "fp") {				// 计算实际相对sp的偏移
				inst[2] = "sp";

				int offset = atoi(inst[3].c_str());
				if(offset >= 0) offset -= saved_reg_addr_end + min_sp; 	// my_params
																	// saved_regs are inserted between these
				else offset -= min_sp;								// run time data
				inst[3] = i2s(offset);
			}
			if(inst[0] == "sw" || inst[0] == "lw") {				// 计算实际相对sp的偏移
				size_t sp_pos = inst[2].find("(fp)");
				if(sp_pos != std::string::npos) {
					inst[2][sp_pos] = 0;

					int offset = atoi(inst[2].c_str());
					if(offset >= 0) offset -= saved_reg_addr_end + min_sp; 	// my_params
																		// saved_regs are inserted between these
					else offset -= min_sp;								// run time data

					inst[2] = i2s(offset) + "(sp)";
				}
			}
			if(inst[0] == "ret") {									// 统一return代码
				inst = {"j", string(".ret_label")+i2s(ret_label)};
			}
		}
	}

	// return时，恢复被保存的寄存器
	
	vector<vector<string> > &func_end = func_insts.back();

	func_end.push_back({string(".")+"ret_label"+i2s(ret_label)});

	saved_reg_addr = 0;
	if(has_call) {
		saved_reg_addr -= 4;
		func_end.push_back({"lw", "ra", i2s(saved_reg_addr + tot_abs_offset) + "(sp)"});
	}
	for(auto &r: reg_set) 
		if(r.used && r.type == 's') {					// pop被调用者保存寄存器
			saved_reg_addr -= 4;
			func_end.push_back({"lw", r.name(), i2s(saved_reg_addr + tot_abs_offset) + "(sp)"});
		}
	
	func_end.push_back({"addi", "sp", "sp", i2s(tot_abs_offset)}); // 恢复sp
	func_end.push_back({"ret"});
	
	// 处理过长常量的加载，包括所有含常量的二元运算，以及lw/sw的常量地址偏移，均使用保留寄存器t6加载常量

	for(auto block_insts: func_insts) {
		for(auto inst: block_insts) {
			if(inst[0][0] == '.') {
				riscvprintf("%s:\n", inst[0].c_str());
				continue;
			}
			if(inst[0].back() == 'i' && inst[0] != "li" && abs(atoi(inst[3].c_str())) > 2047) {// 二元运算
				riscvprintf("  %-4s  ", "li");
				riscvprintf("%-5s", "t6, ");
				riscvprintf("%-5s\n", inst[3].c_str());

				inst[0].pop_back();
				riscvprintf("  %-4s  ", inst[0].c_str());
				riscvprintf("%-5s", (inst[1]+", ").c_str());
				riscvprintf("%-5s", (inst[2]+", ").c_str());
				riscvprintf("%-5s\n", "t6");
				continue;
			}
			if((inst[0] == "sw" || inst[0] == "lw") && abs(atoi(inst[2].c_str())) > 2047) { // load/store
				riscvprintf("  %-4s  ", "li");
				riscvprintf("%-5s", "t6, ");
				size_t pos = inst[2].find("(");
				assert(pos != std::string::npos);
				riscvprintf("%-5s\n", inst[2].substr(0, pos).c_str());

				riscvprintf("  %-4s  ", "add");
				riscvprintf("%-5s", "t6, ");
				riscvprintf("%-5s", "t6, ");
				riscvprintf("%-5s\n", inst[2].substr(pos+1, inst[2].size()-1-(pos+1)).c_str());

				riscvprintf("  %-4s  ", inst[0].c_str());
				riscvprintf("%-5s", (inst[1]+", ").c_str());
				riscvprintf("%-5s\n", "0(t6)");
				continue;
			}
			riscvprintf("  %-4s  ", inst[0].c_str());
			for(size_t i = 1; i < inst.size(); i++) 
				riscvprintf("%-5s", (inst[i]+((i<inst.size()-1)?", ":"")).c_str());
			riscvprintf("\n");
		}
	}	
}

void solve_funcs(koopa_raw_slice_t &funcs)
{
	riscvprintf("  .text\n");
	
	for(auto name: global_link_funcs) // 链接符号
		riscvprintf("  .globl %s\n", name.c_str());

	for(size_t i = 0; i < funcs.len; i++)
		solve_func((koopa_raw_function_t)funcs.buffer[i]);
}

void koopa2riscv(ostringstream &strin)
{
	koopa_program_t program;
	koopa_error_code_t ret = 
		koopa_parse_from_string(strin.str().c_str(), &program);
	assert(ret == KOOPA_EC_SUCCESS);
	
	koopa_raw_program_builder_t builder = koopa_new_raw_program_builder();
	
	koopa_raw_program_t raw = koopa_build_raw_program(builder, program);
	
	koopa_delete_program(program);
	
	koopa_raw_slice_t &decls = raw.values, &funcs = raw.funcs;
	
	solve_global_decls(decls);

	solve_funcs(funcs);
	
	koopa_delete_raw_program_builder(builder);
}

int main(int argc, const char *argv[]) {
  
  assert(argc == 5 || argc == 3);
  assert(argv[1][0] == '-' && (argv[1][1] == 'k' || argv[1][1] == 'r'));
  
  char mode = argv[1][1];
  if(mode == 'p') mode = 'r';
  
  yyin = fopen(argv[2], "r");
  assert(yyin);
  
  if(argc == 5) {
  	assert(strcmp(argv[3], "-o") == 0);
  	output = new ofstream(argv[4]);
  	assert(*(ofstream*)output);
  }
  else {
  	output = &cout;
  }
  
  BaseAST * ast = NULL;

  int ret = yyparse(ast);
  assert(!ret);
  
  ast->dump();
  
  if(mode == 'k') {
  	*output << ast->astout.str();
  	goto end;
  }
  
  riscvout = output;

  koopa2riscv(ast->astout);
  
end:
  if(argc == 5) delete (ofstream*)output;// this will flush and close file

  return 0;
}
