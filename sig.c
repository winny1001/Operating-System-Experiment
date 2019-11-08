#include <stdio.h>
#include <signal.h>
#include <stdlib.h>
#include <sys/types.h>
#include <wait.h>
#include <unistd.h>


int wait_flag = 1;

void stop(){wait_flag = 0;}

int main()
{
	int p1,p2;
	

	while((p1 = fork()) == -1);	//创建子进程1

	if(p1)
	{
		while((p2 = fork()) == -1);	//创建子进程2

		if(p2)		//父进程执行
		{
			signal(SIGQUIT,stop);
			while(wait_flag);	//等待软中断Quit信号(Ctrl + \)
			
			kill(p1,16);
			kill(p2,17);
			
			wait(0);
			wait(0);
			
			sleep(3);
			printf("\nParent process is killed !!\n");
			exit(0);
		}

		else 	//子进程2执行
		{
			signal(SIGQUIT,SIG_IGN);

			wait_flag = 1;

			signal(17,stop);
			while(wait_flag);
			printf("\nChild process 2 is killed by parent !!\n");

			exit(0);
		}
	}

	else	//子进程1执行
	{
		signal(SIGQUIT,SIG_IGN);
		wait_flag = 1;
		
		signal(16,stop);
		while(wait_flag);
		
		printf("\nChild process 1 is killed by parent !!\n");
		exit(0);
	}
	return 0;
}
