// 不再严格划分 size，有连续空位置就存放
// sublime 里那段代码要设计一种算法更加合理地分配空间，不能让 16 号出现在所有盘里
// 存储的时候先存 size 大的，再存 size 小的，石头 -> 沙子 -> 水

// cur_score 必须每一轮都计算！不然这个分数就是扯淡
// 上线段树维护 score 信息，每 UPDATE_DISK_SCORE_FREQUENCY 更新一次，直接 init

// 跳跃的时候，不能只跳那 BLOCK_NUM 个点，这绝对是不行的
// 锁机制，对于当前这个磁头即将读取的一部分 obj，我认为这就是它认定的东西，别人考虑分数的时候就不应该再计算这些 obj 的得分了！
// 存放 obj 的时候，优先放到当前磁头的前方？便于快速读取到

// 跳转到那之后第一个有东西的地方？
// 现在这一发是调了一些参数的，即将交一发把 LOCK_UNITS 按比例调成 1600 的，这是核心！！！


#include <bits/stdc++.h>
using namespace std;

// #define DEBUG

#ifdef DEBUG
#define UPDATE_DISK_SCORE_FREQUENCY (10)
#define MAX_DISK_SIZE (13136)
#define BLOCK_NUM (20)
const int BLOCK_SIZE = MAX_DISK_SIZE / BLOCK_NUM;
#define LOCK_UNITS (1320)
#define LOCK_TIMES (LOCK_UNITS / (340 / 16))
#else
#define UPDATE_DISK_SCORE_FREQUENCY (10)
#define MAX_DISK_SIZE (16384)
#define BLOCK_NUM (20) // 这个参数调大一点可能会变好，比如 20
const int BLOCK_SIZE = MAX_DISK_SIZE / BLOCK_NUM;
#define LOCK_UNITS (825)
#define LOCK_TIMES (LOCK_UNITS / (500 / 16))
#endif

// BLOCK_NUM = 20
// LOCK_UNITS = 1800
// DROP_SCORE = 0.05
// LOCK_TIMES += 3 = JUMP_BIAS 111
// BLOCK_NUM = 25 111
//
// BLOCK_NUM = 25, LOCK_TIMES += 3 = JUMO_BIAS
// BLOCK_NUM = 25, LOCK_TIMES += 5 = JUMO_BIAS
// 莫名其妙 Write
// Hot_Tag
// 重新染色
// PREDICT = 3
// JUMP_BIAS = LOCK_TIMES + 5
// LOCK_UNITS = 1650, JUMP_BIAS = LOCK_TIMES + 5
// LOCK_UNITS = 1650
// Write, LOCK_UNITS = 1650
// Write, LOCK_TIMES = /20
// LOCK_TIMES = /20
// LOCK_UNITS = 1650
// LOCK_UNITS = 1750
// LOCK_UNITS = 1850
// 1650, BLOCK_NUM = 21, UPDATE_DISK = 8
// 1638, BLOCK_NUM = 22, UPDATE_DISK = 8


#define MAX_TAG (16)
#define MAX_SIZE (5)
#define MAX_DISK_NUM (10)

const int DISK_SPLIT_BLOCK = MAX_DISK_SIZE / 35.7;
const int DISK_SPLIT_1 = DISK_SPLIT_BLOCK * 6;
const int DISK_SPLIT_2 = DISK_SPLIT_BLOCK * 14;
const int DISK_SPLIT_3 = DISK_SPLIT_BLOCK * 24.5;
const int DISK_SPLIT_4 = DISK_SPLIT_BLOCK * 31.7;
const int DISK_SPLIT_5 = DISK_SPLIT_BLOCK * 35.7;
// 60 : 40 : 35 : 18 : 8     sum = 161

// #define JUMP_FREQUENCY (8)
#define JUMP_BIAS (LOCK_TIMES + 3)

#define MAX_QUERY_TIME (105)
#define MAX_HEAD_NUM (2)
#define MAX_REQUEST_NUM (30000000)
#define MAX_OBJECT_NUM (100000)
#define REP_NUM (3)
#define FRE_PER_SLICING (1800)
#define EXTRA_TIME (105)
#define MAX_EPOCH (50)
#define MAX_WRITE_LEN (100005)
#define INF (1000000000)
#define EPS (1e-6)
#define PREDICT (2) // 没道理，因为一轮扫不到一块

#define DROP_SCORE (0)
#define DECIDE_CONTINUE_READ (10)
#define TRASH_PERPORTION (0.05)


#define BLOCK_BIAS (5)

#define SEED (11111111)

vector<float> query_times = {0, 1951, 1507, 2660, 1466, 1125, 1352, 1490, 2159, 3368, 2893, 3799, 1606, 1834, 2158, 1421, 2540, };
vector<float> weight(MAX_TAG + 1, 1);

vector<vector<vector<pair<int, int>>>> priority_pos(MAX_TAG + 1, vector<vector<pair<int, int>>>(MAX_SIZE + 1));

int T, M, N, V, G, K;
int fre_del[MAX_TAG + 1][MAX_EPOCH];
int fre_write[MAX_TAG + 1][MAX_EPOCH];
int fre_read[MAX_TAG + 1][MAX_EPOCH];

vector<vector<tuple<int, int, int>>> disk_manage; // 前提是必须有 10 块！
vector<vector<int>> disk_select;
int pre_cost[MAX_DISK_NUM][MAX_HEAD_NUM];
char pre_move[MAX_DISK_NUM][MAX_HEAD_NUM];
vector<vector<int>> pre_jump(MAX_DISK_NUM, vector<int>(MAX_HEAD_NUM, -JUMP_BIAS));
vector<vector<bool>> pre_decide(MAX_DISK_NUM, vector<bool>(MAX_HEAD_NUM, false));

int timestamp = 0; // 全局时间戳

// mt19937_64 rng(chrono::steady_clock::now().time_since_epoch().count());
mt19937_64 rng(SEED);

vector<vector<int>> hot_tags = {
	{},
	{7, 1, 3, },
	{1, 2, 7, },
	{1, },
	{1, },
	{1, },
	{1, 4, 6, },
	{6, 4, 11, },
	{6, 11, 2, },
	{6, 11, 5, },
	{6, 5, 11, },
	{6, 15, 2, },
	{15, 6, 1, },
	{1, 15, 14, },
	{1, 15, 14, },
	{15, 1, 14, },
	{15, 14, 1, },
	{12, 15, 14, },
	{12, },
	{12, },
	{12, 16, 8, },
	{16, 3, 10, },
	{3, 16, 10, },
	{11, 10, 3, },
	{11, 1, 10, },
	{1, 5, 11, },
	{1, 5, 9, },
	{1, 5, 9, },
	{9, 1, 5, },
	{9, },
	{9, 10, 14, },
	{9, 10, 15, },
	{15, 10, 9, },
	{15, 4, 9, },
	{15, 4, 8, },
	{8, 16, 4, },
	{8, 16, 2, },
	{16, 8, 12, },
	{16, 12, 2, },
	{16, 13, 12, },
	{16, 13, 4, },
	{16, 3, 4, },
	{3, 16, 4, },
	{2, 16, 3, },
	{16, 2, 3, },
	{16, 2, 3, },
	{16, 3, 4, },
	{8, 12, 15, },
	{9, 8, 12, },	
};


// ------------------------------------ 全局函数声明 ----------------------------------------

float Get_Pos_Score(int disk_id, int pos, int time);   // 获取一个硬盘上一个位置 pos 的得分
void Pre_Process(); 					     		   // 对总和输入数据的预处理
void total_init();						     		   // 预处理乱七八糟的东西，比如 disk 的余量集合
bool Random_Appear(int p);				     		   // 判断概率 p% 是否发生
pair<float, int> Simulate(int disk_id, int idx, int time, char &pre_mov, int &pre_cos);



