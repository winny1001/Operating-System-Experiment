#include <stdio.h>
#include <unistd.h>
#include <signal.h>
#include <stdlib.h>
#include <wait.h>

int main()
{
	int p1,p2;
	char outpipe[50],inpipe[50];
	int fd[2];
	pipe(fd);

	while((p1 = fork()) == -1);	//创建子进程1

	if(p1)
	{
		while((p2 = fork()) == -1);	//创建子进程2
		
		if(p2)	//父进程执行语句
		{
			//close(fd[1]);
			
			wait(0);	//等待子进程1结束
			read(fd[0],inpipe,50);
			printf("%s\n",inpipe);
			
			wait(0);	//等待子进程2结束
			read(fd[0],inpipe,50);
			printf("%s\n",inpipe);

			exit(0);
			
		}

		else	//子进程2执行语句
		{
			lockf(fd[1],1,0);
			//close(fd[0]);

			sprintf(outpipe,"\nChild process 2 is sending message!\n");
			write(fd[1],outpipe,50);

			sleep(2);

			lockf(fd[1],0,0);
			
			exit(0);
		}
	}

	else	//子进程1执行语句
	{
		lockf(fd[1],1,0);
		//close(fd[0]);

		sprintf(outpipe,"\nChild process 1 is sending message!\n");
		write(fd[1],outpipe,50);

		sleep(2);
		
		lockf(fd[1],0,0);

		exit(0);
	}



	return 0;
}
