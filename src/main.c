#include <stdio.h>
#include <string.h>
#include "game.h"
#include "parser.h"

/* The Main Routing Hub For The Interpreter
 * Matches the entire input line strictly against the grammar.
 * If cursor does not point to null at the end, it rejects the input due to trailing extra tokens.
 */
static int execute_line(GameState *state, const char *line) {

    Parser p;
    init_parser(&p, (char *) line);

    if(parse_keyword(&p, "Ironclad")){
        if(parse_keyword(&p, "gains")){
            int pos_int = 0;
            if(parse_pos_int(&p, &pos_int)){
                if(parse_keyword(&p, "gold")){
                    skip_spaces(&p);
                    if(*(p.cursor) == '\0'){
                        gain_gold(state, pos_int);
                        return 1;
                    }
                }
                else if(parse_keyword(&p, "max")){
                    if(parse_keyword(&p, "hp")){
                        skip_spaces(&p);
                        if(*(p.cursor) == '\0'){
                            gain_max_hp(state, pos_int);
                            return 1;
                        }
                    }
                }
            }
            else if(parse_keyword(&p, "card")){
                char card_name[MAX_LINE_LENGTH];
                if(parse_name(&p, card_name)){
                    skip_spaces(&p);
                    if(*(p.cursor) == '\0'){
                        gain_card(state, card_name);
                        return 1;
                    }
                }
            }
            else if(parse_keyword(&p, "relic")){
                char relic_name[MAX_LINE_LENGTH];
                if(parse_name(&p, relic_name)){
                    skip_spaces(&p);
                    if(*(p.cursor) == '\0'){
                        gain_relic(state, relic_name);
                        return 1;
                    }
                }
            }
            else if(parse_keyword(&p, "potion")){
                char potion_name[MAX_LINE_LENGTH];
                if(parse_name(&p, potion_name)){
                    skip_spaces(&p);
                    if(*(p.cursor) == '\0'){
                        gain_potion(state, potion_name);
                        return 1;
                    }
                }
            }
        }
        else if(parse_keyword(&p, "buys")){
            if(parse_keyword(&p, "card")){
                char card_name[MAX_LINE_LENGTH];
                if(parse_name(&p, card_name)){
                    if(parse_keyword(&p, "for")){
                        int pos_int = 0;
                        if(parse_pos_int(&p, &pos_int)){
                            if(parse_keyword(&p, "gold")){
                                skip_spaces(&p);
                                if(*(p.cursor) == '\0'){
                                    buy_card(state, card_name, pos_int);
                                    return 1;
                                }
                            }
                        }
                    }
                }
            }
            else if(parse_keyword(&p, "relic")){
                char relic_name[MAX_LINE_LENGTH];
                if(parse_name(&p, relic_name)){
                    if(parse_keyword(&p, "for")){
                        int pos_int = 0;
                        if(parse_pos_int(&p, &pos_int)){
                            if(parse_keyword(&p, "gold")){
                                skip_spaces(&p);
                                if(*(p.cursor) == '\0'){
                                    buy_relic(state, relic_name, pos_int);
                                    return 1;
                                }
                            }
                        }
                    }
                }
            }
            else if(parse_keyword(&p, "potion")){
                char potion_name[MAX_LINE_LENGTH];
                if(parse_name(&p, potion_name)){
                    if(parse_keyword(&p, "for")){
                        int pos_int = 0;
                        if(parse_pos_int(&p, &pos_int)){
                            if(parse_keyword(&p, "gold")){
                                skip_spaces(&p);
                                if(*(p.cursor) == '\0'){
                                    buy_potion(state, potion_name, pos_int);
                                    return 1;
                                }
                            }
                        }
                    }
                }
            }
        }
        else if(parse_keyword(&p, "removes")){
            if(parse_keyword(&p, "card")){
                char card_name[MAX_LINE_LENGTH];
                if(parse_name(&p, card_name)){
                    skip_spaces(&p);
                    if(*(p.cursor) == '\0'){
                        remove_card(state, card_name);
                        return 1;
                    }
                }
            }
            else if(parse_keyword(&p, "upgraded")){
                if(parse_keyword(&p, "card")){
                    char card_name[MAX_LINE_LENGTH];
                    if(parse_name(&p, card_name)){
                        skip_spaces(&p);
                        if(*(p.cursor) == '\0'){
                            remove_upgraded_card(state, card_name);
                            return 1;
                        }
                    }
                }
            }
        }
        else if(parse_keyword(&p, "upgrades")){
            if(parse_keyword(&p, "card")){
                char card_name[MAX_LINE_LENGTH];
                if(parse_name(&p, card_name)){
                    skip_spaces(&p);
                    if(*(p.cursor) == '\0'){
                        upgrade_card(state, card_name);
                        return 1;
                    }
                }
            }
        }
        else if(parse_keyword(&p, "enters")){
            char room_type[MAX_LINE_LENGTH];
            if(parse_room(&p, room_type)){
                if(parse_keyword(&p, "room")){
                    skip_spaces(&p);
                    if(*(p.cursor) == '\0'){
                        enter_room(state, room_type);
                        return 1;
                    }
                }
            }
        }
        else if(parse_keyword(&p, "learns")){
            if(parse_keyword(&p, "card")){
                char card_name[MAX_LINE_LENGTH];
                if(parse_name(&p, card_name)){
                    if(parse_keyword(&p, "is")){
                        if(parse_keyword(&p, "effective")){
                            if(parse_keyword(&p, "against")){
                                char enemy_name[MAX_LINE_LENGTH];
                                if(parse_name(&p, enemy_name)){
                                    skip_spaces(&p);
                                    if(*(p.cursor) == '\0'){
                                        learn_card_effective(state, card_name, enemy_name);
                                        return 1;
                                    }
                                }
                            }
                        }
                    }
                }
            }
            else if(parse_keyword(&p, "relic")){
                char relic_name[MAX_LINE_LENGTH];
                if(parse_name(&p, relic_name)){
                    if(parse_keyword(&p, "is")){
                        if(parse_keyword(&p, "effective")){
                            if(parse_keyword(&p, "against")){
                                char enemy_name[MAX_LINE_LENGTH];
                                if(parse_name(&p, enemy_name)){
                                    skip_spaces(&p);
                                    if(*(p.cursor) == '\0'){
                                        learn_relic_effective(state, relic_name, enemy_name);
                                        return 1;
                                    }
                                }
                            }
                        }
                    }
                }
            }
            else if(parse_keyword(&p, "potion")){
                char potion_name[MAX_LINE_LENGTH];
                if(parse_name(&p, potion_name)){
                    if(parse_keyword(&p, "is")){
                        if(parse_keyword(&p, "effective")){
                            if(parse_keyword(&p, "against")){
                                char enemy_name[MAX_LINE_LENGTH];
                                if(parse_name(&p, enemy_name)){
                                    skip_spaces(&p);
                                    if(*(p.cursor) == '\0'){
                                        learn_potion_effective(state, potion_name, enemy_name);
                                        return 1;
                                    }
                                }
                            }
                        }
                    }
                }
            }
        }
        else if(parse_keyword(&p, "fights")){
            char enemy_name[MAX_LINE_LENGTH];
            if(parse_name(&p, enemy_name)){
                skip_spaces(&p);
                if(*(p.cursor) == '\0'){
                    fight_enemy(state, enemy_name);
                    return 1;
                }
                if(parse_keyword(&p, "for")){
                    int pos_int = 0;
                    if(parse_pos_int(&p, &pos_int)){
                        if(parse_keyword(&p, "gold")){
                            skip_spaces(&p);
                            if(*(p.cursor) == '\0'){
                                fight_enemy_bounty(state, enemy_name, pos_int);
                                return 1;
                            }
                        }
                    }
                }
            }
        }
        else if(parse_keyword(&p, "heals")){
            int pos_int = 0;
            if(parse_pos_int(&p, &pos_int)){
                if(parse_keyword(&p, "hp")){
                    skip_spaces(&p);
                    if(*(p.cursor) == '\0'){
                        heal(state, pos_int);
                        return 1;
                    }
                }
            }
        }
        else if(parse_keyword(&p, "takes")){
            int pos_int = 0;
            if(parse_pos_int(&p, &pos_int)){
                if(parse_keyword(&p, "damage")){
                    skip_spaces(&p);
                    if(*(p.cursor) == '\0'){
                        take_damage(state, pos_int);
                        return 1;
                    }
                }
            }
        }
        else if(parse_keyword(&p, "discards")){
            if(parse_keyword(&p, "potion")){
                char potion_name[MAX_LINE_LENGTH];
                if(parse_name(&p, potion_name)){
                    skip_spaces(&p);
                    if(*(p.cursor) == '\0'){
                        discard_potion(state, potion_name);
                        return 1;
                    }
                }
            }
        }
        else if(parse_keyword(&p, "sells")){
            if(parse_keyword(&p, "card")){
                char card_name[MAX_LINE_LENGTH];
                if(parse_name(&p, card_name)){
                    if(parse_keyword(&p, "for")){
                        int pos_int = 0;
                        if(parse_pos_int(&p, &pos_int)){
                            if(parse_keyword(&p, "gold")){
                                skip_spaces(&p);
                                if(*(p.cursor) == '\0'){
                                    sell_card(state, card_name, pos_int);
                                    return 1;
                                }
                            }
                        }
                    }
                }
            }
            else if(parse_keyword(&p, "upgraded")){
                if(parse_keyword(&p ,"card")){
                    char card_name[MAX_LINE_LENGTH];
                    if(parse_name(&p, card_name)){
                        if(parse_keyword(&p, "for")){
                            int pos_int = 0;
                            if(parse_pos_int(&p, &pos_int)){
                                if(parse_keyword(&p, "gold")){
                                    skip_spaces(&p);
                                    if(*(p.cursor) == '\0'){
                                        sell_upgraded_card(state, card_name, pos_int);
                                        return 1;
                                    }
                                }
                            }
                        }
                    }
                }
            }
            else if(parse_keyword(&p, "potion")){
                char potion_name[MAX_LINE_LENGTH];
                if(parse_name(&p, potion_name)){
                    if(parse_keyword(&p, "for")){
                        int pos_int = 0;
                        if(parse_pos_int(&p, &pos_int)){
                            if(parse_keyword(&p, "gold")){
                                skip_spaces(&p);
                                if(*(p.cursor) == '\0'){
                                    sell_potion(state, potion_name, pos_int);
                                    return 1;
                                }
                            }
                        }
                    }
                }
            }
        }
        else if(parse_keyword(&p, "marks")){
            if(parse_keyword(&p, "card")){
                char card_name[MAX_LINE_LENGTH];
                if(parse_name(&p, card_name)){
                    if(parse_keyword(&p, "as")){
                        if(parse_keyword(&p, "exhaust")){
                            skip_spaces(&p);
                            if(*(p.cursor) == '\0'){
                                mark_exhaust(state, card_name);
                                return 1;
                            }
                        }
                    }
                }
            }
        }
    }
    else if(parse_keyword(&p, "Total")){
        if(parse_keyword(&p, "gold")){
            if(parse_keyword(&p, "?")){
                skip_spaces(&p);
                if(*(p.cursor) == '\0'){
                    gold_query(state);
                    return 1;
                }
            }
        }
        else if(parse_keyword(&p, "card")){
            char card_name[MAX_LINE_LENGTH];
            if(parse_name(&p, card_name)){
                if(parse_keyword(&p, "?")){
                    skip_spaces(&p);
                    if(*(p.cursor) == '\0'){
                        total_card_query(state, card_name);
                        return 1;
                    }
                }
            }
        }
        else if(parse_keyword(&p, "upgraded")){
            if(parse_keyword(&p, "card")){
                char card_name[MAX_LINE_LENGTH];
                if(parse_name(&p, card_name)){
                    if(parse_keyword(&p, "?")){
                        skip_spaces(&p);
                        if(*(p.cursor) == '\0'){
                            total_upgraded_card_query(state, card_name);
                            return 1;
                        }
                    }
                }
            }
        }
    }
    else if(parse_keyword(&p, "Floor")){
        if(parse_keyword(&p, "?")){
            skip_spaces(&p);
            if(*(p.cursor) == '\0'){
                floor_query(state);
                return 1;
            }
        }
    }
    else if(parse_keyword(&p, "Where")){
        if(parse_keyword(&p, "?")){
            skip_spaces(&p);
            if(*(p.cursor) == '\0'){
                where_query(state);
                return 1;
            }
        }
    }
    else if(parse_keyword(&p, "Deck")){
        if(parse_keyword(&p, "?")){
            skip_spaces(&p);
            if(*(p.cursor) == '\0'){
                deck_query(state);
                return 1;
            }
        }
        else if(parse_keyword(&p, "size")){
            if(parse_keyword(&p, "?")){
                skip_spaces(&p);
                if(*(p.cursor) == '\0'){
                    deck_size_query(state);
                    return 1;
                }
            }
        }
    }
    else if(parse_keyword(&p, "Relics")){
        if(parse_keyword(&p, "?")){
            skip_spaces(&p);
            if(*(p.cursor) == '\0'){
                relics_query(state);
                return 1;
            }
        }
    }
    else if(parse_keyword(&p, "Potions")){
        if(parse_keyword(&p, "?")){
            skip_spaces(&p);
            if(*(p.cursor) == '\0'){
                potions_query(state);
                return 1;
            }
        }
    }
    else if(parse_keyword(&p, "Health")){
        if(parse_keyword(&p, "?")){
            skip_spaces(&p);
            if(*(p.cursor) == '\0'){
                health_query(state);
                return 1;
            }
        }
    }
    else if(parse_keyword(&p, "Exhausts")){
        if(parse_keyword(&p, "?")){
            skip_spaces(&p);
            if(*(p.cursor) == '\0'){
                exhausts_query(state);
                return 1;
            }
        }
    }
    else if(parse_keyword(&p, "What")){
        if(parse_keyword(&p, "is")){
            if(parse_keyword(&p, "effective")){
                if(parse_keyword(&p, "against")){
                    char enemy_name[MAX_LINE_LENGTH];
                    if(parse_name(&p, enemy_name)){
                        if(parse_keyword(&p, "?")){
                            skip_spaces(&p);
                            if(*(p.cursor) == '\0'){
                                effective_query(state, enemy_name);
                                return 1;
                            }
                        }
                    }
                }
            }
        }
    }
    else if(parse_keyword(&p, "Defeated")){
        char enemy_name[MAX_LINE_LENGTH];
        if(parse_name(&p, enemy_name)){
            if(parse_keyword(&p, "?")){
                skip_spaces(&p);
                if(*(p.cursor) == '\0'){
                    defeated_query(state, enemy_name);
                    return 1;
                }
            }
        }
    }

    return 0;
}

/* The Main Execution Loop
 * Takes user input, trims it, checks for exit command, and passes it to execute_line. 
 * Finally frees the memory.
 */
int main(void) {
    char line[MAX_LINE_LENGTH + 2];
    GameState state;

    init_game_state(&state);

    while (1) {
        printf(">> ");
        fflush(stdout);

        if (fgets(line, sizeof(line), stdin) == NULL) {
            break;
        }

        chomp_newline(line);
        char *pLine = trim_line(line);

        if (strcmp(pLine, "Exit") == 0) {
            break;
        }

        if (!execute_line(&state, pLine)) {
            printf("INVALID\n");
        }
    }
    
    free_game_state(&state);

    return 0;
}