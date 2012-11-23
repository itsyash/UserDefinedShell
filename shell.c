#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include<signal.h>
#include<string.h>
#include<errno.h>
#include<wait.h>
char *user;char dir[100];char path[100];
char hostname[50];int hostsize;
char *my_argv[1000];
char *history[1000];int histindex=0;
void handle_sig(int);
void fill_argv(char *,int *);
void check_function();
void handle_child(int);
typedef struct nod{
	char name[100];
	int pid;
	int active;
}process_store;
int processno=-1;
int pcount=0;
process_store process[100];
int main()
{
	signal(SIGINT, SIG_IGN);
	signal(SIGINT, handle_sig);
	signal(SIGTERM, SIG_IGN);
	signal(SIGTERM, handle_sig);
	signal(SIGCHLD,handle_child);
	hostsize=gethostname(hostname,50);
	user=getenv("USER");
	strcpy(dir,"~");
	getcwd(path,100);
	pid_t pid=fork();
	if(pid==0)
		execlp("/usr/bin/clear","clear",NULL);
	else
		wait(NULL);

	char c = '\0';
	char tmp[10]="";
	printf("<%s@%s:%s>",user,hostname,dir);
	int quit=0;
	while(1) {
		c = getchar();
		if(c==EOF)
		{
			continue;
		}
		if(c == '\n')
		{
			int i=0,count=0;
			if(!strcmp(tmp,""))
			{
				printf("<%s@%s:%s>",user,hostname,dir);
				continue;
			}
			fill_argv(tmp,&count);
			//for(i=0;i<count;i++)
			//	printf("%s\n",my_argv[i]);
			if(!strcmp(my_argv[0],"quit"))
				{quit=1;break;}
			//printf("after parsing %s with count=%d\n",tmp,count);
			check_function();i=0;
			char histcopy[100];
			strcpy(histcopy,my_argv[i++]);
			if(!strcmp(my_argv[0],"ls"))goto label;
			while(my_argv[i]){strcat(histcopy," ");strcat(histcopy,my_argv[i]);i++;};
label:
			history[histindex++]=strdup(histcopy);
			bzero(tmp,sizeof(tmp));
			i=0;
			while(my_argv[i]){my_argv[i]=NULL;i++;}
			printf("<%s@%s:%s>",user,hostname,dir);
		}
		else
		{
			strncat(tmp,&c,1);
			//printf("%s\n",tmp);
		}

	}
	if(quit!=1)printf("\n");int i=0;
	//while(process[i]){free(process[i]);i++;}i=0;
	while(history[i]){free(history[i]);i++;}i=0;
	while(my_argv[i]){free(my_argv[i]);}
	return 0;
}
void handle_sig(int signumber)
{
		printf("\n<%s@%s:%s>",user,hostname,dir);
		fflush(stdout);
}
void handle_child(int signumber)
{
	int pid;
	pid=waitpid(WAIT_ANY,NULL,WNOHANG);
	if(pid!=-1)
	{
		int i=0,index;
		for(i=0;i<processno;i++)
		{
			if(process[i].pid==pid)index=i;
			break;
		}
		process[index].active=0;
		if(pcount==0)
			printf("\n%s %d exited normally\n",process[processno].name,pid);
		printf("<%s@%s:%s>",user,hostname,dir);
		fflush(stdout);
	}
	signal(SIGCHLD,handle_child);
}

