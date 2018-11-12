#include<stdio.h>
#include<string.h>
#include<stdlib.h>
#include<unistd.h>
#include<sys/types.h>
#include<sys/wait.h>
#include<readline/readline.h>
#include<readline/history.h>
#include<fcntl.h>



char alias_name[50][50];
char actual_cmd[50][50];
int alias_index=0;
int STDIN_FILENO_COPY=10;
int STDOUT_FILENO_COPY=11;

//extern char **environ;
int input(char* str)// Function to take input
{
    char* buf;
    buf = readline("mmp$ ");
    if (strlen(buf) != 0)
    {
        strcpy(str, buf);
        return 0;
    }
    else
        return 1;

}

void printDir()// Function to print Current Directory.
{
    char cwd[1024];
    getcwd(cwd, sizeof(cwd));
    printf("%s", cwd);
}

void execArgs(char** parsed)// Function where the system command is executed
{
    pid_t pid = fork();// Forking a child
    if (pid == -1)
    {
        printf("\nFailed forking child..");
        return;
    }
    else if (pid == 0)
    {
        if (execvp(parsed[0], parsed) < 0)
            printf("\nCould not execute command..");
        printf("\n");
        exit(0);
    }
    else
    {
        wait(NULL);// waiting for child to terminate
        return;
    }
}

void execArgsPiped(char ***parsedpipearray,int numofpipes)
{
     // 0 is read end, numofpipes is write end
    int *pipefd=(int*)malloc(sizeof(int)*2*numofpipes),j,i;
    pid_t *p=(int*)malloc(sizeof(int)*numofpipes);


    for(i=0;i<numofpipes;i++)
    {
      if (pipe(pipefd+2*i) < 0)
      {
          printf("\nPipe could not be initialized");
          return;
      }
    }
    for(i=0;i<numofpipes;i++)
		{

				p[i]=fork();
				if(p[i]<0)
				{
		        printf("\nCould not fork");
		        return;
		    }
				if (p[i] == 0)
				{
				    // Child 1 executing..
				    // It only needs to write at the write end
						if(i!=0)
				    	dup2(pipefd[(i-1)*2], STDIN_FILENO);
						if(i!=numofpipes-1)
							dup2(pipefd[i*2+1], STDOUT_FILENO);
				    for(j=0;j<2*numofpipes;j++)
              close(pipefd[j]);
				    if (execvp(parsedpipearray[i][0], parsedpipearray[i]) < 0)
				    {
				        printf("\nCould not execute command 1..");
				        exit(0);
				    }
				}
		}
    for(j=0;j<2*numofpipes;j++)
      close(pipefd[j]);
		for(i=0;i<numofpipes;i++)
      wait(NULL);
}


int command(char** parsed)// Function to execute builtin commands
{
    int cmds = 11, i, s_i = 0;
    char* ListOfCmds[cmds];
    char* username;

    ListOfCmds[0] = "exit";
    ListOfCmds[1] = "cd";
    ListOfCmds[2] = "info";
    ListOfCmds[3] = "dateu";
    ListOfCmds[4] = "day";
    ListOfCmds[5] = "dus";
    ListOfCmds[6] = "history";
    ListOfCmds[7] = "alias";
    ListOfCmds[8] ="wcs";
    ListOfCmds[9] =
    ListOfCmds[10]="editor";

    for (i = 0; i < cmds; i++)
    {
        if (strcmp(parsed[0], ListOfCmds[i]) == 0)
        {
            s_i = i + 1;
            break;
        }
    }

    switch (s_i)
    {
      case 1:
          printf("\nmmp-shell terminated successfully\n");
          exit(0);
      case 2:
          chdir(parsed[1]);
          return 1;
      case 3:
          username = getenv("USER");
          printf("\nUser : %s.\n",  username);
          return 1;
      case 4:
	        parsed[0]="date";
          parsed[1]="-u";
	        parsed[2]=0;
          execArgs(parsed);
	        return 1;
      case 5:
          parsed[0]="date";
          parsed[1]="+%A";
          parsed[2]=0;
          execArgs(parsed);
          return 1;
      case 6:
          parsed[0]="du";
          parsed[1]="-s";
          parsed[2]=0;
          execArgs(parsed);
          return 1;
      case 7:
          parsed[0]="cat";
          parsed[1]="history.txt";
          parsed[2]=0;
          execArgs(parsed);
          return 1;
      case 8:
          strcpy(alias_name[alias_index],parsed[1]);
          strcpy(actual_cmd[alias_index++],parsed[2]);
          return 1;
      case 9:

      case 10:
      case 11:

      default:
          for(i=0;i<alias_index;i++)
          {
            if(strcmp(parsed[0],alias_name[i])==0)
            {
              strcpy(parsed[0],actual_cmd[i]);
              break;
            }
          }
          break;
    }
    return 0;
}

int parsePipe(char* str, char** strpiped)
{
    int i;
    for (i = 0; ; i++)
    {
					strpiped[i] = strsep(&str, "|");
	        if (strpiped[i] == NULL)
	            break;
    }
    return i-1;
}

