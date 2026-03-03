#include <bits/stdc++.h>
using namespace std;

#define MAX_TAG (16)
#define MAX_DISK_NUM (10)
#define MAX_DISK_SIZE (16384)
#define MAX_REQUEST_NUM (30000000)
#define MAX_OBJECT_NUM (100000)
#define REP_NUM (3)
#define FRE_PER_SLICING (1800)
#define EXTRA_TIME (105)
#define MAX_EPOCH (50)

int T, M, N, V, G, K;
int fre_del[MAX_TAG + 1][MAX_EPOCH];
int fre_write[MAX_TAG + 1][MAX_EPOCH];
int fre_read[MAX_TAG + 1][MAX_EPOCH];
int g[MAX_EPOCH];

int timestamp = -1;

struct Disk {
	pair<int, int> d[MAX_DISK_SIZE];
	int head = 0;
	int siz = 0;
	Disk() {
		for (int i = 0; i < MAX_DISK_SIZE; i++) {
			d[i] = {-1, -1};
		}
	}
	int size() {
		return siz;
	}
	int last() {
		return V - siz;
	}
	vector<int> Write(int obj_id, int obj_size, int obj_tag) {
		vector<int> write_idx;
		for (int i = 0; i < V; i++) {
			if (write_idx.size() == obj_size) break;
			if (d[i].first != -1) continue;
			d[i] = {obj_id, write_idx.size()};
			write_idx.emplace_back(i);
			siz++;
		}
		assert(write_idx.size() == obj_size);
		return write_idx;
	}
	void erase(int erase_idx) {
		assert(d[erase_idx].first != -1);
		d[erase_idx] = {-1, -1};
		siz--;
	}
};
Disk disk[MAX_DISK_NUM]; // 硬盘

struct Request {
	int obj_id = -1;
	vector<bool> ned;
	bool is_done = false;
	void update() {
		is_done = accumulate(ned.begin(), ned.end(), 0) == ned.size();
	}
};
Request requests[MAX_REQUEST_NUM + 1];

struct Object {
	vector<int> bel;
	vector<int> unit[REP_NUM];
	int size = 0;
	int tag = -1;
	int write_time = 0;
	bool is_delete = false;
};
Object objects[MAX_OBJECT_NUM + 1];
vector<int> query[MAX_OBJECT_NUM + 1]; // 每个对象的查询








// ------------------------------------ 全局预处理 ----------------------------------------

void Pre_Process() {
	
}



vector<int> Write(17);
vector<int> Read(17);
vector<int> mx(17);

// ------------------------------------ 删除 ----------------------------------------

// vector<pair<int, int>> er; 
void Delete_Action() {
	int n_delete;
	cin >> n_delete;
	static int delete_id[MAX_OBJECT_NUM];
	for (int i = 0; i < n_delete; i++) {
		cin >> delete_id[i];
		int obj_tag = objects[delete_id[i]].tag;
		int obj_size = objects[delete_id[i]].size;
		Write[obj_tag] -= obj_size;
	}
	// for (int i = 0; i < n_delete; i++) {
	// 	er.push_back(make_pair(delete_id[i], objects[delete_id[i]].write_time));
	// }
}




// ------------------------------------ 写入 ----------------------------------------


void Write_Action() {
	int n_write;
	cin >> n_write;
	vector<int> write_id(n_write), write_obj_size(n_write), write_obj_tag(n_write);
	for (int i = 0; i < n_write; i++) {
		cin >> write_id[i] >> write_obj_size[i] >> write_obj_tag[i];
	}
    for (int i = 0; i < n_write; i++) {
		int obj_id = write_id[i];
		int obj_size = write_obj_size[i];
		int obj_tag = write_obj_tag[i];

		objects[obj_id].tag = obj_tag;
		objects[obj_id].size = obj_size;
		objects[obj_id].write_time = timestamp;
	}
    for (int i = 0; i < n_write; i++) {
		Write[write_obj_tag[i]] += write_obj_size[i];
		mx[write_obj_tag[i]] = max(mx[write_obj_tag[i]], Write[write_obj_tag[i]]);
        // cout << write_obj_tag[i] << " " << write_obj_size[i] << "\n";
    }
}



// ------------------------------------ 读取 ----------------------------------------

void Read_Action() {
	int n_read;
	cin >> n_read;
	vector<int> request_id(n_read), read_id(n_read);
	for (int i = 0; i < n_read; i++) {
		cin >> request_id[i] >> read_id[i];
		Read[objects[read_id[i]].tag] += objects[read_id[i]].size;
	}
    // cout << n_read;
    // for (int i = 0; i < n_read; i++) {
    //     cout << " " << read_id[i];
    // }
    // cout << "\n";
}



// ------------------------------------ 磁头移动 ----------------------------------------

void garbage_collection() {
	if (timestamp % FRE_PER_SLICING != 0) return;
	string GARBAGE, COLLECTION;
	cin >> GARBAGE >> COLLECTION;
}

void Solve() {
	Delete_Action();
	Write_Action();
	Read_Action();
	garbage_collection();
}



 

void TimeStamp() {
	string pattern;
	int timeStamp;
	cin >> pattern >> timeStamp;
	timestamp = timeStamp;
}

int main() {
    freopen("../data/sample_official.in", "r", stdin);
    freopen("read_vector.txt", "w", stdout);
	cin >> T >> M >> N >> V >> G >> K;
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
		// cout << "{";
		for (int j = 1; j <= batch_num; j++) {
			cin >> fre_read[i][j];
			// if (j <= 5) 
			// cout << fre_write[i][j] << ", ";
		}
		// cout << "},";
		// cout << endl;
	}
	for (int i = 1; i <= ceil((T + 105) * 1. / 1800); i++) {
		cin >> g[i];
	}
	// for (int i = 1; i <= 16; i++) {
	// 	cout << accumulate(fre_write[i] + 5, fre_write[i] + batch_num + 1, 0) << ' ';
	// }
	// cout << endl;
	
	Pre_Process();
	for (int i = 1; i <= T + 105; i++) {
		TimeStamp();
		Solve();
	}

	// for (auto [obj_id, erase_time] : er) {
	// 	int obj_size = objects[obj_id].size;
	// 	int obj_tag = objects[obj_id].tag;
	// 	cout << obj_id << ' ' << obj_tag << ' ' << obj_size << ' ' << erase_time << endl;
	// }

	for (int i = 1; i <= 16; i++) {
		cout << mx[i] << ", ";
	}
	
	return 0;
}