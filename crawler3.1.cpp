/*
作者：乔裕哲 151220086
版本：v1.3
1.0：完成基本功能
1.1：调整了输出文件的位置
	 增加对覆盖文件夹的询问
	 限定了主机名，防止乱爬
	 新加入了一些注释
	 2016.12.19

1.2：限定主机名改为限定文件数
	 修复了保存数据时可能导致abort()的问题
	 修复部分网站爬不到的问题
	 完成了邻接矩阵和入度出度的统计
	 完成pagerank的简单计算
	 对一个编号数组，根据编号的pr值进行排序
	 2016.12.22

1.3	 修复解析网页数据时可能导致abort()的问题：string.find返回-1
	 用户可以输入MAXFILECOUNT,但是申请的内存不会释放
	 只能爬一次
	 只能爬一次
	 只能爬一次
	 在调用爬虫的任何功能之前（sort_pages,crwal,init_website）,先初始化，调用init_global_vars，
	 初始化前先输入MAXFILECOUNT
	 初始化前先输入MAXFILECOUNT
	 初始化前先输入MAXFILECOUNT
	 只要在整个程序中调用一次init_global_vars，以后无需调用
	 只接受bug反馈，不再接受任何的需求更改
	 2017.1.1

ps1：cslab.nju.edu.cn这类网站爬不了，我也不知道为什么
ps2：
ps3：仅限windows的vc使用，如果用gcc编译需做少量修改
ps4：编码方式utf-8
psp：pagerank值保存在：工程目录\\网站名\\url\\xxx.txt下的第二行，是一个double型，第三行是入度出度
psv：如果有需要可以将下方的pagerank相关的变量改为全局变量
*/

/*
 * 整体思路：
 * 1、将接口函数和接口变量的声明放在了crawler.h中
 * 2、将一部分需要输出的函数作为了CrawlerThread的成员函数，这些函数前面都有CrawlerThread::前缀，调试的时候可以去掉，但是最后要加上
 * 3、由于子线程相关需要，添加crawlerthread.h头文件，可以先注释掉，但是最后要加上
 * 4、其他可能影响爬虫单独调试的代码均以注释方式标出
 * */

#include <io.h>
#include <direct.h>
#include <iostream>
#include <fstream>
#include <string>
#include <set>
#include <queue>
#include <regex>
#include <Ws2tcpip.h>
#include <WinInet.h>
#include <WinSock2.h>
#include <Windows.h>
/* added by zs */
#include <direct.h>
#include "crawlerthread.h" //子线程相关
#include "crawler.h" //接口函数及变量

//#pragma comment (lib, "ws2_32.lib")

#define MAXBUFSIZE 8388608	//2^20

using namespace std;

/*****入口，只访问该主机名的网站*****/
//string entry = "jw.nju.edu.cn";

static pair<string, string> spliturl(const string &url);//将网址分成主机名和资源名
//static void crawler_start();//利用广度优先爬网站
static string crawl(const string &url);//爬取一个网站的源码
static string GetHostByName(const string &HostName);//通过主机名获得ip地址
static SOCKET MakeSocket(string ip);//创建一个socket并连接到主机
static bool SendPacket(SOCKET sock, const string &hostname, const string &source);//发送数据
static string RecvPacket(SOCKET sock);//接受数据
//static void SaveData(const string &url, const string &buf);//将url和buf和buf中的中文分别保存到文件中
static void ParseHTML(string &buf);//解析HTML，将链接加入queue，将资源展开到末尾

static set <string> visitedurls;//记录已经访问过的网站
static queue <string> urls;//要访问的网站
static string filepath = "html\\";
static string textpath = "text\\";
static string urlpath = "url\\";
static int filecount = 0;//记录创建的文件个数
static pair <string, string> pair_result;
static string hostname;

/* pagerank 相关 */
//static bool matrix[MAXFILECOUNT][MAXFILECOUNT];//邻接矩阵
//string website[MAXFILECOUNT + 1];//编号为i的网站的网址
//static double pr[MAXFILECOUNT] = { 1 };//网址的pagerank值
//static int in_degree[MAXFILECOUNT];//网址的入度
//static int out_degree[MAXFILECOUNT];//网址的出度 
static bool **matrix;//邻接矩阵
string *website;//编号为i的网站的网址
static double *pr;//网址的pagerank值
static double *last_pr;
static int *in_degree;//网址的入度
static int *out_degree;//网址的出度 
static void find_url(string buf);


string get_url(int i)
{
    return website[i];
}

double get_pr(int i)
{
    return pr[i];
}

int get_in_degree(int i)
{
    return in_degree[i];
}