void parse(char* str, char** parsed)// function for parsing command words
{
    int i;
    for (i = 0; i < 100; i++)
    {
        parsed[i] = strsep(&str," ");
        if (parsed[i] == NULL)
            break;
        if (strlen(parsed[i]) == 0)
            i--;
    }
}

void redirection(char *temp)
{
    char *str1 = (char*)malloc(50*sizeof(char));
    char *str2 = (char*)malloc(50*sizeof(char));
    strcpy(str1,temp);
    strcpy(str2,temp);
    char *inputredir = (char*)malloc(50*sizeof(char));
    char *outputredir = (char*)malloc(50*sizeof(char));
    int input , output;


    inputredir = strsep(&str1,"<");
    inputredir = strsep(&str1,"<");
    inputredir = strsep(&inputredir,">");
    if(inputredir!=NULL)
    {

        while (inputredir[0]==' ')
            inputredir++;
        input = open(inputredir,O_RDONLY);
        dup2(input,STDIN_FILENO);
        close(input);
    }

    outputredir = strsep(&str2,">");
    outputredir = strsep(&str2,">");
    outputredir = strsep(&outputredir,"<");

    if(outputredir!=NULL)
    {
        while (outputredir[0]==' ')
           outputredir++;
        output = open(outputredir,O_WRONLY | O_CREAT | O_TRUNC);
        dup2(output,STDOUT_FILENO);
        close(output);
    }
}

int processString(char* str, char** parsed, char ***parsedpipe)// function to process the input string
{
    char* strpiped[100];
    int piped = 0;
    char *temp1=(char *)malloc(50 *sizeof(char));
    strcpy(temp1,str);
    temp1=strsep(&temp1,"*");
    if(strcmp(temp1,str)!=0)
    {
      system(str);
      return 0;

    }
    if(strcmp(str,"ls -z")==0)
      strcpy(str,"ls -l | grep \\s0\\s");
        
    if(strcmp(str,"ps -z")==0)
     strcpy(str,"ps | grep -w defunct");
 	if(parsed[0]=="editor")
 	{
 		command(parsed);
 		return 0;
 	}


    redirection(str);
    str=strsep(&str,"<");
    str=strsep(&str,">");

    piped = parsePipe(str, strpiped);
    if (piped)
    {
				int i=0;
				while(i<=piped)
				{
					parse(strpiped[i],parsedpipe[i]);
					i++;
				}
    }
    else
    	  parse(str, parsed);
    if (command(parsed))
        return 0;
    else
        return 1 + piped;

}
/*void historyfunc()
{
	int h;
        for(h=0;h<10;h++)
	{
		printf("%d %s \n",h+1,history[h]);
		if(history[h] == NULL)
			break;
	}
        return 0;
}*/
void wildcard(char *temp)
{
  int rv;int flag;
  regex_t exp; //Our compiled expression
  rv = regcomp(&exp, "[a-z]+[a-z]*[0-9]*\.[a-z]+", REG_EXTENDED);  
  if (rv != 0)
        flag=0;
  
  
  if(match(&exp, temp)==1)
      //matched
  
  regfree(&exp);
  return 0;

}

int match(regex_t *pexp, char *sz)
{
  int flag;
  regmatch_t matches[1]; 
  if (regexec(pexp, sz, 1, matches, 0) == 0)
    flag=1;
   
   else 
    
    flag=0;
  
  return flag;
}


int main()
{
    char inputString[100], *parsedArgs[100], ***multiPiped;
		int i;
		multiPiped = (char***)malloc(sizeof(char**)*100);
		for(i=0;i<100;i++)
			multiPiped[i] = (char**)malloc(sizeof(char*)*100);
    int execFlag = 0;
    if(setenv("SHELL","/mnt/d/My Files/6th Sem/Unix System Programming/Projects/myshell",1)<0)
    {
      printf("Can't set env\n");
      exit(0);
    }
    printf("\033[H\033[J");
    printf("Hello\n");
    FILE *fp;

    dup2(STDIN_FILENO,STDIN_FILENO_COPY);
    //close(STDIN_FILENO);
    dup2(STDOUT_FILENO,STDOUT_FILENO_COPY);
    //close(STDOUT_FILENO);
    

    while(1)
    {
        dup2(STDIN_FILENO_COPY,STDIN_FILENO);
        dup2(STDOUT_FILENO_COPY,STDOUT_FILENO);
        fp=fopen("history.txt","a");

        if (input(inputString))// take input
            continue;
        fprintf(fp,"%s \n",inputString);
      	fclose(fp);
        execFlag = processString(inputString,parsedArgs,multiPiped);// process

        // execflag returns zero if there is no command or it is a builtin command, 1 if it is a simple command or 2 id it is a piped command
        if (execFlag == 1)// execute
            execArgs(parsedArgs);
	      if (execFlag > 1)
            execArgsPiped(multiPiped,execFlag);
    }


    return 0;
}
