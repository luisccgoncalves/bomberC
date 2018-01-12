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

database    authDB;
canary      header;
WINDOW      *custwin[NWIN];

//############################################

void printhelp(){
    wclear(custwin[0]);
    wmove(custwin[0],5,1);
    wprintw(custwin[0],"ACCEPTED INSTRUCTIONS:\n");
    wprintw(custwin[0],"    exit: Aborts the program\n");
    wprintw(custwin[0],"    shutdown: Ends a game\n");
    wprintw(custwin[0],"    start: Starts a game with all logged players\n");
    wprintw(custwin[0],"    add [username] [password]: Creates new user\n");
    wprintw(custwin[0],"    users: Lists logged in users\n");
    wprintw(custwin[0],"    kick [username]: Kicks a player of a game\n");
    wprintw(custwin[0],"    game: Shows information about current game\n");
    wprintw(custwin[0],"    map [filename]: Loads a new map from file.\n");
    wrefresh(custwin[0]);
}

void printbomb(){
    wclear(custwin[0]);
    wmove(custwin[0],5,0);
    wprintw(custwin[0],"        ,--.!,\n");
    wprintw(custwin[0],"     __/   -*-\n");
    wprintw(custwin[0],"   ,d08b.  '|`\n");
    wprintw(custwin[0],"   0088MM \n");
    wprintw(custwin[0],"   `9MMP' \n");
    wprintw(custwin[0],"  _                     _                _____\n");
    wprintw(custwin[0]," | |                   | |              / ____|\n");
    wprintw(custwin[0]," | |__   ___  _ __ ___ | |__   ___ _ __| |     \n");
    wprintw(custwin[0]," | '_ \\ / _ \\| '_ ` _ \\| '_ \\ / _ | '__| |     \n");
    wprintw(custwin[0]," | |_) | (_) | | | | | | |_) |  __| |  | |____\n");
    wprintw(custwin[0]," |_.__/ \\___/|_| |_| |_|_.__/ \\___|_|   \\_____|\n");
    wrefresh(custwin[0]);
}

void shutdown(){

}

void start(level map){

    if (authDB.n_players<1){
        wprintw(custwin[1],"No players to start the game.\n");
        wrefresh(custwin[1]);
        return;
    }

    int i;
    canary header;
    header.structype=3;

    for(i=0;i<authDB.n_players;i++) {
        write(authDB.player[i].fd, &header ,sizeof(header));
        write(authDB.player[i].fd, &map ,sizeof(map));
    }
}

void gracefullexit(){

    header.structype=-1;

    write(authDB.sPipeFd,&header, sizeof(header));

    close(authDB.sPipeFd);
    for(int i=0;i<authDB.n_players;i++) {
        kill(authDB.player[i].pid,SIGINT);
        close(authDB.player[i].fd);
    }
    unlink(S_PIPE);

}

void signal_handler(int signum){

    if((signum==SIGINT)||(signum==SIGUSR1)){
        gracefullexit();
        wprintw(custwin[1],"\nReceived signal %d.\nSHUTTING DOWN.\n",signum);
        wrefresh(custwin[1]);
        endncurses(custwin);
        exit(0);
    }
}

int add_user(char *user, char *passwd, db *usersdb, int i, char *filename){

    if(strchr(user,':')){
        wprintw(custwin[1],"Username can't contain the character ':'. Try again.\n");
        wrefresh(custwin[1]);
        return i;
    }

    for(int n=0;n<=i;n++){
        if(!strcmp(user,usersdb[n].user)){
            wprintw(custwin[1],"Username already exists.\n");
            wrefresh(custwin[1]);
            return i;
        }
    }

    strncpy(usersdb[i].user, user,sizeof(usersdb->user));
    strncpy(usersdb[i].passwd, passwd,sizeof(usersdb->passwd));
    i++;

    FILE *fd;

    fd=fopen(filename,"a");
    if(!fd){
        error(-1,0,"ERROR - Could not open file.");
    }

    fprintf(fd, "%s::%s\n",user, passwd);

    fclose(fd);
    return i;
}