// ------------------------------------ 结构体定义 ----------------------------------------

struct Disk {
	pair<int, int> d[MAX_DISK_SIZE];
	set<int> space[MAX_SIZE + 1];
	vector<int> has_tag;
	int color_tag[MAX_DISK_SIZE];
	float score[BLOCK_NUM];
	vector<int> head;
	int siz = 0;
	int disk_id = 0;
	int max_score_pos = 0;  // 价值最大块位置
	float max_score = -1;  // 价值最大块价值
	vector<float> cur_score; // 当前位置向前走 PREDICT 个块的价值
	int working = -1;
	vector<int> color_start;
	Disk() {
		head.resize(2, 0);
		cur_score.resize(2, 0);
		for (int i = 0; i < MAX_DISK_SIZE; i++) {
			d[i] = {-1, -1};
			color_tag[i] = -1;
		}
	}
	int size() {
		return siz;
	}
	bool capacity(int obj_tag, int obj_size, int is_limit) {
		bool cap = 0;
		if (is_limit == -1) {
			for (int i = 0; i < V - obj_size; i++) {
				if (color_tag[i] != obj_tag || d[i].first != -1) continue;
				bool ok = 1;
				for (int j = i + 1; j < i + obj_size; j++) {
					if (color_tag[j] != obj_tag || d[j].first != -1) ok = 0;
				}	
				if (!ok) continue;
				cap = 1;
				break;
			}
		} else {
			assert(is_limit == -2);
			for (int i = 0; i < V - obj_size; i++) {
				if (d[i].first != -1) continue;
				bool ok = 1;
				for (int j = i + 1; j < i + obj_size; j++) {
					if (d[j].first != -1) ok = 0;
				}	
				if (!ok) continue;
				cap = 1;
				break;
			}
		}

		// is_limit 代表的是是否要强制这个 obj 放在自己 tag 的位置里
		// if (is_limit == -2) return space[obj_size].size() > 0;
		// assert(is_limit == -1);
		// bool cap = 0;
		// for (auto pos : space[obj_size]) {
		// 	if (color_tag[pos] == obj_tag) {
		// 		cap = 1;
		// 		break;
		// 	}
		// }
		return cap;
	}

	void Color() {
		int tot = 0;
		for (auto tag : has_tag) {
			tot += query_times[tag];
		}
		int idx = 0;
		for (auto tag : has_tag) {
			color_start.push_back(idx);
			int len = 1. * query_times[tag] / tot * V * (1 - TRASH_PERPORTION);
			for (int cnt = len; idx < V * (1 - TRASH_PERPORTION) && cnt--; idx++) {
				color_tag[idx] = tag;
			}
		}
		// int shift = rng() % V;
		// auto y = color_tag;
		// for (int i = 0; i < V; i++) {
		// 	color_tag[i] = y[(i + shift) % V];
		// }
	}

	void Cal_Current_Score() {
		for (int head_idx = 0; head_idx < MAX_HEAD_NUM; head_idx++) {
			cur_score[head_idx] = 0;
			int i = head[head_idx];
			// char pre_mov = pre_move[disk_id];
			// int pre_cos = pre_cost[disk_id];
			for (int t = timestamp; t < timestamp + PREDICT; t++) {
				// auto [score, idx] = Simulate(disk_id, i, t, pre_mov, pre_cos);
				// i = idx;
				// cur_score[head_idx] += score;
				for (int cnt = BLOCK_SIZE; cnt--; i = (i + 1) % V) {
					cur_score[head_idx] += Get_Pos_Score(disk_id, i, t);
				}
			}
		}
	}

	void Cal_Max_Score() {
		max_score = -1;
		int block = -1;
		for (int i = 0; i < BLOCK_NUM; i++) {
			if (score[i] > max_score) {
				max_score = score[i];
				block = i;
			}
		}
		assert(block != -1);
		max_score_pos = block * BLOCK_SIZE;
		while (d[max_score_pos].first == -1) {
			max_score_pos = (max_score_pos + 1) % V;
		}
	}
	
	void Cal_Block_Score() {
		for (int block = 0; block < BLOCK_NUM; block++) {
			int i = block * BLOCK_SIZE;
			float sc = 0;
			char pre_mov = 'j';
			int pre_cos = 0;
			for (int t = timestamp; t < timestamp + PREDICT; t++) {
				// auto [score, idx] = Simulate(disk_id, i, t, pre_mov, pre_cos);
				// i = idx;
				// sc += score;
				for (int cnt = BLOCK_SIZE; cnt--; i = (i + 1) % V) {
					sc += Get_Pos_Score(disk_id, i, t);
				}
			}
			score[block] = sc;

			// int l = block * BLOCK_SIZE;
			// int r = l + BLOCK_SIZE - 1;
			// float sc = 0;
			// for (int i = l; i <= r; i++) {
			// 	sc += Get_Pos_Score(disk_id, i, timestamp);
			// }
			// score[block] = sc;
		}
	}
	void Cal_Score() {	    // 计算走 PREDICT - 1 个块的价值
		Cal_Block_Score();
		Cal_Max_Score();
		Cal_Current_Score();
	}
	int Write(int obj_id, int obj_size, int obj_tag, int is_limit) {
		/*
		is_limit:
		-1: 限制必须找到当前颜色
		-2: 不限制颜色
		0~V-1: 直接放到对应位置上去
		*/
		int write_idx = -1;
		siz += obj_size;

		if (is_limit >= 0) {
			write_idx = is_limit;
			for (int i = write_idx, size = 0; size < obj_size; i++, size++) {
				d[i] = {obj_id, size};
			}
			write_idx = write_idx;
			// assert(space[obj_size].count(write_idx));
			// space[obj_size].erase(write_idx);
			return write_idx;
		}

		// if (is_limit == -1) {
		// 	for (int i = 0; i < V - obj_size; i++) {
		// 		if (color_tag[i] != obj_tag || d[i].first != -1) continue;
		// 		bool ok = 1;
		// 		for (int j = i + 1; j < i + obj_size; j++) {
		// 			if (color_tag[j] != obj_tag || d[j].first != -1) ok = 0;
		// 		}	
		// 		if (!ok) continue;
		// 		int continue_white = 1;
		// 		int j = i + 1;
		// 		while (j < V && continue_white < BLOCK_BIAS) {
		// 			continue_white++;
		// 			j++;
		// 		}
		// 		j = i - 1;
		// 		while (j >= 0 && continue_white < BLOCK_BIAS) {
		// 			continue_white++;
		// 			j--;
		// 		}
		// 		if (continue_white == BLOCK_BIAS) continue;
		// 		write_idx = i;
		// 		break;
		// 	}
		// } else {
		// 	assert(is_limit == -2);
		// 	for (int i = 0; i < V - obj_size; i++) {
		// 		if (color_tag[i] != -1) continue; 
		// 		if (d[i].first != -1) continue;
		// 		bool ok = 1;
		// 		for (int j = i + 1; j < i + obj_size; j++) {
		// 			if (d[j].first != -1) ok = 0;
		// 		}	
		// 		if (!ok) continue;
		// 		int continue_white = 1;
		// 		int j = i + 1;
		// 		while (j < V && continue_white < BLOCK_BIAS) {
		// 			continue_white++;
		// 			j++;
		// 		}
		// 		j = i - 1;
		// 		while (j >= 0 && continue_white < BLOCK_BIAS) {
		// 			continue_white++;
		// 			j--;
		// 		}
		// 		if (continue_white == BLOCK_BIAS) continue;
		// 		write_idx = i;
		// 		break;
		// 	}
		// 	if (write_idx == -1) {
		// 		for (int i = 0; i < V - obj_size; i++) {
		// 			if (d[i].first != -1) continue;
		// 			bool ok = 1;
		// 			for (int j = i + 1; j < i + obj_size; j++) {
		// 				if (d[j].first != -1) ok = 0;
		// 			}	
		// 			if (!ok) continue;
		// 			int continue_white = 1;
		// 			int j = i + 1;
		// 			while (j < V && continue_white < BLOCK_BIAS) {
		// 				continue_white++;
		// 				j++;
		// 			}
		// 			j = i - 1;
		// 			while (j >= 0 && continue_white < BLOCK_BIAS) {
		// 				continue_white++;
		// 				j--;
		// 			}
		// 			if (continue_white == BLOCK_BIAS) continue;
		// 			write_idx = i;
		// 			break;
		// 		}
		// 	}
		// }

		if (write_idx == -1) {
			if (is_limit == -1) {
				for (int i = 0; i < V - obj_size; i++) {
					if (color_tag[i] != obj_tag || d[i].first != -1) continue;
					bool ok = 1;
					for (int j = i + 1; j < i + obj_size; j++) {
						if (color_tag[j] != obj_tag || d[j].first != -1) ok = 0;
					}	
					if (!ok) continue;
					write_idx = i;
					break;
				}
			} else {
				assert(is_limit == -2);
				for (int i = 0; i < V - obj_size; i++) {
					if (color_tag[i] != -1) continue; 
					if (d[i].first != -1) continue;
					bool ok = 1;
					for (int j = i + 1; j < i + obj_size; j++) {
						if (d[j].first != -1) ok = 0;
					}	
					if (!ok) continue;
					write_idx = i;
					break;
				}
				if (write_idx == -1) {
					for (int i = 0; i < V - obj_size; i++) {
						if (d[i].first != -1) continue;
						bool ok = 1;
						for (int j = i + 1; j < i + obj_size; j++) {
							if (d[j].first != -1) ok = 0;
						}	
						if (!ok) continue;
						write_idx = i;
						break;
					}
				}
			}
		}



		
		
		assert(write_idx != -1);

		for (int i = write_idx, size = 0; size < obj_size; i++, size++) {
			d[i] = {obj_id, size};
		}
		
		return write_idx;


		// assert(space[obj_size].size());
		// auto it = space[obj_size].begin();
		// while (is_limit == -1 && it != space[obj_size].end() && color_tag[*it] != obj_tag) {
		// 	it++;
		// }
		// assert(it != space[obj_size].end());
		
		// for (int i = *it, size = 0; size < obj_size; i++, size++) {
		// 	d[i] = {obj_id, size};
		// }
		// write_idx = *it;
		// space[obj_size].erase(it);
		
		// return write_idx;
	}
	void erase(int erase_idx) {
		assert(d[erase_idx].first != -1);
		d[erase_idx] = {-1, -1};
		siz--;
	}
};
Disk disk[MAX_DISK_NUM]; // 硬盘

