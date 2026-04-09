#include "common.h"

sem_t room;  // 容量为20
pthread_mutex_t print_mutex;  // 控制输出

void* buyer(void* arg) {
    int id = *(int*)arg;

    pthread_mutex_lock(&print_mutex);
    cout << "购票者 " << id << " 在外等待" << endl;
    pthread_mutex_unlock(&print_mutex);

    sem_wait(&room); // 进入售票厅

    pthread_mutex_lock(&print_mutex);
    cout << "购票者 " << id << " 进入售票厅" << endl;
    pthread_mutex_unlock(&print_mutex);
    sleep(2);

    pthread_mutex_lock(&print_mutex);
    cout << "购票者 " << id << " 离开售票厅" << endl;
    pthread_mutex_unlock(&print_mutex);

    sem_post(&room); // 释放位置
    return nullptr;
}

void run_ticket() {
    sem_init(&room, 0, 20);
    pthread_mutex_init(&print_mutex, nullptr);//后面补的，为了控制输出

    pthread_t t[30];
    int id[30];

    for (int i = 0; i < 30; i++) {
        id[i] = i + 1;
        pthread_create(&t[i], nullptr, buyer, &id[i]);
    }

    for (int i = 0; i < 30; i++) {
        pthread_join(t[i], nullptr);
    }
}