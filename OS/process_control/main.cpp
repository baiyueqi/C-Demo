#include "common.h"

void run_clinic();
void run_ticket();

int main() {
    cout << "===== 看病问题 =====" << endl;
    run_clinic();

    sleep(2);

    cout << "\n===== 售票厅问题 =====" << endl;
    run_ticket();

    return 0;
}