void fill_argv(char *tmp_argv,int *count)
{
	char *dummy=tmp_argv;
	int index=0;
	char str[100];
	bzero(str,100);
	while(*dummy !='\0')
	{
		if(*dummy==' ')
		{
			int space=0,i=0,alpha=0;
			//while(str[i++])if(str[i]!=' ')space=space+1;else{alpha=alpha+1;}
			//if(alpha==0 && space>0)
			//{
			//	bzero(str,100);
			//	continue;
			//}
			if(my_argv[index]==NULL)
				my_argv[index]=(char *)malloc(sizeof(char) * strlen(str)+1);
			else
				bzero(my_argv[index],strlen(my_argv[index]));
			strcpy(my_argv[index],str);
			//printf("%s copied\n",my_argv[index]);
			strncat(my_argv[index],"\0",1);
			bzero(str,100);
			*count=*count+1;
			index=index+1;
		}
		else
			strncat(str,dummy,1);
		dummy++;
	}
	if(str[0]!='\0')
	{
		my_argv[index]=(char *)malloc(sizeof(char) * strlen(str)+1);
		strncpy(my_argv[index],str,strlen(str));
		strncat(my_argv[index],"\0",1);
		//printf("%s copied\n",my_argv[index]);
		*count=*count+1;
	}

}
void check_function()
{
	processno++;
	int ind=1;
	//while(my_argv[ind]){strcat(process[processno].name," ");strcat(process[processno].name,my_argv[ind]);ind++;}
	process[processno].active=0;
	//printf("added %s with no %d & pid %d",process[processno].name,processno,process[processno].pid);
	int len=0,nopipes=0;pcount=0;
	while(my_argv[len]!=NULL)
	{
		if(!(strcmp(my_argv[len],"<")) || !(strcmp(my_argv[len],">")) || !(strcmp(my_argv[len],"|")) )
		{
			pcount++;
			if(!strcmp(my_argv[len],"|"))nopipes++;
		}		
		len++;
	}
	if(pcount!=0)
	{
		int noargs=len;int i;
		int pipes[2*nopipes];
		for(i=0;i<2*nopipes;i+=2)
			pipe(pipes+i);
		int status;
		int command;int commandflag=0,pindex=0,pipecount=0,argsindex,j;
		for (i = 0; i < noargs; i++)
		{
			argsindex=0;
			char *myargs[10];
			if(!strcmp(my_argv[i],"|"))
			{
				//printf("| after hist with nopipes=%d pindex=%d i=%d\n",nopipes,pindex,i);
				commandflag=0;
				pipecount++;
			}
			else
			{
				if(commandflag==1)continue;
				commandflag=1;
				j=i;
				while(j < noargs && my_argv[j] && strcmp(my_argv[j],"|") && strcmp(my_argv[j],">") )
				{
					if(j>=2 && !strcmp(my_argv[j-1],"<"))
					{
						if(strcmp(my_argv[j-2],"cat"))
							j++;
					}
					if(j>1 && !strcmp(my_argv[j-1],">"))
					{
							j++;
					}
					if(j<noargs && strcmp(my_argv[j],"<") && strcmp(my_argv[j],">"))
						myargs[argsindex++]=strdup(my_argv[j]);
					j++;
				}
				myargs[argsindex++]=NULL;
				int ind=0,k=0;
				//strcpy(process[processno].name,myargs[ind++]);
				//while(my_argv[ind]){strcat(process[processno].name," ");strcat(process[processno].name,my_argv[ind]);ind++;}
				//for(k=0;k<argsindex;k++)
				//	if(myargs[k] && printf("%s\n",myargs[k]));
				
				if(pipecount==0)
				{
					strcpy(process[processno].name,myargs[0]);
					if( !strcmp(myargs[i],"hist"))
					{
						FILE *fp=fopen("histrecord","w");
						int r=0;
						for(r=0;r< histindex;r++)
							fprintf(fp,"%s\n",history[r]);
						fclose(fp);
						FILE *file=fopen("histrecord","r");
						char line[1001];
						int n5=read(fileno(file),line,1001);
						write(pipes[pindex+1],line,n5);
						fclose(file);
						//close(pipes[pindex]);
						//close(pipes[pindex+1]);
						remove("histrecord");
					}
					else
					{
						//while(my_argv[ind]){strcat(process[processno].name," ");strcat(process[processno].name,my_argv[ind]);ind++;}
						pid_t pid;
						pid=fork();
						if(pid==-1)
						{
							fprintf(stderr,"error in creating fork %d\n",errno);
							exit(-1);
						}
						else if(pid==0)
						{
							dup2(pipes[pindex+1],1);
							if(!strcmp(my_argv[i+1],"<"))
							{
								FILE *fp=fopen(my_argv[i+2],"r");
								if(fp==NULL){printf("file doesn't exists\n");return;}
								dup2(fileno(fp),0);
								fclose(fp);
							}
							if(!strcmp(my_argv[noargs-2],">"))
							{
								FILE *fp=fopen(my_argv[noargs-1],"a+");
								dup2(fileno(fp),1);
								fclose(fp);
							}
							for(j=0;j<2*nopipes;j++)
								close(pipes[j]);
							processno++;
							execvp(*myargs,myargs);
							exit(0);
						}
						else
						{ process[processno].pid=getpid();processno++;}
					}
				}
				else if(pipecount==nopipes)
				{
					//printf("wc yy\n");
					strcpy(process[processno].name,myargs[0]);
					pid_t pid;
					pid=fork();
					if(pid==-1)
					{
						fprintf(stderr,"error in creating fork %d\n",errno);
						exit(-1);
					}
					else if(pid==0)
					{
						dup2(pipes[pindex-2],0);
						if(!strcmp(my_argv[noargs-2],">"))
						{
							FILE *fp=fopen(my_argv[noargs-1],"a+");
							dup2(fileno(fp),1);
							fclose(fp);
						}
						for(j=0;j<2*nopipes;j++)
							close(pipes[j]);
						execvp(*myargs,myargs);
						exit(0);
					}
					else
					{ process[processno].pid=getpid();}
				}
				else
				{
					strcpy(process[processno].name,myargs[0]);
					pid_t pid;
					pid=fork();
					if(pid==-1)
					{
						fprintf(stderr,"error in creating fork %d\n",errno);
						exit(-1);
					}
					else if(pid==0)
					{
						dup2(pipes[pindex-2],0);
						dup2(pipes[pindex+1],1);
						for(j=0;j<2*nopipes;j++)
							close(pipes[j]);
						execvp(*myargs,myargs);
						exit(0);
					}
					else
					{ process[processno].pid=getpid();processno++;}
				}
				pindex+=2;
			}
		}
		for(j=0;j<2*nopipes;j++)
			close(pipes[j]);
		for(j=0;j<nopipes+1;j++)
			wait(&status);
		fflush(stdout);
	}
	else
	{
		strcpy(process[processno].name,my_argv[0]);
		if(strcmp(my_argv[0],"cd")==0)
		{
			process[processno].pid=getpid();	
			if(len==1 || (len==2 && strcmp(my_argv[1],"~")==0))
			{
				strcpy(dir,"~");
				getcwd(path,100);
				chdir(path);
				//printf("path=%s ",path);
			}
			else if(len==2)
			{
				if(strcmp(my_argv[1],".")==0){}
				else if(strcmp(my_argv[1],"..")==0)
				{
					int i;
					for(i=strlen(dir)-1;i>=0;i--)
						if(dir[i]=='/')
							break;
					dir[i]='\0';
					for(i=strlen(path)-1;i>=0;i--)
						if(path[i]=='/')
							break;
					path[i]='\0';
				}
				else 
				{
					strcat(dir,"/");
					strcat(dir,my_argv[1]);
					strcat(path,"/");
					strcat(path,my_argv[1]);
				}
				chdir(path);
				//printf("path=%s ",path);
			}
		}
		else if(len==1)
		{
			if(strcmp(my_argv[0],"pid")==0)
			{
				int p;
				p=getpid();
				printf("command name: ./a.out process id: %d\n",p);
				process[processno].pid=p;
			}
			else if(my_argv[0][0]=='h' || my_argv[0][0]=='!')
			{
				process[processno].pid=getpid();
				if(strcmp(my_argv[0],"hist")==0)
				{
					int i=0;
					for(i=0;i<histindex;i++)
						printf("%s\n",history[i]);
				}
				else if(my_argv[0][0]=='!')
				{
					//printf("!\n & no=%d",my_argv[0][5]-48);
					int number=my_argv[0][5]-48-1;
					printf("%s\n",history[number]);
				}
				else
				{
					//fprintf(stdout,"histno & n=%d\n",my_argv[0][4]-48);
					int i=0;int number=histindex-(my_argv[0][4]-48);
					for(i=number;i<histindex;i++)
						printf("%s\n",history[i]);
				}
			}
			else if(strcmp(my_argv[0],"quit")==0)
			{
				exit(0);
			}
			else
			{
				if(strcmp(my_argv[0],"ls")==0)
				{
					my_argv[1]=strdup(path);
					//printf("ls with path=%s",path);
				}
				pid_t mypid;
				mypid=fork();
				if(mypid==-1)
				{
					fprintf(stderr,"error creating fork %d\n",errno);
				}
				else if(mypid==0)
				{
					if(execvp(*my_argv,my_argv));
					{
						//printf("error\n");
						char *err;
						perror(err);
						_exit(0);
					}
				//	fflush(stdout);
				//	my_argv[1]=NULL;
				}
				else
				{
					process[processno].pid=getpid();
					wait(NULL);
				}
			}
		}

		else if(len==2)
		{
			if(strcmp(my_argv[1],"&")==0)
			{
				//printf("background with %s ",my_argv[0]);
				my_argv[1]=NULL;
				process[processno].active=1;
				pid_t pid1;
				pid1=fork();
				if(pid1==-1)
				{
					fprintf(stderr,"error in creating fork %d\n",errno);
					exit(-1);
				}
				else if(pid1==0)
				{
					printf("%s command %d pid\n",my_argv[0],getpid());
					printf("<%s@%s:%s>",user,hostname,dir);
					fflush(stdout);
					execvp(*my_argv,my_argv);
				}
				else
				{
					process[processno].pid=getpid();
				}
			}
			else if(strcmp(my_argv[0],"pid")==0 && strcmp(my_argv[1],"current")==0)
			{
				process[processno].pid=getpid();
				printf(" List of currently executing processes spawned from this shell:\n");
				int i=0;
				for(i=0;i<processno;i++)
				{
					if(process[i].active==1)
						printf("command name:%s process id:%d\n",process[i].name,process[i].pid);
				}
			}
			else if(strcmp(my_argv[0],"pid")==0 && strcmp(my_argv[1],"all")==0)
			{
				process[processno].pid=getpid();
				printf("List of all processes spawned from this shell:\n");
				int i=0;
				for(i=0;i<processno;i++)
					printf("command name:%s process id:%d\n",process[i].name,process[i].pid);
			}
			else if(strcmp(my_argv[0],"ls")==0)
			{
				if(strcmp(my_argv[1],"~")==0)getcwd(path,100);
				strcpy(my_argv[1],path);
				pid_t mypid;
				mypid=fork();
				if(mypid==-1)
				{
					fprintf(stderr,"error creating fork %d\n",errno);
				}
				else if(mypid==0)
				{
					if(execvp(*my_argv,my_argv));
					{
						char *err;
						perror(err);
						_exit(0);
					}
					fflush(stdout);
				}
				else
				{
					process[processno].pid=getpid();
					wait(NULL);
				}
			}
			else
			{
				pid_t mypid;
				mypid=fork();
				if(mypid==-1)
				{
					fprintf(stderr,"error creating fork %d\n",errno);
				}
				else if(mypid==0)
				{
					if(execvp(*my_argv,my_argv));
					{
						char *err;
						perror(err);
						_exit(0);
					}
					fflush(stdout);
				}
				else
				{
					process[processno].pid=getpid();
					wait(NULL);

				}	
			}
		}
	}

}



