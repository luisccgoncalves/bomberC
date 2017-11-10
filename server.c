#include "structs.h"
#include "libs.h"


void printhelp(){
    printf("ACCEPTED INTRSTRUCTION:\n");
    printf("    quit: Quits the server\n");
    printf("    shutdown: Ends a game\n");
    printf("    add [username] [password]: Creates a new username\n");
    printf("    users: Lists all the users.\n");
    printf("    kick [username]: Kicks a user out of a game.\n");
    printf("    game: Shows relevant information about the current game.\n");
    printf("    map [filename]: Loads a new map from file.\n");
}

int main(int argc, char** argv) {

    int running=1;
    char uinput[100];

    printf("Type 'help' for help and 'quit' to exit.\n");
    while(running)
    {
        printf("bomber#>");
        scanf(" %99[^\n]",uinput);

        if(!strcmp(uinput,"quit"))
            running=0;

        if(!strcmp(uinput,"help"))
            printhelp();


    }


    return (EXIT_SUCCESS);
}
