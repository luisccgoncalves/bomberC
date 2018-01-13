//
//Function shared by client and server
//

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
    init_pair(3, COLOR_RED,COLOR_GREEN);
    init_pair(4, COLOR_WHITE,COLOR_BLUE);

    //low color support
/*
    init_pair(1, COLOR_WHITE, COLOR_GREEN);     //Pair for game screen
    init_pair(2, COLOR_GREEN, COLOR_BLACK);   //Pair for text windows
    init_pair(3, COLOR_RED,COLOR_GREEN);
    init_pair(4, COLOR_WHITE,COLOR_BLUE);
*/

    curs_set(0);
}

void throwerror(char *message){
    gracefullexit();
    error(-1,0,"%s\n",message);
}