void list_users(bomber *player, int n_players){

    int i;

    wclear(custwin[0]);
    wmove(custwin[0],5,1);
    wprintw(custwin[0],"USERS:\n");
    for(i=0;i<n_players;i++)
        wprintw(custwin[0],"%d: %s\n", i+1, player[i].user);

    wrefresh(custwin[0]);
}

void kick(char *username){
    int i;
    char buffer[USR_TAM];

    if(!authDB.n_players) {
        wprintw(custwin[1],"No players logged in");
        wrefresh(custwin[1]);
        return;
    }

    for(i=0;i<authDB.n_players;i++)
        if(!strcmp(username,authDB.player[i].user)){
            wprintw(custwin[2],"Reason: ");
            wscanw(custwin[2]," %49[^\n]s",buffer);
            wrefresh(custwin[2]);

            header.structype=2;

            write(authDB.player[i].fd,&header, sizeof(header));
            write(authDB.player[i].fd,&buffer, sizeof(buffer));

            return;
        }

    wprintw(custwin[1],"User %s is not logged in.\n", username);
    wrefresh(custwin[1]);

}

void print_game_info(){

    int i;
    wclear(custwin[0]);
    wmove(custwin[0],5,0);
    wprintw(custwin[0],"\t\xc9\xcd\xcd\xcd\xcd\xcd\xcd\xcd\xcb\xcd\xcd\xcd\xcd\xcd\xcd\xcd\xbb\n");
    wprintw(custwin[0],"\t\xba Name\t\xba Pts\t\xba\n");
    wprintw(custwin[0],"\t\xcc\xcd\xcd\xcd\xcd\xcd\xcd\xcd\xce\xcd\xcd\xcd\xcd\xcd\xcd\xcd\xb9\n");

    for(i=0;i<authDB.n_players;i++)
        wprintw(custwin[0],"\t\xba%s\t\xba %d\t\xba\n",
               authDB.player[i].user,
               authDB.player[i].points);

    wprintw(custwin[0],"\t\xc8\xcd\xcd\xcd\xcd\xcd\xcd\xcd\xca\xcd\xcd\xcd\xcd\xcd\xcd\xcd\xbc\n");
    wrefresh(custwin[0]);
}

int load_file2db( char *filename, db *usersdb){

    int i=0;
    FILE *fd;

    fd=fopen(filename,"r");
    if(!fd){                    //maybe file doesn't exist
        fd=fopen(filename,"w"); //creates empty file
        fclose(fd);
        fd=fopen(filename,"r"); //Tries again
        if(!fd){
            error(-1,0,"ERROR - Could not open file.");
        }
    }

    while((fscanf(fd,"%[^':']::%s\n",usersdb[i].user,usersdb[i].passwd))!=EOF)
        i++;

    wprintw(custwin[1],"Successfully loaded %d users.\n",i);
    wrefresh(custwin[1]);
    fclose(fd);

    return i;
}

int userAuth(user newUser){
    int i;

    for(i=0; i< authDB.n_players; i++)
        if(!strcmp(newUser.user,authDB.player[i].user))
            return -1;            //Returns -1 if already logged in.

    for(i=0; i< authDB.userdb_size; i++)
        if(!strcmp(newUser.user,authDB.userdb[i].user)) {
            if (!strcmp(newUser.passwd,authDB.userdb[i].passwd))
                return 1;         //Returns 1 if user logs in successfully
        }

    return 0;                     //Returns 0 if authentication fails
}

void userDeauth(user deauth){
    int i;

    for(i=0;i<authDB.n_players;i++)
        if(authDB.player[i].pid==deauth.pid) {
            wprintw(custwin[1],"User %s logged out.\n", authDB.player[i].user);
            wrefresh(custwin[1]);
            for(i;i<authDB.n_players;i++)
                authDB.player[i]=authDB.player[i+1];
            authDB.n_players--;
        }
}

