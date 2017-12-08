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

//############################################ GLOBAL VARIABLES



//############################################

void printhelp(){
    printf("ACCEPTED INTRSTRUCTIONS:\n");
    printf("    exit: Aborts the program\n");
    printf("    shutdown: Ends a game\n");
    printf("    add [username] [password]: Creates a new username\n");
    printf("    users: Lists logged in users.\n");
    printf("    kick [username]: Kicks a player out of a game.\n");
    printf("    game: Shows relevant information about the current game.\n");
    printf("    map [filename]: Loads a new map from file.\n");
}

void shutdown(){

}

int gracefullexit(int fd){

    user    killpipe;
    killpipe.pid=-1;

    write(fd,&killpipe, sizeof(user));

    return 0;
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

    int i=0;

    printf("UTILIZADORES:\n");

        printf("%d: %s\n", i, player[1].user);

}

void kick(char *username){
    printf("Kicking %s...\n",username);
}

void print_game_info(){

}

void load_map(char *mapname, level *map){

    printf("Loading %s...\n",mapname);


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

int userAuth(user newUser, database authDB){
    int i;

    for(i=0; i< authDB.n_players; i++)
        if(!strcmp(newUser.user,authDB.player[i].user))
            return -1;                                      //Returns -1 if already logged in.

    for(i=0; i< authDB.userdb_size; i++)
        if(!strcmp(newUser.user,authDB.userdb[i].user)) {
            if (!strcmp(newUser.passwd,authDB.userdb[i].passwd))
                return 1;                                   //Returns 1 if user logs in successfully
        }

    return 0;                                               //Returns 0 if authentication fails
}

void *listenclients(void *ptr){

    database    authDB;
    authDB=*((database*)ptr);
    user        newUser;
    int         authstatus=0;

    while(1) {
        read(authDB.sPipeFd, &newUser, sizeof(user));       //Listening
        if(newUser.pid<0)       //Trigger to close thread
            break;
        printf("User \"%s\" is attempting to login.\nbomber#>", newUser.user);

        authstatus=userAuth(newUser, authDB);
        if(authstatus>0) {     //If user authenticates
            printf("User\"%s\" logged in.\nbomber#>", newUser.user);
            strcpy(authDB.player[authDB.n_players].user,newUser.user);//User is now a player
            printf("Player \"%s\" created.\nbomber#>",authDB.player[authDB.n_players].user);
            authDB.player[authDB.n_players].points=0;
            authDB.n_players++;

            //Warn client #######################################
        }
        else if(authstatus<0)
            printf("ERROR: User \"%s\" is already logged.\nbomber#>", newUser.user);
        else
            printf("User \"%s\" failed to login.\nbomber#>", newUser.user);

    }

    close(authDB.sPipeFd);
    unlink(S_PIPE);
}

int openPipe(char *pipename){
    int fd;

    if(access(pipename, F_OK)==-1)
        if(mkfifo(pipename, S_IRWXU)<0)
            error(-1,0,"ERROR - Could not create pipe.");
    fd=open(pipename, O_RDWR);
    if(fd==0)
        error(-1,0,"ERROR - Could not open file.");
    return fd;
}

int main(int argc, char** argv) {

    int         running=1;
    int         arg_n;              //Custom shell argc equivalent
    char        uinput[USR_LINE];
    char        args[3][USR_TAM];   //Custom shell argv equivalent
    level       map;
    pthread_t   listen;
    database    authDB;
    authDB.n_players=0;

    setbuf(stdout, NULL);


    if(argc>1)
        authDB.userdb_size=load_file2db(argv[1], authDB.userdb);
    else
        error(-1,0,"ERROR - Please specify userfile.");

    authDB.sPipeFd=openPipe(S_PIPE);

    if(pthread_create(&listen,NULL, listenclients, (void *)&authDB)!=0)
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

        else if(!strcmp(args[0],"add")&&arg_n==3)
            authDB.userdb_size=add_user(args[1],args[2],authDB.userdb,authDB.userdb_size, argv[1]);

        else if(!strcmp(args[0],"users")&&arg_n==1)
            list_users(authDB.player,authDB.n_players);

        else if(!strcmp(args[0],"kick")&&arg_n==2)
            kick(args[1]);

        else if(!strcmp(args[0],"game")&&arg_n==1)
            print_game_info();

        else if(!strcmp(args[0],"map")&&arg_n==2)
            load_map(args[1], &map);

        else
            printf("Command not found or missing arguments. Try 'help'.\n");

    }

    gracefullexit(authDB.sPipeFd);

    pthread_join(listen, NULL);

    return (EXIT_SUCCESS);
}
