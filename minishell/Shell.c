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
#include <sys/stat.h>

/* Variables */
int nbPids = 0;
pid_t pids[100] ;
char pidsStatus[100][50];
char pidsS[100][50];

/**
 * Initialise le shell
 * @param this le shell
 */
void
shell_init( struct Shell *this )
{
    this->running     = false;
    this->line        = NULL; 
    this->line_number = 0;
    this->line_length = 0;
}

/**
 * Libere le shell
 * @param this le shell
 */
void
shell_free( struct Shell *this )
{
    if ( NULL != this->line ) {
        free( this->line );
        this->line = NULL;
    }
    this->line_length = 0;
}

/**
 * Exucute le shell
 * @param this le shell
 */
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

/**
 * Lit ce qu'à écrit l'utilisateur
 * @param this le shell
 */
void
shell_read_line( struct Shell *this )
{
    this->line_number++;
    char *buf = getcwd( NULL, 0 );
    printf( "%d: %s> ", this->line_number, buf );
    free( buf );
    getline( &this->line, &this->line_length, stdin );
}

/**
 * Commande help dit les commandes que l'on peut executer est appelé par help ou ?
 * @param this le shell
 * @param args les arguments
 */
static void
do_help( struct Shell *this, const struct StringVector *args )
{
    printf( "-> commands: exit, cd, help, ?, ls, mkdir, echo, !, pwd, jobs, kill, xeyes.\n" );
    (void)this;
    (void)args;
}

/**
 * Ajoute un processus fil en cours à notre liste de prcessus
 * @param this
 * @param p le numero du processus fils
 * @param str la commande du processus fils
 */
static 
void addPid(struct Shell *this, pid_t * p, char *str){
    pids[nbPids]= *p ;
    strcpy(pidsStatus[nbPids], "En cours");
    strcpy(pidsS[nbPids],str);
    nbPids++;
    (void)this;
}

/**
 * Supprime les processus fini de notre liste de processus
 */
static 
void delPid(){
    int i = 0;
    while (i < nbPids){
        if (strcmp(pidsStatus[i],"Fini    ") == 0){
            pids[i]=pids[i+1];
            strcpy(pidsS[i],pidsS[i+1]);
            strcpy(pidsStatus[i],pidsStatus[i+1]);
            nbPids--;
        } else {
            i++;
        }
    }
}

/**
 * Met le processus correspondant à l'indice à fini
 * @param indice
*/
static
void downPid(int indice){
    strcpy(pidsStatus[indice], "Fini    ");
}

/**
 * Est appeler dans un fils meurt et cherche ce fils dans la liste pour le signaler comme "Fini"
*/
static
void signal_fin_fils() {
    pid_t pid = wait(NULL);
    for (int i = 0; i < nbPids; i++){
        if (pids[i] == pid){
            downPid(i);
        }
    }
}