struct Request {
	int query_time = 0;
	int obj_id = -1;
	int ned = 0;
	int mask = -1;
	bool is_give_up = false;
	bool is_abort = false;
	bool is_done() {
		return ned == mask;
	}
	bool has_part(int part) {
		return ned >> part & 1;
	}
	void set(int part) {
		ned |= 1 << part;
	}
	void erase(int part) {
		ned &= ~(1 << part);
	}
};
vector<Request> requests(MAX_REQUEST_NUM + 1); // 使用 vector 是因为静态空间不够了
vector<queue<int>> query_per_time(MAX_SIZE + 1); // 必须 tmd 使用 vector，不然会莫名其妙莫名其妙的卡死
// queue<int> query_per_time[MAX_SIZE + 1];


struct Object {
	vector<int> bel;
	int unit[REP_NUM];
	int size = 0;
	int tag = -1;
	bool is_delete = false;
	int lock = -1;
	int lock_time = 0;
	int lock_last = 0;
};
Object objects[MAX_OBJECT_NUM + 1];
unordered_set<int> query[MAX_OBJECT_NUM + 1]; // 每个对象的查询






// ------------------------------------ 全局预处理 ----------------------------------------

void Pre_Process() {
	// int batch_num = (T + FRE_PER_SLICING - 1) / FRE_PER_SLICING;
	// vector<int> tag(MAX_TAG + 1);
	// for (int i = 1; i <= MAX_TAG; i++) {
	// 	for (int j = 1; j <= batch_num; j++) {
	// 		cerr << fre_write[i][j] << ' ';
	// 		tag[i] += fre_read[i][j];
	// 	}
	// 	cerr << endl;
	// }
	// for (int i = 1; i <= MAX_TAG; i++) {
	// 	cerr << tag[i] << ", ";
	// }
	cout << "OK\n";
	cout.flush();
}

// 60 : 40 : 35 : 18 : 8     sum = 161
void total_init() {
	vector<int> best_w = {212, 149, 132, 68, 28};
	vector<int> count(17);
	for (int i = 0; i < MAX_DISK_NUM; i++) {
		disk[i].disk_id = i;
		vector<int> count(6);
		// int j = 0;
		// while (j < V) {
		// 	for (int c = best_w[0]; j < V && c--; j++) {
		// 		disk[i].space[1].insert(j);
		// 		count[1]++;
		// 	}
		// 	for (int c = best_w[1]; j + 1 < V && c--; j += 2) {
		// 		disk[i].space[2].insert(j);
		// 		count[2]++;
		// 	}
		// 	for (int c = best_w[2]; j + 2 < V && c--; j += 3) {
		// 		disk[i].space[3].insert(j);
		// 		count[3]++;
		// 	}
		// 	for (int c = best_w[3]; j + 3 < V && c--; j += 4) {
		// 		disk[i].space[4].insert(j);
		// 		count[4]++;
		// 	}
		// 	for (int c = best_w[4]; j + 4 < V && c--; j += 5) {
		// 		disk[i].space[5].insert(j);
		// 		count[5]++;
		// 	}
		// }

		int divide = 0;
		for (int j = 0; j < MAX_SIZE; j++) {
			divide += (j + 1) * best_w[j];
		}

		// 染色，对应位置只能存储对应 tag
		disk[i].Color();
		
		// int tot = 0;
		// for (auto tag : disk[i].has_tag) {
		// 	tot += query_times[tag];
		// }
		// int idx = 0;
		// cerr << "tot = " << tot << endl;
		// for (auto tag : disk[i].has_tag) {
		// 	int len = 1. * query_times[tag] / tot * V * (1 - TRASH_PERPORTION);
			// vector<int> cnt(MAX_SIZE);
			// for (int j = 0; j < MAX_SIZE; j++) {
			// 	cnt[j] = 1. * best_w[j] / divide * len;
			// }
			// int pre_idx = idx; 
			// if (i == 0) {
			// 	cerr << "i = 0, " << best_w[0] << ' ' << divide << ' ' << len << endl;
			// 	for (int j = 0; j < MAX_SIZE; j++) {
			// 		cerr << cnt[j] << ' ';
			// 	}
			// 	cerr << endl;
			// }

			// 把 size 大的放在前面，能提升一点点
			// for (int c = cnt[4]; idx + 4 < min(V, pre_idx + len) && c--; idx += 5) {
			// 	disk[i].space[5].insert(idx);
			// 	count[5]++;
			// }
			// for (int c = cnt[3]; idx + 3 < min(V, pre_idx + len) && c--; idx += 4) {
			// 	disk[i].space[4].insert(idx);
			// 	count[4]++;
			// }
			// for (int c = cnt[2]; idx + 2 < min(V, pre_idx + len) && c--; idx += 3) {
			// 	disk[i].space[3].insert(idx);
			// 	count[3]++;
			// }
			// for (int c = cnt[1]; idx + 1 < min(V, pre_idx + len) && c--; idx += 2) {
			// 	disk[i].space[2].insert(idx);
			// 	count[2]++;
			// }
			// for (int c = cnt[0]; idx < min(V, pre_idx + len) && c--; idx++) {
			// 	disk[i].space[1].insert(idx);
			// 	count[1]++;
			// }
			

		// 	idx = pre_idx;
		// 	for (int cnt = len; idx < V * (1 - TRASH_PERPORTION) && cnt--; idx++) {
		// 		disk[i].color_tag[idx] = tag;
		// 	}
		// }
		
		// 理论比例：972 648 567 291 129
		// for (int j = 0; j < DISK_SPLIT_1; j++) {
		// 	disk[i].space[1].insert(j);
		// 	count[1]++;
		// }
		// for (int j = DISK_SPLIT_1; j < DISK_SPLIT_2 - 1; j += 2) {
		// 	disk[i].space[2].insert(j);
		// 	count[2]++;
		// }
		// for (int j = DISK_SPLIT_2; j < DISK_SPLIT_3 - 2; j += 3) {
		// 	disk[i].space[3].insert(j);
		// 	count[3]++;
		// }
		// for (int j = DISK_SPLIT_3; j < DISK_SPLIT_4 - 3; j += 4) {
		// 	disk[i].space[4].insert(j);
		// 	count[4]++;
		// }
		// for (int j = DISK_SPLIT_4; j < DISK_SPLIT_5 - 4; j += 5) {
		// 	disk[i].space[5].insert(j);
		// 	count[5]++;
		// }
		// for (int i = 1; i <= 5; i++) {
		// 	cerr << count[i] << " \n"[i == 5];
		// }
	}
}


