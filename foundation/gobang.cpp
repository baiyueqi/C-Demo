#include <iostream>
#include <vector>
using namespace std;

const int SIZE = 15; // 棋盘大小
vector<vector<char>> board(SIZE, vector<char>(SIZE, '.'));

void printBoard() {
    cout << "  ";
    for (int i = 0; i < SIZE; ++i) cout << (i < 10 ? " " : "") << i << " ";
    cout << endl;
    for (int i = 0; i < SIZE; ++i) {
        cout << (i < 10 ? " " : "") << i << " ";
        for (int j = 0; j < SIZE; ++j) {
            cout << " " << board[i][j] << " ";
        }
        cout << endl;
    }
}

bool checkWin(int x, int y, char c) {
    int dx[] = {1, 0, 1, 1};
    int dy[] = {0, 1, 1, -1};
    for (int d = 0; d < 4; ++d) {
        int cnt = 1;
        for (int k = 1; k < 5; ++k) {
            int nx = x + dx[d] * k, ny = y + dy[d] * k;
            if (nx < 0 || nx >= SIZE || ny < 0 || ny >= SIZE || board[nx][ny] != c) break;
            cnt++;
        }
        for (int k = 1; k < 5; ++k) {
            int nx = x - dx[d] * k, ny = y - dy[d] * k;
            if (nx < 0 || nx >= SIZE || ny < 0 || ny >= SIZE || board[nx][ny] != c) break;
            cnt++;
        }
        if (cnt >= 5) return true;
    }
    return false;
}

int main() {
    int turn = 0;
    printBoard();
    while (true) {
        int x, y;
        char c = (turn % 2 == 0) ? 'X' : 'O';
        cout << "玩家" << (turn % 2 + 1) << " (" << c << ") 输入落子坐标（行 列）：";
        cin >> x >> y;
        if (x < 0 || x >= SIZE || y < 0 || y >= SIZE || board[x][y] != '.') {
            cout << "无效坐标，请重新输入！" << endl;
            continue;
        }
        board[x][y] = c;
        printBoard();
        if (checkWin(x, y, c)) {
            cout << "玩家" << (turn % 2 + 1) << " (" << c << ") 获胜！" << endl;
            break;
        }
        turn++;
        if (turn == SIZE * SIZE) {
            cout << "平局！" << endl;
            break;
        }
    }
    return 0;
}