#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include <signal.h>
#include <sys/ipc.h>
#include <sys/shm.h>
#include <sys/msg.h>
#include <sys/sem.h>
#include "structures.h"

int shmid, semid, qid_A, qid_B;

const Unit units_info[4] = {
    {0,100, 1.0, 1.2, 2}, // LEKKA
    {1,250, 1.5, 3.0, 3}, // CIEZKA
    {2,550, 3.5, 1.2, 5}, // JAZDA
    {3,150, 0.0, 0.0, 2}  // ROBOTNICY
};

void lock_sem() {
    struct sembuf sb = {0, -1, 0};
    semop(semid, &sb, 1);
}

void unlock_sem() {
    struct sembuf sb = {0, 1, 0};
    semop(semid, &sb, 1);
}

// === CZYSZCZENIE ZASOBÓW (Ctrl+C) ===
void cleanup(int sig) {
    printf("\nZamykanie serwera... czyszczenie IPC.\n");
    shmctl(shmid, IPC_RMID, NULL);
    semctl(semid, 0, IPC_RMID);
    msgctl(qid_A, IPC_RMID, NULL);
    msgctl(qid_B, IPC_RMID, NULL);
    kill(0, SIGKILL);
    exit(0);
}

double attacking_power(int *att_army){
    double power=0;
    for(int i=0; i<3; i++) 
        power+=att_army[i]*units_info[i].attack;
    return power;
}
double  defending_power(Player *p ){
    double power=0;
    for(int i=0; i<4; i++) 
        power+=p->unit_types[i] * units_info[i].defense;
    return power;
}

void battle(Game *game) {
    
    Player *attacker_base;
    Player *defender_base;
    int *att_army = game->current_attack.att_units;
    
    if (game->current_attack.attacker == PLAYER_A) {
        attacker_base=&game->Player_A;
        defender_base=&game->Player_B;
        printf("#GRACZ A PRZEPROWADZA ATAK NA BAZE GRACZA B\n");
    } else {
        attacker_base = &game->Player_B;
        defender_base = &game->Player_A;
        printf("#GRACZ B PRZEPROWADZA ATAK NA BAZE GRACZA A\n");
    }

    double att_power=attacking_power(att_army);
    double def_power=defending_power(defender_base);
    double def_att_power=attacking_power(defender_base->unit_types);
    double att_def_power=0;
    for(int i=0; i<3; i++) {
        att_def_power += att_army[i] * units_info[i].defense;
    }
    
    printf("#SILA ATAKU: %.2f\n#SILA OBRONY: %.2f\n",att_power,def_power);

    if (att_power>def_power) {
        printf("#ATAK UDANY\n#PUNKT PRZYZNANY GRACZOWI ATAKUJACEMU\n");
        for(int i=0; i<4; i++) 
            defender_base->unit_types[i]=0;
        attacker_base->player_points++;
        if (attacker_base->player_points==5){
            printf("#KONIEC GRY\nGRACZ A WYGRYWA\n");
            sleep(1);
            printf("\nZamykanie serwera... czyszczenie IPC.\n");
            shmctl(shmid, IPC_RMID, NULL);
            semctl(semid, 0, IPC_RMID);
            msgctl(qid_A, IPC_RMID, NULL);
            msgctl(qid_B, IPC_RMID, NULL);
            kill(0, SIGKILL);
            exit(0);
        }
    } else {
        printf("#ATAK NIEUDANY\n");
        for(int i=0; i<4; i++)
            defender_base->unit_types[i]-=(defender_base->unit_types[i])*(att_power/def_power);
    }
    if (def_att_power > att_def_power) {
         for(int i=0; i<3; i++) {
            att_army[i]-=(int)(att_army[i]*(att_def_power/def_att_power));
        }
    }

    for(int i=0; i<3; i++) {
        attacker_base->unit_types[i] += att_army[i];
        att_army[i]=0;
    }
}

void production(Player *p, int tick) {
    if (p->queue[0].slot_active){
        if (p->queue[0].next_training_time==0){
            p->queue[0].next_training_time=tick+units_info[p->queue[0].unit_type].production_time;
        }
        if (tick >= p->queue[0].next_training_time) {
            int type = p->queue[0].unit_type;
            p->unit_types[type]++;
            p->queue[0].remaining_units--;
            p->queue[0].next_training_time=0;
            if (p->queue[0].remaining_units <= 0){
                int i=0;
                while (p->queue[i+1].slot_active && i<9){
                    p->queue[i]=p->queue[i+1];
                    i++;
                }
                p->queue[i].slot_active=0;
                p->queue[i].next_training_time=0;    
            }
        }
    }
}

