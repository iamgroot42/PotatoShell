#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include <sys/wait.h>
#include <fcntl.h>
#include <sys/types.h>
#include <sys/stat.h>

//@Author : Anshuman Suri , 2014021

char LEL[100000]; //Maximum length of a word assumed to be 1e5
char* temp[100]; //Maximum number of arguments 
char* redir[100]; //Maximum number of arguments  , contains without < and >
char* tok; //Tokenization 
int i; //Temporary counter
static char* cirq[300]; //For 'history'
static int front=0,rear=-1; //For circular queue

void ctrlC(int signum) //To handle Ctrl+C "elegantly" 
{
	kill(getpid(),0); 
}

void insert(char* x) //Insert() function for cirxular queue() 
{
    if((front==0&&rear==299)||(front>0&&rear==front-1))
        return;
    else
    {
        if(rear==299&&front>0)
        {
            rear=0;
            cirq[rear]=strdup(x);
        }
        else
        {
            if((front==0&&rear==-1)||(rear!=front-1))
                cirq[++rear]=strdup(x);
        }
    }
}

void display() //Display() function for circular queue
{
    int I,J;
    int iterator;
    iterator=1;
    if(front==0&&rear==-1)
    {
        return ;
    }
    if(front>rear)
    {
        for(I=0;I<=rear;I++,iterator++)
            printf(" [%d] %s\n",iterator,cirq[I]);
        for(J=front;J<=299;J++,iterator++)
            printf(" [%d] %s\n",iterator,cirq[J]);
    }
    else
    {
        for(I=front;I<=rear;I++,iterator++)
        {
            printf(" [%d] %s\n",iterator,cirq[I]);
        }
    }
}

void help() //Lists all custom commands
{
	printf("cd [directory] : goes to given directory ;  directory path can be absolute or relative\n");
	printf("help : lists all custom commands\n");
	printf("history : lists all commands entered in shell (successful and unsuccessful)\n");
	printf("exit : to exit Potato Shell\n");
	printf("< : redirect input from a file into process\n");
	printf("> : redirect output of process to a file\n");
	printf("clear : clear the visible area of shell\n");
	printf("Ctrl+C (^C) : stop the current process ; doesn't do anything if no ongoing process\n");
	printf(" | : single level pipe-lining (without any file I/O redirection,cd in between\n");
}

//This function raken from Internet (with modifications)
void loop_pipe(char *a,char* b) 
{
  int   p[2];
  pid_t pid;
  int   fd_in = 0;
  char*** cmd;
  cmd=(char***)malloc(sizeof(cmd)*3);
  char *tok1,*tok2;
  int i,j;
  char* tempo[100];
  char* tempo2[100];
  i=0;j=0;
  tok1=strtok(a," ");
  while(tok1!=NULL) 
  {
	tempo[i++]=strdup(tok1);
	tok1=strtok(NULL," ");
   }
  tempo[i]=NULL;
  j=0;
  tok2=strtok(b," ");
  while(tok2!=NULL)
  {
  	tempo2[j++]=strdup(tok2);
  	tok2=strtok(NULL," ");
  }
  tempo2[j]=NULL;
  cmd[0]=tempo;
  cmd[1]=tempo2;
  cmd[2]=NULL;
  while (*cmd != NULL)
    {
      pipe(p);
      if ((pid = fork()) == -1)
        {
          exit(EXIT_FAILURE);
        }
      else if (pid == 0)
        {
          dup2(fd_in, 0); //change the input according to the old one 
          if (*(cmd + 1) != NULL)
            dup2(p[1], 1);
          close(p[0]);
          if(!strcmp((*cmd)[0],"history")) //'history' command 
		  {
			display();
			exit(EXIT_FAILURE);
		  }
		  else if(!strcmp((*cmd)[0],"help")) //'help' command
		  {
			help();
			exit(EXIT_FAILURE);
		  }
          execvp((*cmd)[0], *cmd);
          exit(EXIT_FAILURE);
        }
      else
        {
          wait(NULL);
          close(p[1]);
          fd_in = p[0]; //save the input for the next command
          cmd++;
        }
    }
}

