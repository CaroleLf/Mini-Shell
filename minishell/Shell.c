#include <stdio.h>
#include <sys/types.h>
#include <string.h>
#include "Shell.h"

void
shell_init( struct Shell *s )
{
    s->running     = false;
    s->line_number = 0;
    s->buffer_size = 16;
    s->buffer      = malloc( s->buffer_size * sizeof( char ) );
}

void
shell_run( struct Shell *s )
{
    s->running = true;
    printf("Shell is running !\n");
    while(s->running){
        shell_prompt( s );
        shell_read_line( s );
        shell_execute_line( s );
    }
    printf("Shell stopped\n");
}

void
shell_free( struct Shell *s )
{
    free( s->buffer );
}

void
shell_read_line( struct Shell *s )
{
    // man 3 getline
    ssize_t linelen;
    linelen = getline( &( s->buffer ), &( s->buffer_size ), stdin );
}

void
shell_execute_line( struct Shell *s )
{
    if ( strcmp(s->buffer,"exit") == 0 )
    {
        s->running = false;
    } else if ( strcmp(s->buffer,"help") == 0 )
    {
        printf("tapez exit pour arrếter\n");
    } else {
        printf("commande inconnue\n");
    }
}

void shell_prompt( struct Shell *s ){
    printf("$%d ",s->line_number);
    s->line_number ++;
}


