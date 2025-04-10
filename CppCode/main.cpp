// 不再严格划分 size，有连续空位置就存放
// sublime 里那段代码要设计一种算法更加合理地分配空间，不能让 16 号出现在所有盘里
// 存储的时候先存 size 大的，再存 size 小的，石头 -> 沙子 -> 水

// cur_score 必须每一轮都计算！不然这个分数就是扯淡
// 上线段树维护 score 信息，每 UPDATE_DISK_SCORE_FREQUENCY 更新一次，直接 init

// 跳跃的时候，不能只跳那 BLOCK_NUM 个点，这绝对是不行的
// 锁机制，对于当前这个磁头即将读取的一部分 obj，我认为这就是它认定的东西，别人考虑分数的时候就不应该再计算这些 obj 的得分了！
// 存放 obj 的时候，优先放到当前磁头的前方？便于快速读取到

// 分块必须要改，或者就跳转到颜色初始，或者分一个块为 token / 16
// USE_SIZE 是非常重要的参数，要认真考虑
// 是否有必要每个盘都 16 种颜色？8 - 8 分会不会更好？
// 垃圾回收机制要认证改进



#include <bits/stdc++.h>
using namespace std;

#define DEBUG

#ifdef DEBUG
#define UPDATE_DISK_SCORE_FREQUENCY (10000000)
#define MAX_DISK_SIZE (13136)
#define BLOCK_NUM (60)
const int BLOCK_SIZE = MAX_DISK_SIZE / BLOCK_NUM;
#define LOCK_UNITS (BLOCK_SIZE) // 很重要的一个参数，但不知道为啥!!!!!
// #define LOCK_TIMES (LOCK_UNITS / (272 / 16))
#define LOCK_TIMES (50)
#else
#define UPDATE_DISK_SCORE_FREQUENCY (10000000)
#define MAX_DISK_SIZE (16384)
#define BLOCK_NUM (110) // 这个参数调大一点可能会变好，比如 20
const int BLOCK_SIZE = MAX_DISK_SIZE / BLOCK_NUM;
#define LOCK_UNITS (BLOCK_SIZE)
// #define LOCK_TIMES (LOCK_UNITS / (340 / 16))
#define LOCK_TIMES (70)
#endif


#define MAX_TAG (16)
#define MAX_SIZE (5)
#define MAX_DISK_NUM (10)

// #define JUMP_FREQUENCY (8)
#define JUMP_BIAS (LOCK_TIMES)

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

#define DROP_SCORE (0)
#define DECIDE_CONTINUE_READ (10)
#define TRASH_PERPORTION (0)

#define EXTRA_BLOCK (-250)
#define USE_SIZE (MAX_DISK_SIZE / 3 + EXTRA_BLOCK) // [0, USE_SIZE - 1]
#define PREDICT (13) // 当前位置往前看多少个颜色块的得分，这个参数要好好斟酌！！！！！
#define DANGEROUSE_TIME (200) // 废弃了

#define PUNISH_WEIGHT (100)
#define GIVE_UP_LIMIT (105)
#define BLOCK_BIAS (5)

#define SEED (11111111)

vector<int> haha_qid;
vector<float> query_times = {0, 1976, 1509, 3906, 1470, 1195, 2861, 1492, 3498, 4771, 4654, 4609, 1665, 1853, 3637, 1424, 4521, };
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

vector<int> random_color_sequence = {
	8, 5, 2, 4, 9, 7, 3, 1, 0, 6,
};
vector<int> color_use(MAX_DISK_NUM, 3);

vector<vector<int>> color_sequence = {
	{0, },
	{1, },
	{2, 10, 11, 13, },
	{3, 8, },
	{4, },
	{5, 14, 15, },
	{6, },
	{7, 9, 12, },
};

#ifdef DEBUG
vector<vector<pair<int, int>>> color_init = {
	{
		{5, 103},
		{4, 127},
		{2, 129},
		{6, 247},
		{13, 160},
		{8, 302},
		{14, 314},
		{15, 122},
		{12, 143},
		{9, 412},
	},
	{
		{3, 380},
		{1, 192},
		{7, 145},
		{11, 449},
		{16, 441},
		{10, 454},
	},
	{
		{8, 310},
		{5, 105},
		{16, 402},
		{1, 175},
		{15, 126},
		{2, 133},
		{4, 130},
		{6, 254},
		{9, 424},
	},
	{
		{10, 440},
		{7, 141},
		{11, 435},
		{14, 344},
		{13, 175},
		{12, 157},
		{3, 369},
	},
	{
		{8, 293},
		{9, 400},
		{10, 390},
		{5, 99},
		{1, 165},
		{3, 327},
		{11, 386},
	},
	{
		{2, 151},
		{15, 143},
		{7, 150},
		{12, 167},
		{16, 457},
		{13, 187},
		{4, 148},
		{14, 367},
		{6, 289},
	},
	{
		{4, 132},
		{5, 106},
		{8, 313},
		{14, 326},
		{13, 166},
		{11, 413},
		{9, 428},
		{1, 177},
	},
	{
		{7, 139},
		{10, 436},
		{6, 268},
		{3, 365},
		{12, 155},
		{15, 133},
		{2, 140},
		{16, 424},
	},
	{
		{16, 442},
		{2, 146},
		{3, 381},
		{7, 145},
		{15, 138},
		{9, 466},
		{8, 341},
	},
	{
		{1, 170},
		{14, 313},
		{13, 159},
		{4, 127},
		{11, 397},
		{10, 401},
		{5, 102},
		{12, 143},
		{6, 247},
	},
	{
		{8, 298},
		{3, 333},
		{6, 244},
		{13, 158},
		{1, 168},
		{12, 141},
		{9, 408},
		{14, 310},
	},
	{
		{10, 460},
		{4, 145},
		{2, 148},
		{15, 140},
		{11, 455},
		{7, 147},
		{5, 117},
		{16, 447},
	},
	{
		{5, 104},
		{6, 252},
		{8, 307},
		{1, 173},
		{11, 405},
		{16, 398},
		{9, 420},
	},
	{
		{4, 140},
		{13, 177},
		{7, 142},
		{12, 158},
		{15, 135},
		{2, 143},
		{3, 373},
		{10, 444},
		{14, 347},
	},
	{
		{6, 265},
		{9, 442},
		{3, 361},
		{11, 426},
		{10, 431},
		{4, 136},
	},
	{
		{7, 135},
		{16, 410},
		{2, 136},
		{5, 108},
		{8, 317},
		{15, 129},
		{13, 168},
		{1, 178},
		{14, 329},
		{12, 150},
	},
	{
		{9, 447},
		{12, 155},
		{10, 436},
		{2, 140},
		{6, 268},
		{5, 111},
		{4, 137},
		{3, 365},
	},
	{
		{1, 177},
		{13, 166},
		{15, 127},
		{7, 133},
		{14, 326},
		{8, 313},
		{11, 413},
		{16, 406},
	},
	{
		{5, 110},
		{11, 427},
		{4, 136},
		{16, 419},
		{1, 182},
		{12, 154},
		{7, 138},
		{3, 362},
		{15, 131},
	},
	{
		{8, 316},
		{9, 432},
		{14, 329},
		{10, 421},
		{2, 136},
		{13, 167},
		{6, 259},
	},
};
#else
vector<vector<pair<int, int>>> color_init = {
	{
		{9, 5461},
	},
	{
		{10, 5461},
	},
	{
		{11, 5461},
	},
	{
		{16, 5461},
	},
	{
		{3, 5461},
	},
	{
		{14, 5461},
	},
	{
		{8, 3881},
		{15, 1579},
	},
	{
		{6, 3589},
		{7, 1871},
	},
	{
		{1, 2177},
		{2, 1663},
		{4, 1620},
	},
	{
		{13, 2147},
		{12, 1929},
		{5, 1384},
	},
};
#endif

