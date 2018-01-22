#ifndef RID_H
#define RID_H

#include <fstream>
#include <string>
#include <map>
#include <vector>
#include <ctime>
#include <sstream>
#include <cassert>

typedef unsigned int uint32_t;
typedef unsigned short uint16_t;
typedef unsigned char uint8_t;
#define RID_PROTOCOL_TYPE_TRIAL_ONE

// infomation collector
struct infoCollector_node {
	uint32_t index;
	uint32_t freq;
	infoCollector_node *next;
};

// RID protocol
#ifdef RID_PROTOCOL_TYPE_TRIAL_ONE
#define versionCode_Type_Trial_One 19970309
#define maxEntry_idx_Type_Trial_One 19
#define maxList_idx_Type_Trial_One 31
#define maxName_idx_Type_Trial_One 19
#define maxRID_idx_Type_Trial_One 32
#define sizeDh_Type_Trial_One 40
#define sizeIh_Type_Trial_One 32

#define maxNode_Type_Trial_One 1024
#define maxEntry_Type_Trial_One 8192
#define sizeNode_Type_Trial_One 8
#define sizeEntry_Type_Trial_One 8

#define versionCode versionCode_Type_Trial_One
#define maxEntry_idx maxEntry_idx_Type_Trial_One
#define maxList_idx maxList_idx_Type_Trial_One
#define maxName_idx maxName_idx_Type_Trial_One
#define maxRID_idx maxRID_idx_Type_Trial_One
#define sizeDh sizeDh_Type_Trial_One
#define sizeIh sizeIh_Type_Trial_One

#define maxNode maxNode_Type_Trial_One
#define maxEntry maxEntry_Type_Trial_One
#define sizeNode sizeNode_Type_Trial_One
#define sizeEntry sizeEntry_Type_Trial_One

#define RID_Dh RID64_Dh
struct RID64_Dh {
	char declaration[4];	// store string "RID", to distinguish other files
	uint32_t version;	// RID version
	uint32_t LastMod_Time;	// last modification time
	uint32_t index_off;	// .index heading offset
	uint32_t entry_off;	// .entry section offset
	uint32_t name_off;	// .name section offset
	uint32_t list_off;	// .list section offset
	uint32_t reserve;	// reserve
	struct {
		uint16_t constSize_Dh;	// const 40bytes: size of RID64_Dh
		uint16_t constSize_Ih;	// const 32bytes: size of RID64_Ih
		uint8_t constMax_RID_idx;	// const 32: maximum (bit)size of RID, 2 as the base
		uint8_t constMax_entry_idx;	// const 19: maximum (bit)size of .entry section, 2 as the base
		uint8_t constMax_name_idx;	// const 19: maximum (bit)size of .name section, 2 as the base 
		uint8_t constMax_list_idx;	// const 31: maximum (bit)size of .list section, 2 as the base
	};
};
#define SZ_RID64_Dh (sizeof(RID64_Dh))

#define RID_Ih RID64_Ih
struct RID64_Ih {
	struct {
		uint32_t constMax_node;	// const 2^10: maximum number of nodes
		uint32_t constMax_entry;	// const 2^13: maximum number of entries
		uint32_t constSize_node;	// const 2^5bytes: size of node, however in expanded version, that could be 2^8bytes
		uint32_t constSize_entry;	// const 2^3bytes: size of entry, and that is why all the struct in this standard is prefixed with RID64_
	};
	uint32_t currentNum_entry;	// current number of entries
	uint32_t currentNum_web;	// current number of web catagorized
	uint8_t Hash;	// type of Hash, hashStandard by default
	uint8_t ABAND;	// decide whether to ignore num_node_H, name_off_H or init_off_H, 1 to ignore
	struct {
		uint16_t r1;
		uint16_t r2;
		uint16_t r3;
	};	// reserved room
};
#define SZ_RID64_Ih (sizeof(RID64_Ih))
#define hashStandard 0

#define RID_En RID64_En
struct RID64_En {
	struct {
		uint16_t Valid : 1;	// entry validity
		uint16_t Fort : 1;	// used for quick check, if entry exactly matches the "cave"
		uint16_t tag : 13;// used for quick check, storing tag of stored entry, which may not be coherent to whose sequence number(if not Fort, then not equal at any time), in some cases it becomes the middle 13 bits of init offset
		uint16_t Type : 1;	// define the type of the list it points to, 
							// 0 denotes that the list start at init, bearing {num_node_H:num_node} entries, 
							// 1 denotes that the list start at init and stop at particular node, 0xFFFF, while num_node and num_node_H is meaningless
	};
	uint16_t name_off;	// lower 16 bits of name offset
	uint16_t init_off;	// lower 16 bits of list head offset
	uint8_t num_node;	// lower 8 bits of the number of nodes
	struct {
		uint8_t num_node_H : 2;	// higher 2 bits of the number of nodes， whether abandoned or not decided by .index section
		uint8_t name_off_H : 3;	// higher 3 bits of name offset, whether abandoned or not decided by .index section 
		uint8_t init_off_H : 3;	// higher 3 bits of list head offset,  whether abandoned or not decided by .index section
	};
};
#define SZ_RID64_En (sizeof(RID64_En))

