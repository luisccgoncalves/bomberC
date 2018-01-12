//
//Function shared by client and server
//


//void print_lvl(level map, WINDOW *winarray[]){
//
//    int i,j;
//    for(i=0;i<LVL_W;i++)
//        for(j=0;j<LVL_H;j++)
//            wprintw(winarray[0],"%c",map.terrain[i][j]);
//    wrefresh(winarray[0]);
//}

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
    init_pair(3, COLOR_WHITE,COLOR_RED);
    init_pair(4, COLOR_WHITE,COLOR_BLUE);
}

//void endncurses(WINDOW *winarray[]){
//    int i;
//
//    for(i=0;i<NWIN;i++){
//        wgetch(winarray[i]);
//        delwin(winarray[i]);
//    }
//
//    endwin();
//}
//
//void refreshall(WINDOW *winarray[], int nwin){
//    int i;
//    for(i=0;i<nwin;i++)
//        wrefresh(winarray[i]);
//}