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

#include "structs.h"
#include "common.h"

//############################################ GLOBAL VARIABLES

database    authDB;
canary      header;

//############################################

void printhelp(){
    printf("ACCEPTED INSTRUCTIONS:\n");
    printf("    exit: Aborts the program\n");
    printf("    shutdown: Ends a game\n");
    printf("    start: Starts a game with all logged players\n");
    printf("    add [username] [password]: Creates a new username\n");
    printf("    users: Lists logged in users.\n");
    printf("    kick [username]: Kicks a player out of a game.\n");
    printf("    game: Shows relevant information about the current game.\n");
    printf("    map [filename]: Loads a new map from file.\n");
}

void shutdown(){

}

void start(level map){

    if (authDB.n_players<1){
        printf("No players to start the game.\n");
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
        printf("\nReceived signal %d.\nSHUTTING DOWN.\n",signum);
        exit(0);
    }
}

int add_user(char *user, char *passwd, db *usersdb, int i, char *filename){

    if(strchr(user,':')){
        printf("Username can't contain the character ':'. Try again.\n");
        return i;
    }

    for(int n=0;n<=i;n++){
        if(!strcmp(user,usersdb[n].user)){
            printf("Username already exists.\n");
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

    printf("USERS:\n");
    for(i=0;i<n_players;i++)
        printf("%d: %s\n", i+1, player[i].user);

}

void kick(char *username){
    int i;
    char buffer[USR_TAM];

    if(!authDB.n_players) {
        puts("No players logged in");
        return;
    }

    for(i=0;i<authDB.n_players;i++)
        if(!strcmp(username,authDB.player[i].user)){
            printf("Reason: ");
            scanf(" %49[^\n]s",buffer);

            header.structype=2;

            write(authDB.player[i].fd,&header, sizeof(header));
            write(authDB.player[i].fd,&buffer, sizeof(buffer));

            return;
        }

    printf("User %s is not logged in.\n", username);

}

void print_game_info(){

    int i;

    printf("\xc9\xcd\xcd\xcd\xcd\xcd\xcd\xcb\xcd\xcd\xcd\xcd\xcd\xcd\xcd\xcd\xbb\n");
    printf("\xba Name\t\xba Points\t\xba\n");
    printf("\xcc\xcd\xcd\xcd\xcd\xcd\xcd\xce\xcd\xcd\xcd\xcd\xcd\xcd\xcd\xcd\xb9\n");

    for(i=0;i<authDB.n_players;i++)
        printf("\xba%s\t\xba %d\t\xba\n",
               authDB.player[i].user,
               authDB.player[i].points);

    printf("\xc8\xcd\xcd\xcd\xcd\xcd\xcd\xca\xcd\xcd\xcd\xcd\xcd\xcd\xcd\xcd\xbc\n");
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

    printf("Successfully loaded %d users.\n",i);

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
            printf("User %s logged out.\nbomber#>", authDB.player[i].user);
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

    if (newUser.authOK==-1) {
        userDeauth(newUser);
        return;
    }

    printf("User \"%s\" is attempting to login.\nbomber#>", newUser.user);

    sprintf(buffer,"%s_%d",C_PIPE,newUser.pid);

    for(int i=0;(access(buffer, F_OK)==-1)||i<100;i++);
    //waits for the client to create a pipe (has a timeout in case of client crash)

    authDB.player[authDB.n_players].fd=open(buffer,O_RDWR);

    if(authDB.player[authDB.n_players].fd<0)
        error(-1,0,"ERROR - Could not open pipe.");

    authstatus=userAuth(newUser);
    if(authstatus>0) {     //If user authenticates
        printf("User\"%s\" logged in.\nbomber#>", newUser.user);

        strcpy(authDB.player[authDB.n_players].user,newUser.user);//User is now a player
        authDB.player[authDB.n_players].pid=newUser.pid;

        strcpy(authDB.player[authDB.n_players].pipename,buffer);
        authDB.player[authDB.n_players].points=0;
        printf("Player \"%s\" created.\nbomber#>",authDB.player[authDB.n_players].user);

        newUser.pid=getpid();
        newUser.authOK=authstatus;

        write(authDB.player[authDB.n_players].fd,&newUser, sizeof(user));
        authDB.n_players++;
        return;
    }
    else if(authstatus<0)
        printf("ERROR: User \"%s\" is already logged.\nbomber#>", newUser.user);
    else
        printf("User \"%s\" failed to login.\nbomber#>", newUser.user);

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

    //If no enviroment variable is set, a random value [5,20] is generated
    defmap.n_obj=(getenv("NOBJECT")!=NULL)?atoi(getenv("NOBJECT")):rand()%15+5;
    defmap.n_enemies=(getenv("NENEMY")!=NULL)?atoi(getenv("NENEMY")):rand()%15+5;

}

level load_level(char *filename, level map){

    int fd;
    level buffer;
    char lline[USR_LINE];

    fd=open(filename,O_RDONLY);
    if(fd<0) {
        printf("Could not open map \"%s\"\n", filename);
        return map;
    }

    if(read(fd,&buffer.terrain, sizeof(char)*LVL_H*LVL_W)>0) {
        if (read(fd, &lline, sizeof(lline))>0) {
            sscanf(lline, "%d %d %d %d",
                   &buffer.n_obj, &buffer.n_enemies, &buffer.exit[0], &buffer.exit[1]);
            if(!strcmp(filename,DEFLVL_PATH))
                buffer=load_defaultenv(buffer);
            printf("Loaded map name: %s\n", filename);
            return buffer;
        }
    }
    else
        printf("No map was loaded.\nCheck map structure\n");

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

    printf("Type 'help' for help and 'exit' to abort.\n");
    while(running)
    {
        printf("bomber#>");
        scanf(" %1023[^\n]s",uinput);
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

        else
            printf("Command not found or missing arguments. Try 'help'.\n");

    }

    gracefullexit(authDB.sPipeFd);

    pthread_join(listen, NULL);

    return (EXIT_SUCCESS);
}