vector<vector<int>> belong(MAX_TAG + 1);

vector<vector<int>> hot_tags = {
	
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
	bool flag[MAX_DISK_SIZE];
	set<int> space[MAX_SIZE + 1];
	vector<int> has_tag;
	int color_tag[MAX_DISK_SIZE];
	vector<int> color_type;
	vector<int> head;
	vector<pair<int, int>> range;
	vector<pair<int, int>> block;
	int start_write = 0;
	vector<float> score;
	int siz = 0;
	int disk_id = 0;
	vector<int> max_score_pos; // 每个 range 里的价值最大块位置
	vector<float> max_score; // 每个 range 里的价值最大块价值
	vector<float> cur_score; // 当前位置向前走 PREDICT 个块的价值
	int use_size = 0;
	vector<int> working;
	vector<int> color_start;
	Disk() { }
	void init(int disk_id) {
		this->disk_id = disk_id;
		working.resize(MAX_HEAD_NUM, -1);
		head.resize(MAX_HEAD_NUM, 0);
		cur_score.resize(MAX_HEAD_NUM, 0);
		range.resize(MAX_HEAD_NUM, {0, 0});
		max_score_pos.resize(MAX_HEAD_NUM, 0);
		max_score.resize(MAX_HEAD_NUM, 0);
		for (int i = 0; i < MAX_DISK_SIZE; i++) {
			d[i] = {-1, -1};
			color_tag[i] = -1;
			flag[i] = false;
		}

		static int idx = 0;
		for (; color_type.size() < 3; idx = (idx + 1) % N) {
			color_use[random_color_sequence[idx]]--;
			color_type.emplace_back(random_color_sequence[idx]);
		}
		assert(color_type.size() == 3);
		
		int tot = 0;
		for (auto [tag, cnt] : color_init[disk_id]) {
			tot += cnt;
		}
		int split = USE_SIZE; 
		range[0] = {0, split / 2 - 1};
		range[1] = {split / 2, split - 1};
		// 这里也用垃圾桶？把错误的标签优先放到最后 5%？
	}

	void Color() {
		
		int idx = 0;
		
		// int tot = 0;
		// for (int i = 2 * disk_id; i <= 2 * disk_id + 1; i++) {
		// 	range[i - 2 * disk_id].first = tot;
		// 	for (auto [tag, cnt] : color_init[i]) {
		// 		tot += cnt;
		// 		while (cnt--) {
		// 			color_tag[idx++] = tag;
		// 		}
		// 	}	
		// 	range[i - 2 * disk_id].second = tot - 1;
		// }
		// use_size = range[MAX_HEAD_NUM - 1].second + 1;
		// return;


		use_size = USE_SIZE;
		int tot = accumulate(query_times.begin(), query_times.end(), 0);
		for (int i = 0; i < (MAX_HEAD_NUM); i++) {
			int start = range[i].first;
			vector<int> pos(MAX_TAG + 1);
			iota(pos.begin(), pos.end(), 0);
			shuffle(pos.begin() + 1, pos.end(), rng);
			
			
			
			for (int j = 1; j <= MAX_TAG; j++) {
				int i = pos[j];
				int cnt = (USE_SIZE / (MAX_HEAD_NUM)) * 1. * query_times[i] / tot;
				int c = cnt;
				while (cnt-- && idx < USE_SIZE) {
					color_tag[idx++] = i;
				}
				block.push_back({start, c});
				start += c;
			}
			score.resize(block.size());
		}
		return;



		// for (int i = 0; i < color_type.size(); i++) {
		// 	int type = color_type[i];
		// 	int tot = 0;
		// 	for (auto [tag, cnt] : color_init[type]) {
		// 		tot += cnt;
		// 	}
		// 	for (auto [tag, cnt] : color_init[type]) {
		// 		if (i == 0) {
		// 			cnt += EXTRA_BLOCK * 1. * cnt / tot;
		// 		}
		// 		// cnt += EXTRA_BLOCK;
		// 		while (cnt-- && idx < V) {
		// 			color_tag[idx++] = tag;
		// 		}
		// 	}
		// }
		
		// int tot = 0;
		// for (auto tag : has_tag) {
		// 	tot += query_times[tag];
		// }
		// int idx = 0;
		// for (auto tag : has_tag) {
		// 	color_start.push_back(idx);
		// 	int len = 1. * query_times[tag] / tot * V * (1 - TRASH_PERPORTION);
		// 	for (int cnt = len; idx < V * (1 - TRASH_PERPORTION) && cnt--; idx++) {
		// 		color_tag[idx] = tag;
		// 	}
		// }
		// int shift = rng() % V;
		// auto y = color_tag;
		// for (int i = 0; i < V; i++) {
		// 	color_tag[i] = y[(i + shift) % V];
		// }
	}
	
	int size() {
		return siz;
	}
	bool capacity(int obj_tag, int obj_size, int is_limit) {

		bool cap = 0;
		if (is_limit == -1) {
			if (start_write == 0) {
				for (int i = 0; i + obj_size - 1 < use_size / MAX_HEAD_NUM; i++) {
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
				for (int i = use_size / MAX_HEAD_NUM; i + obj_size - 1 < use_size; i++) {
					if (color_tag[i] != obj_tag || d[i].first != -1) continue;
					bool ok = 1;
					for (int j = i + 1; j < i + obj_size; j++) {
						if (color_tag[j] != obj_tag || d[j].first != -1) ok = 0;
					}	
					if (!ok) continue;
					cap = 1;
					break;
				}
			}
		} else if (is_limit == -2) {
			int available = 0;
			for (int i = use_size; i <= V - obj_size && available < obj_size; i++) {
				if (d[i].first != -1) continue;
				available++;
			}
			cap = available == obj_size;
		} else if (is_limit == -3) {
			for (int i = 0; i + obj_size - 1 < use_size; i++) {
				if (d[i].first != -1) continue;
				bool ok = 1;
				for (int j = i + 1; j < i + obj_size; j++) {
					if (d[j].first != -1) ok = 0;
				}	
				if (!ok) continue;
				cap = 1;
				break;
			}
		} else if (is_limit == -4) {
			for (int i = 0; i + obj_size - 1 < use_size; i++) {
				if (color_tag[i] != obj_tag || d[i].first != -1) continue;
				bool ok = 1;
				for (int j = i + 1; j < i + obj_size; j++) {
					if (color_tag[j] != obj_tag || d[j].first != -1) ok = 0;
				}	
				if (!ok) continue;
				cap = 1;
				break;
			}
		}
		
		// 可以搞个 -3 表示需要无色且 -1

		return cap;
	}

	// 以及染色为什么不只考虑分配 query_times 个？？？
	void Cal_Current_Score() { // 待修改------------------------------------------------------------------------------
		for (int head_idx = 0; head_idx < MAX_HEAD_NUM; head_idx++) {
			set<int> cnt;
			cur_score[head_idx] = 0;
			int i = head[head_idx];
			// char pre_mov = pre_move[disk_id];
			// int pre_cos = pre_cost[disk_id];
			while (cnt.size() <= PREDICT) { // 当前位置往前走 n 种颜色的总价值 v.s. 某一种颜色的价值
				cur_score[head_idx] += Get_Pos_Score(disk_id, i, timestamp);
				cnt.insert(color_tag[i]);
				i = (i + 1) % use_size;
			}
			// for (int t = timestamp; t < timestamp + PREDICT; t++) {
			// 	// auto [score, idx] = Simulate(disk_id, i, t, pre_mov, pre_cos);
			// 	// i = idx;
			// 	// cur_score[head_idx] += score;
			// 	for (int cnt = BLOCK_SIZE; cnt--; i = (i + 1) % V) {
			// 		cur_score[head_idx] += Get_Pos_Score(disk_id, i, t);
			// 	}
			// }
		}
	}

	void Cal_Max_Score() {
		for (int head_id = 0; head_id < MAX_HEAD_NUM; head_id++) {
			max_score[head_id] = -1;
			max_score_pos[head_id] = -1;
			for (int i = 0; i < block.size(); i++) {
				auto [start, units] = block[i];
				if (start < range[head_id].first || start + units - 1 > range[head_id].second) {
					continue;
				}
				if (score[i] > max_score[head_id]) {
					max_score[head_id] = score[i];
					max_score_pos[head_id] = start;
				}
			}
			// int block = -1;
			// for (int i = 0; i < BLOCK_NUM; i++) {
			// 	if (i * BLOCK_SIZE < range[head_id].first || (i + 1) * BLOCK_SIZE - 1 > range[head_id].second) {
			// 		continue;
			// 	}
			// 	if (score[i] > max_score[head_id]) {
			// 		max_score[head_id] = score[i];
			// 		block = i;
			// 	}
			// }
			// assert(block != -1);
			// max_score_pos[head_id] = block * BLOCK_SIZE;
			// while (d[max_score_pos[head_id]].first == -1) {
			// 	max_score_pos[head_id] = (max_score_pos[head_id] + 1) % V;
			// }
		}
	}
	
	void Cal_Block_Score() {
		for (int i = 0; i < block.size(); i++) {
			score[i] = 0;
			auto [start, units] = block[i];
			for (int j = start; units--; j++) {
				score[i] += Get_Pos_Score(disk_id, j, timestamp);
			}
		}
	// 	for (int block = 0; block < BLOCK_NUM; block++) {
	// 		int i = block * BLOCK_SIZE;
	// 		float sc = 0;
	// 		char pre_mov = 'j';
	// 		int pre_cos = 0;
	// 		for (int t = timestamp; t < timestamp + PREDICT; t++) {
	// 			// auto [score, idx] = Simulate(disk_id, i, t, pre_mov, pre_cos);
	// 			// i = idx;
	// 			// sc += score;
	// 			for (int cnt = BLOCK_SIZE; cnt--; i = (i + 1) % V) {
	// 				sc += Get_Pos_Score(disk_id, i, t);
	// 			}
	// 		}
	// 		score[block] = sc;

	// 		// int l = block * BLOCK_SIZE;
	// 		// int r = l + BLOCK_SIZE - 1;
	// 		// float sc = 0;
	// 		// for (int i = l; i <= r; i++) {
	// 		// 	sc += Get_Pos_Score(disk_id, i, timestamp);
	// 		// }
	// 		// score[block] = sc;
	// 	}
	}
	void Cal_Score() {	    // 计算走 PREDICT - 1 个块的价值
		Cal_Block_Score();
		Cal_Max_Score();
		Cal_Current_Score();
	}
	vector<int> Write(int obj_id, int obj_size, int obj_tag, int is_limit) {

		vector<int> write_idx;
		siz += obj_size;
		if (is_limit == -1) {
			if (start_write == 0) {
				for (int i = 0; i + obj_size - 1 < use_size / MAX_HEAD_NUM; i++) {
					if (color_tag[i] != obj_tag || d[i].first != -1) continue;
					bool ok = 1;
					for (int j = i + 1; j < i + obj_size; j++) {
						if (color_tag[j] != obj_tag || d[j].first != -1) ok = 0;
					}	
					if (!ok) continue;
					write_idx.push_back(i);
					break;
				}
			} else {
				for (int i = use_size / MAX_HEAD_NUM; i + obj_size - 1 < use_size; i++) {
					if (color_tag[i] != obj_tag || d[i].first != -1) continue;
					bool ok = 1;
					for (int j = i + 1; j < i + obj_size; j++) {
						if (color_tag[j] != obj_tag || d[j].first != -1) ok = 0;
					}	
					if (!ok) continue;
					write_idx.push_back(i);
					break;
				}
			}
			start_write = (start_write + 1) % MAX_HEAD_NUM;
		} else if (is_limit == -2) {
			for (int i = V - obj_size; i >= use_size && write_idx.size() < obj_size; i--) {
				if (d[i].first != -1) continue;
				write_idx.push_back(i);
			}
			reverse(write_idx.begin(), write_idx.end());
		} else if (is_limit == -3) {
			for (int i = 0; i + obj_size - 1 < use_size; i++) {
				if (d[i].first != -1) continue;
				bool ok = 1;
				for (int j = i + 1; j < i + obj_size; j++) {
					if (d[j].first != -1) ok = 0;
				}	
				if (!ok) continue;
				write_idx.push_back(i);
				break;
			}
		} else if (is_limit == -4) {
			for (int i = 0; i + obj_size - 1 < use_size; i++) {
				if (color_tag[i] != obj_tag || d[i].first != -1) continue;
				bool ok = 1;
				for (int j = i + 1; j < i + obj_size; j++) {
					if (color_tag[j] != obj_tag || d[j].first != -1) ok = 0;
				}	
				if (!ok) continue;
				write_idx.push_back(i);
				break;
			}
		}
		
		assert(write_idx.size() == 1 || write_idx.size() == obj_size);

		flag[write_idx[0]] = true;
		
		if (write_idx.size() == 1) {
			for (int i = write_idx[0], size = 0; size < obj_size; i++, size++) {
				d[i] = {obj_id, size};
			}
		} else {
			int size = 0;
			for (auto pos : write_idx) {
				d[pos] = {obj_id, size++};
			}
		}
		
		return write_idx;

		{

		// if (is_limit >= 0) {
		// 	write_idx = is_limit;
		// 	for (int i = write_idx, size = 0; size < obj_size; i++, size++) {
		// 		d[i] = {obj_id, size};
		// 	}
		// 	write_idx = write_idx;
		// 	// assert(space[obj_size].count(write_idx));
		// 	// space[obj_size].erase(write_idx);
		// 	return write_idx;
		// }

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

		// if (write_idx == -1) {
		// 	if (is_limit == -1) {
		// 		for (int i = 0; i < V - obj_size; i++) {
		// 			if (color_tag[i] != obj_tag || d[i].first != -1) continue;
		// 			bool ok = 1;
		// 			for (int j = i + 1; j < i + obj_size; j++) {
		// 				if (color_tag[j] != obj_tag || d[j].first != -1) ok = 0;
		// 			}	
		// 			if (!ok) continue;
		// 			write_idx = i;
		// 			break;
		// 		}
		// 	} else {
		// 		assert(is_limit == -2);
		// 		for (int i = 0; i < V - obj_size; i++) {
		// 			if (color_tag[i] != -1) continue; 
		// 			if (d[i].first != -1) continue;
		// 			bool ok = 1;
		// 			for (int j = i + 1; j < i + obj_size; j++) {
		// 				if (d[j].first != -1) ok = 0;
		// 			}	
		// 			if (!ok) continue;
		// 			write_idx = i;
		// 			break;
		// 		}
		// 		if (write_idx == -1) {
		// 			for (int i = 0; i < V - obj_size; i++) {
		// 				if (d[i].first != -1) continue;
		// 				bool ok = 1;
		// 				for (int j = i + 1; j < i + obj_size; j++) {
		// 					if (d[j].first != -1) ok = 0;
		// 				}	
		// 				if (!ok) continue;
		// 				write_idx = i;
		// 				break;
		// 			}
		// 		}
		// 	}
		// }



		
		
		// assert(write_idx != -1);

		// for (int i = write_idx, size = 0; size < obj_size; i++, size++) {
		// 	d[i] = {obj_id, size};
		// }
		
		// return write_idx;
		}
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
queue<int> waste_qid;


struct Object {
	vector<int> bel;
	vector<int> unit[REP_NUM];
	int size = 0;
	int tag = -1;
	bool is_delete = false;
	pair<int, int> lock = {-1, -1};
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
	for (int i = 0; i < MAX_DISK_NUM; i++) {
		// 染色，对应位置只能存储对应 tag
		disk[i].Color();
	}
	for (int i = 0; i < MAX_DISK_NUM; i++) {
		set<int> c;
		for (int j = 0; j < V; j++) {
			if (disk[i].color_tag[j] == -1) continue;
			c.insert(disk[i].color_tag[j]);
		}
		for (auto t : c) {
			belong[t].push_back(i);
		}
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
	if (finish_time - start_time >= DANGEROUSE_TIME) {
		score += (105 - (finish_time - start_time)) / 105. * PUNISH_WEIGHT;
	}
	return score;
}

float Punish_Function(int start_time, int end_time, int obj_size) {
	return 1. * (end_time - start_time) / MAX_QUERY_TIME * (obj_size + 1) * 0.5; 
}

float Get_Pos_Score(int disk_id, int pos, int time) {
	int obj_id = disk[disk_id].d[pos].first;	
	if (obj_id == -1) return 0.;
	int obj_size = objects[obj_id].size;
	int obj_tag = objects[obj_id].tag;
	float score = 0;

	if (objects[obj_id].lock.first != -1 && objects[obj_id].lock.first != disk_id) {
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
	// score /= query[obj_id].size();
	return score;
}


// 注意这个函数绝对不能放到多线程里用！！！！！！！！！
pair<float, int> Simulate(int disk_id, int idx, int time, char &pre_mov, int &pre_cos) {
	return {0., 0};
	// int step = G;

	// float score = 0;
	// vector<pair<int, int>> change;
	
	// while (step) {
	// 	auto [obj_id, obj_part] = disk[disk_id].d[idx];
	// 	int cost = INF;
	// 	if (pre_mov == 'r') {
	// 		cost = max(16, (int)ceil(pre_cos * 0.8));
	// 	} else {
	// 		cost = 64;
	// 	}
	// 	if (step < cost) {
	// 		break;
	// 	}
	// 	bool is_hit = false;
	// 	for (auto it = query[obj_id].begin(); it != query[obj_id].end(); ) {
	// 		int qry = *it;
	// 		auto prev = it;
	// 		it++;
	// 		if (requests[qry].is_give_up) {
	// 			query[obj_id].erase(prev);
	// 			continue;
	// 		}
	// 		if (requests[qry].is_done()) continue;
	// 		bool has = requests[qry].has_part(obj_part);
	// 		if (has) continue;
	// 		is_hit = true;
	// 		change.push_back({qry, obj_part});
	// 		requests[qry].set(obj_part);
	// 		if (requests[qry].is_done()) {
	// 			int x = time - requests[qry].query_time;
	// 			double g = (objects[obj_id].size + 1) * 0.5;
	// 			double f;
	// 			if (x <= 10) {
	// 				f = -0.005 * x + 1;
	// 			} else if (x <= 105) {
	// 				f = -0.01 * x + 1.05;
	// 			} else {
	// 				f = 0.;
	// 			}
	// 			score += f * g;
	// 		}
	// 	}
	// 	if (is_hit) {
	// 		pre_mov = 'r';
	// 		step -= cost;
	// 	} else {
	// 		pre_mov = 'p';
	// 		step--;
	// 	}
	// 	pre_cos = cost;
	// 	idx = (idx + 1) % V;
	// }
	// for (auto [qry, obj_part] : change) {
	// 	requests[qry].erase(obj_part);
	// }
	// return {score, idx};
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
			int block = objects[obj_id].unit[j][0];
			if (objects[obj_id].unit[j].size() == 1) {
				// priority_pos[obj_tag][obj_size].push_back({disk_id, block});
				// disk[disk_id].space[obj_size].insert(block);
				disk[disk_id].flag[block] = false;
				for (int size = 0; size < obj_size; size++) {
					disk[disk_id].erase(block + size);
				}
			} else {
				disk[disk_id].flag[objects[obj_id].unit[j][0]] = false;
				for (auto pos : objects[obj_id].unit[j]) {
					disk[disk_id].erase(pos);
				}
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
		{3, 4, 5, },
		{6, 7, 8, },
		{9, 3, 4, },
		{9, 5, 0, },
		{1, 2, 9, },
		{5, 3, 4, },
		{0, 6, 7, },
		{8, 5, 3, },
		{4, 1, 2, },
		{9, 0, 6, },
		{7, 8, 5, },
		{7, 4, 3, },
		{1, 2, 9, },
		{8, 5, 7, },
		{4, 3, 0, },
	};
	// 每个 second_choice 有 3 个 盘块
	// second_choice.resize(MAX_TAG + 1);
	// for (int i = 1; i <= MAX_TAG; i++) {
	// 	set<int> oth;
	// 	while (oth.size() < REP_NUM) {
	// 		int x = rng() % 10;
	// 		if (oth.count(x) || find(random_write_disk[i].begin(), random_write_disk[i].end(), x) != random_write_disk[i].end()) {
	// 			continue;
	// 		}
	// 		oth.insert(x);
	// 	}
	// 	for (auto hash_id : oth) {
	// 		second_choice[i].push_back(hash_id);
	// 	}
	// }
	for (int i = 1; i <= MAX_TAG; i++) {
		for (auto disk_id : belong[i]) {
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

	for (int cnt = MAX_DISK_NUM; cnt-- && write_disk.size() < 1; idx = (idx + 1) % N) {
		if (select >> idx & 1) continue;
		if (disk[idx].capacity(obj_tag, obj_size, -1)) {
			write_disk.push_back({idx, -1});
			select |= 1 << idx;
		}
	}
	for (int cnt = MAX_DISK_NUM; cnt-- && write_disk.size() < 1; idx = (idx + 1) % N) {
		if (select >> idx & 1) continue;
		if (disk[idx].capacity(obj_tag, obj_size, -4)) {
			write_disk.push_back({idx, -4});
			select |= 1 << idx;
		}
	}
	for (int cnt = MAX_DISK_NUM; cnt-- && write_disk.size() < 1; idx = (idx + 1) % N) {
		if (select >> idx & 1) continue;
		if (disk[idx].capacity(obj_tag, obj_size, -3)) {
			write_disk.push_back({idx, -3});
			select |= 1 << idx;
		}
	}
	for (int cnt = MAX_DISK_NUM; cnt-- && write_disk.size() < REP_NUM; idx = (idx + 1) % N) {
		if (select >> idx & 1) continue;
		if (disk[idx].capacity(obj_tag, obj_size, -2)) {
			write_disk.push_back({idx, -2});
			select |= 1 << idx;
		}
	}
	assert(write_disk.size() == REP_NUM);
	return write_disk;

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


	for (auto disk_id : belong[obj_tag]) {
		if (write_disk.size() == REP_NUM) break;
		if (disk[disk_id].capacity(obj_tag, obj_size, -1)) {
			write_disk.push_back({disk_id, -1});
			select |= 1 << disk_id;
		}
	}
	// assert(write_disk.size() == 1);
	// for (int cnt = 100; cnt-- && write_disk.size() < REP_NUM; idx = (idx + 1) % N) {
	// 	assert(cnt > 10);
	// 	if ((!(select >> idx & 1)) && disk[idx].capacity(obj_tag, obj_size, -2)) {
	// 		write_disk.push_back({idx, -2});
	// 		select |= 1 << idx;
	// 	}
	// }
	// assert(write_disk.size() == REP_NUM);
	// return write_disk;



	// hash 法，把 hash 值相同的放到一起
	// for (auto disk_id : random_write_disk[obj_tag]) {
	// 	if (disk[disk_id].capacity(obj_tag, obj_size, -1)) {
	// 		write_disk.push_back({disk_id, -1});
	// 		select |= 1 << disk_id;
	// 	}
	// }
	// for (auto disk_id : random_write_disk[obj_tag]) {
	// 	if (select >> disk_id & 1) continue;
	// 	if (disk[disk_id].capacity(obj_tag, obj_size, -2)) {
	// 		write_disk.push_back({disk_id, -2});
	// 		select |= 1 << disk_id;
	// 	}
	// }

	// for (auto disk_id : second_choice[obj_tag]) {
	// 	if (write_disk.size() < REP_NUM && disk[disk_id].capacity(obj_tag, obj_size, -2)) {
	// 		write_disk.push_back({disk_id, -2});
	// 		select |= 1 << disk_id;
	// 	}
	// }

	// 查缺补漏，如果不够 REP_NUM 个再顺序选几个
	for (auto disk_id : belong[obj_tag]) {
		if (write_disk.size() == REP_NUM) break;
		if (select >> disk_id & 1) continue;
		if (disk[disk_id].capacity(obj_tag, obj_size, -2)) {
			write_disk.push_back({disk_id, -2});
			select |= 1 << disk_id;
		}
	}
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
			objects[obj_id].lock = {-1, -1};
			objects[obj_id].lock_time = timestamp;
			
			for (int j = 0; j < REP_NUM; j++) {
				auto [disk_idx, is_limit] = write_disk[j];
				vector<int> write_idx = disk[disk_idx].Write(obj_id, obj_size, obj_tag, is_limit);
				// cout << "obj: " << obj_id << ' ' << obj_size << " WriteDisk: " << disk_idx << " WriteIdx: ";
				// for (auto t : write_idx) cout << t << " "; cout << endl;
				objects[obj_id].unit[j] = write_idx;
			}

			
			cout << obj_id << "\n";
			for (int j = 0; j < REP_NUM; j++) {
				cout << objects[obj_id].bel[j] + 1;
				if (objects[obj_id].unit[j].size() == 1) {
					for (int k = 0; k < objects[obj_id].size; k++) {
						cout << " " << objects[obj_id].unit[j][0] + k + 1;
					}
					cout << "\n";
				} else {
					for (auto pos : objects[obj_id].unit[j]) {
						cout << " " << pos + 1;
					}
					cout << "\n";
				}
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

int error_cnt = 0;

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
		// int mi = 1e9;
		bool give_up = 1;
		for (int j = 0; j < REP_NUM; j++) {
			// mi = min(mi, objects[obj_id].unit[j][0]);
			if (objects[obj_id].unit[j][0] + objects[obj_id].size <= disk[objects[obj_id].bel[j]].use_size) {
				give_up = 0;
			}
		}
		if (give_up) {
			waste_qid.push(qry_id);
			error_cnt++;
			continue;
		}
		query[obj_id].insert(qry_id);
		query_per_time[objects[obj_id].size].push(qry_id);
	}
}



// ------------------------------------ 磁头移动 ----------------------------------------

int Decide_Jump_Pos(int disk_id, int head_id) {
	return disk[disk_id].max_score_pos[head_id];
}

void Process(int i) {
	disk[i].Cal_Score();
}

void Lock(int disk_id, int head_id, bool all_color, int lock_num, int lock_last) {
	if (all_color) {
		int head = disk[disk_id].head[head_id];
		for (int i = head; disk[disk_id].color_tag[i] == disk[disk_id].color_tag[head]; i++) {
			int obj_id = disk[disk_id].d[i].first;
			if (obj_id == -1) continue;
			objects[obj_id].lock = {disk_id, head_id}; // here
			objects[obj_id].lock_time = timestamp;
			objects[obj_id].lock_last = lock_last; // 即便是锁所有的颜色，也不能允许锁太久！
		}
		return;
	}
	for (int cnt = lock_num, idx = disk[disk_id].head[head_id]; cnt--; idx = (idx + 1) % V) {
		int obj_id = disk[disk_id].d[idx].first;
		if (obj_id == -1) continue;
		objects[obj_id].lock = {disk_id, head_id}; // here
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
	// if (X < base) {
	// 	Lock(disk_id, head_id, false, continue_white + 7, 2);
	// }
	// cerr << X << ' ' << base << endl;
	return X < base;
	
	
	
	

	
	// 注意 obj_id != -1 !!!!!!!
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
	return timestamp - requests[qry_id].query_time == GIVE_UP_LIMIT;
}

int jump_cnt = 0;
int extra_jump_cnt = 0;
int drop = 0;
int continue_cnt = 0;
float punish_score = 0;
int exchange_cnt = 0;
int read_white = 0;
int read_cnt = 0;
int pass_cnt = 0;

void show(string name, int &cnt, string type) {
	if (type == "%") {
		if (timestamp % 1800 == 0) {
			cerr << name << " = " << 1. * cnt / N / 1800 * 100 << '%' << endl;
			cnt = 0;
		}
	} else {
		if (timestamp % 1800 == 0) {
			cerr << name << " = " << 1. * cnt / N << endl;
			cnt = 0;
		}
	}
}

void show(string name, float &cnt) {
	if (timestamp % 1800 == 0) {
		cerr << name << " = " << 1. * cnt / N / 1800 << endl;
		cnt = 0;
	}
}

bool locate_in(int x, int l, int r) {
	return x >= l && x <= r;
}
bool locate_in(int x, pair<int, int> range) {
	return x >= range.first && x <= range.second;
}

int Jump_to(int disk_id, int pos) {
	for (int cnt = G; cnt-- && Get_Pos_Score(disk_id, pos, timestamp) <= DROP_SCORE; pos = (pos + 1) % V) {

	}
	return pos;
}

void Move() {
	#ifdef DEBUG
	if (timestamp % 1800 == 0) cerr << endl;
	show("punish_score", punish_score);
	show("jump_cnt", jump_cnt, "%");
	// show("extra_jump_cnt", extra_jump_cnt, "%");
	show("exchange_cnt", exchange_cnt, "");
	show("error_cnt", error_cnt, "");
	show("read_white", read_white, "");
	show("read_cnt", read_cnt, "");
	show("pass_cnt", pass_cnt, "");
	#endif
	
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
		if (obj_id == -1 || objects[obj_id].lock != make_pair(-1, -1) && objects[obj_id].lock != make_pair(disk_id, head_id)) {
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
		objects[obj_id].lock = {-1, -1};
		objects[obj_id].lock_time = 0;
		return is_hit;
	};

	auto Jump = [&](int disk_id, int head_id) -> void {
		jump_cnt++;
		int jump_to = Jump_to(disk_id, disk[disk_id].range[head_id].first);
		disk[disk_id].head[head_id] = jump_to;
		moves[disk_id][head_id] = "j " + to_string(jump_to + 1);
		pre_move[disk_id][head_id] = 'j';
		pre_cost[disk_id][head_id] = 0;
		// Lock(disk_id, head_id, true, LOCK_UNITS, LOCK_TIMES); // 被迫跳回来之后应该上锁吗？
		pre_jump[disk_id][head_id] = timestamp;
		pre_decide[disk_id][head_id] = false;
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
			// if (timestamp % UPDATE_DISK_SCORE_FREQUENCY == 0 && disk[i].cur_score[head_id] < disk[i].max_score[head_id] && timestamp - pre_jump[i][head_id] > JUMP_BIAS) {
			// 	assert(locate_in(disk[i].max_score_pos[head_id], disk[i].range[head_id]));
			// 	int jump_to = Jump_to(i, disk[i].max_score_pos[head_id]);
			// 	jump_cnt++;
			// 	extra_jump_cnt++;
			// 	disk[i].head[head_id] = jump_to;
			// 	moves[i][head_id] = "j " + to_string(jump_to + 1);
			// 	pre_move[i][head_id] = 'j';
			// 	pre_cost[i][head_id] = 0;
			// 	Lock(i, head_id, true, LOCK_UNITS, LOCK_TIMES);
			// 	pre_jump[i][head_id] = timestamp;
			// 	pre_decide[i][head_id] = false;
			// 	continue;
			// }

			// if (disk[i].head[head_id] > disk[i].range[head_id].second - 10) {
			// 	Jump(i, head_id);
			// 	continue;
			// }

			// 方案 1.5: 如果到右边界都是空格子就直接跳到左边界
			if (timestamp % UPDATE_DISK_SCORE_FREQUENCY == 0) {
				bool bound = 1;
				float last_score = 0;
				for (int j = disk[i].head[head_id]; j <= disk[i].range[head_id].second; j++) {
					last_score += Get_Pos_Score(i, j, timestamp);
					if (last_score > 10) {
						bound = 0;
						break;
					}
					// if (disk[i].d[j].first != -1) {
					// 	bound = 0;
					// 	break;
					// }
				}
				if (bound) {
					// if (timestamp % UPDATE_DISK_SCORE_FREQUENCY == 0) {
					// 	int jump_to = Jump_to(i, disk[i].max_score_pos[head_id]);
					// 	jump_cnt++;
					// 	extra_jump_cnt++;
					// 	disk[i].head[head_id] = jump_to;
					// 	moves[i][head_id] = "j " + to_string(jump_to + 1);
					// 	pre_move[i][head_id] = 'j';
					// 	pre_cost[i][head_id] = 0;
					// 	// Lock(i, head_id, true, LOCK_UNITS, LOCK_TIMES);
					// 	pre_jump[i][head_id] = timestamp;
					// 	pre_decide[i][head_id] = false;
					// 	continue;
					// }
					Jump(i, head_id);
					continue;
				}
			}
			
			// 方案二：每个磁头固定扫描的区域
			if (!locate_in(disk[i].head[head_id], disk[i].range[head_id])) {
				Jump(i, head_id);
				continue;
			}

			while (step) {
				auto [obj_id, obj_part] = disk[i].d[disk[i].head[head_id]];
				if (obj_id != -1 && timestamp - objects[obj_id].lock_time > objects[obj_id].lock_last) {
					objects[obj_id].lock = {-1, -1};
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
				if (obj_id != -1 && objects[obj_id].lock == make_pair(i, head_id)) {
					objects[obj_id].lock = {-1, -1};
					objects[obj_id].lock_time = 0;
				}
	
				float score = Get_Pos_Score(i, disk[i].head[head_id], timestamp);
				
				// 必须把 read 和 write 割裂开，不然的话 hit 了就得 r
				
				if (score > DROP_SCORE) {
					read_cnt++;
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
					read_white++;
					read_cnt++;
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
				pass_cnt++;
				// if (score <= DROP_SCORE) {
				// 	move += 'p';
				// 	pre_move[i] = 'p';
				// 	pre_cost[i] = 0;
				// 	step--;
				// 	disk[i].head = (disk[i].head + 1) % V;
					
				// 	if (obj_id != -1 && query[obj_id].size()) {
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
	while (!waste_qid.empty()) {
		give_up_qid.emplace_back(waste_qid.front());
		waste_qid.pop();
	}
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
				{
					static vector<int> punish_cnt(MAX_TAG + 1);
					int obj_id = requests[qry_id].obj_id;
					punish_cnt[objects[obj_id].tag]++;
					if (timestamp % 1800 == 0) {
						// for (int i = 1; i <= MAX_TAG; i++) {
						// 	cerr << punish_cnt[i] << " ";
						// }
						// cerr << endl;
					}
					int mi = 1e9;
					for (int i = 0; i < REP_NUM; i++) {
						mi = min(mi, objects[obj_id].unit[i][0]);
						// cerr << objects[obj_id].bel[i] << ' ' << objects[obj_id].unit[i] << ' ' << objects[obj_id].tag << endl;
					}
				}
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
	for (auto give_up : give_up_qid) {
		haha_qid.emplace_back(give_up);
	}
	cout.flush();
}

void garbage_collection() {
	if (timestamp % FRE_PER_SLICING != 0) return;
	string GARBAGE, COLLECTION;
	cin >> GARBAGE >> COLLECTION;
	cout << "GARBAGE COLLECTION\n";
	for (int i = 0; i < N; i++) {
		vector<pair<int, int>> exchange;
		int token = K;
		for (int j = disk[i].use_size - 1; j >= 0; j--) {
			if (!disk[i].flag[j]) {
				continue;
			}
			if (disk[i].d[j].first == -1) {
				continue;
			}
			if (j) {
				if (disk[i].d[j].first == disk[i].d[j - 1].first) {
					cerr << "HHHHHHHHHHHHHHHHHHHHHHHHHH" << endl;
					cerr << disk[i].flag[j - 1] << ' ' << disk[i].flag[j] << endl;
					cerr << disk[i].d[j - 1].first << " " << disk[i].d[j].first << endl;
				}
				assert(disk[i].d[j].first != disk[i].d[j - 1].first);
			}
			auto [obj_id, obj_part] = disk[i].d[j];
			int obj_tag = objects[obj_id].tag;
			int obj_size = objects[obj_id].size;
			if (disk[i].color_tag[j] == -1) {
				continue;
			}
			if (obj_size > token) {
				continue;
			}

			int change_to = -1;

			// 第一选择：把前 1/3 的杂色踢出去
			if (disk[i].color_tag[j] != obj_tag) {
				for (int k = 0; k < disk[i].use_size; k++) {
					if (disk[i].color_tag[k] != obj_tag) continue;
					if (disk[i].d[k].first == -1) continue;
					if (!disk[i].flag[k]) continue;
					int cur_obj_id = disk[i].d[k].first;
					int cur_obj_tag = objects[obj_id].tag;
					int cur_obj_size = objects[obj_id].size;
					if (cur_obj_size != obj_size) continue;
					if (disk[i].color_tag[k] == cur_obj_tag) continue;
					change_to = k;
					break;
				}
				if (change_to == -1) {
					for (int k = 0; k + obj_size - 1 < disk[i].use_size; k++) {
						if (disk[i].color_tag[k] != obj_tag) continue;
						if (disk[i].d[k].first != -1) continue;
						bool ok = 1;
						for (int p = 0; p < obj_size; p++) {
							if (disk[i].d[k + p].first != -1 || disk[i].color_tag[k + p] != obj_tag) {
								ok = 0;
								break;
							}
						}
						if (!ok) continue;
						change_to = k;
						break;
					}		
				}
				// 把以后不会热起来的标签直接扔到后面去？
			}
			
			if (change_to == -1) {
				continue;
			}
			token -= obj_size;
			for (int p = 0; p < REP_NUM; p++) {
				assert(objects[obj_id].unit[p].size() == 1);
				if (objects[obj_id].bel[p] == i) {
					objects[obj_id].unit[p][0] = change_to;
					break;
				}
			}
			assert(disk[i].flag[j]);
			disk[i].flag[change_to] = true;
			if (disk[i].d[change_to].first != -1) {
				assert(disk[i].flag[change_to]);
				disk[i].flag[j] = true;
				for (int p = 0; p < REP_NUM; p++) {
					assert(objects[disk[i].d[change_to].first].unit[p].size() == 1);
					if (objects[disk[i].d[change_to].first].bel[p] == i) {
						objects[disk[i].d[change_to].first].unit[p][0] = j;
						break;
					}
				}
			} else {
				disk[i].flag[j] = false;
			}
			// if (i == 9) {
			// 	cerr << "####" << j << ' ' << change_to << ' ' << obj_size << ' ' << disk[i].d[change_to].first << endl;
			// 	for (int k = 0; k < obj_size; k++) {
			// 		cerr << disk[i].d[j + k].first << ' ';
			// 	}
			// 	cerr << endl;
			// 	for (int k = 0; k < obj_size; k++) {
			// 		cerr << disk[i].d[change_to + k].first << ' ';
			// 	}
			// 	cerr << endl;
			// }
			for (int k = 0; k < obj_size; k++) {
				int a = (j + k) % V, b = (change_to + k) % V;
				assert(disk[i].d[b].first == -1);
				exchange.push_back({a, b});
				swap(disk[i].d[a], disk[i].d[b]);
			}
		}
		for (int j = disk[i].use_size - 1; j >= 0; j--) {
			if (!disk[i].flag[j]) {
				continue;
			}
			if (disk[i].d[j].first == -1) {
				continue;
			}
			if (j) {
				if (disk[i].d[j].first == disk[i].d[j - 1].first) {
					cerr << "HHHHHHHHHHHHHHHHHHHHHHHHHH" << endl;
					cerr << disk[i].flag[j - 1] << ' ' << disk[i].flag[j] << endl;
					cerr << disk[i].d[j - 1].first << " " << disk[i].d[j].first << endl;
				}
				assert(disk[i].d[j].first != disk[i].d[j - 1].first);
			}
			auto [obj_id, obj_part] = disk[i].d[j];
			int obj_tag = objects[obj_id].tag;
			int obj_size = objects[obj_id].size;
			if (disk[i].color_tag[j] == -1) {
				continue;
			}
			if (obj_size > token) {
				continue;
			}

			int change_to = -1;

			// 第二选择：把一个色块内部的东西往色块最前面堆
			if (change_to == -1) {
				for (int k = 0; k + obj_size - 1 < j; k++) {
					if (disk[i].d[k].first != -1) continue;
					if (disk[i].color_tag[k] != obj_tag) continue;
					bool ok = 1;
					for (int p = 0; p < obj_size; p++) {
						if (disk[i].d[k + p].first != -1 || disk[i].color_tag[k + p] != obj_tag) {
							ok = 0;
							break;
						}
					}
					if (!ok) continue;
					change_to = k;
					break;
				}
			}
			if (change_to == -1) {
				continue;
			}
			token -= obj_size;
			for (int p = 0; p < REP_NUM; p++) {
				assert(objects[obj_id].unit[p].size() == 1);
				if (objects[obj_id].bel[p] == i) {
					objects[obj_id].unit[p][0] = change_to;
					break;
				}
			}
			assert(disk[i].flag[j]);
			disk[i].flag[change_to] = true;
			if (disk[i].d[change_to].first != -1) {
				assert(disk[i].flag[change_to]);
				disk[i].flag[j] = true;
				for (int p = 0; p < REP_NUM; p++) {
					assert(objects[disk[i].d[change_to].first].unit[p].size() == 1);
					if (objects[disk[i].d[change_to].first].bel[p] == i) {
						objects[disk[i].d[change_to].first].unit[p][0] = j;
						break;
					}
				}
			} else {
				disk[i].flag[j] = false;
			}
			// if (i == 9) {
			// 	cerr << "####" << j << ' ' << change_to << ' ' << obj_size << ' ' << disk[i].d[change_to].first << endl;
			// 	for (int k = 0; k < obj_size; k++) {
			// 		cerr << disk[i].d[j + k].first << ' ';
			// 	}
			// 	cerr << endl;
			// 	for (int k = 0; k < obj_size; k++) {
			// 		cerr << disk[i].d[change_to + k].first << ' ';
			// 	}
			// 	cerr << endl;
			// }
			for (int k = 0; k < obj_size; k++) {
				int a = (j + k) % V, b = (change_to + k) % V;
				assert(disk[i].d[b].first == -1);
				exchange.push_back({a, b});
				swap(disk[i].d[a], disk[i].d[b]);
			}
		}
		exchange_cnt += exchange.size();
		cout << exchange.size() << "\n";
		for (auto [a, b] : exchange) {
			cout << a + 1 << ' ' << b + 1 << '\n';
		}
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
	// cerr << "time = " << timestamp << endl;
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
	// if (G != 340) {
	// 	cout << "G!=340\n";
	// 	cout.flush();
	// }
	// if (K != 40) {
	// 	cout << "K!=40\n";
	// 	cout.flush();
	// }
	for (int i = 0; i < N; i++) {
		disk[i].init(i);
	}
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