bool Random_Appear(int p) {
	return rng() % 100 + 1 <= p;
}

// 分数应该与 obj_size 正相关
// 分数应该与 wait_time 正相关 (因为目标是优先提高命中率？)
// 分数应该与 predict_delete_time 负相关
int Predict_Delete_Time(int obj_id) {
	int obj_size = objects[obj_id].size;
	int obj_tag = objects[obj_id].tag;
	int predict_delete_time = INF;
	//
	//
	//
	return predict_delete_time;
}

float Value_Function(int start_time, int finish_time, int obj_size, int obj_tag) {
	int x = finish_time - start_time;
	float g = (obj_size + 1) * 0.5;
	float f;
	if (x <= 10) {
		f = -0.005 * x + 1;
	} else if (x <= 105) {
		f = -0.01 * x + 1.05;
	} else {
		f = 0.;
	}
	float score = f * weight[obj_tag];
	return score;
}

float Punish_Function(int start_time, int end_time, int obj_size) {
	return 1. * (end_time - start_time) / MAX_QUERY_TIME * (obj_size + 1) * 0.5; 
}

float Get_Pos_Score(int disk_id, int pos, int time) {
	int obj_id = disk[disk_id].d[pos].first;	
	int obj_size = objects[obj_id].size;
	int obj_tag = objects[obj_id].tag;
	float score = 0;

	if (objects[obj_id].lock != -1 && objects[obj_id].lock != disk_id) {
		return 0.;
	}

	if (query[obj_id].empty()) {
		return 0.;
	}
	
	// 研究表明，任意取两个查询计算得分的平均值会使得结果变得更好
	int qry_id = *(query[obj_id].begin());  
	while (requests[qry_id].is_give_up) {
		query[obj_id].erase(query[obj_id].begin());
		if (query[obj_id].empty()) return 0.;
		qry_id = *query[obj_id].begin();
	}
	float a = Value_Function(requests[qry_id].query_time, timestamp, obj_size, obj_tag);
	float b = a;
	if (query[obj_id].size() == 1) {
		b = a;
	} else {
		assert(query[obj_id].size() > 1);
		qry_id = *(next(query[obj_id].begin()));
		while (requests[qry_id].is_give_up) {
			query[obj_id].erase((next(query[obj_id].begin())));
			if (query[obj_id].size() <= 1) {
				b = a;
				break;
			}
			qry_id = *(next(query[obj_id].begin()));
		}
		if (!requests[qry_id].is_give_up) {
			b = Value_Function(requests[qry_id].query_time, timestamp, obj_size, obj_tag);
		}
	}
	
	score = (a + b) / 2 * query[obj_id].size();
	
	// for (auto qry : query[obj_id]) {
	// 	score += Value_Function(requests[qry].query_time, timestamp, obj_size, obj_tag);
	// }
	return score;
}


// 注意这个函数绝对不能放到多线程里用！！！！！！！！！
pair<float, int> Simulate(int disk_id, int idx, int time, char &pre_mov, int &pre_cos) {
	int step = G;

	float score = 0;
	vector<pair<int, int>> change;
	
	while (step) {
		auto [obj_id, obj_part] = disk[disk_id].d[idx];
		int cost = INF;
		if (pre_mov == 'r') {
			cost = max(16, (int)ceil(pre_cos * 0.8));
		} else {
			cost = 64;
		}
		if (step < cost) {
			break;
		}
		bool is_hit = false;
		for (auto it = query[obj_id].begin(); it != query[obj_id].end(); ) {
			int qry = *it;
			auto prev = it;
			it++;
			if (requests[qry].is_give_up) {
				query[obj_id].erase(prev);
				continue;
			}
			if (requests[qry].is_done()) continue;
			bool has = requests[qry].has_part(obj_part);
			if (has) continue;
			is_hit = true;
			change.push_back({qry, obj_part});
			requests[qry].set(obj_part);
			if (requests[qry].is_done()) {
				int x = time - requests[qry].query_time;
				double g = (objects[obj_id].size + 1) * 0.5;
				double f;
				if (x <= 10) {
					f = -0.005 * x + 1;
				} else if (x <= 105) {
					f = -0.01 * x + 1.05;
				} else {
					f = 0.;
				}
				score += f * g;
			}
		}
		if (is_hit) {
			pre_mov = 'r';
			step -= cost;
		} else {
			pre_mov = 'p';
			step--;
		}
		pre_cos = cost;
		idx = (idx + 1) % V;
	}
	for (auto [qry, obj_part] : change) {
		requests[qry].erase(obj_part);
	}
	return {score, idx};
}






// ------------------------------------ 删除 ----------------------------------------

void Delete_Action() {
	int n_delete;
	cin >> n_delete;
	static int delete_id[MAX_OBJECT_NUM];
	for (int i = 0; i < n_delete; i++) {
		cin >> delete_id[i];
	}
	int n_abort = 0;
	vector<int> aborts;
	for (int i = 0; i < n_delete; i++) {
		int obj_id = delete_id[i];
		for (auto query_idx : query[obj_id]) {
			if (requests[query_idx].is_done() == false && !requests[query_idx].is_give_up) {
				n_abort++;
				aborts.emplace_back(query_idx);
				requests[query_idx].is_abort = true;
			}
		}
		query[obj_id].clear();
	}
	cout << n_abort << "\n";
	for (auto fail : aborts) {
		cout << fail << "\n";
	}
	cout.flush();


	for (int i = 0; i < n_delete; i++) {
		int obj_id = delete_id[i];
		for (int j = 0; j < REP_NUM; j++) {
			int obj_size = objects[obj_id].size;
			int obj_tag = objects[obj_id].tag;
			int disk_id = objects[obj_id].bel[j];
			int block = objects[obj_id].unit[j];
			// priority_pos[obj_tag][obj_size].push_back({disk_id, block});
			// disk[disk_id].space[obj_size].insert(block);
			for (int size = 0; size < obj_size; size++) {
				disk[disk_id].erase(block + size);
			}
		}
		objects[obj_id].is_delete = true;
	}
}