int init_website(string entry, int MAXFILECOUNT){
    string filename;
    cout << "filename=" << filename << endl;
    int i;
    for (i = 0; i < MAXFILECOUNT; ++i){
        filename = entry + "/url/" + to_string(i) + ".txt";
        ifstream ifs(filename);
        if (ifs.fail()){
            break;
        }
        getline(ifs, website[i]);
        ifs.close();
    }
    filecount = i;
    return i;
}

void init_global_vars(int MAXFILECOUNT){
    matrix = new bool*[MAXFILECOUNT];
    for (int i = 0; i < MAXFILECOUNT; ++i)
        matrix[i] = new bool[MAXFILECOUNT];
    for (int i = 0; i < MAXFILECOUNT; ++i)
        for (int j = 0; j < MAXFILECOUNT; ++j)
            matrix[i][j] = false;
    website = new string[MAXFILECOUNT];
    pr = new double[MAXFILECOUNT];
    last_pr = new double[MAXFILECOUNT];
    for (int i = 0; i < MAXFILECOUNT; ++i){
        pr[i] = 1;
        last_pr[i] = 1;
    }
    in_degree = new int[MAXFILECOUNT];
    out_degree = new int[MAXFILECOUNT];
    for (int i = 0; i < MAXFILECOUNT; ++i){
        in_degree[i] = 0;
        out_degree[i] = 0;
    }

}

int CrawlerThread::crawler(){
    string stdentry = entry.toStdString();
    if (_access(stdentry.c_str(), 0) != -1){
        int ret = 0;
        emit crawlQuestion(tr("目录已存在"),tr("%1目录已存在，您是否要覆盖它？").arg(entry),ret);//用于询问是否覆盖，用ret表示，这里需要重新写一下
        if(ret==1)
        {
            _rmdir(stdentry.c_str());
            _mkdir(stdentry.c_str());
        }
        else
            return 0;
	}
	else //文件夹不存在
        _mkdir(stdentry.c_str());

	//创建文件夹
    _mkdir((stdentry + "\\" + "html").c_str());
    _mkdir((stdentry + "\\" + "text").c_str());
    _mkdir((stdentry + "\\" + "url").c_str());

    filepath = stdentry + "\\" + filepath;
    textpath = stdentry + "\\" + textpath;
    urlpath = stdentry + "\\" + urlpath;

	WSADATA wsadata;//初始化
	if (WSAStartup(MAKEWORD(2, 2), &wsadata) != 0){
        emit crawlWarning(tr("启动失败"),tr("WSA启动失败"));//仅仅是显示警告信息，可以直接注释掉，最后要加上
        cout << "failed to start up WSA" << endl;
        return -1;
	}

    urls.push(stdentry);
    crawler_start();//爬

	WSACleanup();//结束

    init_matrix(MAXFILECOUNT);

    pagerank(MAXFILECOUNT);

	return filecount;
}

void CrawlerThread::crawler_start(){
	/* 在此限定主机名 
	pair <string, string> temp = spliturl(urls.front());
	hostname = GetHostByName(temp.first);*/

	while (!urls.empty()){
		if (filecount >= MAXFILECOUNT){
			break;
		}
		/* 进行一些过滤 */
        if (//urls.front().find("nju.") == string::npos ||      /* 对nju的限制 */
			urls.front().find("desktop.nju.edu.cn") != string::npos ||
			urls.front().find(".pdf") != string::npos){
			urls.pop();
			continue;
		}

		/* pagerank时需要加入东西 */
		else if (visitedurls.count(urls.front())){
			urls.pop();
			continue;
		}

		string buf = crawl(urls.front());

		if (!buf.empty()){
			ParseHTML(buf);
            emit crawlBrowserAppend(QObject::tr("%1\t=>count %2").arg(QString::fromStdString(urls.front())).arg(urls.size()));//用于在界面里面输出信息
            //cout << urls.front() << "\t=>count " << urls.size() << endl;//打印访问过的网站并打印剩下的网站个数

            SaveData(urls.front(), buf);
			visitedurls.insert(urls.front());
		}
		urls.pop();
	}
}

static string crawl(const string &url){
	pair_result = spliturl(url);

	/* 获得ip */
	string ip = GetHostByName(pair_result.first);
//	if (ip.empty() || ip != hostname){
	if (ip.empty()){
		//找不到ip过滤
		return "";
	}

	/* 连接服务器 */
	SOCKET sock = MakeSocket(ip);
	if (sock == 0){
		closesocket(sock);
		return "";
	}

	/* 发送请求 */
	if (!SendPacket(sock, pair_result.first, pair_result.second)){
		closesocket(sock);
		return "";
	}

	/* 收包 */
	string buf = RecvPacket(sock);
	if (buf.empty()){
		closesocket(sock);
		return "";
	}

	closesocket(sock);

	/* 请求不正确 */
	if (buf.find("Bad Request") != string::npos){
		return "";
	}
	
	return buf;
}

