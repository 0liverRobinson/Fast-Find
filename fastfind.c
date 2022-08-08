#include <stdio.h>
#include <stdlib.h>
#include <dirent.h>
#include <string.h>
#include <stdbool.h>
#include <sys/stat.h>
#include <pthread.h>

#define NEWLINE printf("\n")
#define ROOT "/"
#define HOME "/home/"

typedef struct Path
{
    char *path;
    struct Path *next;
} Path;

typedef struct Args
{
    char *target;
    bool verbose;
    bool file;
    bool directory;
    bool extraVerbose;
} Args;

Args * args;

void *find ( void *currentDir )
{   
    
    char *dir = (char *) currentDir;
    
    DIR *dirEnts;
    dirEnts = opendir( dir );
    
    struct dirent *dirEntries;
    
    // If we cannot open the dir then exit
    if ( dirEnts == NULL )
            return NULL;

    // Print out the dir we are checking
    if ( args->extraVerbose )
        printf("> %s\n", dir);


    // Create linked list for directory entries.
    Path* currentEntries;
    currentEntries = (Path*)malloc(sizeof(Path));
    currentEntries->next = NULL;
    currentEntries->path = dir;

    // Traverse through directories
    if ( dirEnts != NULL )
    {
        while ( ( (dirEntries = readdir( dirEnts ))  != NULL ) )
        {  
            // Filter out "." and ".."
            if ( strcmp( dirEntries->d_name , "..") == 0  || strcmp( dirEntries->d_name , ".") == 0 || strcmp( dirEntries->d_name , "!") == 0 )
                continue;
            
            char *type;
            
            // Determine file type
            switch (dirEntries->d_type)
            {
                case DT_REG:
                    type = "FILE";
                    break;
                
                case DT_DIR:
                    type = "DIR";
                    break;
                            
                case DT_LNK:
                    type = "LINK";
                    break;

                default:
                    type = "UNKNOWN";
                    break;
            } 

            if ( strcmp( type, "DIR" ) == 0 )
            {

                // Get length of currentdir name + previousdir name
                int newLen = strlen ( dir ) + strlen ( dirEntries->d_name ) + 10;

                // Formulate new dirname current/found
                char * newDir = (char*)malloc( sizeof( char ) * newLen );
                strcat(newDir, dir);
                strcat(newDir, dirEntries->d_name);
                strcat(newDir, ROOT);
            
                // Append to linked list (to traverse down these dirs later)
                Path *newPath = (Path * ) malloc(sizeof(Path*));
                newPath->next=currentEntries;
                newPath->path = newDir;
                currentEntries = newPath;  
            }
        
            // Check to see if its our target file
            if (strcmp( dirEntries->d_name, args->target ) == 0) 
            {
                // Print out the results
                if ( ( ( strcmp(type, "DIR") ==0 || strcmp(type, "LINK") ==0 )  && args->directory ==true)
                    || ( ( strcmp(type, "FILE") == 0 || strcmp(type, "UNKNOWN") == 0 ) && args->file ==true) ) {
                            printf ("\e[0;31m[\e[1;94mFOUND\e[0;31m]\t\e[0;32m%s\e[1;93m%s\e[0m", dir, args->target);
                                if ( args->verbose )
                                    printf ("\t[%s]", type);
                        NEWLINE;
                }
            }

        }
    } 
    
    // Close dir
    closedir( dirEnts );
    
    // Go through found dirs..
    for ( ; currentEntries->next != NULL; currentEntries = currentEntries->next) {
        //printf("%s\t", currentEntries->path);
        find(currentEntries->path);
    }
    return NULL;
}


void help()
{
    printf("####Fast find####\n\n");
    printf("-f [Search for files]\n");
    printf("-d [Search for directories]\n");
    printf("-v [Add extra details]\n");
    printf("-vv [Print everything]\n");
    printf("-h [Help]\n\n");
    printf("-----------------------\nExample: ./fastfind -f swapfind\n");
    exit(0);
}

int main (int argc, char **argv)
{   
    // Set up program arguments
    args = (Args*)(malloc( sizeof( Args* ) ));
    args->file=args->directory=false;

    // Handle arguments
    for ( ; --argc >= 1;  )

        if ( strcmp ( argv[argc], "-f" ) == 0)
            args->file = true;

        else if ( strcmp ( argv[argc], "-d" ) == 0)
            args->directory = true;

        else if ( strcmp( argv[argc], "-v" ) == 0)
            args->verbose = true;
        else if ( strcmp( argv[argc], "-vv" ) == 0)
            args->extraVerbose = true;
        // Help Screen
        else if ( strcmp(argv[argc], "-h") == 0)
            help();
        // Set target dir
        else
            args->target = argv[argc];

    // Handle desired file type
    if ( args->file == false && args->directory ==false )
            args->file=args->directory=true;

    // Create thread for root directory
    pthread_t rootDir;
    
    // Create thread for home directory
    pthread_t homeDir;

    // Start search
    pthread_create( &rootDir, NULL, find, (void *)ROOT ); 
    pthread_create( &homeDir, NULL, find, (void*)HOME ); 
    
    pthread_join( rootDir , NULL);
    return (0);
}