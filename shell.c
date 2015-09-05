#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include <sys/wait.h>

//@Author : Anshuman Suri , 2014021

char LEL[100000]; //Maximum length of a word assumed to be 1e5
char* temp[100]; //Maximum number of arguments 
char* tok; //Tokenization 
int i; //Temporary counter

void ctrlC(int signum) //To handle Ctrl+C elegantly 
{
	kill(getpid(),0);
}

int main()
{
	printf("....Potato Shell.....\n"); //Welcome Screen
	signal(SIGINT,ctrlC);
	while(1)
	{
			printf("%s/$: ",getcwd(NULL,0));
			fgets(LEL,100000,stdin); //Read input
			i=0;
			LEL[strlen(LEL)-1]='\0'; //Replace newline with \0
			tok=strtok(LEL," "); //Break by space
			if(tok==NULL) continue; //If blank,ignore (to avoid segmentation fault)
			if(!strcmp("exit",strdup(tok))) break; //exit 
			while(tok!=NULL)
			{
				temp[i++]=strdup(tok);
				tok=strtok(NULL," ");
			}
			temp[i]=NULL;
			if(!strcmp(temp[0],"whoami"))
			{
				printf("%s\n",getlogin());
				continue;
			}
			if(!strcmp(temp[0],"cd")) //If 'cd' command ; checking here otherwise it will change the directory of child process,which is not what we want
			{
				if(temp[1]==NULL || (!strcmp(temp[1],"~")))
				{
					char* yolo=getlogin();
					char *d;
					d=strdup("/home/");
					strcat(d,yolo);
					temp[1]=strdup(d); //To handle the case ehwn only 'cd' or 'cd ~', which should take to user
				}
				int gg=chdir(temp[1]);
				if(gg)
				{
					printf("Cannot access folder : %s\n",temp[1]); //Access right/Invalid folder problem
				}
				continue;
			}
			if(!strcmp(temp[0],"pwd")) //If 'pwd' command ; checking here otherwise it will change the directory of child process,which is not what we want
			{
				printf("%s\n",getcwd(NULL,0));
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
				int tt=0;
				tt=execvp(temp[0],temp); //Execute command
				if(tt) //execvp() returns vale in case of failure
				{
					printf("Sorry, invalid command\n"); //If not in-built
					break; //Read next command
				}
			}
		}
	return 0;
}
