#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include "game.h"

/* Converts Weakness Type To String
 * Returns the string representation of the ItemType enum.
 */
static const char *weakness_type_to_string(ItemType type){
    if(type == TYPE_CARD) return "card";
    if(type == TYPE_RELIC) return "relic";
    return "potion";
}

/* Compares Cards For Sorting
 * Uses strncmp to order cards alphabetically.
 */
static int compare_cards(const void *a, const void *b){
    const Card *card_a = (const Card *)a;
    const Card *card_b = (const Card *)b;
    return(strncmp(card_a->name, card_b->name, MAX_LINE_LENGTH));
}

/* Compares Weaknesses For Sorting
 * Combines type and name into a single string to compare them lexicographically.
 */
static int compare_weaknesses(const void *a, const void *b){
    const Weakness *weakness_a = (const Weakness *)a;
    const Weakness *weakness_b = (const Weakness *)b;

    char left[MAX_LINE_LENGTH + 16];
    char right[MAX_LINE_LENGTH + 16];

    snprintf(left, sizeof(left), "%s %s",
             weakness_type_to_string(weakness_a->type), weakness_a->name);
    snprintf(right, sizeof(right), "%s %s",
             weakness_type_to_string(weakness_b->type), weakness_b->name);

    return strcmp(left, right);
}

/* Compares Strings For Sorting
 * Basic wrapper for strncmp used in qsort.
 */
static int compare_strings(const void *a, const void *b){
    return strncmp((const char *)a, (const char *)b, MAX_LINE_LENGTH);
}

/* Initializes Game State
 * Sets default values and allocates initial memory for dynamic arrays.
 */
void init_game_state(GameState *state) {
    state->gold = 0;
    state->current_hp = 80;
    state->max_hp = 80;
    state->floor = 0;
    strcpy(state->current_room, "NONE");

    state->potion_count = 0;

    state->relic_count = 0;
    state->relic_capacity = 5;
    state->relics = calloc(state->relic_capacity, sizeof(char[MAX_LINE_LENGTH]));
    if(state->relics == NULL){
        printf("Memory allocation failed at relics.\n");
        exit(1);
    }

    state->deck_count = 0;
    state->deck_capacity = 10;
    state->deck = calloc(state->deck_capacity, sizeof(Card));
    if(state->deck == NULL){
        printf("Memory allocation failed at deck.\n");
        exit(1);
    }

    state->enemy_count = 0;
    state->enemy_capacity = 10;
    state->enemies = calloc(state->enemy_capacity, sizeof(Enemy));
    if(state->enemies == NULL){
        printf("Memory allocation failed at enemies.\n");
        exit(1);
    }
}

/* Frees Game State To Prevent Memory Leaks
 * Ensures nested dynamically allocated weakness arrays within each enemy 
 * are freed before the main enemies array.
 */
void free_game_state(GameState *state) {
    if (state->enemies != NULL) {
        for (int i = 0; i < state->enemy_count; i++) {
            if (state->enemies[i].weaknesses != NULL) {
                free(state->enemies[i].weaknesses);
                state->enemies[i].weaknesses = NULL;
            }
        }
        free(state->enemies);
        state->enemies = NULL;
    }
    if (state->deck != NULL) {
        free(state->deck);
        state->deck = NULL;
    }
    if (state->relics != NULL) {
        free(state->relics);
        state->relics = NULL;
    }
}

/* Gains Gold
 * Simply adds the amount to total gold.
 */
void gain_gold(GameState *state, int amount){
    state->gold += amount;
    printf("Gold obtained\n");
}

/* Adds A Base Card To The Deck
 * If the array is full, it dynamically reallocates double the capacity.
 */
void gain_card(GameState *state, const char *card_name){
    for(int i = 0; i < state->deck_count; i++){
        if(strncmp(state->deck[i].name, card_name, MAX_LINE_LENGTH) == 0){
            state->deck[i].base_count++;
            printf("Card added: %s\n", card_name);
            return;
        }
    }
    if(state->deck_count == state->deck_capacity){
        state->deck_capacity *= 2;
        Card *temp = realloc(state->deck, state->deck_capacity * sizeof(Card));
        if(temp == NULL){
            printf("Memory reallocation failed at gain_card.\n");
            exit(1);
        }
        state->deck = temp;
        temp = NULL;
    }
    strcpy(state->deck[state->deck_count].name, card_name);
    state->deck[state->deck_count].base_count = 1;
    state->deck[state->deck_count].is_exhaust = 0;
    state->deck[state->deck_count].upgraded_count = 0;
    state->deck_count++;
    printf("Card added: %s\n", card_name);
}

/* Gains A Relic
 * Checks if the relic is already owned. Reallocates the array if needed.
 */
