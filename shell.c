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
char* tok; //Tokenization 
int i; //Temporary counter
static char* cirq[300]; //For 'history'
static int front=0,rear=-1; //For circular queue

void ctrlC(int signum) //To handle Ctrl+C elegantly 
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
    if(front==0&&rear==-1)
    {
        return ;
    }
    if(front>rear)
    {
        for(I=0;I<=rear;I++)
            printf("%s\n",cirq[I]);
        for(J=front;J<=299;J++)
            printf("%s\n",cirq[J]);
    }
    else
    {
        for(I=front;I<=rear;I++)
        {
            printf("%s\n",cirq[I]);
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
			int fraud=0; //Indicates presence/absence of I/O redirection
			LEL[strlen(LEL)-1]='\0'; //Replace newline with \0
			insert(LEL); //Insert to history
			tok=strtok(LEL," "); //Break by space
			if(tok==NULL) continue; //If blank,ignore (to avoid segmentation fault)
			if(!strcmp("exit",strdup(tok))) break; //exit 
			while(tok!=NULL) 
			{
				temp[i++]=strdup(tok);
				tok=strtok(NULL," ");
			}
			temp[i]=NULL;
			if(!strcmp(temp[0],"history")) //'history' command
			{
				display();
				continue;
			}
			if(!strcmp(temp[0],"cd")) //If 'cd' command ; checking here otherwise it will change the directory of child process,which is not what we want
			{
				if((temp[1]==NULL) || (temp[1][0]=='~'))
				{
					char* yolo=getlogin();
					char *d;
					d=strdup("/home/");
					strcat(d,yolo);
					char* cooking=temp[1]+1;
					strcat(d,cooking);
					temp[1]=strdup(d); //To handle the case ehwn only 'cd' or 'cd ~', which should take to /home/user
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
				int tt=0;
				int k,l;
				char* redir[100]; //Extra array to store commands without redirection
				l=0;
				for(k=0;k<i;k++)
				{
					if(!strcmp(temp[k],"<"))
					{
						if(k<i-1)
						{
							int in;
							fraud=1;
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
							fraud=1;
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
				redir[l]=NULL; //Set last as NULL
				exec:
				if(!fraud) tt=execvp(temp[0],temp); //Execute command
				else tt=execvp(redir[0],redir); //Execute command with redirected I/O
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
