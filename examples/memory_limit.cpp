#include <vector>
using namespace std;

int main() {
    vector<int*> blocks;

    while (true) {
        int* p = new int[1024 * 1024]; // 每次大约申请 4 MB
        blocks.push_back(p);
    }

    return 0;
}
