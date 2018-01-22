# 搜索引擎Prototype Phoenix
Powered by 张帅, 杨一江, 乔裕哲



![起始界面](docs/起始界面.png)



详细说明在[这里](docs/Manual.pdf)



## 整体介绍

Maintained by 张帅

### 1、载入数据
	“在线载入”：在线爬取网页
	“本地载入”：使用使用本地数据进行初始化
	使用结巴分词
	
	“在线载入”：设定爬取起始地址和最大网页数，开始爬取
		*构建了起始地址栏和最大网页数栏与“在线载入”的链接，因此编辑完成之后可以直接按ENTER键，而不需要点击
		*使用子线程机制沟通爬虫与主界面，避免了网页获取较慢时界面卡死
		*使用子线程沟通倒排索引与主界面，避免了倒排索引写文件较慢时界面卡死
		*爬取完成之后自动分词并建立倒排索引
	“本地载入”：设定载入目录和最大网页数
		*载入数据之后自动建立倒排索引
### 2、检索数据
	检查并识别布尔表达式
	根据布尔表达式计算并分词，交付倒排索引
	倒排索引调入文件并搜索，搜索结果返回主线程
	主线程为搜索结果建立超链接，并对关键词标红
	点击超链接可以调用系统浏览器访问
### 3、界面设计
	开始界面
	进度条：使用信号-槽机制与子线程沟通，实时显示子线程进度
### 4、其他设计
	程序打包及安装
		使用Qtframework，安装程序自动在开始菜单和桌面创建快捷方式，可在“控制面板>卸载程序”卸载



## 爬虫介绍

作者：乔裕哲 151220086
版本：v1.3
	修复解析网页数据时可能导致abort()的问题：string.find返回-1
	用户可以输入MAXFILECOUNT来限定爬取网页数
	增加根据入度进行直接选择排序的函数
	在调用爬虫的任何功能之前（sort_pages,crwal,init_website）,先初始化，调用init_global_vars，
	初始化前先输入MAXFILECOUNT
### 爬虫部分
	1.先输入MAXFILECOUNT，限定爬取的网页数量
	2.然后调用init_global_vars初始化一些全局变量以供爬虫使用
	3.将入口链接入队，然后取队首链接，通过套接字获取对应的html代码，并解析出其中的所有链接，加入队列中
	4.保存数据，分为三部分，html文件夹下保存html代码，text文件夹下保存代码里的中文，供分词和倒排索引使用，url文件夹下保存对应网站链接。
	5.将队首链接加入已访问网站集合，出队。
	6.直到队空或爬到限定个数的网页，终止爬虫。
### pagerank
	1.利用爬虫得到一个链接数组，依次从磁盘中读取对应链接的html文档，并解析出其中的链接，解析方法同爬虫。
	2.若解析的链接能在数组中找到，则修改对应的网页有向图的邻接矩阵，建立矩阵完毕。
	3.根据矩阵得到每个网页的入度出度，然后利用pagerank公式计算。
### 布尔检索
	1.用户输入一个字符串，遍历其中的字符数组，将表达式划分为若干个token
	2.利用栈将中缀表达式转换成后缀表达式
	3.对每个token的内容，如果该token是操作数，就对其分词，并搜索，搜索结果也保存在token中
	4.将搜索后的token用栈计算表达式的值，最终得到一个搜索结果



## 倒排索引介绍

Maintained by 杨一江
Current RID version: Modified Trial One Nirvana


版本改进：
	++增加RID文件的分页机制
	++改进收集策略，减少未响应情况
	++更新RI_intsc_symdif函数，支持分页文件的读取
	++更新loader函数，支持最新改进
	++增加RI_exclude函数，支持not逻辑
```
description
	The streamline of the whole process contains four parts:
		collect information
		write RID file
		load RID file
		integrate results
```
	在第一部分，该系统从索引文件夹中读取索引文件，然后根据每个索引文件中出现的词条以及对应的文件号码，建立一个初步的倒排索引，该倒排索引是以红黑树的存储结构存储的。
	每个初步的倒排索引大小不超过4000个词条，这是由于当索引的大小超过这个数字的时候，倒排索引的处理时间将会急剧的增长，而且有机会使得程序崩溃。
	
	在第二部分，系统将会把初步的倒排索引写入RID文件中，格式根据RID Type Trial One格式进行存储。这个版本实现了分页写文件，与之前版本有较大的改进，每个页文件词条上限4000，并存储词条字符和索引。
	在这一步，将倒排索引的存储结构转变为散列表，该散列表为闭散列，防止文件过于膨胀，超出文件存储格式所能寻求的寻址空间。哈希函数采用移位折叠法并平方，为的是增加散列表的稀疏程度。散列冲突策略为二次探查法。
	
	第三部分，每次搜索的时候，将会载入分页文件，并收集对应词条所有的分页文件信息。
	第四部分，为了处理不同的逻辑结果，将采用位向量表记录倒排索引，方便逻辑处理。

```
	RID Type Trial One
		This RID standard is written in the heading file. This standard contains two parts: storage foramtion and reading strategy
		storage formation:
			The RID file is written in binary way. The structure is refer to ELF file, including Database Heading, Entry Section, List Section, Name Section and Index Heading.
			Database Heading contains the basic information of the RID file. Starting with "RID", it is easy to distinguish it from other files. The streampos and some constants concerning the standard, which includes maxFile size and so on.
			Index Heading tells the contemporary information of the RID file, such as maxWeb_num and Hash strategy, and some constants surrounding entry units and list units.
			Entry Section stores the information of reverse index each entry contains. It is laid out as a hash list, which means the size of this section is fixed. Each entry unit tells the validity, tag, name offset, list offset and list size. We introduce a F bit to accelerate the finding procedure.
			List Section stores the real information of reverse index, which is laid out as a linear list without fixed size. Entry unit will tell the start node and the size of list.
			Name Section stores the strings that each entry features. The formation is relatively arbitary. The size is not fixed and each string ends with '\0', which means search of entry name only requires one parameter: offset in Name Section.
		Reading strategy:
			The streampos of each section is recorded in Database Heading, which means you should read it first.
			If a file is freshly created, you can ignore Index Heading since you already owns.
			Otherwise, you should read contemporary information form Index Heading.
			When searching for the entry, you should check V bit first.
				First probe:
				V = 0, end searching and report "NO FOUND"
				otherwise, check F
					F = 0, quadratic probing
					F = 1, check tag
						tag distincted, quadratic probing
						otherwise, check Name
							Name distincted, quadratic probing
							otherwise, end searching and report "FOUND"
				Quadratic probe:
				(H ± i^2), i++
				V = 0, end searching and report "NOT FOUND"
				otherwise, check F
					F = 1, continue
					otherwise, check tag
						tag distincted, continue
						otherwise, check Name
							Name distincted, continue
							otherwise, end searching and report "FOUND"
```