int main() {
    signal(SIGINT, cleanup);

    key_t key_shm = ftok(".",SHM_ID);
    key_t key_sem = ftok(".",SEM_ID);
    key_t key_qa = ftok(".",ID_QUEUE_A);
    key_t key_qb = ftok(".",ID_QUEUE_B);

    shmid = shmget(key_shm,sizeof(Game),0666|IPC_CREAT);
    semid = semget(key_sem,1,0666|IPC_CREAT);
    qid_A = msgget(key_qa,0666|IPC_CREAT);
    qid_B = msgget(key_qb,0666|IPC_CREAT);

    Game *game = (Game*) shmat(shmid, NULL, 0);
    memset(game, 0, sizeof(Game));
    
    game->Player_A.resources = 300;
    game->Player_B.resources = 300;
    game->game_active=1;

    semctl(semid, 0, SETVAL, 1);
    printf("#SERWER URUCHOMIONY\n");

    if (fork() == 0) {
        MessageBuff msg_auto;
        int tick = 0;
        while(1) {
            sleep(1);
            tick++;
            lock_sem();
            game->Player_A.resources += 50 + (game->Player_A.unit_types[ROBOTNICY] * 5);
            game->Player_B.resources += 50 + (game->Player_B.unit_types[ROBOTNICY] * 5);

            production(&game->Player_A,tick);
            production(&game->Player_B,tick);

            if (game->current_attack.attack_active) {
                if (game->current_attack.end_time==0) {
                    game->current_attack.end_time=tick+5;
                }
                if (tick == game->current_attack.end_time){
                    battle(game);
                    game->current_attack.attack_active=0;
                    game->current_attack.end_time=0;
                }
            }

            msg_auto.mtype = MSG_UPDATE;
            msg_auto.current_state = game->Player_A;
            msgsnd(qid_A, &msg_auto, sizeof(MessageBuff)-sizeof(long), IPC_NOWAIT); 

            msg_auto.mtype = MSG_UPDATE;
            msg_auto.current_state = game->Player_B;
            msgsnd(qid_B, &msg_auto, sizeof(MessageBuff)-sizeof(long), IPC_NOWAIT);

            unlock_sem();
        }
    }else{
        MessageBuff msg;
        while(1) {
            // GRACZ A
            if (msgrcv(qid_A, &msg, sizeof(MessageBuff)-sizeof(long), MSG_ORDER, IPC_NOWAIT) != -1) {
                lock_sem();
                if (msg.action==TRAIN) {
                     int cost=units_info[msg.unit_type].price*msg.count;
                     if (game->Player_A.resources>=cost) {
                         for(int i=0; i<10; i++) {
                             if (!game->Player_A.queue[i].slot_active) {
                                 game->Player_A.queue[i].slot_active=1;
                                 game->Player_A.resources-=cost;
                                 game->Player_A.queue[i].unit_type=msg.unit_type;
                                 game->Player_A.queue[i].remaining_units=msg.count;
                                 game->Player_A.queue[i].next_training_time=0;
                                 break;
                             }
                         }
                     }else 
                        printf("#BRAK SRODKOW NA WYKONANIE POLECENIA GRACZA\n");
                } else if (msg.action==ATTACK) {
                    if (!game->current_attack.attack_active && game->Player_A.unit_types[0]>=msg.army[0] && game->Player_A.unit_types[1]>=msg.army[1] && game->Player_A.unit_types[2]>=msg.army[2]) {
                        game->current_attack.attack_active=1;
                        game->current_attack.attacker=PLAYER_A;
                        game->current_attack.end_time=0;
                        for (int i=0;i<3;i++){
                            game->Player_A.unit_types[i]-=msg.army[i];
                            game->current_attack.att_units[i]=msg.army[i];
                        }
                    }
                }
            msg.mtype = MSG_UPDATE;
            msg.current_state = game->Player_A;
            unlock_sem();
            msgsnd(qid_A, &msg, sizeof(MessageBuff)-sizeof(long), 0);
            }

            // GRACZ B
            if (msgrcv(qid_B, &msg, sizeof(MessageBuff)-sizeof(long), MSG_ORDER, IPC_NOWAIT) != -1) {
                lock_sem();
                if (msg.action==TRAIN) {
                     int cost=units_info[msg.unit_type].price*msg.count;
                     if (game->Player_B.resources>=cost) {
                         for(int i=0; i<10; i++) {
                             if (!game->Player_B.queue[i].slot_active) {
                                 game->Player_B.queue[i].slot_active=1;
                                 game->Player_B.resources-=cost;
                                 game->Player_B.queue[i].unit_type=msg.unit_type;
                                 game->Player_B.queue[i].remaining_units=msg.count;
                                 game->Player_B.queue[i].next_training_time=0;
                                 break;
                             }
                         }
                     }else 
                        printf("#BRAK SRODKOW NA WYKONANIE POLECENIA GRACZA\n");
                } else if (msg.action==ATTACK) {
                    if (!game->current_attack.attack_active && game->Player_B.unit_types[0]>=msg.army[0] && game->Player_B.unit_types[1]>=msg.army[1] && game->Player_B.unit_types[2]>=msg.army[2]) {
                        game->current_attack.attack_active=1;
                        game->current_attack.attacker=PLAYER_B;
                        game->current_attack.end_time=0;
                        for (int i=0;i<3;i++){
                            game->Player_B.unit_types[i]-=msg.army[i];
                            game->current_attack.att_units[i]=msg.army[i];
                        }
                    }
                }
            msg.mtype = MSG_UPDATE;
            msg.current_state = game->Player_B;
            unlock_sem();
            msgsnd(qid_B, &msg, sizeof(MessageBuff)-sizeof(long), 0);
            }
            usleep(10000);
        }
    }
    return 0;
}