void authclient(int clientpid){

    user        newUser;
    int         authstatus=0;
    char        buffer[USR_TAM];
    //int         cPipeFd;

    while (1) {

        read(authDB.sPipeFd, &newUser, sizeof(user));       //Listening

        if(newUser.pid==clientpid)
            break;
    }

    if (newUser.authOK==2) {
        userDeauth(newUser);
        return;
    }

    wprintw(custwin[1],"User \"%s\" is attempting to login.\n", newUser.user);
    wrefresh(custwin[1]);

    sprintf(buffer,"%s_%d",C_PIPE,newUser.pid);

    for(int i=0;(access(buffer, F_OK)==-1)||i<100;i++);
    //waits for the client to create a pipe (has a timeout in case of client crash)

    authDB.player[authDB.n_players].fd=open(buffer,O_RDWR);

    if(authDB.player[authDB.n_players].fd<0)
        error(-1,0,"ERROR - Could not open pipe.");

    //Server is full
    if(authDB.n_players==authDB.max_players){
        newUser.authOK=3;
        write(authDB.player[authDB.n_players].fd,&newUser, sizeof(user));
        return;
    }


    authstatus=userAuth(newUser);
    if(authstatus>0) {     //If user authenticates
        wprintw(custwin[1],"User\"%s\" logged in.\n", newUser.user);

        strcpy(authDB.player[authDB.n_players].user,newUser.user);//User is now a player
        authDB.player[authDB.n_players].pid=newUser.pid;

        strcpy(authDB.player[authDB.n_players].pipename,buffer);
        authDB.player[authDB.n_players].points=0;
        wprintw(custwin[1],"Player \"%s\" created.\n",authDB.player[authDB.n_players].user);
        wrefresh(custwin[1]);

        newUser.pid=getpid();
        newUser.authOK=authstatus;

        write(authDB.player[authDB.n_players].fd,&newUser, sizeof(user));
        authDB.n_players++;
        return;
    }
    else if(authstatus<0)
        wprintw(custwin[1],"ERROR: User \"%s\" is already logged.\nbomber#>", newUser.user);
    else
        wprintw(custwin[1],"User \"%s\" failed to login.\nbomber#>", newUser.user);

    wrefresh(custwin[1]);

    newUser.authOK=authstatus;
    write(authDB.player[authDB.n_players].fd,&newUser, sizeof(user));
}

void *listenclients(void *ptr){

    canary header;
    header.structype=1;

    while(header.structype!=-1) {

        read(authDB.sPipeFd, &header, sizeof(header));

        switch (header.structype){

            case -1: //this kills the thread
                break;
            case 1:  //struct type user
                authclient(header.clientpid);
                break;
        }
    }
}
level load_defaultenv(level defmap){

    srand(time(NULL));

    //If no environment variable is set, a random value [5,20] is generated
    defmap.n_obj=(getenv("NOBJECT")!=NULL)?atoi(getenv("NOBJECT")):rand()%15+5;
    defmap.n_enemies=(getenv("NENEMY")!=NULL)?atoi(getenv("NENEMY")):rand()%15+5;

    return defmap;
}

level load_level(char *filename, level map){

    int fd;
    level buffer;
    char lline[USR_LINE];

    fd=open(filename,O_RDONLY);
    if(fd<0) {
        wprintw(custwin[1],"Could not open map \"%s\"\n", filename);
        wrefresh(custwin[1]);
        return map;
    }

    if(read(fd,&buffer.terrain, sizeof(char)*LVL_H*LVL_W)>0) {
        if (read(fd, &lline, sizeof(lline))>0) {
            sscanf(lline, "%d %d %d %d",
                   &buffer.n_obj, &buffer.n_enemies, &buffer.exit[0], &buffer.exit[1]);
            if(!strcmp(filename,DEFLVL_PATH))
                buffer=load_defaultenv(buffer);
            wprintw(custwin[1],"Loaded map name: %s\n", filename);
            wrefresh(custwin[1]);
            return buffer;
        }
    }
    else {
        wprintw(custwin[1], "No map was loaded.\nCheck map structure\n");
        wrefresh(custwin[1]);
    }

    close(fd);
    return map;
}



