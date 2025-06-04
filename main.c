#include <stdarg.h>
#include <stdio.h>
#include <stdlib.h>
#include <time.h>
#include <string.h>

#define random(min, max) ((rand() % (max - min + 1)) + min)

typedef struct Room {
    char description[120];
    struct Room* north;
    struct Room* south;
    struct Room* east;
    struct Room* west;
    // 1-fight, 2-maze, 3-trade, 4-trap, 5-alchemy, 6-boss
    int room_type;
    int visited;
} Room;
typedef struct WorldMap {
    Room* starting_room;
    Room* current_room;
    Room** all_rooms;  // Dynamicke pole vsetkych miestnosti
    int total_rooms;
} WorldMap;
typedef struct {
    char name[50];
    int specialPoints;
    int strength, endurance, charisma, agility, luck;
    int min_Damage, max_Damage;
    int hp;
    int coins;
    int revives;
    int keys;
} Hero;
typedef struct {
    int min_Damage, max_Damage;
    int hp;
} Enemy;
typedef struct {
    char name[50];
    int price;
} Merchant_Item;

FILE* log_file = NULL;
Merchant_Item merchant_Item[] = {
    {"Small health potion", 10},
    {"Large health potion", 20},
    {"Luck amulet", 15},
    {"Sword", 20},
    {"Shield", 20},
    {"Stealth cloak", 15},
    {"Revive token", 20},
    {"Key", 10}
};

void start_logging(const char* player_name);
void log_message(const char* format, ...);
void end_logging();

Room* create_room(int room_type, const char* description) {
    Room* new_room = (Room*)malloc(sizeof(Room));
    if (new_room == NULL) {
        log_message("Error. Couldn\'t allocate memory for the room\n");
        exit(1);
    }

    new_room->room_type = room_type;
    strncpy(new_room->description, description, 119);
    new_room->description[119] = '\0';
    new_room->visited = 0;
    new_room->north = NULL, new_room->south = NULL, new_room->east = NULL, new_room->west = NULL;

    return new_room;
}
WorldMap* initialize_world() {
    WorldMap* world = (WorldMap*)malloc(sizeof(WorldMap));
    if (world == NULL) {
        log_message("Error. Couldn\'t allocate memory for the world\n");
        exit(1);
    }
    
    world->total_rooms = 6;
    world->all_rooms = (Room**)malloc(world->total_rooms * sizeof(Room*));
    if (world->all_rooms == NULL) {
        log_message("Error. Couldn\'t allocate memory for rooms array\n");
        free(world);
        exit(1);
    }
    
    world->all_rooms[0] = create_room(1, "Fight Room: A dangerous enemy appears!");
    world->all_rooms[1] = create_room(2, "Maze: A complex maze stretches before you.");
    world->all_rooms[2] = create_room(3, "Merchant: A mysterious trader offers his wares.");
    world->all_rooms[3] = create_room(4, "Trap: Be careful where you step!");
    world->all_rooms[4] = create_room(5, "Alchemy: Strange potions line the walls.");
    world->all_rooms[5] = create_room(6, "Boss Room: The final challenge awaits.");

    // Prepojime miestnosti
    for (int i = 0; i < world->total_rooms - 1; i++) {
        world->all_rooms[i]->east = world->all_rooms[i + 1];
        world->all_rooms[i + 1]->west = world->all_rooms[i];
    }
    
    world->starting_room = world->all_rooms[0];
    world->current_room = world->starting_room;

    return world;
}
Enemy initialize_Enemy(int endurance) {
    Enemy enemy;
    enemy.min_Damage = random(1, 50) / (int)endurance;
    enemy.max_Damage = random(1, 50) / (int)endurance;
    enemy.hp = random(1, 100);

    return enemy;
}
Enemy initialize_Boss(int endurance) {
    Enemy enemy;
    enemy.min_Damage = random(1, 50) / (int)endurance * 5;
    enemy.max_Damage = random(1, 50) / (int)endurance * 5;
    enemy.hp = random(1, 100) * 5;

    return enemy;
}
void assign_Points(Hero *hero);
Hero initialize_Hero(const char *name) {
    Hero hero;
    strcpy(hero.name, name);
    hero.specialPoints = 5;
    assign_Points(&hero);

    Enemy enemy = initialize_Enemy(hero.endurance);
    hero.min_Damage = enemy.min_Damage * hero.strength;
    hero.max_Damage = enemy.max_Damage * hero.strength;
    hero.hp = enemy.hp * hero.endurance;

    hero.coins = 50 * hero.luck;
    hero.revives = 0 + random(1, 10)*(hero.luck-1) > 9;
    hero.keys = 0 ;

    return hero;
}

