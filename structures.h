#ifndef STRUCTURES_H
#define STRUCTURES_H

#include <sys/types.h>
#include <sys/ipc.h>
#include <stdlib.h>

// Klucze do kolejek i pamieci (ftok)
#define ID_QUEUE_A 1001  // ID kolejki dla gracza A
#define ID_QUEUE_B 1002 // ID kolejki dla gracza B
#define SHM_ID 2001      // ID pamięci współdzielonej
#define SEM_ID 2002   // ID semaforów
#define MSG_ORDER 3001     // Zlecenie od klienta
#define MSG_UPDATE 3002    // Aktualizacja stanu dla klienta

//ID poszcegolnych typow jednostek
#define LEKKA_PIECHOTA 0
#define CIEZKA_PIECHOTA 1
#define JAZDA 2
#define ROBOTNICY 3

// Typy opcji gracza
#define TRAIN 1
#define ATTACK 2
//ID gracza
#define PLAYER_A 0
#define PLAYER_B 1

//struktura typu jednostki
typedef struct{
    int unit_type_id;
    int price;
    double attack;
    double defense;
    int production_time;
}Unit;

// Kolejka zleceń trenowania jednostek
typedef struct{
    int slot_active;
    int unit_type;
    int remaining_units;
    int next_training_time;
}Training;

// Struktura stanu gracza
typedef struct {
    int resources;
    int unit_types[4];
    int player_points;
    Training queue[11];
} Player;

//struktura ataku
typedef struct{
    int attack_active;
    int attacker;
    int att_units[3];
    int end_time;
}Attack;

// struktura gry w pamięci współdzielonej
typedef struct {
    int game_active;
    Player Player_A;
    Player Player_B;
    Attack current_attack;
} Game;

// struktura komunikatu do kolejki
typedef struct {
    long mtype;         // Typ komunikatu
    int player_id;      // 0 lub 1
    int action;         // np. 1=buduj, 2=atak
    int unit_type;      // jakiego typu jednostka
    int count;          // ile jednostek treining
    int army[3];
    Player current_state; // Serwer odsyła tu stan gry
    char player_message[100];
} MessageBuff;

#endif