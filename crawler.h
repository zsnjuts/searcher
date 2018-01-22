#ifndef CRAWLER_H
#define CRAWLER_H

#include <vector>
#include <string>
using namespace std;

/* pagerank */
void init_global_vars(int MAXFILECOUNT);//爬虫相关变量初始化，使用爬虫前先调用这个，只要一次
void sort_by_pagerank(int *arr, int size);//根据PageRank结果进行排序
void sort_by_in_degree(int *arr, int size);//根据入度排序
string get_url(int i);//根据序号获取网站名
double get_pr(int i);//根据序号获取PageRank值
int get_in_degree(int i);//根据序号获取入度
int init_website(string entry, int MAXFILECOUNT);//用文件初始化website数组，返回值为文件中最大序号
void pagerank(int MAXFILECOUNT);//计算PageRank值
void init_matrix(int MAXFILECOUNT);//初始化矩阵

#endif // CRAWLER_H