void free_world(WorldMap* world);
void save_game(const Hero* hero, const WorldMap* world, const char* filename);
int load_game(Hero* hero, WorldMap* world, const char* filename);

void intro();
void crossroad(Hero *hero);
void check_Stats(const Hero *hero);
void fight(Hero *hero, int isBoss);
int check_Enemy_Death(Hero *hero, Enemy enemy, int hero_Damage);
void empty_Room(Hero *hero);
void check_Hero_Death(Hero *hero);
void run_Away(Hero *hero);
void distract(Hero *hero);
void fight_Room(Hero *hero);
void trade(Hero *hero);
void choosing_Item(Hero *hero, int i);
void trap(Hero *hero);
void maze(Hero *hero);
void alchemy(Hero *hero);
void boss_Fight(Hero *hero);


int main(int argc, char *argv[]) {
    char name[50], filename[60];
    srand(time(NULL));
    if (argc > 1) strcpy(name, argv[1]);
    else {
        printf("Name of the hero: ");
        scanf("%[^\n]s", name);
    }

    start_logging(name);
    Hero hero;
    WorldMap* world = initialize_world();
    
    snprintf(filename, sizeof(filename), "%s.sav", name);
    if (!load_game(&hero, world, filename)) {
        // If no save file exists, create new hero
        hero = initialize_Hero(name);
        intro();
    }
    else log_message("Loaded %s game file\n", name);

    while (world->current_room != NULL && hero.hp > 0) {
        Room* current = world->current_room;
        log_message("\n%s\n", current->description);

        // Add save option before each room
        char choice;
        log_message("Would you like to save the game? (y/n): ");
        scanf(" %c", &choice);
        if (choice == 'y' || choice == 'Y') save_game(&hero, world, filename);
        log_message("Would you like to end the game? (y/n): ");
        scanf(" %c", &choice);
        if (choice == 'y' || choice == 'Y') exit(1);

        switch (current->room_type) {
            case 1:
                fight_Room(&hero);
                break;
            case 2:
                maze(&hero);
                break;
            case 3:
                trade(&hero);
                break;
            case 4:
                trap(&hero);
                break;
            case 5:
                alchemy(&hero);
                break;
            case 6:
                if (hero.keys >= 5) boss_Fight(&hero);
                else log_message("You need 5 keys to enter this room!\n");
                break;
            default:
                log_message("Not an option!\n");
                break;
        }

        current->visited = 1;
        world->current_room = current->east;
    }

    free_world(world);
    end_logging();
    return 0;
}

