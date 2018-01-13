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
#include "shared.c"

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

void print_lvl(level map, winl *win){

    int i, j, color;

    wclear(win->lwin);
    for(i=0;i<LVL_W;i++)
        for(j=0;j<LVL_H;j++) {
            if(map.terrain[i][j]=='\xB1')
                wattron(win->lwin,COLOR_PAIR(3));
            else
                color = 1;


            wprintw(win->lwin,"%c", map.terrain[i][j]);
            wrefresh(win->lwin);
            wattroff(win->lwin,COLOR_PAIR(3));
        }
    wrefresh(win->lwin);
    wrefresh(win->rwin);
}

int main(int argc, char** argv) {


    int         running=1;
    level       map;
    winl        win;
    int         ch;

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


    print_lvl(map, &win);
    ch=wgetch(win.lwin);


    endncurses(&win);

    endwin();

    return (EXIT_SUCCESS);
}
