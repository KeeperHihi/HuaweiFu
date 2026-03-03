#include <bits/stdc++.h>
using namespace std;

#define MAX_TAG 16
#define MAX_EPOCH 86506

int tag_frequency[MAX_TAG + 1][MAX_EPOCH];

vector<vector<int>> cnt = {
	{},
	{0, 358, 249, 222, 111, 34, },
	{0, 190, 136, 105, 39, 23, },
	{0, 335, 239, 232, 113, 63, },
	{0, 293, 221, 182, 115, 51, },
	{0, 225, 155, 121, 60, 14, },
	{0, 128, 78, 80, 45, 19, },
	{0, 298, 248, 200, 116, 38, },
	{0, 251, 192, 177, 86, 37, },
	{0, 150, 73, 80, 39, 18, },
	{0, 237, 193, 159, 96, 39, },
	{0, 159, 95, 100, 41, 22, },
	{0, 323, 195, 181, 100, 44, },
	{0, 121, 98, 82, 39, 9, },
	{0, 258, 159, 142, 66, 31, },
	{0, 136, 109, 87, 41, 9, },
	{0, 370, 252, 236, 123, 55, },
};
// [tag][size] 的写入数量

int main() {
	double fact = 0.810387;
	vector<int> tag_cnt(17);
	for (int i = 1; i <= 16; i++) {
		for (int j = 1; j <= 5; j++) {
			tag_cnt[i] += cnt[i][j] * j;
		}
	}
	for (int i = 1; i <= 16; i++) {
		cout << tag_cnt[i] << " \n"[i == 16];
	}
	int sum = 0;
	for (int i = 1; i <= 16; i++) {
		for (int j = 1; j <= 5; j++) {
			sum += cnt[i][j] * j;
		}
	}
	cout << sum << endl;
	sum *= 3;
	
	return 0;
}