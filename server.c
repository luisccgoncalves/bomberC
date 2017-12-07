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

int sPipeKeepAlive=1;  //keeps the server pipe open

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

void gracefullexit(){

    sPipeKeepAlive=0;
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

void list_users(bomber *player){

    printf("UTILIZADORES:\n");

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

void *listenclients(void *ptr){

    int     fd;
    user    *newUser;

    if(access("sPipe", F_OK)==-1)
        if(mkfifo("sPipe", S_IRWXU)<0)
            error(-1,0,"ERROR - Could not create pipe.");
    fd=open("sPipe", O_RDWR);
    if(fd==0)
        error(-1,0,"ERROR - Could not open file.");
    int reading=1;


        read(fd, newUser, sizeof(user));
        sleep(1);
        printf("plim!");
        //printf("\n%s\n%s\n%d",newUser->user,newUser->passwd, newUser->pid);

    printf("plim--------!");

    close(fd);
    unlink("sPipe");
}

int main(int argc, char** argv) {

    int         running=1;
    int         arg_n;              //Custom shell argc equivalent
    char        uinput[USR_LINE];
    char        args[3][USR_TAM];   //Custom shell argv equivalent
    db          usersdb[100];       //User database
    int         userdb_size;
    level       map;
    bomber      player[20];
    pthread_t   listen;


    if(argc>1)
        userdb_size=load_file2db(argv[1], usersdb);
    else
        error(-1,0,"ERROR - Please specify userfile.");

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

        else if(!strcmp(args[0],"add")&&arg_n==3)
            userdb_size=add_user(args[1],args[2],usersdb,userdb_size, argv[1]);

        else if(!strcmp(args[0],"users")&&arg_n==1)
            list_users(player);

        else if(!strcmp(args[0],"kick")&&arg_n==2)
            kick(args[1]);

        else if(!strcmp(args[0],"game")&&arg_n==1)
            print_game_info();

        else if(!strcmp(args[0],"map")&&arg_n==2)
            load_map(args[1], &map);

        else
            printf("Command not found or missing arguments. Try 'help'.\n");

    }

    gracefullexit();
    pthread_join(listen, NULL);

    return (EXIT_SUCCESS);
}
