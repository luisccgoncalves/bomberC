//
//Function shared by client and server
//


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

    init_color(COLOR_BRICK, 700, 700, 700);     //Defines brick color
    init_color(COLOR_BLACK, 0, 0, 0);           //Redefines true black
    init_color(COLOR_MYGREEN, 0, 1000, 0);      //Redefines true green
    init_color(COLOR_GREEN, 0, 565, 0);         //Defines grass color
    init_pair(1, COLOR_BRICK, COLOR_GREEN);     //Pair for game screen
    init_pair(2, COLOR_MYGREEN, COLOR_BLACK);   //Pair for text windows
}

void endncurses(WINDOW *custwin[]){
    int i;

    for(i=0;i<NWIN;i++){
        wgetch(custwin[i]);
        delwin(custwin[i]);
    }

    endwin();
}

void refreshall(WINDOW *winarray[], int nwin){
    int i;
    for(i=0;i<nwin;i++)
        wrefresh(winarray[i]);
}