#ifndef BOMBERC_COMMON_H
#define BOMBERC_COMMON_H

void print_lvl(level map){

    int i,j;
    for(i=0;i<LVL_W;i++)
        for(j=0;j<LVL_H;j++)
            printf("%c",map.terrain[i][j]);
}

void initncurses() {
    initscr();
    keypad(stdscr, TRUE); // Enable arrow keys
    start_color();

    init_color(COLOR_BRICK, 600, 600, 600);
    init_color(COLOR_BLACK, 0, 0, 0);
    init_pair(1, COLOR_BRICK, COLOR_GREEN);
    init_pair(2, COLOR_GREEN, COLOR_BLACK);
}



#endif //BOMBERC_COMMON_H
