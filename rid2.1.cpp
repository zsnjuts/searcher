#include "rid.h"
#include "initridthread.h"
#include <cstring>
using namespace std;

// parameters to modify
//int init_stat;
int compulsory_write_RID;
std::string INDEX_DIR_PATH = "text-index\\";
std::string filenameRID = "RID.dat";

// local parameters
static stringstream stringstreambuf;
static map<string, infoCollector_node*> infoCollector_web;
static int maxWeb_num;
static const int sizeEntrysection = 0x0000ffff + 1;
static RID_En *entryBase;
static RID_Nd *listBase;
static char *nameBase;
static int entryNum, listNum, nameSize;
static int currentPage_num, isTerminate;
static const int page_maxWeb_size = 4000;

// information collector
/* LATEST: ALLOW PAGE WRITING
  * in this function, we will collect index information from index directory
  * the structure of this section is black-red tree
  * in terms of complex ops on the tree, the maxWeb_num is set to a const to avoid NO RESPONSE
  * this modification will smooth the procedure, which is a huge leap from the last version, which is troubled by uneven distribution of keywords
  * it is most costly procedure, please wait with patience
  */
void infoCollector(int webNum) {
	uint32_t currentWeb = maxWeb_num;
	string filePath;
	ifstream fn;
	infoCollector_web.clear();
	while (true) {
		// if size reach the top, abort to avoid complex ops
		if (infoCollector_web.size() > page_maxWeb_size) {
			currentWeb--;
			currentPage_num++;
			break;
		}
		stringstreambuf.clear();
		stringstreambuf << INDEX_DIR_PATH << currentWeb << "-index.txt";
		stringstreambuf >> filePath;
		fn.open(filePath, ios::in);
		if (!fn || currentWeb == webNum) {
			isTerminate = true;
			currentPage_num++;
			break;
		}
		string targetName;
		uint32_t targetFreq;
		while (true) {
			targetFreq = 0;
			fn >> targetName;
			if (fn.eof())
				break;
			if (targetName[0] <= '9' && targetName[0] >= '0') {
				fn >> targetName;
			}
			fn >> targetFreq;
			infoCollector_node *targetNode_ptr = new infoCollector_node;
			targetNode_ptr->index = currentWeb;
			targetNode_ptr->freq = targetFreq;
			targetNode_ptr->next = NULL;
			map<string, infoCollector_node*>::iterator iter;
			iter = infoCollector_web.find(targetName);
			if (iter == infoCollector_web.end()) {
				infoCollector_web.insert(pair<string, infoCollector_node*>(targetName, targetNode_ptr));
			}
			else {
				infoCollector_node* targetNode_scptr = iter->second;
				for (; targetNode_scptr->freq > targetNode_ptr->freq
					&& targetNode_scptr != NULL
					&&targetNode_scptr->next != NULL; targetNode_scptr = targetNode_scptr->next);
				if (targetNode_scptr == NULL)
					targetNode_scptr = iter->second;
				targetNode_ptr->next = targetNode_scptr->next;
				targetNode_scptr->next = targetNode_ptr;
			}
		}
		fn.close();
		currentWeb++;
	}
	maxWeb_num = currentWeb;
}

// RID assistant Func
/*this section is meant to facilicate the usage of RID units
 * including some ops of initiation and copy
 * on most occasions, it is not necessay to use them
 */