static string GetHostByName(const string &HostName){
	hostent *host = gethostbyname(HostName.c_str());
	if (!host){
		return "";
	}
	return inet_ntoa(*(in_addr *)host->h_addr);
	//return inet_ntop(host->h_addrtype, host->h_addr, str, 32);
}

static pair<string, string> spliturl(const string &url){
	int pos;
	pair <string, string> result(url, "");
	pos = url.find("/");
	if (pos != string::npos){
		result.first = url.substr(0, pos);
		result.second = url.substr(pos + 1);
	}
	return result;
}

static void ParseHTML(string &buf){
	string temp;

	/* 找到网站中的资源（表格、文字）并展开到buf末尾 */
	char *tag = "<iframe";
	int pos = buf.find(tag, 0);
	if (pos!=string::npos){
		tag = "src='";
		pos = buf.find(tag, pos);
		if (pos != string::npos){
			pos += strlen(tag);
			int end = buf.find("'", pos);
			string url = buf.substr(pos, end - pos);
			temp = crawl(pair_result.first + "/" + url);
			pos = temp.find("Content-Length:", 0);
			if (pos != string::npos){
				pos = temp.find("<", pos);
				if (pos != string::npos)buf += temp.substr(pos, temp.length() - pos);
				else;
			}
			else;
		}
	}

	/* 找出网址并加入队列 */
	/* pagerank时需要加东西 */
	for (int i = 0; i < 2; ++i){
		if (i == 0)
			tag = "href='";
		else
			tag = "href=\"";
		pos = buf.find(tag, 0);
		while (pos != string::npos){
			pos += strlen(tag);
			int end;
			if (i == 0)	end = buf.find("'", pos);
			else end = buf.find("\"", pos);
			string url = buf.substr(pos, end - pos);

			if (url.find("http://") == string::npos &&
				url.find("https://") == string::npos &&
				url.find(".css") == string::npos){
				url = pair_result.first + "/" + url;
				if (visitedurls.count(url));
				else urls.push(url);
			}

			else if (url.find("http://") != string::npos && url.find("https://") == string::npos){
				int pos = url.find("http://") + 7;
				url = url.substr(pos);
				urls.push(url);
			}

			else if (url.find("https://") != string::npos && url.find("http://") == string::npos){
				int pos = url.find("https://") + 8;
				url = url.substr(pos);
				urls.push(url);
			}
			pos = buf.find(tag, end);
		}
	}
}

static bool SendPacket(SOCKET sock, const string &hostname, const string &source){
	string http = "GET " + ("/" + source) + " HTTP/1.1\r\n"
		+ "HOST: " + hostname + "\r\n"
		+ "Connection: close\r\n\r\n";
	if (send(sock, http.c_str(), http.length(), 0) == SOCKET_ERROR)
		return false;
	else
		return true;
}

static string RecvPacket(SOCKET sock){
	string buf(MAXBUFSIZE, '\0');
	int len = 0;
	int reclen = 0;
	do{
		reclen = recv(sock, &buf[0] + len, MAXBUFSIZE - len, 0);
		len += reclen;
	} while (reclen > 0);
	
	return buf.substr(0, len);
}

static SOCKET MakeSocket(string ip){

	SOCKET sock = socket(AF_INET, SOCK_STREAM, 0);
	if (sock == INVALID_SOCKET){
		//		cout << "invalid socket" << endl;
		return 0;
	}

	SOCKADDR_IN addr;
	addr.sin_family = AF_INET;
	addr.sin_port = htons(80);
	addr.sin_addr.s_addr = inet_addr(ip.c_str());
	if (connect(sock, (sockaddr *)&addr, sizeof(addr)) == SOCKET_ERROR){
		//		cout << "failed to connect to " << url << endl;
		return 0;
	}
	return sock;
}