int gain_relic(GameState *state, const char *relic_name){
    for(int i = 0; i < state->relic_count; i++){
        if(strncmp(state->relics[i], relic_name, MAX_LINE_LENGTH) == 0){
            printf("Already has relic: %s\n", relic_name);
            return 0;
        }
    }
    if(state->relic_count == state->relic_capacity){
        state->relic_capacity *= 2;
        char (*temp)[MAX_LINE_LENGTH] = realloc(state->relics, state-> relic_capacity * sizeof(char[MAX_LINE_LENGTH]));
        if(temp == NULL){
            printf("Memory reallocation failed at gain_relic.\n");
            exit(1);
        }
        state->relics = temp;
        temp = NULL;
    }
    strcpy(state->relics[state->relic_count], relic_name);
    state->relic_count++;
    printf("Relic obtained: %s\n", relic_name);
    return 1;
}

/* Gains A Potion
 * Checks if the potion belt is full before adding it.
 */
int gain_potion(GameState *state, const char *potion_name){
    if(state->potion_count == 3){
        printf("Potion belt is full\n");
        return 0;
    }
    strcpy(state->potions[state->potion_count], potion_name);
    printf("Potion obtained: %s\n", potion_name);
    state->potion_count++;
    return 1;
}

/* Gains Max HP
 * Increases the max hp limit.
 */
void gain_max_hp(GameState *state, int amount){
    state->max_hp += amount;
    printf("Max health increased to %d\n", state->max_hp);
}

/* Buys Card From Shop
 * Checks if there is enough gold before calling the gain function.
 */
void buy_card(GameState *state, const char *card_name, int pos_int){
    if(state->gold < pos_int){
        printf("Not enough gold\n");
        return;
    }
    gain_card(state, card_name);
    state->gold -= pos_int;
}

/* Buys Relic From Shop
 * Checks if there is enough gold before calling the gain function.
 */
void buy_relic(GameState *state, const char *relic_name, int pos_int){
    if(state->gold < pos_int){
        printf("Not enough gold\n");
        return;
    }
    if(gain_relic(state, relic_name)){
        state->gold -= pos_int;
    }
}

/* Buys Potion From Shop
 * Checks if there is enough gold before calling the gain function.
 */
void buy_potion(GameState *state, const char *potion_name, int pos_int){
    if(state->gold < pos_int){
        printf("Not enough gold\n");
        return;
    }
    if(gain_potion(state, potion_name)){
        state->gold -= pos_int;
    }
}

/* Decreases A Base Card
 * Iterates over the deck and decreases the base count if found.
 */
int decrease_card(GameState *state, const char *card_name){
    for(int i = 0; i < state->deck_count; i++){
        if(strncmp(state->deck[i].name, card_name, MAX_LINE_LENGTH) == 0){
            if(state->deck[i].base_count > 0){
                state->deck[i].base_count--;
                return 1;
            }
        }
    }
    return 0;
}

/* Decreases An Upgraded Card
 * Iterates over the deck and decreases the upgraded count if found.
 */
int decrease_upgraded_card(GameState *state, const char *card_name){
    for(int i = 0; i < state->deck_count; i++){
        if(strncmp(state->deck[i].name, card_name, MAX_LINE_LENGTH) == 0){
            if(state->deck[i].upgraded_count > 0){
                state->deck[i].upgraded_count--;
                return 1;
            }
        }
    }
    return 0;
}

/* Removes Card From Deck
 * Calls decrease function and prints the result.
 */
void remove_card(GameState *state, const char *card_name){
    if(decrease_card(state, card_name)){
        printf("Card removed: %s\n", card_name);
        return;
    }
    printf("Card not found: %s\n", card_name);
}

/* Removes Upgraded Card From Deck
 * Calls decrease function and prints the result.
 */
void remove_upgraded_card(GameState *state, const char *card_name){
    if(decrease_upgraded_card(state, card_name)){
        printf("Upgraded card removed: %s\n", card_name);
        return;
    }
    printf("Upgraded card not found: %s\n", card_name);
}

/* Upgrades A Card
 * Decreases the base count and increases the upgraded count.
 */
void upgrade_card(GameState *state, const char *card_name){
    for(int i = 0; i < state->deck_count; i++){
        if(strncmp(state->deck[i].name, card_name, MAX_LINE_LENGTH) == 0){
            if(state->deck[i].base_count > 0){
                state->deck[i].base_count--;
                state->deck[i].upgraded_count++;
                printf("Card upgraded: %s\n", card_name);
                return;
            }
        }
    }
    printf("Card not found: %s\n", card_name);
}

/* Enters A Room
 * Increments floor and updates the current room string.
 */
