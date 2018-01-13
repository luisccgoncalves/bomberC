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

//############################################ GLOBAL VARIABLES

int sPipeFd, cPipeFd;
int ServerPID=0;

//############################################

void gracefullexit(){

    char    clt_pipe[USR_TAM];
    canary  header;
    user    deauth;

    header.structype=1;         //populates the header struct
    header.clientpid=getpid();
    deauth.authOK=2;           //populates the struct with deauth warning
    deauth.pid=header.clientpid;

    //warns the server this client is going down
    write(sPipeFd, &header, sizeof(header));
    write(sPipeFd, &deauth, sizeof(user));

    sprintf(clt_pipe,"%s_%d",C_PIPE,header.clientpid);

    close(sPipeFd);
    close(cPipeFd);
    unlink(clt_pipe);

    endwin();

}

void signal_handler(int signum){

    if(signum==SIGINT){
        gracefullexit();
        printf("Received SIGNAL %d\nSHUTTING DOWN.\n",signum);
        exit(0);
    }
}

void *activewait(void *ptr){

    //Shuts down the client if the server closes
    //This thread ends after the client handshakes, server now has a pid to send signals to

    while(ServerPID==0){
        if(access(S_PIPE, F_OK)==-1) { //If pipe isn't created, server is offline
            gracefullexit();
            error(-1, 0, "ERROR - Server is offline");
        }
        sleep(5);
    }
}

void openpipe(char *pipename){

    if(access(pipename, F_OK)==-1)
        if(!strcmp(S_PIPE,pipename))    //If server pipe isn't created, quits
            error(-1, 0, "ERROR - Server is offline");

        else if(mkfifo(pipename, 0666)<0) //If client pipe isn't created, create it
            error(-1,0,"ERROR - Could not create pipe.");

    if(!strcmp(S_PIPE,pipename)){

        sPipeFd=open(pipename,O_RDWR);
        if(sPipeFd<0)
            error(-1,0,"ERROR - Could not open pipe.");
    }
    else {

        cPipeFd=open(pipename, O_RDWR);
        if(cPipeFd<0)
            error(-1,0,"ERROR - Could not open pipe.");
    }

}

void print_lvl(level map, winl *win){

    int i, j, color;

    wclear(win->lwin);
    for(i=0;i<LVL_W;i++)
        for(j=0;j<LVL_H;j++) {
            if(map.terrain[i][j]=='\xB0')
                color = 3;
            else
                color = 1;

            //attron(COLOR_PAIR(color));
            waddch(win->lwin, map.terrain[i][j]);
            wrefresh(win->lwin);
        }
    wrefresh(win->lwin);
    wrefresh(win->rwin);
}

user login(user newUser, winl win){

    canary header;
    newUser.authOK=0;
    wprintw(win.foot,"bomberC\nPlease login.\n");
    while(newUser.authOK<=0){
        wprintw(win.foot,"user:");
        wscanw(win.foot," %49[^\n]s",newUser.user);
        wrefresh(win.foot);
        wprintw(win.foot,"pass:");
        wscanw(win.foot," %49[^\n]s",newUser.passwd);
        wrefresh(win.foot);

        header.clientpid=getpid();
        newUser.pid=getpid();
        header.structype=1;

        write(sPipeFd, &header, sizeof(header));
        write(sPipeFd, &newUser, sizeof(user));

        read(cPipeFd,&newUser,sizeof(user));
        ServerPID=newUser.pid;

        if(newUser.authOK<1) {
            wprintw(win.foot, "Wrong username or password.\n");
            wrefresh(win.foot);
        }
        else if (newUser.authOK==3) {
            wprintw(win.foot,"Server is full.\n");
            wrefresh(win.foot);
            gracefullexit();
            exit(0);
        }
        else {
            wprintw(win.rwin, "Welcome %s.\n", newUser.user);
            wrefresh(win.rwin);
        }
    }
    return newUser;
}

void kicked(void *ptr){

    char buffer[USR_TAM];

    winl *win= (winl *)ptr;

    read(cPipeFd, &buffer, sizeof(buffer));
    wprintw(win->foot,"\nYou got kicked!\nReason: %s\nQuitting in 5 seconds.\n",buffer);
    wrefresh(win->foot);
    sleep(5);
    gracefullexit();
    exit(0);
}

void start_game(winl *win){

    level map;

    read(cPipeFd, &map, sizeof(level));

    wprintw(win->foot,"Starting game.\n");
    wrefresh(win->foot);
    print_lvl(map, win);

}

void *listenserver(void *ptr){

    //No need to cast ptr, this function will only pass it through
    canary header;
    header.structype=1;

    while(header.structype!=-1) {

        read(cPipeFd, &header, sizeof(header));

        switch (header.structype){

            case -1: //this kills the thread
                break;
            case 1:  //not used
                break;
            case 2:  //client will receive kick reason
                kicked(ptr);
                break;
            case 3:  //client will receive map and start the game
                start_game(ptr);

        }
    }
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

int main(int argc, char** argv) {

    user        newUser;
    int         running=1;
    int         arg_n;              //Custom shell argc equivalent
    char        uinput[USR_LINE];
    char        args[3][USR_TAM];   //Custom shell argv equivalent
    char        buffer[USR_TAM];
    pthread_t   keepalive, listen;
    winl        win;

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


    setbuf(stdout, NULL);
    signal(SIGINT, signal_handler);

    if(pthread_create(&keepalive,NULL, activewait, NULL)!=0)
        error(-1,0,"ERROR - Error creating thread");


    openpipe(S_PIPE);    //Open Server Pipe

    sprintf(buffer,"%s_%d",C_PIPE,getpid());
    openpipe(buffer);    //Open Client Pipe

    newUser=login(newUser,win);

    if(pthread_create(&listen,NULL, listenserver, (void *)&win)!=0)
        error(-1,0,"ERROR - Error creating thread");

    while(running) {
        wprintw(win.foot,"bomber#>");
        wscanw(win.foot," %1023[^\n]s", uinput);
        wrefresh(win.foot);
        arg_n = sscanf(uinput, "%s %s %s", args[0], args[1], args[2]);

        if (!strcmp(args[0], "exit"))
            running = 0;
    }

    endncurses(&win);

    gracefullexit();

    return (EXIT_SUCCESS);
}