// ------------------------------------ 写入 ----------------------------------------

vector<vector<int>> random_write_disk;
vector<vector<int>> second_choice;

void hash_init() {
	int idx = 0;
	random_write_disk = {
		{},
		{0, 1, 2, },
		{3, 6, 5, },
		{7, 4, 9, },
		{8, 3, 6, },
		{8, 5, 2, },
		{0, 1, 8, },
		{5, 4, 7, },
		{9, 3, 6, },
		{2, 0, 1, },
		{8, 5, 7, },
		{4, 9, 6, },
		{3, 2, 1, },
		{0, 3, 8, },
		{5, 7, 4, },
		{2, 1, 0, },
		{3, 9, 8, },
	};
	// 每个 second_choice 有 3 个 盘块
	second_choice.resize(MAX_TAG + 1);
	for (int i = 1; i <= MAX_TAG; i++) {
		set<int> oth;
		while (oth.size() < REP_NUM) {
			int x = rng() % 10;
			if (oth.count(x) || find(random_write_disk[i].begin(), random_write_disk[i].end(), x) != random_write_disk[i].end()) {
				continue;
			}
			oth.insert(x);
		}
		for (auto hash_id : oth) {
			second_choice[i].push_back(hash_id);
		}
	}
	for (int i = 1; i <= MAX_TAG; i++) {
		for (auto disk_id : random_write_disk[i]) {
			disk[disk_id].has_tag.emplace_back(i);
		}
	}
	// for (int i = 1; i <= MAX_TAG; i++) {
	// 	for (int j = 0; j < REP_NUM; j++) {
	// 		random_write_disk[i].emplace_back(idx);
	// 		idx = (idx + 1) % N;
	// 	}
	// 	// set<int> set;
	// 	// while (set.size() < REP_NUM) {
	// 	// 	set.insert(rng() % N);
	// 	// }
	// 	// for (auto disk_id : set) {
	// 	// 	random_write_disk[i].emplace_back(disk_id);
	// 	// }
	// }

	// for (int i = 1; i <= 16; i++) {
	// 	for (auto t : random_write_disk[i]) {
	// 		cerr << t << ' ';
	// 	}
	// 	cerr << endl;
	// }
	// for (int i = 0; i < N; i++) {
	// 	for (auto t : random_write_disk[i]) {
	// 		cout << t << ' ';
	// 	}
	// 	cout << endl;
	// }
}

vector<pair<int, int>> Decide_Write_disk(int obj_id, int obj_size, int obj_tag) {
	static int idx = 0;
	// select 用位表示选过的 disk
	int select = 0;
	vector<pair<int, int>> write_disk;

	// 随机分布 -----------------------------------------------------
	// for (; write_disk.size() < REP_NUM; idx = (idx + 1) % N) {
	// 	if (find(write_disk.begin(), write_disk.end(), idx) != write_disk.end()) continue;
	// 	if (!disk[idx].capacity(obj_size)) continue;
	// 	write_disk.emplace_back(idx);
	// }
	// assert(write_disk.size() == REP_NUM);
	// return write_disk;
	// ---------------------------------------------------------------

	// size 法，把 size 相同的放到一起，但效果并不好
	// for (auto disk_id : disk_select[obj_size]) {
	// 	if (disk[disk_id].capacity(obj_size)) continue;
	// 	write_disk.emplace_back(disk_id);
	// }

	// hash 法，把 hash 值相同的放到一起
	for (auto hash_id : random_write_disk[obj_tag]) {
		if (disk[hash_id].capacity(obj_tag, obj_size, -1)) {
			write_disk.push_back({hash_id, -1});
			select |= 1 << hash_id;
		}
	}

	for (auto hash_id : second_choice[obj_tag]) {
		if (write_disk.size() < REP_NUM && disk[hash_id].capacity(obj_tag, obj_size, -2)) {
			write_disk.push_back({hash_id, -2});
			select |= 1 << hash_id;
		}
	}

	// 查缺补漏，如果不够 REP_NUM 个再顺序选几个
	for (int cnt = 0; write_disk.size() < REP_NUM; cnt++, idx = (idx + 1) % N) {
		assert(cnt <= N);
		if (select >> idx & 1) continue;
		if (!disk[idx].capacity(obj_tag, obj_size, -2)) continue;
		write_disk.push_back({idx, -2});
	}
	assert(write_disk.size() == REP_NUM);
	return write_disk;
}








vector<pair<int, int>> Get_Priority_Disk(int obj_tag, int obj_size) {
	vector<pair<int, int>> write_disk;
	assert(priority_pos[obj_tag][obj_size].size() >= 3 && priority_pos[obj_tag][obj_size].size() % 3 == 0);
	for (int i = 0; i < 3; i++) {
		auto [disk_id, pos] = priority_pos[obj_tag][obj_size].back();
		priority_pos[obj_tag][obj_size].pop_back();
		write_disk.push_back({disk_id, pos});
	}
	return write_disk;
}

void Write_Action() {
	int n_write;
	cin >> n_write;
	static int write_id[MAX_WRITE_LEN], write_obj_size[MAX_WRITE_LEN], write_obj_tag[MAX_WRITE_LEN];
	vector<vector<pair<int, int>>> tag_vec(MAX_TAG + 1);
	for (int i = 0; i < n_write; i++) {
		cin >> write_id[i] >> write_obj_size[i] >> write_obj_tag[i];
		tag_vec[write_obj_tag[i]].emplace_back(write_id[i], write_obj_size[i]);
	}

	// 简单把所有相同 tag 的先放到一起存储，后续具体放在哪里由 Decide_Write_Disk 来考虑
	static int tag = 1;
	for (int obj_tag = 1; obj_tag <= MAX_TAG; obj_tag++) {
		auto &vec = tag_vec[obj_tag];
		for (auto [obj_id, obj_size] : vec) {
			vector<pair<int, int>> write_disk;
			write_disk = Decide_Write_disk(obj_id, obj_size, obj_tag);
			// if (!priority_pos[obj_tag][obj_size].empty()) {
			// 	write_disk = Get_Priority_Disk(obj_tag, obj_size);
			// } else {
			// 	for (int cnt = MAX_TAG; cnt--; tag = (tag % MAX_TAG) + 1) {
			// 		if (!priority_pos[tag][obj_size].empty()) {
			// 			write_disk = Get_Priority_Disk(tag, obj_size);
			// 			break;
			// 		}
			// 	}
			// 	if (write_disk.empty()) {
			// 		write_disk = Decide_Write_disk(obj_id, obj_size, obj_tag);
			// 	}
			// }
			
			// assert(write_disk.size() == REP_NUM);
			// for (auto t : write_disk) {
			// 	assert(disk[t].last() >= obj_size);
			// }

			for (auto [disk_id, is_limit] : write_disk) {
				objects[obj_id].bel.push_back(disk_id);
			}
			objects[obj_id].size = obj_size;
			objects[obj_id].tag = obj_tag;
			objects[obj_id].is_delete = false;
			objects[obj_id].lock = -1;
			objects[obj_id].lock_time = timestamp;
			
			for (int j = 0; j < REP_NUM; j++) {
				auto [disk_idx, is_limit] = write_disk[j];
				int write_idx = disk[disk_idx].Write(obj_id, obj_size, obj_tag, is_limit);
				// cout << "obj: " << obj_id << ' ' << obj_size << " WriteDisk: " << disk_idx << " WriteIdx: ";
				// for (auto t : write_idx) cout << t << " "; cout << endl;
				objects[obj_id].unit[j] = write_idx;
			}

			
			cout << obj_id << "\n";
			for (int j = 0; j < REP_NUM; j++) {
				cout << objects[obj_id].bel[j] + 1;
				for (int k = 0; k < objects[obj_id].size; k++) {
					cout << " " << objects[obj_id].unit[j] + k + 1;
				}
				cout << "\n";
			}
		}
	}

	// for (int i = 0; i < n_write; i++) {
	// 	int obj_id = write_id[i];
	// 	int obj_size = write_obj_size[i];
	// 	int obj_tag = write_obj_tag[i];

	// 	auto write_disk = Decide_Write_disk(obj_id, obj_size, obj_tag);
	// 	assert(write_disk.size() == REP_NUM);

	// 	objects[obj_id].bel = write_disk;
	// 	objects[obj_id].size = obj_size;
	// 	objects[obj_id].tag = obj_tag;
	// 	objects[obj_id].is_delete = false;
	// 	for (int j = 0; j < REP_NUM; j++) {
	// 		objects[obj_id].unit[j].clear();
	// 	}

	// 	for (int j = 0; j < REP_NUM; j++) {
	// 		int disk_idx = write_disk[j];
	// 		auto write_idx = disk[disk_idx].Write(obj_id, obj_size, obj_tag);
	// 		// cout << "obj: " << obj_id << ' ' << obj_size << " WriteDisk: " << disk_idx << " WriteIdx: ";
	// 		// for (auto t : write_idx) cout << t << " "; cout << endl;
	// 		objects[obj_id].unit[j] = write_idx;
	// 	}

	// 	cout << obj_id << "\n";
	// 	for (int j = 0; j < REP_NUM; j++) {
	// 		cout << objects[obj_id].bel[j] + 1;
	// 		assert(objects[obj_id].unit[j].size() == obj_size);
	// 		for (int k = 0; k < objects[obj_id].size; k++) {
	// 			cout << " " << objects[obj_id].unit[j][k] + 1;
	// 		}
	// 		cout << "\n";
	// 	}
	// }
	cout.flush();
}