void RID64_Dh_init(RID64_Dh &tar) {
	tar.constMax_entry_idx = maxEntry_idx_Type_Trial_One;
	tar.constMax_list_idx = maxList_idx_Type_Trial_One;
	tar.constMax_name_idx = maxName_idx_Type_Trial_One;
	tar.constMax_RID_idx = maxRID_idx_Type_Trial_One;
	tar.constSize_Dh = SZ_RID64_Dh;
	tar.constSize_Ih = SZ_RID64_Ih;
	strcpy(tar.declaration, "RID");
	tar.entry_off = 0;
	tar.index_off = 0;
	tar.reserve = 0;
	tar.LastMod_Time = 0;
	tar.list_off = 0;
	tar.name_off = 0;
	tar.version = versionCode_Type_Trial_One;
}
void RID64_Dh_copy(RID64_Dh &dest, RID64_Dh src) {
	//memcpy_s(&dest, SZ_RID64_Dh, &src, SZ_RID64_Dh);
	memcpy(&dest, &src, SZ_RID64_Dh);
}
void RID64_Ih_init(RID64_Ih &tar) {
	tar.constMax_entry = maxEntry_Type_Trial_One;
	tar.constMax_node = maxNode_Type_Trial_One;
	tar.constSize_entry = sizeEntry_Type_Trial_One;
	tar.constSize_node = sizeNode_Type_Trial_One;
	tar.Hash = hashStandard;
	tar.ABAND = true;
	tar.r1 = tar.r2 = tar.r3 = 0;
	tar.currentNum_entry = tar.currentNum_web = 0;
}
void RID64_En_init(RID64_En &tar) {
	tar.Fort = 0;
	tar.init_off = 0;
	tar.init_off_H;
	tar.name_off = 0;
	tar.name_off_H = 0;
	tar.num_node = 0;
	tar.num_node_H = 0;
	tar.tag = 0;
	tar.Type = 0;
	tar.Valid = 0;
}