void enter_room(GameState *state, const char *room_type){
    state->floor++;
    strcpy(state->current_room, room_type);
    printf("Entered %s room\n", room_type);
}

/* Learns An Effective Card Against Enemy
 * Finds the enemy or creates a new one with realloc. 
 * Then adds the weakness to the enemy's dynamically allocated weaknesses array.
 */
void learn_card_effective(GameState *state, const char *card_name, const char *enemy_name){
    for(int i = 0; i < state->enemy_count; i++){
        if(strncmp(state->enemies[i].enemy_name, enemy_name, MAX_LINE_LENGTH) == 0){
            for(int j = 0; j < state->enemies[i].weakness_count; j++){
                if(state->enemies[i].weaknesses[j].type == TYPE_CARD && strncmp(state->enemies[i].weaknesses[j].name, card_name, MAX_LINE_LENGTH) == 0){
                    printf("Effectiveness already noted\n");
                    return;
                }
            }
            if(state->enemies[i].weakness_capacity == 0){
                state->enemies[i].weakness_capacity = 5;
                Weakness *temp = calloc( state->enemies[i].weakness_capacity, sizeof(Weakness));
                if(temp == NULL){
                    printf("Memory allocation failed at learn_card_effective.\n");
                    exit(1);
                }
                state->enemies[i].weaknesses = temp;
                temp = NULL;
                strcpy(state->enemies[i].weaknesses[0].name, card_name);
                state->enemies[i].weaknesses[0].type = TYPE_CARD;
                state->enemies[i].weakness_count = 1;
                printf("Codex entry updated: %s\n", enemy_name);
                return;
            }
            if(state->enemies[i].weakness_capacity == state->enemies[i].weakness_count){
                state->enemies[i].weakness_capacity *= 2;
                Weakness *temp = realloc(state->enemies[i].weaknesses, state->enemies[i].weakness_capacity * sizeof(Weakness));
                if(temp == NULL){
                    printf("Memory allocation failed at learn_card_effective.\n");
                    exit(1);
                }
                state->enemies[i].weaknesses = temp;
                temp = NULL;
            }
            strcpy(state->enemies[i].weaknesses[state->enemies[i].weakness_count].name, card_name);
            state->enemies[i].weaknesses[state->enemies[i].weakness_count].type = TYPE_CARD;
            state->enemies[i].weakness_count++;
            printf("Codex entry updated: %s\n", enemy_name);
            return;
        }
    }
    if(state->enemy_capacity == state->enemy_count){
        state->enemy_capacity *= 2;
        Enemy *temp = realloc(state->enemies, state->enemy_capacity * sizeof(Enemy));
        if(temp == NULL){
            printf("Memory allocation failed at learn_card_effective.\n");
            exit(1);
        }
        state->enemies = temp;
        temp = NULL;
    }
    strcpy(state->enemies[state->enemy_count].enemy_name, enemy_name);
    state->enemies[state->enemy_count].defeated_count = 0;
    state->enemies[state->enemy_count].weakness_capacity = 5;
    state->enemies[state->enemy_count].weakness_count = 1;
    Weakness *temp = calloc( state->enemies[state->enemy_count].weakness_capacity, sizeof(Weakness));
    if(temp == NULL){
        printf("Memory allocation failed at learn_card_effective.\n");
        exit(1);
    }
    state->enemies[state->enemy_count].weaknesses = temp;
    temp = NULL;
    strcpy(state->enemies[state->enemy_count].weaknesses[0].name, card_name);
    state->enemies[state->enemy_count].weaknesses[0].type = TYPE_CARD;
    state->enemy_count++;
    printf("Codex entry created: %s\n", enemy_name);
}

/* Learns An Effective Relic Against Enemy
 * Finds the enemy or creates a new one with realloc.
 * Then adds the weakness to the enemy's dynamically allocated weaknesses array.
 */