int main(int argc, char** argv) {

    int         running=1;
    int         arg_n;              //Custom shell argc equivalent
    char        uinput[USR_LINE];
    char        args[3][USR_TAM];   //Custom shell argv equivalent
    level       map;
    pthread_t   listen;
    authDB.n_players=0;

    initncurses();
    custwin[0]=newwin(23,50,0,0);
    custwin[1]=newwin(23,30,0,51);
    custwin[2]=newwin(6,80,24,0);
    wbkgd(custwin[0],COLOR_PAIR(1));
    wbkgd(custwin[1],COLOR_PAIR(2));
    wbkgd(custwin[2],COLOR_PAIR(2));
    scrollok(custwin[1],TRUE);
    scrollok(custwin[2],TRUE);

    refreshall(custwin, NWIN);


    //Gets max players from environment variable, if missing, defaults to 20
    authDB.max_players=getenv("NMAXPLAY")?atoi(getenv("NMAXPLAY")):20;

    setbuf(stdout, NULL);
    signal(SIGINT, signal_handler);
    signal(SIGUSR1, signal_handler);

//================================================================= CREATES SERVER PIPE

    if(access(S_PIPE, F_OK)==-1) {
        if (mkfifo(S_PIPE, 0666) < 0)
            error(-1, 0, "ERROR - Could not create pipe.");
    }
    else
        error(-1, 0, "ERROR - Only one server instance permitted");

    authDB.sPipeFd=open(S_PIPE, O_RDWR);
    if(authDB.sPipeFd<0)
        error(-1,0,"ERROR - Could not open pipe.");

//====================================================================================

    if(argc>1)
        authDB.userdb_size=load_file2db(argv[1], authDB.userdb);
    else
        error(-1,0,"ERROR - Please specify userfile.");

    //Load the default map from the file default.lvl
    map=load_level(DEFLVL_PATH, map);

    if(pthread_create(&listen,NULL, listenclients, NULL)!=0)
        error(-1,0,"ERROR - Error creating thread");

    printbomb();

    wprintw(custwin[2],"Type 'help' for help and 'exit' to abort.\n");

    while(running)
    {
        wprintw(custwin[2],"bomber#>");
        wscanw(custwin[2]," %1023[^\n]s",uinput);
        wrefresh(custwin[2]);
        arg_n=sscanf(uinput,"%s %s %s",args[0],args[1],args[2]);

        if(!strcmp(args[0],"exit"))
            running=0;

        else if(!strcmp(args[0],"help")&&arg_n==1)
            printhelp();

        else if(!strcmp(args[0],"shutdown")&&arg_n==1)
            shutdown();

        else if(!strcmp(args[0],"start")&&arg_n==1)
            start(map);

        else if(!strcmp(args[0],"add")&&arg_n==3)
            authDB.userdb_size=add_user(args[1],args[2],authDB.userdb,authDB.userdb_size, argv[1]);

        else if(!strcmp(args[0],"users")&&arg_n==1) {
            list_users(authDB.player, authDB.n_players);
        }

        else if(!strcmp(args[0],"kick")&&arg_n==2)
            kick(args[1]);

        else if(!strcmp(args[0],"game")&&arg_n==1)
            print_game_info();

        else if(!strcmp(args[0],"map")&&arg_n==2)
            map=load_level(args[1], map);

        else {
            wprintw(custwin[2], "Command not found or missing arguments. Try 'help'.\n");
            wrefresh(custwin[2]);
        }
    }

    gracefullexit(authDB.sPipeFd);

    pthread_join(listen, NULL);


    endncurses(custwin);

    return (EXIT_SUCCESS);
}
