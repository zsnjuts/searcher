/*
乔裕哲 151220086

栈的用法：先#include "mystack.h"
		 然后mystack <T> st;创建容器
		 与c++一样，但只提供push,pop,top,empty,size五种操作
		 并且push和pop返回布尔型，表示操作是否成功，如果栈满push会返回false，栈空pop返回false
		 top返回！！临时的！！栈顶元素！！！不是引用型！！！，若要修改栈顶先top,再修改再pop再push
		 empty返回是否空，1=空，0=非空
		 size返回栈内元素个数。
		 
使用方法：调用analysis_expr，会修改全局变量post_order_token,
		 如果post_order_token[i].type==OP,就是操作数，需要进行搜索,
		 搜索的内容是post_order_token[i].str，要先分词再搜索
		 其他的都是操作符，根据后缀表达式计算即可
*/


#include "mystack.h"
#include <string>
#include <iostream>
#include "expr.h"

using namespace std;


/*		   汉字，（，），*，+，-		*/
static int isp[7] = { -1, 1, 8, 5, 3, 7, 0 };
static int icp[7] = { -1, 8, 1, 4, 2, 6, 0 };

Token post_order_token[1024];/*这个是最终的后缀表达式*/
int nr_post_token = 0;/*这个是后缀表达式的token个数*/

static Token token[1024];
static int nr_token = 0;

static void to_post_order();
static Token calc_and(Token lop, Token rop);
static Token calc_or(Token lop, Token rop);
static Token calc_not(Token lop, Token rop);

bool analysis_expr(const string& expr){
    nr_token = 0;
    nr_post_token = 0;
	int len = expr.length();
	token[0].type = END;
	token[0].str = '#';
	nr_token++;
	for (int i = 0; i < len; ++i){
		if (expr[i] == '+'){
			token[nr_token].type = OR;
			token[nr_token].str = expr[i];
			nr_token++;
		}
		else if (expr[i] == '-'){
			token[nr_token].type = NOT;
			token[nr_token].str = expr[i];
			nr_token++;
		}
		else if (expr[i] == '*'){
			token[nr_token].type = AND;
			token[nr_token].str = expr[i];
			nr_token++;
		}
		else if (expr[i] == '('){
			token[nr_token].type = LB;
			token[nr_token].str = expr[i];
			nr_token++;
		}
		else if (expr[i] == ')'){
			token[nr_token].type = RB;
			token[nr_token].str = expr[i];
			nr_token++;
		}
		else{
			token[nr_token].type = OP;
			int count;
			for (count = i + 1;
				count < len && //不要越界		而且
				(!(expr[count] >= 0 && expr[count] <= 127) || expr[count] == ' ');//该字符不在0~127范围内，或者为空格
				++count);
			token[nr_token].str = expr.substr(i, count - i);
			nr_token++;
			i = count - 1;
		}
	}
	token[nr_token].type = END;
	token[nr_token].str = '#';
	nr_token++;
	to_post_order();
	if (nr_post_token == 1){
		return false;
	}
	else{
		return true;
	}
}

void to_post_order(){
	mystack <Token> st;
	int in_pos = 0, post_pos = 0;
	st.push(token[0]);
	in_pos++;
	while (!(st.top().type == END && token[in_pos].type == END)){
		if (token[in_pos].type == OP){
			post_order_token[post_pos] = token[in_pos];
			in_pos++;
			post_pos++;
		}
		else{
			if (icp[token[in_pos].type] > isp[st.top().type]){
				st.push(token[in_pos]);
				in_pos++;
			}
			else if (icp[token[in_pos].type] < isp[st.top().type]){
				post_order_token[post_pos] = st.top();
				post_pos++;
				st.pop();
			}
			else if (icp[token[in_pos].type] == isp[st.top().type]){
				if (st.top().type == LB) in_pos++;
				st.pop();
			}
		}
	}
	nr_post_token = post_pos;
}

static Token calc_not(Token lop, Token rop){//-
	Token result;
	result.type = OP;
	result.size = lop.size;
	result.arr = new int[result.size];
	result.size = 0;
	for (int i = 0; i < lop.size; ++i){
		int j;
		for (j = 0; j < rop.size; ++j){
			if (lop.arr[i] == rop.arr[j]){
				break;
			}
		}
		if (j == rop.size){
			result.arr[result.size] = lop.arr[i];
			++result.size;
		}
	}
	return result;
}

static Token calc_and(Token lop, Token rop){//*
	Token result;
	result.type = OP;
	result.size = lop.size > rop.size ? rop.size : lop.size;
	result.arr = new int[result.size];
	result.size = 0;
	for (int i = 0; i < lop.size; ++i){
		for (int j = 0; j < rop.size; ++j){
			if (lop.arr[i] == rop.arr[j]){
				result.arr[result.size] = lop.arr[i];
				result.size++;
			}
		}
	}
	return result;
}

static Token calc_or(Token lop, Token rop){//+
	Token result;
	result.type = OP;
	result.size = lop.size + rop.size;
	result.arr = new int[result.size];
	result.size = 0;
	for (int i = 0; i < lop.size; ++i){
		int j;
		for (j = 0; j < result.size; ++j){
			if (lop.arr[i] == result.arr[j]){
				break;
			}
		}
		if (j == result.size){
			result.arr[result.size] = lop.arr[i];
			++result.size;
		}
	}
	for (int i = 0; i < rop.size; ++i){
		int j;
		for (j = 0; j < result.size; ++j){
			if (rop.arr[i] == result.arr[j]){
				break;
			}
		}
		if (j == result.size){
			result.arr[result.size] = rop.arr[i];
			++result.size;
		}
	}
	return result;
}

int calc_post_order_expr(int *&result){
	Token temp;
	mystack <Token> st;
	int i = 0;
	while (i < nr_post_token){
		if (post_order_token[i].type == OP){
			st.push(post_order_token[i]);
			++i;
		}
		else{
			Token rop = st.top();
			st.pop();
			Token lop = st.top();
			st.pop();
			switch (post_order_token[i].type){
			case AND:
				st.push(calc_and(lop, rop));
				break;
			case NOT:
				st.push(calc_not(lop, rop));
				break;
			case OR:
				st.push(calc_or(lop, rop));
				break;
			}
			delete[]rop.arr;
			delete[]lop.arr;
			++i;
		}
	}
	temp = st.top();
	result = new int[temp.size];
	for (int i = 0; i < temp.size; ++i)
		result[i] = temp.arr[i];
	return temp.size;
}