// ------------------------------------ 读取 ----------------------------------------

void Read_Action() {
	int n_read;
	cin >> n_read;
	vector<int> request_id(n_read), read_id(n_read);
	for (int i = 0; i < n_read; i++) {
		cin >> request_id[i] >> read_id[i];
	}

	for (int i = 0; i < n_read; i++) {
		int qry_id = request_id[i];
		int obj_id = read_id[i];
		requests[qry_id] = {
			timestamp,
			obj_id,
			0,
			(1 << objects[obj_id].size) - 1,
			false,
			false
		};
		query[obj_id].insert(qry_id);
		query_per_time[objects[obj_id].size].push(qry_id);
	}
}



// ------------------------------------ 磁头移动 ----------------------------------------

int Decide_Jump_Pos(int disk_id) {
	return disk[disk_id].max_score_pos;
}

void Process(int i) {
	disk[i].Cal_Score();
}

void Lock(int disk_id, int head_id, bool all_color, int lock_num, int lock_last) {
	if (all_color) {
		for (int cnt = lock_num, idx = disk[disk_id].head[head_id]; cnt--; idx = (idx + 1) % V) {
			int obj_id = disk[disk_id].d[idx].first;
			if (obj_id == -1) continue;
			objects[obj_id].lock = disk_id; // here
			objects[obj_id].lock_time = timestamp;
		}
		return;
	}
	for (int cnt = lock_num, idx = disk[disk_id].head[head_id]; cnt--; idx = (idx + 1) % V) {
		int obj_id = disk[disk_id].d[idx].first;
		if (obj_id == -1) continue;
		objects[obj_id].lock = disk_id; // here
		objects[obj_id].lock_time = timestamp;
		objects[obj_id].lock_last = lock_last;
	}
}

bool decide_continue_read(int disk_id, int head_id) {

	int idx = disk[disk_id].head[head_id];
	char p_move = pre_move[disk_id][head_id];
	int p_cost = pre_cost[disk_id][head_id];
	
	int continue_white = 0;
	assert(Get_Pos_Score(disk_id, idx, timestamp) <= DROP_SCORE);

	for (int i = idx, cnt = 15; cnt--; i = (i + 1) % V) {
		if (Get_Pos_Score(disk_id, i, timestamp) <= DROP_SCORE) {
			continue_white++;
		} else {
			break;
		}
	}
	if (continue_white > 11) {
		return false;
	}

	// int score_cnt = 0;
	// assert(Get_Pos_Score(disk_id, (idx + continue_white) % V, timestamp) > DROP_SCORE);
	// for (int i = (idx + continue_white) % V, cnt = 7; cnt--; i = (i + 1) % V) {
	// 	if (Get_Pos_Score(disk_id, i, timestamp) > DROP_SCORE) {
	// 		score_cnt++;
	// 	}
	// }
	// if (score_cnt < 2) {
	// 	return false;
	// }
	// 后面怎么判断连续 7 个是否值得连读，判断只要后面不是只有一个就连读

	int base = continue_white + 262;
	int X = 0;

	for (int cnt = continue_white + 7; cnt--; idx = (idx + 1) % V) {
		auto [obj_id, obj_part] = disk[disk_id].d[idx];
		int cost = INF;
		if (p_move == 'r') {
			cost = max(16, (int)ceil(p_cost * 0.8));
		} else {
			cost = 64;
		}
		X += cost;
		p_cost = cost;
		p_move = 'r';
	}
	if (X < base) {
		Lock(disk_id, head_id, false, continue_white + 7, 2);
	}
	// cerr << X << ' ' << base << endl;
	return X < base;
	
	
	
	

	
	// int yes = 0;
	// int no = 0;
	// p_move = pre_move[disk_id];
	// p_cost = pre_cost[disk_id];
	// for (int cnt = DECIDE_CONTINUE_READ; cnt--; idx = (idx + 1) % V) {
	// 	auto [obj_id, obj_part] = disk[disk_id].d[idx];
	// 	int cost = INF;
	// 	if (p_move == 'r') {
	// 		cost = max(16, (int)ceil(p_cost * 0.8));
	// 	} else {
	// 		cost = 64;
	// 	}
	// 	if (Get_Pos_Score(disk_id, idx, timestamp) > DROP_SCORE) {
	// 		no += cost;
	// 		p_cost = cost;
	// 		p_move = 'r';
	// 	} else {
	// 		no += 1;
	// 		p_cost = 0;
	// 		p_move = 'p';
	// 	}
	// }
	// idx = disk[disk_id].head;
	// p_move = pre_move[disk_id];
	// p_cost = pre_cost[disk_id];
	// for (int cnt = DECIDE_CONTINUE_READ; cnt--; idx = (idx + 1) % V) {
	// 	auto [obj_id, obj_part] = disk[disk_id].d[idx];
	// 	int cost = INF;
	// 	if (p_move == 'r') {
	// 		cost = max(16, (int)ceil(p_cost * 0.8));
	// 	} else {
	// 		cost = 64;
	// 	}
	// 	yes += cost;
	// 	p_cost = cost;
	// 	p_move = 'r';
	// }

	// if (Get_Pos_Score(disk_id, disk[disk_id].head, timestamp) > DROP_SCORE) {
	// 	return yes < no;
	// }
	// return yes < no;
	
	// idx = disk[disk_id].head;
	// p_move = 'p';
	// p_cost = 0;

	// int oth = 0;
	// int cnt = DECIDE_CONTINUE_READ;
	// while (cnt-- && Get_Pos_Score(disk_id, idx, timestamp) <= DROP_SCORE) {
	// 	idx = (idx + 1) % V;
	// }
	// for (; cnt > 0; cnt--, idx = (idx + 1) % V) {
	// 	auto [obj_id, obj_part] = disk[disk_id].d[idx];
	// 	int cost = INF;
	// 	if (p_move == 'r') {
	// 		cost = max(16, (int)ceil(p_cost * 0.8));
	// 	} else {
	// 		cost = 64;
	// 	}
	// 	oth += cost;
	// 	p_cost = cost;
	// 	p_move = 'r';
	// }
	// return yes <= no && yes <= oth;
}

