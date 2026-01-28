#include <stdio.h>
#include <stdlib.h>
#include <sys/msg.h>
#include <unistd.h>
#include <string.h>
#include <signal.h>
#include "structures.h"

void wait_for_enter() {
    char c;
    while(read(0, &c, 1) > 0) {
        if (c == '\n') break;
    }
}

int read_na_int() {
    char buf[10];
    int wynik = 0;
    int n = read(0, buf, 10);
    if (n <= 0) return -1;
    for (int i = 0; i < n; i++) {
        if (buf[i] >= '0' && buf[i] <= '9') {
            wynik = wynik * 10 + (buf[i] - 48);
        } else if (buf[i] == '\n') {
            break;
        }else{
            printf("WRONG INPUT");
            return 0;
        }
    }
    return wynik;
}

void interface(Player current_state, int player_id) {
    printf("\033[H\033[J");
    printf("PUNKTY ZWYCIESTWA: %d\n",current_state.player_points);
    printf("\n=== MOJA WIOSKA ===\n");
    printf("Zasoby: %d\n", current_state.resources);
    printf("Piechota lekka:  %d\n", current_state.unit_types[LEKKA_PIECHOTA]);
    printf("Piechota ciezka: %d\n", current_state.unit_types[CIEZKA_PIECHOTA]);
    printf("Jazda:           %d\n", current_state.unit_types[JAZDA]);
    printf("Robotnicy:       %d\n", current_state.unit_types[ROBOTNICY]);
    
    printf("\nKOLEJKA PRODUKCJI:\nLP.   TYP   ILOSC\n");
    for (int i = 0; i < 10; i++) {
        if (current_state.queue[i].slot_active) {
            printf("%d.   Typ: %d    Sztuk: %d\n",i+1,current_state.queue[i].unit_type,current_state.queue[i].remaining_units);
        }
    }
    printf("\n[ Wcisnij ENTER aby wykonać akcje ]");
    fflush(stdout);
}

int main(int argc, char *argv[]) {
    int queue_id_player;
    if (argc > 1 && *argv[1] == 'A') {
        queue_id_player = ID_QUEUE_A;
    }else if (argc > 1 && *argv[1] == 'B') {
        queue_id_player = ID_QUEUE_B;
    }else{
        printf("WRONG CLIENT\n");
    }
    int qid = msgget(ftok(".", queue_id_player), 0666);
    if (qid == -1) {
        perror("Brak dostepu do kolejki\n");
        return 1;
    }
    MessageBuff msg_out, msg_in;
    pid_t pid=fork();
    if (pid==0) {
        while(1) {
            if (msgrcv(qid, &msg_in, sizeof(MessageBuff)-sizeof(long), MSG_UPDATE, 0) != -1) {
                interface(msg_in.current_state, queue_id_player);
            }
        }
    } else {
        int action_input, unit_input, count_input;
        while(1) {
            wait_for_enter();
            kill(pid,SIGSTOP);
            printf("\n--- OPCJE ---\n");
            printf("1. Trenuj jednostki\n");
            printf("2. Atakuj przeciwnika\n");
            printf("\nTwoj wybor (1-2): ");
            fflush(stdout);
            action_input=read_na_int();
            switch (action_input)
            {
            case 1:
                msg_out.action=TRAIN;
                printf("Podaj typ jednostki (0-Lekka piechota, 1-Ciezka piechota, 2-Jazda, 3-Robotnik): ");
                fflush(stdout);
                unit_input=read_na_int();
                if (unit_input>=0 && unit_input<=3){
                    printf("Podaj ilosc:");
                    fflush(stdout);
                    count_input=read_na_int();
                }else{
                    printf("Bład przy wprowadzaniu danych. Spróbuj ponownie.\n");
                    usleep(50000);
                    break;
                }
                msg_out.mtype = MSG_ORDER;
                msg_out.unit_type = unit_input;
                msg_out.count = count_input;
                msgsnd(qid,&msg_out,sizeof(MessageBuff)-sizeof(long),0);
                break;
            
            case 2:
                msg_out.action=ATTACK;
                printf("=== SKONSTRUUJ ARMIE ====\n");
                printf("Podaj ilosc jednostek każdego typu: \n");
                fflush(stdout);
                printf("Lekka piechota: ");
                fflush(stdout);
                msg_out.army[0]=read_na_int();
                printf("Ciezka piechota: ");
                fflush(stdout);
                msg_out.army[1]=read_na_int();
                printf("Jazda: ");
                fflush(stdout);
                msg_out.army[2]=read_na_int();
                if (msg_out.army[0]<0 ||msg_out.army[1]<0 ||msg_out.army[2]<0){
                    printf("Bład przy wprowadzaniu danych. Spróbuj ponownie.\n");
                    usleep(50000);
                    break;
                }
                msg_out.mtype = MSG_ORDER;
                msgsnd(qid,&msg_out,sizeof(MessageBuff)-sizeof(long),0);
                break;
                }
            kill(pid,SIGCONT);
        }
    }
    return 0;
}