#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <error.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <signal.h>
#include <pthread.h>
#include <fcntl.h>
#include <ncurses.h>

#include "structs.h"


void signal_handler(int signum){

    if(signum==SIGINT){
        endwin();
        printf("Received SIGNAL %d\nSHUTTING DOWN.\n",signum);
        exit(0);
    }
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
    endwin();
    error(-1,0,"%s\n",message);
}

level load_level(char *filename, level map, winl win){

    int fd;
    level buffer;
    char lline[USR_LINE];

    fd=open(filename,O_RDONLY);
    if(fd<0) {
        wprintw(win.rwin,"Could not open map \"%s\"\n", filename);
        wrefresh(win.rwin);
        return map;
    }

    if(read(fd,&buffer.terrain, sizeof(char)*LVL_H*LVL_W)>0) {
        if (read(fd, &lline, sizeof(lline))>0) {
            sscanf(lline, "%d %d %d %d",
                   &buffer.n_obj, &buffer.n_enemies, &buffer.exit[0], &buffer.exit[1]);
            wprintw(win.rwin,"Loaded map name: %s\n", filename);
            wrefresh(win.rwin);
            return buffer;
        }
    }
    else {
        wprintw(win.rwin, "No map was loaded.\nCheck map structure\n");
        wrefresh(win.rwin);
    }

    close(fd);
    return map;
}

void refreshall(winl *win){

    wrefresh(win->lwin);
    wrefresh(win->rwin);
    wrefresh(win->foot);
}

void endncurses(winl *win){

    delwin(win->lwin);
    delwin(win->rwin);
    delwin(win->foot);
}

void print_lvl(level *map, winl *win){

    int i, j, color;

    wclear(win->lwin);
    for(i=0;i<LVL_W;i++)
        for(j=0;j<LVL_H;j++) {
            if(map->terrain[i][j]=='\xB1')
                wattron(win->lwin,COLOR_PAIR(3));

            wprintw(win->lwin,"%c", map->terrain[i][j]);
            wattroff(win->lwin,COLOR_PAIR(3));
        }
}

void print_user(bomber *player, level *map, winl *win){

    mvwprintw(win->lwin,player->y_pos,player->x_pos,"%c",player->user[0]);
}

void startgame(bomber *player, level *map, winl *win){

    int running=1, ch;
    keypad(win->lwin, TRUE);
    int prev[2];

    while(running){
        print_lvl(map,win);
        print_user(player, map, win);

        wrefresh(win->lwin);

        ch=wgetch(win->lwin);

        prev[0]=player->y_pos;
        prev[1]=player->x_pos;

        switch (ch){
            case KEY_DOWN:
                player->y_pos++;
                break;
            case KEY_UP:
                player->y_pos--;
                break;
            case KEY_LEFT:
                player->x_pos--;
                break;
            case KEY_RIGHT:
                player->x_pos++;
                break;
            case 'q':
                running=0;
                break;
        }
        if((A_CHARTEXT & mvwinch(win->lwin,player->y_pos,player->x_pos))!=' '){
            player->y_pos=prev[0];
            player->x_pos=prev[1];
        }

    }
}

int main(int argc, char** argv) {


    int         running=1;
    level       map;
    winl        win;
    bomber      player;

    strcpy(player.user,"Luis");
    player.x_pos=1;
    player.y_pos=1;
    player.n_bombs=5;
    player.n_bobombs=2;
    player.points=0;
    player.pid=getpid();


    signal(SIGINT, signal_handler);

    initncurses();
    win.lwin=newwin(23,49,1,1);
    win.rwin=newwin(23,27,1,51);
    win.foot=newwin(6,78,24,1);
    wbkgd(win.lwin,COLOR_PAIR(1));
    wbkgd(win.rwin,COLOR_PAIR(2));
    wbkgd(win.foot,COLOR_PAIR(2));
    scrollok(win.rwin,TRUE);
    scrollok(win.foot,TRUE);
    refreshall(&win);

    map=load_level(DEFLVL_PATH, map, win);


    startgame(&player, &map, &win);


    endncurses(&win);

    endwin();

    return (EXIT_SUCCESS);
}
