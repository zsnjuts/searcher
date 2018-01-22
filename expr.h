#ifndef EXPR_H
#define EXPR_H

#include <string>

enum OpType{ OP, LB, RB, AND, OR, NOT, END };

struct Token{
    OpType type;
    std::string str;
    int size;
    int *arr;
};

extern Token post_order_token[1024];/*这个是最终的后缀表达式*/
extern int nr_post_token;/*这个是后缀表达式的token个数*/

int calc_post_order_expr(int *&result);/*用来计算后缀表达式*/
bool analysis_expr(const std::string& );/*这个是最终的调用函数，调用之后自动解析并转换成后缀表达式*/


#endif // EXPR_H