void CrawlerThread::SaveData(const string &url, const string &buf){
	if (!buf.empty()){

		/* 将buf存入html下 */
		string filename = filepath + to_string(filecount) + ".txt";
		ofstream ofs(filename);
		//除去buf头的那段信息描述再存入
		int pos = buf.find("<!DOCTYPE");
		if (pos == string::npos){
			ofs.close();
			return;
		}
		ofs << buf.substr(pos);
		ofs.close();

        website[filecount] = url;
		
		/* 将buf中的中文挑出并存入text下 */
		filename = textpath + to_string(filecount) + ".txt";
		ofs.open(filename);
        for (unsigned i = 0; i < buf.size(); i++){
            if (buf.find("font", i) == i){
                i += 4;
                i = buf.find(";", i);
            }
            else if (!(buf[i] >= 0 && buf[i] <= 127)) {
                ofs << buf[i];
            }
        }

/*		for (unsigned i = 0; i < buf.size(); i++){
			if (!(buf[i] >= 0 && buf[i] <= 127)) {
				ofs << buf[i];
			}
        }*/
		ofs.close();

		/* 将网址存入url下 */
		filename = urlpath + to_string(filecount) + ".txt";
		ofs.open(filename);
		ofs << url << endl;
		ofs.close();
		filecount++;
        //qDebug() << "filecount=" << filecount << endl;
        emit crawlProgress(filecount);//为进度条使用，可以先注释掉，最后要加上
	}
}

void init_matrix(int MAXFILECOUNT){
    char *tag;
    int pos;
    for (int i = 0; i < filecount; ++i){
        /* 读取html文件的全部内容，保存在buf中 */
        pair_result = spliturl(website[i]);
        string filename = filepath + to_string(i) + ".txt";
        ifstream in(filename, ios::in);
        istreambuf_iterator<char> beg(in), end;
        string buf(beg, end);
        in.close();
        /*两次循环查找buf中链接*/
        for (int k = 0; k < 2; ++k){
            if(k==0)tag = "href='";
            else tag = "href=\"";
            pos = buf.find(tag, 0);

            /* 找到buf中的全部网址 */
            while (pos != string::npos){
                pos += strlen(tag);
                int end;
                if (k == 0)
                    end = buf.find("'", pos);
                else end = buf.find("\"", pos);
                string url = buf.substr(pos, end - pos);

                if (url.find("http://") == string::npos &&
                    url.find("https://") == string::npos &&
                    url.find(".css") == string::npos){
                    url = pair_result.first + "/" + url;
                }

                else if (url.find("http://") != string::npos && url.find("https://") == string::npos){
                    int pos = url.find("http://") + 7;
                    url = url.substr(pos);
                }

                else if (url.find("https://") != string::npos && url.find("http://") == string::npos){
                    int pos = url.find("https://") + 8;
                    url = url.substr(pos);
                }
                /* 顺序查找website中的网址 */
                int j;
                for (j = 0; j < filecount; ++j){
                    if (j == i) continue;
                    if (website[j] == url) break;
                }
                /* 找到，则对应入度出度+1，修改邻接矩阵 */
                if (j != MAXFILECOUNT && matrix[i][j] == false){
                    matrix[i][j] = true;
                    in_degree[j] ++;
                    out_degree[i] ++;
                }

                pos = buf.find(tag, end);
            }
        }
/*		ofstream ofs("testdata.txt");
        for (int i = 0; i < filecount; ++i){
            for (int j = 0; j < filecount; ++j){
                ofs << matrix[i][j] << " ";
            }
            ofs << endl;
        }*/
    }

}

void pagerank(int MAXFILECOUNT){
	double last_pr[MAXFILECOUNT];
	double damping_factor = 0.85;
	for (int iteration_times = 0; iteration_times < 100; ++iteration_times){
		for (int i = 0; i < filecount; ++i) { 
			last_pr[i] = pr[i]; 
			pr[i] = 0;
		}
		for (int i = 0; i < filecount; ++i){
			for (int j = 0; j < filecount; ++j){
				if (j == i)continue;
				if (matrix[i][j]){//这个网页指向另一个网页
					pr[j] += last_pr[i] / out_degree[i];
				}
			}
		}
		for (int i = 0; i < filecount; ++i)
			pr[i] = (pr[i] * damping_factor + (1 - damping_factor) / filecount);
	}
	for (int i = 0; i < filecount; ++i){
		string filename = urlpath + to_string(i) + ".txt";
		ofstream ofs(filename, ios::app);
		ofs << std::fixed << pr[i] << endl;
		ofs << "in = " << in_degree[i] << " out = " << out_degree[i] << endl;
	}
}

void sort_by_pagerank(int *arr, int size){
	for (int i = 0; i < size; ++i){
		int max = i;
		for (int j = i; j < size; ++j)
			if (pr[arr[j]]>pr[arr[max]]) max = j;
		int temp = arr[max];
		arr[max] = arr[i];
		arr[i] = temp;
	}
}


void sort_by_in_degree(int *arr, int size){
    for (int i = 0; i < size; ++i){
        int max = i;
        for (int j = i; j < size; ++j)
            if (in_degree[arr[j]]>in_degree[arr[max]]) max = j;
        int temp = arr[max];
        arr[max] = arr[i];
        arr[i] = temp;
    }
}
