#include <bits/stdc++.h>
using namespace std;

void write(vector<vector<tuple<int, int, int>>> &vec, string name) {
    cout << "vector<vector<tuple<int, int, int>>> " << name << " = {\n";
    for (int i = 0; i < vec.size(); i++) {
        cout << "\t{\n";
        for (int j = 0; j < vec[i].size(); j++) {
            auto [a, b, c] = vec[i][j];
            cout << "\t\tmake_tuple(" << a << ", " << b << ", " << c << "),\n";
        }
        cout << "},\n";
    }
    cout << "};";
}

int main() {
    freopen("special_obj.txt", "r", stdin);
    freopen("include.cpp", "w", stdout);

    cout << "#include <bits/stdc++.h>\n";
    cout << "using namespace std;\n";

    string name = "special_objs";

    
    vector<vector<tuple<int, int, int>>> vec(17);

    for (int i = 1; i <= 16; i++) {
        int n;
        cin >> n;
        for (int j = 0; j < n; j++) {
            int a, b, c;
            cin >> a >> b >> c;
            vec[i].emplace_back(a, b, c);
        }
    }
    write(vec, name);
    
    return 0;
}