/*
 * toysh.c
 *
 * a simple shell program that parses the path
 * variable and executes the program passed by
 * the user
 *
 * Carick Wienke
 *
 * 2011.3.31
 */

#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <unistd.h>
#include <sys/wait.h>

#include "toysh.h"

char* lookup_path( char **, char[][MAX_PATH_LEN], int );
void parse_cmd( char *, command_t * );
int parse_path( char[][MAX_PATH_LEN] );
void print_prompt();
void read_cmd( char* );
void cleanup_cmd( command_t* );

   /* 
    * run the program
    */
int main()
{
      // holds the directories in the path
   char path_dirs[ MAX_PATHS ][ MAX_PATH_LEN ];
      
   int num_dirs = parse_path( path_dirs );
   
   char line[ LINE_LEN ];
   command_t cmd;

      // get input
   print_prompt();
   read_cmd( line );
   parse_cmd( line, &cmd );

      // until we get a line that starts with exit or quit
   while( !cmd.argv[0] || 
          ( strcmp( cmd.argv[0], "quit" ) && 
            strcmp( cmd.argv[0], "exit" ) ) )
   {
      if( cmd.argv[0] )
         cmd.name = lookup_path( cmd.argv, path_dirs, num_dirs );

      if( cmd.name && cmd.argc > 0 )
      {
            // create the child
         pid_t child_pid = fork();
         if( child_pid < 0 )
            perror( "fork" );
         else if( child_pid == 0 ) // child
            execv( cmd.name, cmd.argv );
         else  // parent
         {
            if( !cmd.concurrent )
            {
               int status;
               waitpid( child_pid, &status, 0 );
            }
         }
      }

      cleanup_cmd( &cmd );
         
         // get input
      print_prompt();
      read_cmd( line );
      parse_cmd( line, &cmd );
   }

   return 0;
}

   /*
    * parse the path environment variable into
    * the different paths. it stores them in the
    * preallocated 2d array passed and returns the 
    * number of directories
    */
int parse_path( char dirs[][MAX_PATH_LEN] )
{
   char *path_env = getenv( "PATH" );

   int i = 0;
   char *single_path = strtok( path_env, ":" );
   while( single_path )
   {
      strncpy( dirs[i], single_path, MAX_PATH_LEN );
      single_path = strtok( NULL, ":" );
      ++i;
      if( i >= MAX_PATHS )
         break;
   }

   return i;
}

   /*
    * print the user prompt.
    * this case it is just the simple %
    */
void print_prompt()
{
   printf( "%% " );
}

   /*
    * get the command from the user 
    * and saves it in the string passed
    */
void read_cmd( char* buffer )
{
   if( buffer )
      fgets( buffer, LINE_LEN, stdin );
}

   /*
    * break the command up into tokens and 
    * fill command_t except for the absolute
    * path 'name'
    */
void parse_cmd( char *line, command_t *cmd )
{
   char* tok = strtok( line, " \t\n" );
   int i = 0;
   while( tok )
   {
      cmd->argv[i] =
         (char*) malloc( sizeof(char) * strlen(tok) + 1 );
      strcpy( cmd->argv[i], tok );

      tok = strtok( NULL, " \t\n" );
      ++i;

      if( i >= MAX_ARGS - 1 )
         break;
   }
   cmd->argv[i] = NULL;
   cmd->argc = i;

   if( cmd->argc > 0 )
   {
      char* last_arg = cmd->argv[ cmd->argc - 1 ];
      int len = strlen( last_arg );
      if( last_arg[ len - 1 ] == '&' )
      {
         last_arg[ len - 1 ] = '\0';
         if( strlen( last_arg ) == 0 )
         {
            free( last_arg );
            cmd->argv[ cmd->argc - 1] = NULL;
         }
         cmd->concurrent = 1;
      }
      else
         cmd->concurrent = 0;
   }
}

   /*
    * determine the which directory in path has the
    * executable and returns the absolute path name
    */
char* lookup_path( char *argv[], char dir[][MAX_PATH_LEN], int n_dir )
{
   if( argv[0][0] == '/' )
      return argv[0];
   else
   {
      char *name = (char*) malloc( sizeof(char) * MAX_PATH_LEN );

      int i;
      for( i = 0; i < n_dir; ++i )
      {
         strncpy( name, dir[i], MAX_PATH_LEN );
         strcat( name, "/" );
         strcat( name, argv[0] );
         if( !access( name, F_OK ) )
            return name;
      }

      printf( "%s: command not found.\n", argv[0] );
      return NULL;
   }
}

   /*
    * frees the allocated memory in the
    * command passed
    */
void cleanup_cmd( command_t * cmd )
{
   if( cmd->name )
      free( cmd->name );
   
   int i;
   for( i = 0; i < MAX_ARGS; ++i )
   {
      if( cmd->argv[i] )
      {
         free( cmd->argv[i] );
         cmd->argv[i] = NULL;
      }
   }
}