int main()
{
	printf("....Potato Shell.....\n"); //Welcome Screen
	signal(SIGINT,ctrlC); //Readying to handle Ctrl+C interrupt
	while(1)
	{
		printf("%s/$$: ",getcwd(NULL,0));
		fgets(LEL,100000,stdin); //Read input
		i=0;
		LEL[strlen(LEL)-1]='\0'; //Replace newline with \0
		if(strstr(LEL,"|")!=NULL)
		{
			char *x,*y;
			insert(LEL); //Insert to history, add only non-empty commands
			tok=strtok(LEL,"|");
			x=strdup(tok);
			tok=strtok(NULL,"|");
			if(tok==NULL)
			{
				printf("Second argument of pipe missing.\n");
				continue;
			}
			y=strdup(tok); 
			// printf("Passing2 %s\n",y);
			loop_pipe(x,y);
			continue;
		}
		tok=strtok(LEL," "); //Break by space
		if(tok==NULL) continue; //If blank,ignore (to avoid segmentation fault)
		if(!strcmp("exit",strdup(tok))) break; //exit 
		insert(LEL); //Insert to history, add only non=empty commands
		while(tok!=NULL) 
		{
			temp[i++]=strdup(tok);
			tok=strtok(NULL," ");
		}
		temp[i]=NULL;
		if(!strcmp(temp[0],"cd")) //If 'cd' command ; checking here otherwise it will change the directory of child process,which is not what we want
		{
			if((temp[1]==NULL) || (temp[1][0]=='~'))
			{
				char* yolo=getlogin();
				char *d=strdup("/home/");
				strcat(d,yolo);
				char *cooking=(char*)malloc(sizeof(cooking)*strlen(temp[1]));
				strncpy(cooking,temp[1]+1,strlen(temp[1])-1);
				strcat(d,cooking);
				temp[1]=strdup(d); //To handle the case when only 'cd' or 'cd ~', which should take to /home/user
			}
			int gg=chdir(temp[1]);
			if(gg)
			{
				printf("Cannot access folder : %s\n",temp[1]); //Access right/Invalid folder problem
			}
			continue;
		}	
		int t=fork();
		if(t<0)
		{
			printf("Fatal Error"); //Fork failed
		}
		else if(t>0)
		{
			wait(NULL); //Wait for child process
		}
		else
		{
			//Take care of I/O redirection :
			int tt=0; 
			int k,l;
			char* redir[100]; //Extra array to store just commands
			l=0;
			for(k=0;k<i;k++)
			{
				if(!strcmp(temp[k],"<"))
				{
					if(k<i-1)
					{
						int in;
						in=open(temp[k+1],O_RDONLY); //Set input file as I/O for process
						if(in>0)
						{
							dup2(in, 0);
							close(in);	
							k++; //Exclude I/O file from execvp()
						}
						else
						{
							printf("Error opening file for input\n");
							goto done; //Failed.Read next command
						}
					}
					else
					{
						printf("Provide input file\n");
						goto done; //Failed.Read next command						
					}
				}
				else if(!strcmp(temp[k],">"))
				{
					if(k<i-1)
					{
						int out;
						out=open(temp[k+1],O_WRONLY | O_TRUNC | O_CREAT, S_IRUSR | S_IRGRP | S_IWGRP | S_IWUSR); //Set output file as I/O for process
						if(out>0)
						{
							dup2(out,1);
							close(out);
							k++; //Exclude I/O file from execvp()
						}
						else
						{
							printf("Error opening/creating file for output\n");
							goto done; //Failed.Read next command							
						}
					}
					else
					{
						printf("Provide output file\n");
						goto done; //Failed.Read next command
					}
				}
				else
				{
					redir[l]=strdup(temp[k]); //Copy non I/O commands
					l++;
				}
			}
			redir[l]=NULL;
			if(!strcmp(redir[0],"history")) //'history' command 
			{
				display();
				break;
			}
			else if(!strcmp(redir[0],"help")) //'help' command
			{
				help();
				break;
			}
			exec:
			tt=execvp(redir[0],redir); //Execute command 
			if(tt) //execvp() returns vale in case of failure
			{
				printf("Sorry, invalid command\n"); //If not in-built
				done:
				break; //Read next command
			}
		}
	}
	return 0;
}