//RID unit Func
// in this part, we define internal functions
// you can have a look at the various ops that we offer
/*entry part*/
void section_entry::flush(fstream &fn) {
	if (security)
		return;
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
}
void section_entry::write(uint32_t name_offset, pair<int, int> listInfo, uint16_t afterhash, fstream &fn) {
	assert(numCurrent < maxEntry);
	assert(security);
	RID_En preEntry;
	RID_En entrybuf;
	//	RID64_En_init(preEntry);
	preEntry.Type = 0;
	preEntry.tag = afterhash;
	preEntry.num_node = listInfo.second & 0x000000ff;
	preEntry.num_node_H = (listInfo.second & 0x00000300) >> 8;
	if (preEntry.num_node_H)
		H_abandon = 0;
	preEntry.name_off = name_offset & 0x0000ffff;
	preEntry.name_off_H = (name_offset & 0x00070000) >> 16;
	if (preEntry.name_off_H)
		H_abandon = 0;
	preEntry.init_off = listInfo.first & 0x0000ffff;
	preEntry.init_off_H = (listInfo.first & 0x00070000) >> 16;
	if (preEntry.init_off_H)
		H_abandon = 0;
	fn.seekg(startpos);
	fn.seekg(afterhash * sizeof(entrybuf), ios::cur);
	fn.read((char*)(void*)&entrybuf, sizeof(entrybuf));
	if (entrybuf.Valid == 0) {
		preEntry.Fort = preEntry.Valid = 1;
		fn.seekp(startpos);
		fn.seekp(afterhash * sizeof(preEntry), ios::cur);
		fn.write((char*)(void*)&preEntry, sizeof(preEntry));
		numCurrent++;
		return;
	}
	else {
		int disp = 1;
		int count = 1;
		int32_t _afterhash;
		while (true) {
			_afterhash = int((afterhash + (count * disp *disp))) % maxEntry;
			while (_afterhash < 0)
				_afterhash += maxEntry;
			fn.seekg(startpos);
			fn.seekg(_afterhash * sizeof(entrybuf), ios::cur);
			fn.read((char*)(void*)&entrybuf, sizeof(entrybuf));
			if (entrybuf.Valid == 0) {
				preEntry.Fort = 0;
				preEntry.Valid = 1;
				fn.seekp(startpos);
				fn.seekp(_afterhash * sizeof(preEntry), ios::cur);
				fn.write((char*)(void*)&preEntry, sizeof(preEntry));
				numCurrent++;
				return;
			}
			else {
				if (count == -1)
					disp++;
				count = -count;
			}
		}
	}
}
uint32_t section_entry::isAbandon() const {
	return H_abandon;
}
streampos section_entry::position() const {
	return startpos;
}
uint32_t section_entry::numEntry() const {
	return numCurrent;
}
void section_entry::disArm() {
	security = false;
}
/*list part*/
pair<int, int> list_section::write(infoCollector_node *head, fstream &fn) {
	pair<int, int> result;
	if (head == NULL) {
		result.first = 0;
		result.second = 0;
		return result;
	}
	accCount = 0;
	ptrCurrent = 0;
	result.first = sizeCurrent;
	while (head != NULL) {
		section_buf[ptrCurrent].web = head->index;
		section_buf[ptrCurrent].hit = head->freq;
		accCount++;
		ptrCurrent++;
		sizeCurrent += sizeNode;
		if (head == NULL)
			break;
		if (ptrCurrent >= bufNum_max) {
			list_section::store(fn, bufSize);
			ptrCurrent = 0;
		}
		head = head->next;
	}
	list_section::store(fn, ptrCurrent * sizeof(RID64_Nd));
	result.second = accCount;
	return result;
}
uint32_t list_section::store(fstream &fn, uint32_t size) {
	//	assert(fn);
	char *transbuf = (char*)(void*)section_buf;
	fn.seekp(listSection_pos);
	fn.write(transbuf, size);
	listSection_pos = fn.tellp();
	return size;
}
streampos list_section::positionCurrent() const {
	return listSection_pos;
}
/*name part*/
uint32_t name_section::write(string request) {
	if (ptrCurrent + request.length() >= maxSize)
		return maxSize;
	uint32_t ptrStart = ptrCurrent;
	memcpy(section_buf + ptrCurrent, request.c_str(), request.length() + 1);
	ptrCurrent += request.length() + 1;
	numCurrent++;
	return ptrStart;
}
string name_section::read(uint32_t ask_offset) {
	if (ask_offset >= maxSize)
		return "";
	if (ask_offset > 0 && section_buf[ask_offset - 1] != '\0') {
		for (; ask_offset > 0 && section_buf[ask_offset] != '\0'; ask_offset--);
		if (ask_offset > 0)
			ask_offset++;
	}
	string result = section_buf + ask_offset;
	return result;
}
uint32_t name_section::end() const {
	return maxSize;
}
uint32_t name_section::num() const {
	return numCurrent;
}
uint32_t name_section::store(fstream &fn) {
	if (!fn)
		return 0;
	if (ptrCurrent == 0)
		return 0;
	fn.write(section_buf, ptrCurrent);
	return ptrCurrent;
}
uint32_t name_section::store_align_4k(fstream &fn) {
	if (!fn)
		return 0;
	if (ptrCurrent == 0)
		return 0;
	fn.write(section_buf, ptrCurrent);
	return (ptrCurrent & 0xfffff000) + 0x00000100;
}

// integral operations
/*this is the major interface of RID
 * HASH_MAPING provides hash function to act hash mapping, in this scenario we apply direct folding
 * writeRID write file according to web
 * RID_initiator_internal is a slight collection of initiation
 * RID_loader load file to memory, which is a legacy of last version and in order to quicken the process
 * RID_initiation provide alternative initiation ways, which is a legacy as well
 * findKey is specialized function in order find keywords in the files
 * mark mark the existence of target string, which is prepared for further logic algebra
 * RI_itsc_symdif shows the intersection and symmetry differences of requests
 * RID_terminate release unused memory space, which is a legacy as well
 */
#define nameBase_location(afterhash) (((entryBase[afterhash].name_off_H << 16) + entryBase[afterhash].name_off))
#define listBase_location(afterhash) (((entryBase[afterhash].init_off_H << 16) + entryBase[afterhash].init_off))
#define nodeNum(afterhash) (((entryBase[afterhash].num_node_H << 8) + entryBase[afterhash].num_node))
uint16_t HASH_MAPPING(string request) {
	uint8_t num = request.length() / 3 + 1;
	uint16_t resultbuf = 0;
	union {
		char charbuf[3];
		uint16_t algobuf;
	} dirtrans;
	char *strbuf = new char[request.length() + 1];
	strncpy(strbuf, request.data(), request.length() + 1);
	for (int base = 0; base < num; base += 3) {
		dirtrans.algobuf = 0;
		for (int cpsc = 0; cpsc < 3; cpsc++)
			dirtrans.charbuf[cpsc] = strbuf[base + cpsc];
		resultbuf += dirtrans.algobuf;
	}
	delete[]strbuf;
	return (resultbuf * resultbuf) & 0x00001fff;
}