void assign_Points(Hero *hero) {
    int tmp = 0;
    log_message("You have %d special points.\n", hero->specialPoints);
    getchar();

    strength:
    log_message("How many do you wanna put towards strength: ");
    scanf("%d", &tmp);
    if (tmp > hero->specialPoints) {
        log_message("You don\'t have that many points.\n");
        goto strength;
    }
    else {
        hero->strength = tmp+1;
        hero->specialPoints -= tmp;
    }

    endurance:
    log_message("How many do you wanna put towards endurance: ");
    scanf("%d", &tmp);
    if (tmp > hero->specialPoints) {
        log_message("You don\'t have that many points.\n");
        goto endurance;
    }
    else {
        hero->endurance = tmp+1;
        hero->specialPoints -= tmp;
    }

    charisma:
    log_message("How many do you wanna put towards charisma: ");
    scanf("%d", &tmp);
    if (tmp > hero->specialPoints) {
        log_message("You don\'t have that many points.\n");
        goto charisma;
    }
    else {
        hero->charisma = tmp+1;
        hero->specialPoints -= tmp;
    }

    agility:
    log_message("How many do you wanna put towards agility: ");
    scanf("%d", &tmp);
    if (tmp > hero->specialPoints) {
        log_message("You don\'t have that many points.\n");
        goto agility;
    }
    else {
        hero->agility = tmp+1;
        hero->specialPoints -= tmp;
    }

    luck:
    log_message("How many do you wanna put towards luck: ");
    scanf("%d", &tmp);
    if (tmp > hero->specialPoints) {
        log_message("You don\'t have that many points.\n");
        goto luck;
    }
    else {
        hero->luck = tmp+1;
        hero->specialPoints -= tmp;
    }
}
void intro() {
    log_message("You awaken in a dimly lit room, the air heavy with mystery. "
        "There are no windows, only a single wooden door, You don't remember how you got here, "
        "but you know one thing for sure: you need to find the keys to escape.\n");
    getchar();
    log_message("The room is filled with shadows, and you feel the presence of something or someone watching. "
        "The door is locked, and the key is somewhere in the house. "
        "Will you be able to find it before time runs out?\n");
    getchar();
}
void fight_Room(Hero *hero) {
    log_message("As you step forward, the door creaks open, revealing a dimly lit hallway. "
        "At the end of the corridor, is a path, shrouded in mist and shadow, "
        "Suddenly, a dark figure emerges from the fog, standing tall and menacing.\n");
    getchar();
    log_message("You're not sure how it found you, but there's no turning back now. "
           "You must decide quickly: fight, flee, or try to outsmart it.?\n");
    getchar();

    int choice = 0;
    choose_Crossroad_Left:
    log_message("  1. Fight \n");
    log_message("  2. Run away \n");
    log_message("  3. Distract \n");
    log_message("What will you do? (input number): \n");
    scanf("%d", &choice);
    switch (choice) {
        case 1:
            fight(hero, 0);
            break;
        case 2:
            run_Away(hero);
            break;
        case 3:
            distract(hero);
            break;
        default:
            log_message("You freeze, unsure of what to do next. "
                "The enemy\'s smirk widens, and you can feel "
                "its confidence growing with each passing second.\n");
        log_message("Finally, you shake off the confusion, gathering your courage.\n");
        goto choose_Crossroad_Left;
    }

    log_message("After a bit of rest you decide to move on.\n");
    getchar();
}
void check_Stats(const Hero *hero) {
    log_message("HP: %d\n", hero->hp);
    log_message("Min damage: %d\n", hero->min_Damage);
    log_message("Max damage: %d\n", hero->max_Damage);
    log_message("Coins: %d\n", hero->coins);
    log_message("Revives: %d\n", hero->revives);
    log_message("Keys: %d\n", hero->keys);
    log_message("Strength: %d\n", hero->strength);
    log_message("Endurance: %d\n", hero->endurance);
    log_message("Charisma: %d\n", hero->charisma);
    log_message("Agility: %d\n", hero->agility);
    log_message("Luck: %d\n", hero->luck);
    getchar();
}
void fight(Hero *hero, const int isBoss) {
    log_message("Without a word, you charge forward, heart racing as the adrenaline floods your veins.\n");
    getchar();
    Enemy enemy;
    if (!isBoss) enemy = initialize_Enemy(hero->endurance);
    else enemy = initialize_Boss(hero->endurance);
    int i = 1;

    log_message("  Your HP: %d \tEnemy HP: %d\n", hero->hp, enemy.hp);
    while (1) {
        log_message("%d.round:\n", i++);

        int hero_Damage = random(hero->min_Damage, hero->max_Damage);
        int enemy_Damage = random(enemy.min_Damage, enemy.max_Damage);
        if (random(1, 100)*hero->agility > 20) {
            if (check_Enemy_Death(hero, enemy, hero_Damage)) return;
            enemy.hp -= hero_Damage;
            log_message("  Your attack caused %d damage.\n", hero_Damage);
        }
        else {
            log_message("  You lunge forward, weapon raised, your heart pounding in your chest. "
                "With all your might, you strike, but the enemy swiftly sidesteps. "
                "You quickly regain your stance, frustration bubbling inside you.\n");
            getchar();
        }

        if (random(1, 100)*(int)hero->agility < 90) {
            hero->hp -= enemy_Damage;
            log_message("  Enemy\'s attack caused %d damage.\n", enemy_Damage);
            check_Hero_Death(hero);
        }
        else {
            log_message("  The enemy lunges at you with swift, deadly intent. "
                "You barely manage to dodge in time, feeling the air rush past as the attack narrowly misses.\n");
            getchar();
        }

        log_message("  Your HP: %d \tEnemy HP: %d\n", hero->hp, enemy.hp);
        getchar();
    }
}
int check_Enemy_Death(Hero *hero, Enemy enemy, int hero_Damage) {
    if (enemy.hp - hero_Damage <= 0) {
        enemy.hp = 0;
        log_message("  Your attack caused %d damage.\n", hero_Damage);
        log_message("  With one final, decisive blow, you strike your enemy down. "
               "The tension that once filled the air now fades, "
               "leaving an eerie stillness in its wake.\n");
        getchar();
        empty_Room(hero);
        return 1;
    }
    return 0;
}
void check_Hero_Death(Hero *hero) {
    if (hero->hp <= 0 && hero->revives <= 0) {
        log_message("Everything seems to slow as the final blow lands. Pain explodes through your body, "
            "and you gasp for breath, but it\'s too late. Despite your efforts to stay conscious, "
            "you feel yourself slipping away.\n");
        getchar();
        exit(0);
    }
    else if (hero->hp <= 0 && hero->revives >= 1) {
        log_message("Everything seems to slow as the final blow lands. Pain explodes through your body, "
            "and you gasp for breath, but it\'s too late. Despite your efforts to stay conscious, "
            "you feel yourself slipping away.\n");
        getchar();
        log_message("But then, as if drawn from the depths of despair, "
            "a faint light begins to flicker in the distance.\n");
        getchar();
        log_message("You\'re back. The battle isn\'t over, and neither are you. "
            "Death may have claimed you once, but now you stand again.\n");
        getchar();
        hero->revives--;
        hero->hp = 100;
    }
}
void empty_Room(Hero *hero) {
    log_message("As you search the area, something catches your eye. A small object partially hidden under some loose rubble. "
        "You kneel down, brushing aside the dust and debris, and your fingers close around a cold, metal key.\n");
    getchar();
    hero->keys++;
    log_message("With a sense of quiet satisfaction, you tuck the key into your pocket, "
        "knowing it might prove essential on your journey.\n");
    getchar();
}
void run_Away(Hero *hero) {
    Enemy enemy = initialize_Enemy(hero->endurance);

    if (random(0, 100)*hero->agility > 50) {
        log_message("With a burst of adrenaline, you push yourself into a full sprint. "
            "The enemy\'s shout echoes behind you, but you don\'t look back.\n");
        getchar();
        log_message("Breathing heavily, you find a safe spot to rest. "
            "Your heart still races, but you\'ve managed to escape for now.\n");
        getchar();
    }
    else {
        log_message("You make a split-second decision to retreat, turning and dashing toward the nearest exit."
            "As you try to sprint, you stumble, your footing unsure. "
            "The enemy is quick to react, seizing the opportunity.\n");
        getchar();
        log_message("A sharp pain erupts as their strike lands, taking advantage of your moment of weakness. "
            "You\'ll have to fight your way out.\n");
        getchar();

        int enemy_Damage = random(enemy.min_Damage, enemy.max_Damage);
        if (random(1, 100)*(int)hero->agility <= 20) {
            hero->hp -= enemy_Damage;
            log_message("  Enemy\'s attack caused %d damage.\n", enemy_Damage);
            check_Hero_Death(hero);
        }
        else {
            log_message("  The enemy lunges at you with swift, deadly intent, their weapon aimed straight for you. "
                "You barely manage to dodge in time, feeling the air rush past as the attack narrowly misses.\n");
            getchar();
        }
        fight(hero, 0);
    }
    empty_Room(hero);
}
void distract(Hero *hero) {
    Enemy enemy = initialize_Enemy(hero->endurance);

    if (random(0, 100)*hero->charisma >= 50 && hero->coins >= 10) {
        hero->coins -= 10;
        log_message("Desperation drives you to reach into your pouch, grabbing a handful of coins. "
            "With a sharp motion, you fling them across the room.\n");
        getchar();
        log_message("The enemy lower their weapon slightly and move toward the distraction. "
            "Seizing the opportunity, you quietly slip away, narrowly avoiding danger.\n");
        getchar();
    }
    else {
        log_message("\n%d\n", hero->coins);
        if (hero->coins < 10) {
            log_message("You instinctively reach for your pouch, for coins to throw and create a distraction. "
                "Panic sets in as your fingers find only the empty leather. "
                "The enemy notices your hesitation, their eyes narrowing as they step closer.\n");
            getchar();
        }
        else {
            hero->coins -= 10;
            log_message("You grab a handful of coins and hurl them to the side, "
                "hoping to draw the enemy\'s attention.\n");
            getchar();
            log_message("\"Did you really think that would work?\" they sneer, "
                "stepping closer with renewed determination.\n");
            getchar();
        }

        int enemy_Damage = random(enemy.min_Damage, enemy.max_Damage);
        if (random(1, 100)*(int)hero->agility >= 20) {
            hero->hp -= enemy_Damage;
            log_message("  Enemy\'s attack caused %d damage.\n", enemy_Damage);
            check_Hero_Death(hero);
        }
        else {
            log_message("The enemy lunges at you with swift, deadly intent. "
                "You barely manage to dodge in time, feeling the air rush past as the attack narrowly misses.\n");
            getchar();
        }
        hero->hp -= enemy_Damage;
        fight(hero, 0);
    }
    empty_Room(hero);
}
void trade(Hero *hero) {
    int item = 99999;
    log_message("In the center of the room, a merchant sits by a crackling fire. "
        "\"Ah, a traveler! Come, have a look. I\'ve got items to aid you on your journey, for"
        " the right price, of course.\" You glance at the goods:\n");
    getchar();

    for (int i = 0; i < 8; i++)
        log_message("  %d. %s:\t%d coins\n", (i+1), merchant_Item[i].name, merchant_Item[i].price);
    log_message("  9. Leave\n");

    shop:
    log_message("What will you buy (input number): ");
    scanf("%d", &item);
    switch (item) {
        case 1:
            choosing_Item(hero, item);
            hero->hp += 30;
            goto shop;
        case 2:
            choosing_Item(hero, item);
            hero->hp += 80;
            goto shop;
        case 3:
            choosing_Item(hero, item);
            hero->luck++;
            goto shop;
        case 4:
            choosing_Item(hero, item);
            hero->min_Damage += 10;
            hero->max_Damage += 10;
            goto shop;
        case 5:
            choosing_Item(hero, item);
            hero->endurance++;
            goto shop;
        case 6:
            choosing_Item(hero, item);
            hero->agility++;
            goto shop;
        case 7:
            choosing_Item(hero, item);
            hero->revives++;
        goto shop;
        case 8:
            choosing_Item(hero, item);
            hero->keys++;
            goto shop;
        case 9:
            break;
        default:
            log_message("You point to an item on the blanket. "
                "\"Ah, that one... Not available, I'm afraid. You\'ll have to choose something else.\" "
                "They gesture to the other items spread out before you.\n");
            getchar();
            goto shop;
    }
    log_message("After some thought, you thank the merchant and turn away, deciding to move on.\n");
}
void choosing_Item(Hero *hero, const int i) {
    if (hero->coins < merchant_Item[i-1].price) {
        log_message("You point to the item you want, and the merchant looks at the coins in your hand. "
            "\"Ah, it seems you're a bit short on funds, my friend. "
            "Perhaps something else more within your reach?\"\n");
    }
    else {
        log_message("You have purchased %s\n", merchant_Item[i-1].name);
        log_message("\"Excellent choice, sir\", says the merchant.\n");
        hero->coins -= merchant_Item[i-1].price;
    }
}
void trap(Hero *hero) {
    log_message("An old stone archway is soon revealed, marking the entrance to another room. "
       "With a deep breath, you step through the archway.\n");
    getchar();
    log_message("As you step through the archway, you sense something is off.\n");
    getchar();

    if (random(0, 100)*hero->agility < 80) {
        log_message("Just as you notice the faint outline of a hidden pressure plate, it\'s too late. "
            "Before you can react, a net of sharp spikes shoots up from the ground. "
            "The pain is immediate. \n");
        hero->hp -= 20;
        check_Hero_Death(hero);
    }
    else {
        log_message("Just as you lift your foot to take another step, "
            "you spot the faint outline of a hidden pressure plate beneath the dust. "
            "Your instincts kick in, avoiding the trap just in time. "
            "You continue on, now more alert than ever.\n");
    }
    getchar();
    log_message("After narrowly avoiding further danger, "
        "you steady yourself and push forward, your pulse still racing.\n");
    empty_Room(hero);
}
void maze(Hero *hero) {
    log_message("The maze stretches out before you, "
        "its labyrinthine passages seeming to shift and change with every turn. "
        "The walls close in, and time is running out. "
        "If you don\'t find a way out soon, the maze will become your tomb.\n");
    getchar();
    int map[6];
    int choice = 99999;

    for (int i = 0; i < sizeof(map)/sizeof(map[0]); i++) map[i] = random(1, 3);
    //for (int i = 0; i < sizeof(map)/sizeof(map[0]); i++) log_message("map[%d] = %d\n", i, map[i]);

    log_message("In front of are 3 seemingly identical paths: \n");
    time_t startTime = time(NULL);
    time_t timeLimit = 25;

    for (int i = 0; i < sizeof(map)/sizeof(map[0]); i++) {
        if (time(NULL) - startTime >= timeLimit) {
            log_message("Suddenly a deep, resonant sound echoes through the maze, "
                "The flickering light guiding your way begins to fade, "
                "replaced by an impenetrable darkness. The maze has claimed you.\n");
            exit(0);
        }

        log_message("  1. Left\n");
        log_message("  2. Forward\n");
        log_message("  3. Right\n");
        log_message("What path will you choose: ");
        scanf("%d", &choice);

        if (choice == map[i]) {
            log_message("You make your choice and head down the path. As you move deeper into the passage, "
                "the walls seem to open slightly, You\'ve made the right decision this time, "
                "but the time is still running out.\n");
        }
        else {
            i--;
            log_message("You make your choice and head down the path. The path twists sharply, "
                "leading you into a dead end. The disorienting feeling of being lost sinks in "
                "as you\'re forced to retrace your steps. The clock is ticking.\n");
        }
    }
    log_message("After what feels like an eternity, the walls seem to open up, "
        "and you burst into the open air. You\'ve made it. "
        "And what\'s more a key has somehow appeared in your pocket.\n");
    hero->keys++;
    getchar();
}
void alchemy(Hero *hero) {
    int elixir = 99999;
    int combination[] = {99999, 99999, 99999}, cauldron[] = {0, 0, 0};

    log_message("You step through a heavy wooden door. "
        "At the center of the room stands a table, and on it, "
        "5 elixirs glowing faintly with its own unique color.\n");
    getchar();
    log_message("As you approach the table, your eyes are drawn to a scroll on it "
        "Unrolling it, you read a cryptic message: "
        "\"In these 5 elixirs lies the path to creation. Choose wisely, "
        "for the correct combination of 3 will forge the Key of Passage.\n");
    hero->keys++;
    getchar();

    for (int i = 0; i < sizeof(combination)/sizeof(combination[0]); i++) {
        int tmp = random(1, 5);
        for (int j = 0; j < sizeof(combination)/sizeof(combination[0]); j++) {
            if (tmp == combination[j]) i--;
        }
        combination[i] = tmp;

    }
    for (int i = 0; i < sizeof(combination)/sizeof(combination[0]); i++)
        log_message("combination[%d] = %d\n", i, combination[i]);

    mixAgain:
    for (int i = 0; i < sizeof(combination)/sizeof(combination[0]); i++) {
        mix:
        log_message("  1. Crimson elixir.\n");
        log_message("  2. Golden elixir.\n");
        log_message("  3. Silver elixir.\n");
        log_message("  4. Azure elixir.\n");
        log_message("  5. Rose elixir.\n");
        log_message("Which ones will you combine. (input number): ");

        scanf("%d", &elixir);
        switch (elixir) {
            case 1:
                log_message("You pour a small portion of the crimson elixir into the cauldron.\n");
                cauldron[i] = 1;
                break;
            case 2:
                log_message("You pour a small portion of the golden elixir into the cauldron.\n");
                cauldron[i] = 2;
                break;
            case 3:
                log_message("You pour a small portion of the silver elixir into the cauldron.\n");
                cauldron[i] = 3;
                break;
            case 4:
                log_message("You pour a small portion of the azure elixir into the cauldron.\n");
                cauldron[i] = 4;
                break;
            case 5:
                log_message("You pour a small portion of the rose elixir into the cauldron.\n");
                cauldron[i] = 5;
                break;
            default:
                log_message("You stop to ponder your choices.\n");
                getchar();
                log_message("Alas you have to make one.\n");
                getchar();
                goto mix;
        }
    }
    if (cauldron[0] != combination[0] || cauldron[1] != combination[1]
        || cauldron[2] != combination[2]) {
        log_message("For a moment, the concoction seems stable, but then, the mixture begins to bubble violently. "
           "Your heart sinks as you realize the combination was wrong. "
           "You steady yourself, vowing to try again.\n");
        goto mixAgain;
    }
    log_message("The mixture begins to glow brighter, illuminating the room. "
        "A soft hum fills the air, and with a sudden flash, "
        "the liquid solidifies. \"The Key of Passage.\"\n");
    hero->keys++;
    getchar();
    log_message("With renewed determination, you turn to face whatever lies beyond.\n");
    getchar();
}
void boss_Fight(Hero *hero) {
    log_message("You approach an immense door, its surface carved with intricate patterns "
        "of ancient battles and forgotten runes. "
        "With struggle you open the heavy door. At the far end, seated upon a throne "
        "is a figure cloaked in darkness."
        "\"So, you\'ve made it this far. Impressive, but this is where your journey ends.\" "
        "The figure rises, and you feel an overwhelming sense of dread. "
        "This is no ordinary foe. This is the master of this place, "
        "the one who has been watching your every move.\n");
    fight(hero, 1);
    log_message("A deafening roar reverberates through the chamber as the boss collapses, "
        "As the dust settles, you notice something glinting among the remains: a key. "
        "You turn to the great door behind the throne. "
        "The keys fit perfectly, and as you turn them, a brilliant light is revealed. "
        "You\'ve escaped, victorious and changed by the trials you\'ve faced.\n");
    hero->keys++;
}
void free_world(WorldMap* world) {
    if (world == NULL) return;
    
    for (int i = 0; i < world->total_rooms; i++) free(world->all_rooms[i]);
    free(world->all_rooms);
    free(world);
}

