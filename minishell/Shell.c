#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include <ctype.h>
#include <assert.h>
#include <dirent.h>
#include "Shell.h"
#include "StringVector.h"
#include <signal.h>



pid_t pids[100] ;
char pidsS[100][5];

void
shell_init( struct Shell *this )
{
    this->running     = false;
    this->line        = NULL;
    this->line_number = 0;
    this->line_length = 0;
}

void
shell_free( struct Shell *this )
{
    if ( NULL != this->line ) {
        free( this->line );
        this->line = NULL;
    }
    this->line_length = 0;
}

void
shell_run( struct Shell *this )
{
    this->running = true;
    printf( "* Shell started\n" );
    while ( this->running ) {
        shell_read_line( this );
        shell_execute_line( this );
    }
    printf( "* Shell stopped\n" );
}

void
shell_read_line( struct Shell *this )
{
    this->line_number++;
    char *buf = getcwd( NULL, 0 );
    printf( "%d: %s> ", this->line_number, buf );
    free( buf );
    getline( &this->line, &this->line_length, stdin );
}

static void
do_help( struct Shell *this, const struct StringVector *args )
{
    printf( "-> commands: exit, cd, help, ?.\n" );
    (void)this;
    (void)args;
}



static 
void addPid(struct Shell *this,pid_t * p, char *str){
    int len = sizeof( *pids);
    int i = 0;
    bool find = false;
    while (i<len && find == false){
        if(pids[i]==0){
            pids[i]= *p ;
            strcpy(pidsS[i],str);
            find = true;
        }
        i++;
            
    }
    
}

static
 void do_system(struct Shell *this, const struct StringVector *args)
{
    int nb_tokens = string_vector_size( args );
    int back = strcmp (string_vector_get(args, nb_tokens-1), "&");
    char * command = string_vector_get(args, 1);
    char *file = string_vector_get(args, 1);
    char *argument[nb_tokens-1];
    if ( 2 <= nb_tokens ){
        for (size_t i = 1; i < nb_tokens; i++)
        {
            argument[i-1] = string_vector_get(args,i);
        }
        if(back==0){
            argument[nb_tokens-2]=NULL;
        }
        else{
            argument[nb_tokens-1]=NULL;
        }
    }
    pid_t p = fork();
    if (p == 0) {
        execvp(file, argument);
        exit(EXIT_SUCCESS);
    }
    if (back != 0){
        wait(p);
    }
    else{
        addPid(this,&p, command);
    }
    (void)this;
    (void)args;

}


static void 
do_jobs(struct Shell *this, const struct StringVector *args ){
    int len = 100;
    for(int i =0 ; i<len ; i++){
        if(pids[i]!= 0 && strcmp (pidsS[i], "")!=0 ){
            printf("[%d] %d nom: %s  \n",(i+1),pids[i],pidsS[i]);  
        }
    }
    (void)this;
    (void)args;
}

static 
void do_kill(struct Shell *this, const struct StringVector *args ){
    int nb_tokens = string_vector_size(args);
    bool find = false;
    int i = 0;
    int len = 100;
    char myNum[10]; 
    if ( 2 == nb_tokens ){
       while (find == false && i<len ){
        int test = sprintf(myNum, "%d", pids[i]);
        if(strcmp (myNum, string_vector_get(args,1))==0){
            kill(pids[i],SIGKILL);
            pids[i]= 0;
            strcpy(pidsS[i],""); 
            find = true;        
        }
        i = i +1;
       }
       if(find==false){
        printf("Aucun processus de trouvé\n");
       }
    }  
    else 
    {
        printf("kill: opérande manquant\n");
    }
}




static void 
do_mkdir(struct Shell *this, const struct StringVector *args ){
    int nb_tokens = string_vector_size(args);
    char *dir_name = string_vector_get( args , 1);

    if ( 2 <= nb_tokens ){
        int res = mkdir(dir_name,0777);
        if ( res != 0 ){
            printf("mkdir: impossible de créer le répertoire «%s»: Le fichier existe\n", dir_name);
        }
    } 
    else 
    {
        printf("mkdir: opérande manquant\n");
    }

    (void)this;
    (void)args;
}