bool decide_give_up(int qry_id) {
	// float pos_score = 0;
	// float error_score = 0;
	// int query_time = requests[qry_id].query_time;
	// int obj_id = requests[qry_id].obj_id;
	// int	obj_size = objects[obj_id].size;
	// int obj_tag = objects[obj_id].tag;
	// pos_score = Value_Function(query_time, timestamp, obj_size, obj_tag);
	// float g = (obj_size + 1) * 0.5;
	// float h = (timestamp - query_time) / 105.0;
	// error_score = g * h;
	// return error_score >= pos_score;
	return timestamp - requests[qry_id].query_time == MAX_QUERY_TIME;
}

int jump_cnt = 0;
int drop = 0;
int continue_cnt = 0;
float punish_score = 0;

void show(string name, int &cnt) {
	if (timestamp % 1800 == 0) {
		cerr << name << " = " << 1. * cnt / N / 1800 * 100 << '%' << endl;
		cnt = 0;
	}
}

void show(string name, float &cnt) {
	if (timestamp % 1800 == 0) {
		cerr << name << " = " << 1. * cnt / N / 1800 << endl;
		cnt = 0;
	}
}

void Move() {
	show("punish_score", punish_score);
	show("jump_cnt", jump_cnt);
	vector<int> finish_qid;
	if (timestamp % UPDATE_DISK_SCORE_FREQUENCY == 0 && timestamp >= 10) {
		// for (int i = 0; i < N; i++) {
		// 	Process(i);
		// }
		// vector<thread> threads;
		// for (int i = 0; i < N; i++) {
		// 	threads.emplace_back(Process, i);
		// }
		// for (auto &thread : threads) {
		// 	thread.join();
		// }
	}
	vector<int> drop_num(N);
	vector<int> pos(N);
	iota(pos.begin(), pos.end(), 0);
	shuffle(pos.begin(), pos.end(), rng);
	vector<vector<string>> moves(N, vector<string>(MAX_HEAD_NUM));

	auto read = [&](int disk_id, int head_id) -> bool {
		auto [obj_id, obj_part] = disk[disk_id].d[disk[disk_id].head[head_id]];
		if (objects[obj_id].lock != -1 && objects[obj_id].lock != disk_id) {
			return false;
		}
		bool is_hit = false;
		for (auto it = query[obj_id].begin(); it != query[obj_id].end(); ) {
			int qry = *it;
			auto prev = it;
			it++;
			if (requests[qry].is_give_up) {
				query[obj_id].erase(prev);
				continue;
			}
			assert(!requests[qry].is_done());
			bool has = requests[qry].has_part(obj_part);
			if (has) continue;
			is_hit = true;
			requests[qry].set(obj_part);
			if (requests[qry].is_done()) {
				query[obj_id].erase(prev);
				finish_qid.emplace_back(qry);
			}
		}
		objects[obj_id].lock = -1;
		objects[obj_id].lock_time = 0;
		return is_hit;
	};
	
	// for (int i = 0; i < N; i++) {
	for (auto i : pos) {
		if (timestamp % UPDATE_DISK_SCORE_FREQUENCY == 0 && timestamp >= 10) {
			Process(i);
		}

		// 方案零：每次更新完考虑跳跃
		// if (timestamp % UPDATE_DISK_SCORE_FREQUENCY == 0 && disk[i].cur_score < disk[i].max_score && Random_Appear(JUMP_FREQUENCY)) {
		// 	jump_cnt++;
		// 	int jump_to = disk[i].max_score_pos;
		// 	disk[i].head = jump_to;
		// 	cout << "j " << jump_to + 1 << "\n";
		// 	pre_move[i] = 'j';
		// 	pre_cost[i] = 0;
		// 	continue;
		// }

		for (int head_id = 0; head_id < MAX_HEAD_NUM; head_id++) {
			int step = G;
			string move;
			// if (disk[i].working != -1) {
			// 	if (disk[i].color_tag[disk[i].head[head_id]] != disk[i].working) {
			// 		int jump_to = -1;
			// 		for (auto idx : disk[i].color_start) {
			// 			if (disk[i].color_tag[idx] == disk[i].working) {
			// 				jump_to = idx;
			// 				break;
			// 			}
			// 		}
			// 		assert(jump_to != -1);
			// 		jump_cnt++;
			// 		disk[i].head[head_id] = jump_to;
			// 		moves[i][head_id] = "j " + to_string(jump_to + 1);
			// 		pre_move[i][head_id] = 'j';
			// 		pre_cost[i][head_id] = 0;
			// 		Lock(i, head_id, true, LOCK_UNITS, LOCK_TIMES);
			// 		pre_jump[i][head_id] = timestamp;
			// 	} else {
			// 		while (step) {
			// 			auto [obj_id, obj_part] = disk[i].d[disk[i].head[head_id]];
			// 			int cost = INF;
			// 			if (pre_move[i][head_id] == 'r') {
			// 				cost = max(16, (int)ceil(pre_cost[i][head_id] * 0.8));
			// 			} else {
			// 				cost = 64;
			// 			}
			// 			if (step < cost) {
			// 				break;
			// 			}
			// 			read(i, head_id);
			// 			move += 'r';
			// 			step -= cost;
			// 			pre_cost[i][head_id] = cost;
			// 			pre_move[i][head_id] = move.back();
			// 			disk[i].head[head_id] = (disk[i].head[head_id] + 1) % V;
			// 			if (objects[obj_id].lock == i) {
			// 				objects[obj_id].lock = -1;
			// 				objects[obj_id].lock_time = 0;
			// 			}
			// 		}
			// 		Lock(i, head_id, true, LOCK_UNITS, LOCK_TIMES);
			// 		move += '#';
			// 		moves[i][head_id] = move;
			// 	}
			// 	continue;
			// }
	
			// 方案一：比较往前走两个块根跳转在走一个块的价值决定是否 jump
			// if (disk[i].cur_score < disk[i].max_score && Random_Appear(JUMP_FREQUENCY) && timestamp - pre_jump[i] > JUMP_BIAS) {
			if (disk[i].cur_score[head_id] * 2 < disk[i].max_score && timestamp - pre_jump[i][head_id] > JUMP_BIAS) {
				int jump_to = disk[i].max_score_pos;
				int help = BLOCK_SIZE;
				while (help-- && Get_Pos_Score(i, jump_to, timestamp + 1) <= DROP_SCORE) {
					// cerr << i << ' ' << jump_to << endl;
					jump_to = (jump_to + 1) % V;
				}
				if (help == 0) {
					goto not_jump;
				}
				jump_cnt++;
				disk[i].head[head_id] = jump_to;
				moves[i][head_id] = "j " + to_string(jump_to + 1);
				pre_move[i][head_id] = 'j';
				pre_cost[i][head_id] = 0;
				Lock(i, head_id, false, LOCK_UNITS, LOCK_TIMES);
				pre_jump[i][head_id] = timestamp;
				pre_decide[i][head_id] = false;
				continue;
			}
			
			not_jump:
	
			while (step) {
				auto [obj_id, obj_part] = disk[i].d[disk[i].head[head_id]];
				if (timestamp - objects[obj_id].lock_time > objects[obj_id].lock_last) {
					objects[obj_id].lock = -1;
					objects[obj_id].lock_time = 0;
				}
				int cost = INF;
				if (pre_move[i][head_id] == 'r') {
					cost = max(16, (int)ceil(pre_cost[i][head_id] * 0.8));
				} else {
					cost = 64;
				}
				if (step < cost) {
					break;
				}
				if (objects[obj_id].lock == i) {
					objects[obj_id].lock = -1;
					objects[obj_id].lock_time = 0;
				}
	
				float score = Get_Pos_Score(i, disk[i].head[head_id], timestamp);
				
				// 必须把 read 和 write 割裂开，不然的话 hit 了就得 r
				
				if (score > DROP_SCORE) {
					read(i, head_id);
					move += 'r';
					step -= cost;
					pre_cost[i][head_id] = cost;
					pre_move[i][head_id] = move.back();
					disk[i].head[head_id] = (disk[i].head[head_id] + 1) % V;
					pre_decide[i][head_id] = false;
					continue;
				}
	
				// if (pre_decide[i]) {
				// 	continue_cnt++;
				// 	read(i);
				// 	move += 'r';
				// 	step -= cost;
				// 	pre_cost[i] = cost;
				// 	pre_move[i] = move.back();
				// 	disk[i].head = (disk[i].head + 1) % V;
				// 	continue;
				// }
				if (decide_continue_read(i, head_id)) {
					pre_decide[i][head_id] = true;
					continue_cnt += cost;
					read(i, head_id);
					// if (Get_Pos_Score(i, disk[i].head, timestamp) <= DROP_SCORE) {
					// 	Lock(i, false); // 相当于是决定跳转到原地了，不能继续跳跃，但事实证明效果不好
					// 	pre_jump[i] = timestamp;
					// }
					move += 'r';
					step -= cost;
					pre_cost[i][head_id] = cost;
					pre_move[i][head_id] = move.back();
					disk[i].head[head_id] = (disk[i].head[head_id] + 1) % V;
					continue;
				}
				pre_decide[i][head_id] = false;
	
				// if (score <= DROP_SCORE) {
				// 	move += 'p';
				// 	pre_move[i] = 'p';
				// 	pre_cost[i] = 0;
				// 	step--;
				// 	disk[i].head = (disk[i].head + 1) % V;
					
				// 	if (query[obj_id].size()) {
				// 		drop_num[i]++;
				// 		drop++;
				// 	}
				// 	continue;
				// }
				
				move += 'p';
				step--;
				pre_cost[i][head_id] = cost;
				pre_move[i][head_id] = move.back();
				disk[i].head[head_id] = (disk[i].head[head_id] + 1) % V;
			}
			while (move.back() != 'r' && step && Get_Pos_Score(i, disk[i].head[head_id], timestamp) <= DROP_SCORE) {
				move += 'p';
				step--;
				pre_move[i][head_id] = 'p';
				disk[i].head[head_id] = (disk[i].head[head_id] + 1) % V;
			}
			move += '#';
			moves[i][head_id] = move;
		}
	}
	for (int i = 0; i < N; i++) {
		for (int j = 0; j < MAX_HEAD_NUM; j++) {
			cout << moves[i][j] << "\n";
		}
	}
	cout << finish_qid.size() << "\n";
	for (auto finish : finish_qid) {
		cout << finish << "\n";
	}
	// 处理放弃的请求
	vector<int> give_up_qid;
	for (int size = 1; size <= MAX_SIZE; size++) {
		while (!query_per_time[size].empty()) {
			auto qry_id = query_per_time[size].front();
			int query_time = requests[qry_id].query_time;
			if (requests[qry_id].is_done() || requests[qry_id].is_abort) {
				query_per_time[size].pop();
				continue;
			}
			if (decide_give_up(qry_id)) {
				punish_score += Punish_Function(query_time, timestamp, objects[requests[qry_id].obj_id].size);
				give_up_qid.push_back(qry_id);
				requests[qry_id].is_give_up = true;
				query_per_time[size].pop();
			} else {
				break;
			}
		}
	}
	cout << give_up_qid.size() << "\n";
	for (auto give_up : give_up_qid) {
		cout << give_up << "\n";
	}
	cout.flush();
}

