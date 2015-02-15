#ifndef __INTERPRETER_H__
#define __INTERPRETER_H__

#include <stdio.h>
#include <string.h>
#include "ST7735.h"

#define COMMAND(NAME) { #NAME, Command_ ## NAME }
#define DECL_COMMAND(NAME) int Command_ ## NAME ## ( char* );

// #define DEF_COMMAND(NAME) int Command_ ## NAME ## ( char * param_str ) { \
//   char id[16]; \
//   char args[64]; \
//   const char* ptr = param_str; \
//   sscanf(ptr, "%s %63s", id, args); \

int Command_disp_message(char * args);
int Command_list(char * args);


struct command
{
  char *label;
  int (*function) (char *);
};

struct command commands[] =
{
  COMMAND(list),
  COMMAND(disp_message)
  //COMMAND(not_found)
};



/*
 Want support for get, set, route, and list
*/

void interpreter(void){
  char buffer[64];
  char m_command[16];
  char *buff_ptr = buffer;
  struct command* command_ptr = commands;
  struct command* end_ptr = command_ptr + sizeof(commands)/sizeof(commands[0]);
  int (*function)(char *);
	int n = 0;
  printf(">> ");
  //scanf("%s\n", buffer); //, buffer);
	// fgets(buffer, 64, stdin);
  while(n < 64){
    int c = fgetc(stdin);
    if(c == EOF){
      buffer[n] = '\0';
      break;
    }

    if(c == 8) { //Backspace
      n--;
    }
    else{
      buffer[n] = (char)c;
      n++;
    }
  }
	sscanf(buff_ptr, "%s%n", m_command, &n);
	printf("m_command: %s\n", m_command);
	buff_ptr += n;
	printf("Buffer: %s\n", buff_ptr);
  while (command_ptr < end_ptr){
    function = command_ptr->function;
    if(strcmp(m_command, command_ptr->label) == 0){
			printf("Doing Command: %s\n", command_ptr->label);
      function(buff_ptr);
			
      return;
    }

    command_ptr++;
  }
  printf("[Warning] Interpreter Command not found\n");
  return;
}


// ====================================================
int Command_list(char * args)
{
  struct command* command_ptr = commands;
  struct command* end_ptr = command_ptr + sizeof(commands)/sizeof(commands[0]);

  while(command_ptr < end_ptr){
    printf("%s\n", command_ptr->label);
    command_ptr++;
  }
  return 0;
}

int Command_disp_message(char * args)
{
  //printf("Not implemented yet\n");
  int disp, line, n;
  char str[64];
  sscanf(args, "%d %d %[^\t\r\n]", &disp, &line, str);
	//str = args + n;
	printf("\n Got disp=%d\tline=%d, %s\n", disp, line, str);
  ST7735_Message(disp, line, str, 0);    

  return -1;
}


#endif /* __INTERPRETER_H__ */