/**
 * Methode qui execute toute les commandes prefixé de ! 
 * Peu les lancer en arrière plan si il y a &
 * Ajoute à la liste le processus fils si il est en arrière plan
 * @param this
 * @param args
*/
static
 void do_system(struct Shell *this, const struct StringVector *args)
{
    int nb_tokens = string_vector_size( args );
    int back = strcmp (string_vector_get(args, nb_tokens-1), "&");
    char *command = string_vector_get(args, 1);
    char *argument[nb_tokens-1];
    if ( 2 <= nb_tokens ){
        for (int i = 1; i < nb_tokens; i++)
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

    struct sigaction sa;
    sa.sa_handler = signal_fin_fils;
    sigemptyset( &sa.sa_mask );
    sa.sa_flags = SA_RESTART;  // Restart functions if interrupted by handler
    int retval = sigaction( SIGCHLD, &sa, NULL );
    if ( retval < 0 ) {
        perror( "sigaction failed" );
        exit( EXIT_FAILURE );
    } 

    if (p == 0) {
        execvp(command, argument);
        exit(EXIT_SUCCESS);
    }
    if (back != 0){
            wait(p);
        }
        
    if (back == 0){
        char *text = string_vector_get( args , 1);
        for (int i = 2; i < nb_tokens-1; i++)
        {
            strcat(text, " ");
            strcat(text,string_vector_get( args , i));
        }

        addPid(this, &p, text);
    }
    
    (void)this;
    (void)args;

}

/**
 * Affiche la liste des processus fils
 * Et appelle la méthode delPid pour supprimer les processus fini
 * @param this
 * @param args
*/
static void 
do_jobs(struct Shell *this, const struct StringVector *args ){
    for(int i =0 ; i<nbPids ; i++){
        printf("[%d] %d     status : %s     nom : %s  \n",(i+1),pids[i],pidsStatus[i], pidsS[i]);  
    }
    delPid();
    (void)this;
    (void)args;
}

/**
 * Tue le processus fils dont le numéro est passé en paramètre
 * @param this
 * @param args
*/
static 
void do_kill(struct Shell *this, const struct StringVector *args ){
    int nb_tokens = string_vector_size(args);
    bool find = false;
    int i = 0;
    char myNum[50]; 
    if ( 2 == nb_tokens ){
       while (find == false && i<nbPids ){
        sprintf(myNum, "%d", pids[i]);
        if(strcmp (myNum, string_vector_get(args,1))==0){
            kill(pids[i],SIGKILL);
            downPid(i);
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
    (void)this;
}

/**
 * Méthode mkdir crée un repertoire avec les arguments passer en paramètre
 * @param this
 * @param args
*/
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


/**
 * Permet de naviguer dans les différents répertoires
 * Si aucun argument est passé en paramètre, on se déplace dans le répertoire home
 * @param this
 * @param args
*/
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

/**
 * Méthode echo
 * Affiche les arguments passer en paramètre
 * Si il y a un > alors il redirige la sortie dans le fichier passer en paramètre
 * @param this
 * @param args
*/
static void 
do_echo( struct Shell *this, const struct StringVector *args )
{
    bool redirection = false;
    bool fini = false;
    int nb_tokens = string_vector_size( args );
    int i = 1;
    char *text = NULL;
    text = malloc(this->line_length+1);
    strcpy(text,"");
    char *file = "";
    char *tmp;
    while ( i < nb_tokens && !fini )
    {
        tmp = string_vector_get(args, i);
        if (strcmp(tmp,">")==0){
            redirection = true;
        } 
        else if ( !redirection ){
            strcat(text, tmp);
            strcat(text," ");
        }
        else {
            fini = true;
            file = tmp;
        }
        i++;
    }
    if ( !redirection ){
       printf("%s\n",text);
    }
    else if ( fini ){
        FILE* fichier = NULL;
        fichier = fopen(file, "w+");
        if (fichier != NULL)
        {
            fputs(text, fichier);
            fclose(fichier);
        }
    }
    else {
        printf("erreur\n");
    }
    (void)this;
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

/**
 * Méthode qui permet de quitter le shell
 * @param this
 * @param args
*/
static void
do_exit( struct Shell *this, const struct StringVector *args )
{
    this->running = false;
    (void)this;
    (void)args;
}

/**
 * Permet d'afficher le directory passé en paramètre
 * @parma directory
*/
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
                    printf("%s  ", dir->d_name);
                }
            }
            closedir(d);
        }
}

/**
 * Permet d'afficher le chemin jusqu'à notre répertoire
 * @param this
 * @param args
*/
void do_pwd(struct Shell *this, const struct StringVector *args){
    char *buf = getcwd( NULL, 0 );
    printf("%s \n",buf);
    (void)this;
    (void)args;
}

/**
 * Affiche xeyes en premier plan
 * @param this
 * @param args
 */
void do_xeyes(struct Shell *this, const struct StringVector *args){
    int   nb_tokens = string_vector_size( args );
    char *command = string_vector_get(args, 0);
    char *argument[nb_tokens+1];
    for(int i = 0; i<nb_tokens; i++){
        argument[i] = string_vector_get(args, i);
    }
    argument[nb_tokens] = NULL;
    pid_t p = fork();
    if (p == 0) {
        execvp(command, argument);
        exit(EXIT_SUCCESS);
    }
    wait(p);
    
    (void)this;
    (void)args;
}

/**
 * Liste les elements ce trouvant dans notre repertoire
 * @param this
 * @param args
*/
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
    printf("\n");
    (void)this;
}

/*Regroupe toute les actions possibles*/
typedef void ( *Action )( struct Shell *, const struct StringVector * );

static struct {
    const char *name;
    Action      action;
} actions[] = { { .name = "exit", .action = do_exit },     { .name = "cd", .action = do_cd },
                { .name = "rappel", .action = do_rappel }, { .name = "help", .action = do_help },
                { .name = "?", .action = do_help },        { .name = "!", .action = do_system },
                { .name = "xeyes", .action = do_xeyes}  ,  { .name = "ls", .action = do_ls },
                { .name = "echo", .action = do_echo },     { .name = "jobs", .action = do_jobs }, 
                { .name = "pwd", .action = do_pwd },       { .name = "mkdir", .action = do_mkdir},
                { . name = "kill", .action = do_kill},
                { .name = NULL, .action = do_execute }
                };

/**
 * Permet de recuperer l'action à effectuer
 * @param name
*/
Action
get_action( char *name ){
    int i = 0;
    while ( actions[i].name != NULL && strcmp( actions[i].name, name ) != 0 ) {
        i++;
    }
    return actions[i].action;
}

/**
 * Execute une action selon ce que l'utilisateur a entré
 * @param this
*/
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