void save_game(const Hero* hero, const WorldMap* world, const char* filename) {
    FILE* file = fopen(filename, "wb");
    if (file == NULL) {
        log_message("Error. Couldn\'t create the save file\n");
        return;
    }

    // Save hero data
    fwrite(hero, sizeof(Hero), 1, file);

    // Save current room index
    int current_room_index = -1;
    for (int i = 0; i < world->total_rooms; i++) {
        if (world->current_room == world->all_rooms[i]) {
            current_room_index = i;
            break;
        }
    }
    fwrite(&current_room_index, sizeof(int), 1, file);

    // Save room visited states
    for (int i = 0; i < world->total_rooms; i++)
        fwrite(&world->all_rooms[i]->visited, sizeof(int), 1, file);

    fclose(file);
    log_message("The game has been saved!\n");
}
int load_game(Hero* hero, WorldMap* world, const char* filename) {
    FILE* file = fopen(filename, "rb");
    if (file == NULL) return 0;

    // Load hero data
    fread(hero, sizeof(Hero), 1, file);

    // Load current room index
    int current_room_index;
    fread(&current_room_index, sizeof(int), 1, file);
    if (current_room_index >= 0 && current_room_index < world->total_rooms) 
        world->current_room = world->all_rooms[current_room_index];

    // Load room visited states
    for (int i = 0; i < world->total_rooms; i++)
        fread(&world->all_rooms[i]->visited, sizeof(int), 1, file);

    fclose(file);
    return 1;
}

void start_logging(const char* player_name) {
    char log_filename[64];
    snprintf(log_filename, sizeof(log_filename), "%s_log.txt", player_name);
    log_file = fopen(log_filename, "a");
    if (log_file == NULL) {
        printf("Failed to create log file\n");
        return;
    }

    time_t now = time(NULL);
    fprintf(log_file, "\n--- Game started at %s\n", ctime(&now));
}
void log_message(const char* format, ...) {
    if (log_file == NULL) return;

    va_list args;
    va_start(args, format);

    // Write to console
    vprintf(format, args);

    // Write to log file
    va_end(args);
    va_start(args, format);
    vfprintf(log_file, format, args);

    va_end(args);
    fflush(log_file);
}
void end_logging() {
    if (log_file != NULL) {
        time_t now = time(NULL);
        fprintf(log_file, "--- Game finished at %s\n", ctime(&now));
        fclose(log_file);
        log_file = NULL;
    }
}