void learn_relic_effective(GameState *state, const char *relic_name, const char *enemy_name){
    for(int i = 0; i < state->enemy_count; i++){
        if(strncmp(state->enemies[i].enemy_name, enemy_name, MAX_LINE_LENGTH) == 0){
            for(int j = 0; j < state->enemies[i].weakness_count; j++){
                if(state->enemies[i].weaknesses[j].type == TYPE_RELIC && strncmp(state->enemies[i].weaknesses[j].name, relic_name, MAX_LINE_LENGTH) == 0){
                    printf("Effectiveness already noted\n");
                    return;
                }
            }
            if(state->enemies[i].weakness_capacity == 0){
                state->enemies[i].weakness_capacity = 5;
                Weakness *temp = calloc( state->enemies[i].weakness_capacity, sizeof(Weakness));
                if(temp == NULL){
                    printf("Memory allocation failed at learn_relic_effective.\n");
                    exit(1);
                }
                state->enemies[i].weaknesses = temp;
                temp = NULL;
                strcpy(state->enemies[i].weaknesses[0].name, relic_name);
                state->enemies[i].weaknesses[0].type = TYPE_RELIC;
                state->enemies[i].weakness_count = 1;
                printf("Codex entry updated: %s\n", enemy_name);
                return;
            }
            if(state->enemies[i].weakness_capacity == state->enemies[i].weakness_count){
                state->enemies[i].weakness_capacity *= 2;
                Weakness *temp = realloc(state->enemies[i].weaknesses, state->enemies[i].weakness_capacity * sizeof(Weakness));
                if(temp == NULL){
                    printf("Memory allocation failed at learn_relic_effective.\n");
                    exit(1);
                }
                state->enemies[i].weaknesses = temp;
                temp = NULL;
            }
            strcpy(state->enemies[i].weaknesses[state->enemies[i].weakness_count].name, relic_name);
            state->enemies[i].weaknesses[state->enemies[i].weakness_count].type = TYPE_RELIC;
            state->enemies[i].weakness_count++;
            printf("Codex entry updated: %s\n", enemy_name);
            return;
        }
    }
    if(state->enemy_capacity == state->enemy_count){
        state->enemy_capacity *= 2;
        Enemy *temp = realloc(state->enemies, state->enemy_capacity * sizeof(Enemy));
        if(temp == NULL){
            printf("Memory allocation failed at learn_relic_effective.\n");
            exit(1);
        }
        state->enemies = temp;
        temp = NULL;
    }
    strcpy(state->enemies[state->enemy_count].enemy_name, enemy_name);
    state->enemies[state->enemy_count].defeated_count = 0;
    state->enemies[state->enemy_count].weakness_capacity = 5;
    state->enemies[state->enemy_count].weakness_count = 1;
    Weakness *temp = calloc( state->enemies[state->enemy_count].weakness_capacity, sizeof(Weakness));
    if(temp == NULL){
        printf("Memory allocation failed at learn_relic_effective.\n");
        exit(1);
    }
    state->enemies[state->enemy_count].weaknesses = temp;
    temp = NULL;
    strcpy(state->enemies[state->enemy_count].weaknesses[0].name, relic_name);
    state->enemies[state->enemy_count].weaknesses[0].type = TYPE_RELIC;
    state->enemy_count++;
    printf("Codex entry created: %s\n", enemy_name);
}

/* Learns An Effective Potion Against Enemy
 * Finds the enemy or creates a new one with realloc.
 * Then adds the weakness to the enemy's dynamically allocated weaknesses array.
 */
void learn_potion_effective(GameState *state, const char *potion_name, const char *enemy_name){
    for(int i = 0; i < state->enemy_count; i++){
        if(strncmp(state->enemies[i].enemy_name, enemy_name, MAX_LINE_LENGTH) == 0){
            for(int j = 0; j < state->enemies[i].weakness_count; j++){
                if(state->enemies[i].weaknesses[j].type == TYPE_POTION && strncmp(state->enemies[i].weaknesses[j].name, potion_name, MAX_LINE_LENGTH) == 0){
                    printf("Effectiveness already noted\n");
                    return;
                }
            }
            if(state->enemies[i].weakness_capacity == 0){
                state->enemies[i].weakness_capacity = 5;
                Weakness *temp = calloc( state->enemies[i].weakness_capacity, sizeof(Weakness));
                if(temp == NULL){
                    printf("Memory allocation failed at learn_potion_effective.\n");
                    exit(1);
                }
                state->enemies[i].weaknesses = temp;
                temp = NULL;
                strcpy(state->enemies[i].weaknesses[0].name, potion_name);
                state->enemies[i].weaknesses[0].type = TYPE_POTION;
                state->enemies[i].weakness_count = 1;
                printf("Codex entry updated: %s\n", enemy_name);
                return;
            }
            if(state->enemies[i].weakness_capacity == state->enemies[i].weakness_count){
                state->enemies[i].weakness_capacity *= 2;
                Weakness *temp = realloc(state->enemies[i].weaknesses, state->enemies[i].weakness_capacity * sizeof(Weakness));
                if(temp == NULL){
                    printf("Memory allocation failed at learn_potion_effective.\n");
                    exit(1);
                }
                state->enemies[i].weaknesses = temp;
                temp = NULL;
            }
            strcpy(state->enemies[i].weaknesses[state->enemies[i].weakness_count].name, potion_name);
            state->enemies[i].weaknesses[state->enemies[i].weakness_count].type = TYPE_POTION;
            state->enemies[i].weakness_count++;
            printf("Codex entry updated: %s\n", enemy_name);
            return;
        }
    }
    if(state->enemy_capacity == state->enemy_count){
        state->enemy_capacity *= 2;
        Enemy *temp = realloc(state->enemies, state->enemy_capacity * sizeof(Enemy));
        if(temp == NULL){
            printf("Memory allocation failed at learn_potion_effective.\n");
            exit(1);
        }
        state->enemies = temp;
        temp = NULL;
    }
    strcpy(state->enemies[state->enemy_count].enemy_name, enemy_name);
    state->enemies[state->enemy_count].defeated_count = 0;
    state->enemies[state->enemy_count].weakness_capacity = 5;
    state->enemies[state->enemy_count].weakness_count = 1;
    Weakness *temp = calloc( state->enemies[state->enemy_count].weakness_capacity, sizeof(Weakness));
    if(temp == NULL){
        printf("Memory allocation failed at learn_potion_effective.\n");
        exit(1);
    }
    state->enemies[state->enemy_count].weaknesses = temp;
    temp = NULL;
    strcpy(state->enemies[state->enemy_count].weaknesses[0].name, potion_name);
    state->enemies[state->enemy_count].weaknesses[0].type = TYPE_POTION;
    state->enemy_count++;
    printf("Codex entry created: %s\n", enemy_name);
}

