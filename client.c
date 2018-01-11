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

int sPipeFd, cPipeFd;
int ServerPID=0;

//############################################

void gracefullexit(){

    char    clt_pipe[USR_TAM];
    canary  header;
    user    deauth;

    header.structype=1;         //populates the header struct
    header.clientpid=getpid();
    deauth.authOK=-1;           //populates the struct with deauth warning
    deauth.pid=header.clientpid;

    //warns the server this client is going down
    write(sPipeFd, &header, sizeof(header));
    write(sPipeFd, &deauth, sizeof(user));

    sprintf(clt_pipe,"%s_%d",C_PIPE,header.clientpid);

    close(sPipeFd);
    close(cPipeFd);
    unlink(clt_pipe);

    printf("Exiting...\n");
    exit(0);
}

void signal_handler(int signum){

    if(signum==SIGINT){
        printf("\nSHUTTING DOWN.\n");
        gracefullexit();
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

user login(user newUser){

    canary header;
    newUser.authOK=0;
    printf("bomberC\nPlease login.\n");
    while(newUser.authOK<=0){
        printf("user:");
        scanf(" %49[^\n]s",newUser.user);
        printf("pass:");
        scanf(" %49[^\n]s",newUser.passwd);

        header.clientpid=getpid();
        newUser.pid=getpid();
        header.structype=1;

        write(sPipeFd, &header, sizeof(header));
        write(sPipeFd, &newUser, sizeof(user));

        read(cPipeFd,&newUser,sizeof(user));
        ServerPID=newUser.pid;

        if(newUser.authOK<1)
            printf("Wrong username or password.\n");
        else
            printf("Welcome %s.\n",newUser.user);
    }

    return newUser;
}

void kicked(){

    char buffer[USR_TAM];

    read(cPipeFd, &buffer, sizeof(buffer));
    printf("\nYou got kicked!\nReason: %s\n",buffer);
    gracefullexit();
}

void start_game(){

    level map;

    read(cPipeFd, &map, sizeof(level));

    printf("Starting game.\n");
    print_lvl(map);

}

void *listenserver(void *ptr){

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
                kicked();
                break;
            case 3:  //client will receive map and start the game
                start_game();

        }
    }
}

int main(int argc, char** argv) {

    user        newUser;
    int         running=1;
    int         arg_n;              //Custom shell argc equivalent
    char        uinput[USR_LINE];
    char        args[3][USR_TAM];   //Custom shell argv equivalent
    char        buffer[USR_TAM];
    pthread_t   keepalive, listen;

    setbuf(stdout, NULL);
    signal(SIGINT, signal_handler);

    if(pthread_create(&keepalive,NULL, activewait, NULL)!=0)
        error(-1,0,"ERROR - Error creating thread");


    openpipe(S_PIPE);    //Open Server Pipe

    sprintf(buffer,"%s_%d",C_PIPE,getpid());
    openpipe(buffer);    //Open Client Pipe

    newUser=login(newUser);

    if(pthread_create(&listen,NULL, listenserver, NULL)!=0)
        error(-1,0,"ERROR - Error creating thread");

    while(running) {
        printf("bomber#>");
        scanf(" %1023[^\n]s", uinput);
        arg_n = sscanf(uinput, "%s %s %s", args[0], args[1], args[2]);

        if (!strcmp(args[0], "exit"))
            running = 0;
    }

    gracefullexit();

    return (EXIT_SUCCESS);
}
