#include "common.h"

sem_t patient_ready;   // 病人来了
sem_t doctor_ready;    // 医生准备好
sem_t mutex; // 控制进入顺序

void* doctor(void*) {
    while (true) {
        sem_wait(&patient_ready); // 等病人

        cout << "医生：开始看病" << endl;
        sleep(1);

        cout << "医生：看病结束" << endl;
        sem_post(&doctor_ready); // 通知病人结束
    }
    return nullptr;
}

void* patient(void* arg) {
    int id = *(int*)arg;

    sem_wait(&mutex);   //保证一个一个进入

    cout << "病人 " << id << " 进入诊室" << endl;

    sem_post(&patient_ready);
    sem_wait(&doctor_ready);

    cout << "病人 " << id << " 离开" << endl;

    sem_post(&mutex);   //释放

    return nullptr;
}

void run_clinic() {
    sem_init(&patient_ready, 0, 0);
    sem_init(&doctor_ready, 0, 0);
    sem_init(&mutex, 0, 1);

    pthread_t doc;
    pthread_create(&doc, nullptr, doctor, nullptr);

    pthread_t p[5];
    int id[5];

    for (int i = 0; i < 5; i++) {
        id[i] = i + 1;
        pthread_create(&p[i], nullptr, patient, &id[i]);
        sleep(1);
    }

    for (int i = 0; i < 5; i++) {
        pthread_join(p[i], nullptr);
    }
}