/* Heals Ironclad
 * Increases current HP but clamps it to Max HP.
 */
void heal(GameState *state, int amount){
    state->current_hp += amount;
    if(state->current_hp > state->max_hp){
        state->current_hp = state->max_hp;
    }
    printf("Ironclad heals to %d\n", state->current_hp);
}

/* Takes Damage
 * Decreases current HP but clamps it to 0.
 */
void take_damage(GameState *state, int amount){
    state->current_hp -= amount;
    if(state->current_hp < 0){
        state->current_hp = 0;
    }
    printf("Ironclad health drops to %d\n", state->current_hp);
}

/* Removes A Potion From The Belt
 * Implements an array compaction step to prevent empty holes in the middle 
 * of the array by shifting all subsequent elements left.
 */
int decrease_potion(GameState *state, const char *potion_name){
    for(int i = 0; i < state->potion_count; i++){
        if(strncmp(state->potions[i], potion_name, MAX_LINE_LENGTH) == 0){
            for(int j = i; j < state->potion_count - 1; j++){
                strcpy(state->potions[j], state->potions[j + 1]);
            }
            state->potion_count--;
            return 1;
        }
    }
    return 0;
}

/* Discards A Potion
 * Calls decrease potion and prints output.
 */
void discard_potion(GameState *state, const char *potion_name){
    if(decrease_potion(state, potion_name)){
        printf("Potion discarded: %s\n", potion_name);
        return;
    }
    printf("Potion not found: %s\n", potion_name);
}

/* Sells Card For Gold
 * Decreases the item count and adds gold.
 */
void sell_card(GameState *state, const char *card_name, int pos_int){
    if(decrease_card(state, card_name)){
        state->gold += pos_int;
        printf("Card sold: %s\n", card_name);
        return;
    }
    printf("Card not found: %s\n", card_name);
}

/* Sells Upgraded Card For Gold
 * Decreases the item count and adds gold.
 */
void sell_upgraded_card(GameState *state, const char *card_name, int pos_int){
    if(decrease_upgraded_card(state, card_name)){
        state->gold += pos_int;
        printf("Upgraded card sold: %s\n", card_name);
        return;
    }
    printf("Upgraded card not found: %s\n", card_name);
}

/* Sells Potion For Gold
 * Decreases the item count and adds gold.
 */
void sell_potion(GameState *state, const char *potion_name, int pos_int){
    if(decrease_potion(state, potion_name)){
        state->gold += pos_int;
        printf("Potion sold: %s\n", potion_name);
        return;
    }
    printf("Potion not found: %s\n", potion_name);
}

/* Marks Card As Exhaust
 * Sets the is_exhaust flag to 1. If the card is not in the deck, 
 * it creates a 0-count entry for it.
 */
void mark_exhaust(GameState *state, const char *card_name){
    for(int i = 0; i < state->deck_count; i++){
        if(strncmp(state->deck[i].name, card_name, MAX_LINE_LENGTH) == 0){
            if(state->deck[i].is_exhaust){
                printf("Card already exhausts: %s\n", card_name);
                return;
            }
            state->deck[i].is_exhaust = 1;
            printf("Card marked as exhaust: %s\n", card_name);
            return;
        }
    }
    if(state->deck_count == state->deck_capacity){
        state->deck_capacity *= 2;
        Card *temp = realloc(state->deck, state->deck_capacity * sizeof(Card));
        if(temp == NULL){
            printf("Memory reallocation failed at gain_card.\n");
            exit(1);
        }
        state->deck = temp;
        temp = NULL;
    }
    strcpy(state->deck[state->deck_count].name, card_name);
    state->deck[state->deck_count].base_count = 0;
    state->deck[state->deck_count].is_exhaust = 1;
    state->deck[state->deck_count].upgraded_count = 0;
    state->deck_count++;
    printf("Card marked as exhaust: %s\n", card_name);
}