static void
do_cd( struct Shell *this, const struct StringVector *args )
{
    int   nb_tokens = string_vector_size( args );
    char *tmp;
    if ( 1 == nb_tokens ) {
        tmp = getenv( "HOME" );
    }
    else {
        tmp = string_vector_get( args, 1 );
    }
    int rc = chdir( tmp );
    if ( 0 != rc )
        printf( "directory '%s' not valid\n", tmp );
    (void)this;
}

static void 
do_echo( struct Shell *this, const struct StringVector *args )
{
    int nb_tokens = string_vector_size( args );
    if ( 2 == nb_tokens ){
       char *text = string_vector_get( args, 1 );
       
       printf(text);
       printf("\n"); 
    }
    else if ( 4 == nb_tokens ){
        FILE* fichier = NULL;
        char *text = string_vector_get( args, 1);
        fichier = fopen("test.txt", "w+");
        if (fichier != NULL)
        {
            fputs(text, fichier);
            fclose(fichier);
        }
    }
}

static void
do_rappel( struct Shell *this, const struct StringVector *args )
{
    (void)this;
    (void)args;
}

static void
do_execute( struct Shell *this, const struct StringVector *args )
{
    (void)this;
    (void)args;
}

static void
do_exit( struct Shell *this, const struct StringVector *args )
{
    this->running = false;
    (void)this;
    (void)args;
}


void printDirectory(char * directory){
    struct dirent *dir;
    DIR *d = opendir(directory); 
    char * name;
        if (d)
        {
            while ((dir = readdir(d)) != NULL)
            {
                name = dir->d_name;
                if(strcmp (name, ".")!=0 && strcmp(name,"..")!=0){    
                    printf("%s\n", dir->d_name);
                }
            }
            closedir(d);
        }
}

void do_pwd(struct Shell *this, const struct StringVector *args){
    char *buf = getcwd( NULL, 0 );
    printf("%s \n",buf);
}

void do_xeyes(struct Shell *this, const struct StringVector *args){
    system("xeyes");
    (void)this;
    (void)args;
}

static void do_ls(struct Shell *this, const struct StringVector *args){
    int   nb_tokens = string_vector_size( args );
    char * tmp;
    if ( 1 == nb_tokens ) {
        tmp = ".";
        printDirectory(tmp);
    }
    else {
        tmp = string_vector_get( args, 1 );
        printDirectory(tmp);
    }
    int rc = chdir(tmp);
    if ( 0 != rc )
        printf( "directory '%s' not valid\n", tmp );
    (void)this;
}



typedef void ( *Action )( struct Shell *, const struct StringVector * );

static struct {
    const char *name;
    Action      action;
} actions[] = { { .name = "exit", .action = do_exit },     { .name = "cd", .action = do_cd },
                { .name = "rappel", .action = do_rappel }, { .name = "help", .action = do_help },
                { .name = "?", .action = do_help },        { .name = "!", .action = do_system },
                { .name = "xeyes", .action = do_xeyes}  ,  
                { .name = "echo", .action = do_echo },   { .name = "jobs", .action = do_jobs }, 
                 { .name = "pwd", .action = do_pwd }, { .name = "mkdir", .action = do_mkdir},
                 { . name = "kill", .action = do_kill},
                 { .name = NULL, .action = do_execute }
                };


Action
get_action( char *name ){
    int i = 0;
    while ( actions[i].name != NULL && strcmp( actions[i].name, name ) != 0 ) {
        i++;
    }
    return actions[i].action;
}

void
shell_execute_line( struct Shell *this )
{
    struct StringVector tokens    = split_line( this->line );
    int                 nb_tokens = string_vector_size( &tokens );

    if ( nb_tokens == 0 ) {
        printf( "-> Nothing to do !\n" );
    }
    else {
        char  *name   = string_vector_get( &tokens, 0 );
        Action action = get_action( name );
        action( this, &tokens );
    }

    string_vector_free( &tokens );
}