// LATEST: ALLOW PAGE WRITING
bool InitRidThread::writeRID(fstream &fn) {
	//	assert(fn);
	streampos Dhstartpos = fn.tellp();
	fn.seekp(sizeof(RID64_Dh), ios::beg);
	streampos Enstartpos = fn.tellp();
	fn.seekp(sizeEntrysection, ios::cur);
	streampos Liststartpos = fn.tellp();
	name_section sectionName_man;
	list_section sectionList_man(Liststartpos);
	section_entry sectionEntry_man(Enstartpos, fn);

    //init_stat = WRITING_ENTRY;
    //emit showProgress(init_stat);
	map<std::string, infoCollector_node*>::iterator writeScan;
	string namebuf;
	infoCollector_node *listbuf;
	for (auto writeScan : infoCollector_web) {
		namebuf = writeScan.first;
		listbuf = writeScan.second;
		uint16_t afterhash = HASH_MAPPING(namebuf);
		uint32_t nameLocation = sectionName_man.write(namebuf);
		pair<int, int>listReply = sectionList_man.write(listbuf, fn);
		sectionEntry_man.write(nameLocation, listReply, afterhash, fn);
	}
    //init_stat = WRITEN_ENTRY;
    //emit showProgress(init_stat);

    //init_stat = WRITING_NAME;
    //emit showProgress(init_stat);
	streampos Nmstartpos = sectionList_man.positionCurrent();
	fn.seekp(Nmstartpos);
	sectionName_man.store(fn);
    //init_stat = WRTEN_NAME;
    //emit showProgress(init_stat);

    //init_stat = WRITE_HEADS;
    //emit showProgress(init_stat);
	streampos Idstartpos = fn.tellp();
	RID_Ih indexHead;
	RID64_Ih_init(indexHead);
	indexHead.ABAND = sectionEntry_man.isAbandon();
	indexHead.currentNum_entry = infoCollector_web.size();
	indexHead.currentNum_web = maxWeb_num;
	fn.write((char*)(void*)&indexHead, sizeof(indexHead));

	RID_Dh databaseHead;
	RID64_Dh_init(databaseHead);
	databaseHead.LastMod_Time = time(NULL);
	databaseHead.index_off = Idstartpos;
	databaseHead.entry_off = Enstartpos;
	databaseHead.list_off = Liststartpos;
	databaseHead.name_off = Nmstartpos;
	fn.seekp(Dhstartpos);
	fn.write((char*)(void*)&databaseHead, sizeof(databaseHead));
    //init_stat = WRITEN_HEADS;
    //emit showProgress(init_stat);

	return true;
}
void InitRidThread::RID_initiator_internal(int webNum) {
	init_stat = NOT_START;
    emit showProgress(init_stat);
	init_stat = COLLECTING;
    emit showProgress(init_stat);
	maxWeb_num = 0;
	currentPage_num = 0;
	isTerminate = false;
    init_stat = INFO_COLLECTED;
    emit showProgress(init_stat);
	while (!isTerminate) {
		infoCollector(webNum);
		ofstream _fn;
		string cur_filenameRID;
		stringstreambuf.clear();
		stringstreambuf << filenameRID << "Page" << currentPage_num;
		stringstreambuf >> cur_filenameRID;
		_fn.open(cur_filenameRID, ios::out | ios::binary);
		_fn << "If you see these words, it indicates the failure of creating a RID file";
		_fn.close();
		fstream fn;
		assert(fn);
		fn.open(cur_filenameRID, ios::out | ios::binary | ios::in);
		writeRID(fn);
		fn.close();
	}
	init_stat = RID_COMPLETE;
    emit showProgress(init_stat);
}
void RID_loader(ifstream &fn) {
	assert(fn);
	RID_Dh datbse_head;
	fn.seekg(ios::beg);
	fn.read((char*)(void*)&datbse_head, sizeof(datbse_head));
	if (strcmp(datbse_head.declaration, "RID")) {
		exit(0);
	}
	streampos entryStart = datbse_head.entry_off;
	streampos listStart = datbse_head.list_off;
	streampos nameStart = datbse_head.name_off;
	streampos indexStart = datbse_head.index_off;

	fn.seekg(entryStart);
	entryNum = (listStart - entryStart) / sizeof(RID_En);
	entryBase = new RID_En[entryNum];
	fn.read((char*)(void*)entryBase, sizeof(RID_En)*entryNum);

	fn.seekg(listStart);
	listNum = (nameStart - listStart) / sizeof(RID_Nd);
	listBase = new RID_Nd[listNum];
	fn.read((char*)(void*)listBase, sizeof(RID_Nd)*listNum);

	fn.seekg(nameStart);
	nameSize = (indexStart - nameStart);
	nameBase = new char[nameSize];
	fn.read(nameBase, nameSize);
}
void InitRidThread::RID_initiation(int webNum) {
	ifstream attempt;
	attempt.open(filenameRID, ios::in | ios::binary);
	if (!attempt || compulsory_write_RID) {
	WRITE:
		compulsory_write_RID = false;
		attempt.close();
		RID_initiator_internal(webNum);
	}
	else {
		RID_Dh bufDh;
		attempt.read((char*)(void*)&bufDh, sizeof(bufDh));
		if (strcmp(bufDh.declaration, "RID"))
			goto WRITE;
		streampos Ihpos = bufDh.index_off;
		attempt.seekg(Ihpos, ios::beg);
		RID_Ih bufIh;
		attempt.read((char*)(void*)&bufIh, sizeof(bufIh));
		maxWeb_num = bufIh.currentNum_web;
		attempt.close();
	}
    qDebug() << "RID initiation ok";
}