/* Executes Gold Query
 * Prints the requested state variable.
 */
void gold_query(GameState *state){
    printf("%d\n", state->gold);
}

/* Executes Total Card Query
 * Prints the requested state variable.
 */
void total_card_query(GameState *state, const char *card_name){
    for(int i = 0; i < state->deck_count; i++){
        if(strncmp(state->deck[i].name, card_name, MAX_LINE_LENGTH) == 0){
            printf("%d\n", state->deck[i].base_count + state->deck[i].upgraded_count);
            return;
        }
    }
    printf("0\n");
}

/* Executes Total Upgraded Card Query
 * Prints the requested state variable.
 */
void total_upgraded_card_query(GameState *state, const char *card_name){
    for(int i = 0; i < state->deck_count; i++){
        if(strncmp(state->deck[i].name, card_name, MAX_LINE_LENGTH) == 0){
            printf("%d\n", state->deck[i].upgraded_count);
            return;
        }
    }
    printf("0\n");
}

/* Executes Floor Query
 * Prints the requested state variable.
 */
void floor_query(GameState *state){
    printf("%d\n", state->floor);
}

/* Executes Where Query
 * Prints the requested state variable.
 */
void where_query(GameState *state){
    printf("%s\n", state->current_room);
}

/* Executes Deck Query
 * Sorting functions use qsort before printing.
 */
void deck_query(GameState *state){
    int is_first_time = 1;
    qsort(state->deck, state->deck_count, sizeof(Card), compare_cards);
    for(int i = 0; i < state->deck_count; i++){
        if(state->deck[i].base_count > 0){
            if(state->deck[i].is_exhaust){
                if(is_first_time){
                    printf("%d %s*", state->deck[i].base_count, state->deck[i].name);
                    is_first_time = 0;
                }
                else{
                    printf(", %d %s*", state->deck[i].base_count, state->deck[i].name);
                }
            }
            else{
                if(is_first_time){
                    printf("%d %s", state->deck[i].base_count, state->deck[i].name);
                    is_first_time = 0;
                }
                else{
                    printf(", %d %s", state->deck[i].base_count, state->deck[i].name);
                }
            }
        }
        if(state->deck[i].upgraded_count > 0){
            if(state->deck[i].is_exhaust){
                if(is_first_time){
                    printf("%d %s+*", state->deck[i].upgraded_count, state->deck[i].name);
                    is_first_time = 0;
                }
                else{
                    printf(", %d %s+*", state->deck[i].upgraded_count, state->deck[i].name);
                }
            }
            else{
                if(is_first_time){
                    printf("%d %s+", state->deck[i].upgraded_count, state->deck[i].name);
                    is_first_time = 0;
                }
                else{
                    printf(", %d %s+", state->deck[i].upgraded_count, state->deck[i].name);
                }
            }
        }
    }
    if(is_first_time){
        printf("None");
    }
    printf("\n");
}

/* Executes Deck Size Query
 * Prints the requested state variable.
 */
void deck_size_query(GameState *state){
    int counter = 0;
    for(int i = 0; i < state->deck_count; i++){
        counter += state->deck[i].base_count + state->deck[i].upgraded_count;
    }
    printf("%d\n", counter);
}

/* Executes Relics Query
 * Sorting functions use qsort before printing.
 */
void relics_query(GameState *state){
    if(state->relic_count == 0){
        printf("None\n");
        return;
    }
    qsort(state->relics, state->relic_count, sizeof(char[MAX_LINE_LENGTH]), compare_strings);
    printf("%s", state->relics[0]);
    for(int i = 1; i < state->relic_count; i++){
        printf(", %s", state->relics[i]);
    }
    printf("\n");
}

/* Executes Potions Query
 * Sorting functions use qsort before printing.
 */
