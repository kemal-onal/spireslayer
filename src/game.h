#ifndef GAME_H
#define GAME_H

#define MAX_LINE_LENGTH 1024

/* Represents A Card In The Deck
 * Base and upgraded versions of the same card name are tracked together 
 * to optimize memory and simplify sorting logic.
 */
typedef struct {
    char name[MAX_LINE_LENGTH];
    int base_count;
    int upgraded_count;
    int is_exhaust;
} Card;

typedef enum {TYPE_CARD, TYPE_RELIC, TYPE_POTION} ItemType;

/* Represents A Weakness Item
 * Holds the type and the name of the effective item.
 */
typedef struct {
    ItemType type;
    char name[MAX_LINE_LENGTH];
} Weakness;

/* Represents An Enemy And Its Codex
 * Weaknesses array is dynamically reallocated as the player learns more.
 */
typedef struct {
    char enemy_name[MAX_LINE_LENGTH];
    int defeated_count;
    Weakness *weaknesses;
    int weakness_count;
    int weakness_capacity;
} Enemy;

/* The Main State Of The Game
 * Combines scalar values, fixed arrays like potions, and dynamic arrays like deck and enemies.
 */
typedef struct {
    int gold;
    int current_hp;
    int max_hp;
    int floor;
    char current_room[16];

    char potions[3][MAX_LINE_LENGTH];
    int potion_count;

    char (*relics)[MAX_LINE_LENGTH];
    int relic_count;
    int relic_capacity;

    Card *deck;
    int deck_count;
    int deck_capacity;

    Enemy *enemies;
    int enemy_count;
    int enemy_capacity;

} GameState;

// Function Declarations

// Initialize State and Free Memory Functions
void init_game_state(GameState *state);
void free_game_state(GameState *state);

// State Mutating Command Functions
void gain_gold(GameState *state, int amount);
void gain_card(GameState *state, const char *card_name);
int gain_relic(GameState *state, const char *relic_name);
int gain_potion(GameState *state, const char *potion_name);
void gain_max_hp(GameState *state, int amount);
void buy_card(GameState *state, const char *card_name, int pos_int);
void buy_relic(GameState *state, const char *relic_name, int pos_int);
void buy_potion(GameState *state, const char *potion_name, int pos_int);
int decrease_card(GameState *state, const char *card_name);
int decrease_upgraded_card(GameState *state, const char *card_name);
int decrease_potion(GameState *state, const char *potion_name);
void remove_card(GameState *state, const char *card_name);
void remove_upgraded_card(GameState *state, const char *card_name);
void upgrade_card(GameState *state, const char *card_name);
void enter_room(GameState *state, const char *room_type);
void learn_card_effective(GameState *state, const char *card_name, const char *enemy_name);
void learn_relic_effective(GameState *state, const char *relic_name, const char *enemy_name);
void learn_potion_effective(GameState *state, const char *potion_name, const char *enemy_name);
void heal(GameState *state, int amount);
void take_damage(GameState *state, int amount);
void discard_potion(GameState *state, const char *potion_name);
void sell_card(GameState *state, const char *card_name, int pos_int);
void sell_upgraded_card(GameState *state, const char *card_name, int pos_int);
void sell_potion(GameState *state, const char *potion_name, int pos_int);
void mark_exhaust(GameState *state, const char *card_name);
void fight_enemy(GameState *state, const char *enemy_name);
void fight_enemy_bounty(GameState *state, const char *enemy_name, int pos_int);

// Read Only Query Functions
void gold_query(GameState *state);
void total_card_query(GameState *state, const char *card_name);
void total_upgraded_card_query(GameState *state, const char *card_name);
void floor_query(GameState *state);
void where_query(GameState *state);
void deck_query(GameState *state);
void deck_size_query(GameState *state);
void relics_query(GameState *state);
void potions_query(GameState *state);
void effective_query(GameState *state, const char *enemy_name);
void defeated_query(GameState *state, const char *enemy_name);
void health_query(GameState *state);
void exhausts_query(GameState *state);

#endif