void garbage_collection() {
	if (timestamp % FRE_PER_SLICING != 0) return;
	string GARBAGE, COLLECTION;
	cin >> GARBAGE >> COLLECTION;
	cout << "GARBAGE COLLECTION\n";
	for (int i = 0; i < N; i++) {
		cout << 0 << "\n";
	}
	cout.flush();
}

void Solve() {	

	if (timestamp % FRE_PER_SLICING == 1 && timestamp / FRE_PER_SLICING + 1 <= (T + FRE_PER_SLICING - 1) / FRE_PER_SLICING) {
		int epoch = timestamp / 1800 + 1;
		// for (int i = 0; i < N; i++) {
		// 	disk[i].working = -1;
		// }
		// for (auto hot_tag : hot_tags[epoch]) {
		// 	int working_disk = random_write_disk[hot_tag][rng() % 3];
		// 	while (disk[working_disk].working != -1) {
		// 		working_disk = random_write_disk[hot_tag][rng() % 3];
		// 	}
		// 	disk[working_disk].working = hot_tag;
		// }
		// float tot = 0;
		// for (int i = 1; i <= MAX_TAG; i++) {
		// 	tot += fre_read[i][timestamp / FRE_PER_SLICING + 1];
		// }
		// for (int i = 1; i <= MAX_TAG; i++) {
		// 	weight[i] = 1 + fre_read[i][timestamp / FRE_PER_SLICING + 1] / tot;
		// }
		// auto pre_fre = query_times;
		// for (int i = 1; i <= MAX_TAG; i++) {
		// 	query_times[i] *= weight[i];
		// }
		// for (int i = 0; i < N; i++) {
		// 	disk[i].Color();
		// }
		// query_times = pre_fre;
		// weight.assign(MAX_TAG + 1, 1.);
		// for (int i = 1; i <= MAX_TAG; i++) {
		// 	cerr << weight[i] << ' ';
		// }
		// cerr << endl;
	}
	Delete_Action();
	Write_Action();
	Read_Action();
	Move();
	garbage_collection();
}



 








void TimeStamp() {
	string pattern;
	int timeStamp;
	cin >> pattern >> timeStamp;
	cout << pattern << " " << timeStamp << "\n";
	cout.flush();
}

int main() {
	ios::sync_with_stdio(false);
	cin.tie(nullptr);
	cout.tie(nullptr);

	cin >> T >> M >> N >> V >> G >> K;
	hash_init();
	total_init();
	
	int batch_num = (T + FRE_PER_SLICING - 1) / FRE_PER_SLICING;
	for (int i = 1; i <= M; i++) {
		for (int j = 1; j <= batch_num; j++) {
			cin >> fre_del[i][j];
		}
	}
	for (int i = 1; i <= M; i++) {
		for (int j = 1; j <= batch_num; j++) {
			cin >> fre_write[i][j];
		}
	}
	for (int i = 1; i <= M; i++) {
		for (int j = 1; j <= batch_num; j++) {
			cin >> fre_read[i][j];
		}
	}
	Pre_Process();
	for (int i = 1; i <= T + 105; i++) {
		timestamp = i;
		TimeStamp();
		Solve();
	}
	return 0;
}