void potions_query(GameState *state){
    switch(state->potion_count){
        case 0:
            printf("None\n");
            break;
        case 1:
            printf("1 %s\n", state->potions[0]);
            break;
        case 2:
            qsort(state->potions, 2, sizeof(char[MAX_LINE_LENGTH]), compare_strings);
            if(strncmp(state->potions[0], state->potions[1], MAX_LINE_LENGTH) == 0){
                printf("2 %s\n", state->potions[0]);
            }
            else{
                printf("1 %s, 1 %s\n", state->potions[0], state->potions[1]);
            }
            break;
        case 3:
            qsort(state->potions, 3, sizeof(char[MAX_LINE_LENGTH]), compare_strings);
            if(strncmp(state->potions[0], state->potions[1], MAX_LINE_LENGTH) == 0){
                if(strncmp(state->potions[1], state->potions[2], MAX_LINE_LENGTH) == 0){
                    printf("3 %s\n", state->potions[0]);
                }
                else{
                    printf("2 %s, 1 %s\n", state->potions[0], state->potions[2]);
                }
            }
            else{
                if(strncmp(state->potions[1], state->potions[2], MAX_LINE_LENGTH) == 0){
                    printf("1 %s, 2 %s\n", state->potions[0], state->potions[2]);
                }
                else{
                    printf("1 %s, 1 %s, 1 %s\n", state->potions[0], state->potions[1], state->potions[2]);
                }
            }
            break;
    }
}

/* Executes Effective Query
 * Sorting functions use qsort before printing.
 */
void effective_query(GameState *state, const char *enemy_name){
    for(int i = 0; i < state->enemy_count; i++){
        if(strncmp(state->enemies[i].enemy_name, enemy_name, MAX_LINE_LENGTH) == 0){
            qsort(state->enemies[i].weaknesses, state->enemies[i].weakness_count, sizeof(Weakness), compare_weaknesses);
            printf("%s %s", weakness_type_to_string(state->enemies[i].weaknesses[0].type), state->enemies[i].weaknesses[0].name);
            for(int j = 1; j < state->enemies[i].weakness_count; j++){
                printf(", %s %s", weakness_type_to_string(state->enemies[i].weaknesses[j].type), state->enemies[i].weaknesses[j].name);
            }
            printf("\n");
            return;
        }
    }
    printf("No codex data for %s\n", enemy_name);
}

/* Executes Defeated Query
 * Prints the requested state variable.
 */
void defeated_query(GameState *state, const char *enemy_name){
    for(int i = 0; i < state->enemy_count; i++){
        if(strncmp(state->enemies[i].enemy_name, enemy_name, MAX_LINE_LENGTH) == 0){
            printf("%d\n", state->enemies[i].defeated_count);
            return;
        }
    }
    printf("0\n");
}

/* Executes Health Query
 * Prints the requested state variable.
 */
void health_query(GameState *state){
    printf("%d/%d\n", state->current_hp, state->max_hp);
}

/* Executes Exhausts Query
 * Sorting functions use qsort before printing.
 */
void exhausts_query(GameState *state){
    qsort(state->deck, state->deck_count, sizeof(Card), compare_cards);
    int is_first_time = 1;
    for(int i = 0; i < state->deck_count; i++){
        if(state->deck[i].is_exhaust){
            if(is_first_time){
                printf("%s", state->deck[i].name);
                is_first_time = 0;
            }
            else{
                printf(", %s", state->deck[i].name);
            }
        }
    }
    if(is_first_time){
        printf("None");
    }
    printf("\n");
}

/* Evaluates The Combat Logic
 * Iterates over codex weaknesses and cross-references the player's inventory 
 * to determine victory. Consumes exactly one exhaust card or potion upon success.
 */