// INTERNAL FUNCS: HIDDEN FROM THE OUTSIDE
// this function find the location where tar lies in the loader buf
// the return value tells the location, -1 if not finding it
int findKey(string tar) {
	assert(entryBase&&listBase&&nameBase);
	uint16_t afterhash = HASH_MAPPING(tar);
	if (!entryBase[afterhash].Valid)
		return -1;
	if (entryBase[afterhash].Fort) {
		char *name = &nameBase[nameBase_location(afterhash)];
		if (!strcmp(name, tar.data()))
			return afterhash;
	}

	int sign = 1;
	int i = 1;
	int curLocation = (afterhash + sign*i*i) % entryNum;
	while (curLocation < 0)
		curLocation += entryNum;
	while (entryBase[curLocation].Valid) {
		if (!entryBase[curLocation].Fort) {
			char *name = &nameBase[nameBase_location(curLocation)];
			if (!strcmp(name, tar.data()))
				return curLocation;
		}
		if (sign == -1)
			i++;
		sign = -sign;
		curLocation = (afterhash + sign*i*i) % entryNum;
		while (curLocation < 0)
			curLocation += entryNum;
	}
	return -1;
}

// INTERNAL FUNCS: HIDDEN FROM THE OUTSIDE
// this function create a bitVector according to the request
int mark(string token, int *request) {
	ifstream attempt;
    //init_stat = LOADING;
	int currentPage_idx = 1;
	int SIG = 0;
	string cur_filenameRID;
	for (; currentPage_idx <= currentPage_num; currentPage_idx++) {
		stringstreambuf.clear();
		stringstreambuf << filenameRID << "Page" << currentPage_idx;
		stringstreambuf >> cur_filenameRID;
		attempt.open(cur_filenameRID, ios::in | ios::binary);
		RID_loader(attempt);
		attempt.close();
        //init_stat = LOAD_COMPLETE;
		int locationFirst = findKey(token);
		if (locationFirst != -1) {
			for (int firstcount = 0; firstcount < nodeNum(locationFirst); firstcount++) {
				RID_Nd *node = (RID_Nd*)((char*)(void*)listBase + listBase_location(locationFirst)) + firstcount;
				request[node->web] = true;
			}
			SIG = 1;
		}
		delete[]entryBase;
		delete[]nameBase;
		delete[]listBase;
	}
	return SIG;
}