#define RID_Nd RID64_Nd
struct RID64_Nd {
	uint32_t web;
	uint32_t hit;
};
#define SZ_RID64_Nd (sizeof(RID64_Nd))
#endif

// RID assistance Func
#define RID_Dh_init RID64_Dh_init
#define RID_Dh_copy RID64_Dh_copy
void RID64_Dh_init(RID64_Dh &tar);
void RID64_Dh_copy(RID64_Dh &dest, RID64_Dh src);
void RID64_Ih_init(RID64_Ih &tar);
void RID64_En_init(RID64_En &tar);

// RID unit definition
// classes in this section manage the load, store, modify and initiate of the different sections
// the section includes: database heading, entry section, list section namesection and index heading
// the database heading tells the basic infomation of the RID file
// 
class section_entry {
	uint32_t maxSize;
	uint32_t numCurrent;
	std::streampos startpos;
	uint32_t entryConfig;
	uint32_t H_abandon;
	bool security;
	//	void flush(fstream &fn);
public:
	section_entry(std::streampos position, std::fstream &fn) {
		maxSize = (~(0xffffffff << maxEntry_idx));
		numCurrent = 0;
		startpos = position;
		entryConfig = sizeNode;
		H_abandon = 1;
		RID_En flushentry[0x000000ff + 1];
		for (auto &initflush : flushentry) {
			initflush.Valid = initflush.Type = initflush.Fort = 0;
			initflush.tag = 0;
			initflush.init_off = initflush.init_off_H = 0;
			initflush.name_off = initflush.name_off_H = 0;
			initflush.num_node = initflush.num_node_H = 0;
		}
		fn.seekp(startpos);
		for (unsigned i = 0; i < (0x00001fff + 1) / (0x000000ff + 1); i++) {
			fn.write((char*)(void*)&flushentry, sizeof(flushentry));
		}
		security = true;
	}
	void flush(std::fstream &fn);
	void write(uint32_t name_offset, std::pair<int, int> listInfo, uint16_t afterhash, std::fstream &fn);
	uint32_t isAbandon() const;
	std::streampos position() const;
	uint32_t numEntry() const;
	void disArm();
};
class list_section {
	RID64_Nd *section_buf;
	const int bufSize_const;
	unsigned maxSize;
	unsigned bufSize;
	unsigned bufNum_max;
	std::streampos listSection_pos;
	int accCount;
	int ptrCurrent;
	int numCurrent;
	unsigned sizeCurrent;
	uint32_t store(std::fstream &fn, uint32_t size);
public:
	list_section(std::streampos position) : bufSize_const(0x00000fff + 1) {
		bufSize = bufSize_const;
		maxSize = (~(0xffffffff << maxList_idx));
		bufNum_max = bufSize / sizeNode;
		section_buf = new RID64_Nd[bufNum_max + 1];
		accCount = 0;
		ptrCurrent = 0;
		numCurrent = 0;
		listSection_pos = position;
		sizeCurrent = 0;
	}
	~list_section() {
		delete[]section_buf;
	}
	std::pair<int, int> write(infoCollector_node *head, std::fstream &fn);
	std::streampos positionCurrent() const;
};
class name_section {
	char *section_buf;
	unsigned maxSize;
	int ptrCurrent;
	int numCurrent;
public:
	name_section() {
		maxSize = (~(0xffffffff << maxName_idx));
		section_buf = new char[maxSize + 1];
		section_buf[maxSize] = '\0';
		ptrCurrent = numCurrent = 0;
	}
	~name_section() {
		delete[]section_buf;
	}
	uint32_t write(std::string request);
	std::string read(uint32_t ask_offset);
	uint32_t end() const;
	uint32_t num() const;
	uint32_t store(std::fstream &fn);
	uint32_t store_align_4k(std::fstream &fn);
};

// integral operations
extern std::string INDEX_DIR_PATH;	// 索引文件路径
extern std::string filenameRID;	//RID文件名
extern int init_stat;	//状态
extern int compulsory_write_RID;	//强制写入符号

// 状态值及对应提示语句
enum {
	NOT_START, COLLECTING, INFO_COLLECTED, RID_PREPARED,
	WRITING_ENTRY, WRITEN_ENTRY, WRITING_NAME, WRTEN_NAME, WRITE_HEADS, WRITEN_HEADS, RID_COMPLETE, LOADING, LOAD_COMPLETE
};// 状态值
static std::string RID_NOTICE[] = { "...\n","Collecting informations from INDEX files\n", "...Finished\n", "RID Prepared\n",
"Writing RID: Entry & List section\n", "..Finished\n",
"Writing RID: Name section\n", "..Finished\n" ,
"Writing RID: Index & Database Heads\n", "...Finished\n", "___COMPETE___\n", "Loading...\n", "___READY___\n" };

void RID_initiation(int webNum);
int RI_itsc_symdif(std::string *request, int szRequest,
	int *&intersection, int &szIntersection, int *&symDifference, int &szSymdifference);
int RI_exclude(std::string base, std::string *exclude, int szExclude, int *&result, int &szResult);
void RID_terminate();

#endif