void fight_enemy(GameState *state, const char *enemy_name){

    for(int i = 0; i < state->enemy_count; i++){
        if(strncmp(state->enemies[i].enemy_name, enemy_name, MAX_LINE_LENGTH) == 0){
            int can_defeat = 0;

            for(int j = 0; j < state->enemies[i].weakness_count; j++){
                if(state->enemies[i].weaknesses[j].type == TYPE_CARD){
                    for(int k = 0; k < state->deck_count; k++){
                        if(strncmp(state->deck[k].name, state->enemies[i].weaknesses[j].name, MAX_LINE_LENGTH) == 0){
                            if(state->deck[k].base_count > 0 || state->deck[k].upgraded_count > 0){
                                can_defeat = 1;
                                break;
                            }
                        }
                    }
                }
                else if(state->enemies[i].weaknesses[j].type == TYPE_RELIC){
                    for(int k = 0; k < state->relic_count; k++){
                        if(strncmp(state->relics[k], state->enemies[i].weaknesses[j].name, MAX_LINE_LENGTH) == 0){
                            can_defeat = 1;
                            break;
                        }
                    }
                }
                else if(state->enemies[i].weaknesses[j].type == TYPE_POTION){
                    for(int k = 0; k < state->potion_count; k++){
                        if(strncmp(state->potions[k], state->enemies[i].weaknesses[j].name, MAX_LINE_LENGTH) == 0){
                            can_defeat = 1;
                            break;
                        }
                    }
                }

                if(can_defeat){
                    break;
                }
            }

            if(!can_defeat){
                state->current_hp -= 15;
                if(state->current_hp < 0){
                    state->current_hp = 0;
                }
                printf("Ironclad is outmatched and flees with %d hp remaining\n", state->current_hp);
                return;
            }

            for(int j = 0; j < state->enemies[i].weakness_count; j++){
                if(state->enemies[i].weaknesses[j].type == TYPE_POTION){
                    decrease_potion(state, state->enemies[i].weaknesses[j].name);
                }
                else if(state->enemies[i].weaknesses[j].type == TYPE_CARD){
                    for(int k = 0; k < state->deck_count; k++){
                        if(strncmp(state->deck[k].name, state->enemies[i].weaknesses[j].name, MAX_LINE_LENGTH) == 0){
                            if(state->deck[k].is_exhaust){
                                if(state->deck[k].base_count > 0){
                                    state->deck[k].base_count--;
                                }
                                else if(state->deck[k].upgraded_count > 0){
                                    state->deck[k].upgraded_count--;
                                }
                            }
                            break;
                        }
                    }
                }
            }

            state->enemies[i].defeated_count++;
            printf("Ironclad defeats %s\n", enemy_name);
            return;
        }
    }

    state->current_hp -= 15;
    if(state->current_hp < 0){
        state->current_hp = 0;
    }
    printf("Ironclad is outmatched and flees with %d hp remaining\n", state->current_hp);
}

/* Evaluates The Combat Logic With Bounty
 * Follows the same logic with figtht_enemy but adds gold upon victory.
 */
void fight_enemy_bounty(GameState *state, const char *enemy_name, int pos_int){

    for(int i = 0; i < state->enemy_count; i++){
        if(strncmp(state->enemies[i].enemy_name, enemy_name, MAX_LINE_LENGTH) == 0){
            int can_defeat = 0;

            for(int j = 0; j < state->enemies[i].weakness_count; j++){
                if(state->enemies[i].weaknesses[j].type == TYPE_CARD){
                    for(int k = 0; k < state->deck_count; k++){
                        if(strncmp(state->deck[k].name, state->enemies[i].weaknesses[j].name, MAX_LINE_LENGTH) == 0){
                            if(state->deck[k].base_count > 0 || state->deck[k].upgraded_count > 0){
                                can_defeat = 1;
                                break;
                            }
                        }
                    }
                }
                else if(state->enemies[i].weaknesses[j].type == TYPE_RELIC){
                    for(int k = 0; k < state->relic_count; k++){
                        if(strncmp(state->relics[k], state->enemies[i].weaknesses[j].name, MAX_LINE_LENGTH) == 0){
                            can_defeat = 1;
                            break;
                        }
                    }
                }
                else if(state->enemies[i].weaknesses[j].type == TYPE_POTION){
                    for(int k = 0; k < state->potion_count; k++){
                        if(strncmp(state->potions[k], state->enemies[i].weaknesses[j].name, MAX_LINE_LENGTH) == 0){
                            can_defeat = 1;
                            break;
                        }
                    }
                }

                if(can_defeat){
                    break;
                }
            }

            if(!can_defeat){
                state->current_hp -= 15;
                if(state->current_hp < 0){
                    state->current_hp = 0;
                }
                printf("Ironclad is outmatched and flees with %d hp remaining\n", state->current_hp);
                return;
            }

            for(int j = 0; j < state->enemies[i].weakness_count; j++){
                if(state->enemies[i].weaknesses[j].type == TYPE_POTION){
                    decrease_potion(state, state->enemies[i].weaknesses[j].name);
                }
                else if(state->enemies[i].weaknesses[j].type == TYPE_CARD){
                    for(int k = 0; k < state->deck_count; k++){
                        if(strncmp(state->deck[k].name, state->enemies[i].weaknesses[j].name, MAX_LINE_LENGTH) == 0){
                            if(state->deck[k].is_exhaust){
                                if(state->deck[k].base_count > 0){
                                    state->deck[k].base_count--;
                                }
                                else if(state->deck[k].upgraded_count > 0){
                                    state->deck[k].upgraded_count--;
                                }
                            }
                            break;
                        }
                    }
                }
            }

            state->enemies[i].defeated_count++;
            state->gold += pos_int;
            printf("Ironclad defeats %s and gains %d gold\n", enemy_name, pos_int);
            return;
        }
    }

    state->current_hp -= 15;
    if(state->current_hp < 0){
        state->current_hp = 0;
    }
    printf("Ironclad is outmatched and flees with %d hp remaining\n", state->current_hp);
}