// NOTICE: this function tells the intersection and symmetry difference of all the request strings
// the return value is the size of the union of the request strings
// request ptr is allocated from the outside, while intersection ptr and symDifference ptr otherwise
int RI_itsc_symdif(string *request, int szRequest, int *&intersection, int &szIntersection, int *&symDifference, int &szSymdifference) {
	if (!maxWeb_num)
		return 0;
	int *buf = new int[maxWeb_num];
	int *_union = new int[maxWeb_num];
	int *_intsc = new int[maxWeb_num];
    for(int initsc = 0; initsc < maxWeb_num; initsc++){
        buf[initsc] = _union[initsc] = 0;
        _intsc[initsc] = 1;
    }
    //qDebug() << szRequest;
	for (int requestsc = 0; requestsc < szRequest; requestsc++) {
		int chk = mark(request[requestsc], buf);
        //qDebug() << buf[0] << " " << buf[1];
		if (chk) {
			for (int i = 0; i < maxWeb_num; i++) {
				if (buf[i] || _union[i])
					_union[i] = 1;
				if (buf[i] && _intsc[i])
					_intsc[i] = 1;
				else
					_intsc[i] = 0;
				buf[i] = 0;
			}
		}
		else {
			for (int i = 0; i < maxWeb_num; i++)
				_intsc[i] = 0;
		}
	}
	intersection = new int[maxWeb_num];
	symDifference = new int[maxWeb_num];
	int szUnion = 0;
	int symDifference_idx = 0, intersection_idx = 0;
	for (int i = 0; i < maxWeb_num; i++) {
		if (_union[i]) {
			szUnion++;
			if (!_intsc[i]) {
				symDifference[symDifference_idx] = i;
				symDifference_idx++;
			}
		}
		if (_intsc[i]) {
			intersection[intersection_idx] = i;
			intersection_idx++;
		}
	}
	szIntersection = intersection_idx;
	szSymdifference = symDifference_idx;
	return szUnion;
}

// NOTICE: this function tells the set of webs which contains base but not exclude strings
// the math representation is:
//				x belongs to {webs | webs contains base, excluding those contain all the strings of exclude}
// the return value is the size of the result set
// result ptr is allocated from the inside, exclude ptr otherwise
int RI_exclude(string base, string *exclude, int szExclude, int *&result, int &szResult) {
	if (!maxWeb_num)
		return 0;
	int *buf = new int[maxWeb_num];
	int *basemark = new int[maxWeb_num];
	for (int initsc = 0; initsc < maxWeb_num; initsc++) {
		basemark[initsc] = buf[initsc] = 0;
	}
	if (mark(base, basemark) == 0)
		return 0;
	for (int requestsc = 0; requestsc < szExclude; requestsc++) {
		int chk = mark(exclude[requestsc], buf);
		if (chk) {
			for (int i = 0; i < maxWeb_num; i++) {
				if (buf[i] && basemark[i])
					basemark[i] = 0;
				buf[i] = 0;
			}
		}
	}
	result = new int[maxWeb_num];
	szResult = 0;
    int count_szResult = 0;
	for (count_szResult = 0; count_szResult < maxWeb_num; count_szResult++) {
		if (basemark[count_szResult]) {
			result[szResult] = count_szResult;
			szResult++;
		}
	}
	return szResult;
}
void RID_terminate() {
	return;
}
