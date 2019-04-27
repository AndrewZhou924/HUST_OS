#include<sys/wait.h>
#include<sys/types.h>
#include<unistd.h>
#include<stdio.h>
#include<stdlib.h>
#include<errno.h>
#include<signal.h>
 
 
//自定义信号
#define SIG_MY_DEFINE_TEST    __SIGRTMIN+10

int count = 0;
 
void sigactionProcess(int nsig)
{
	//================================================================
	//TODO:ADD YOU CODE
  count += 1;
  printf("count is %d\n",count);

	// printf("son process sigactionProcess end !pid is:%d\n", getpid()) ;
	// //================================================================
	exit(0);
}

//信号处理函数注册
void sigactionReg(void) 
{
	struct sigaction act,oldact;
 
	act.sa_handler  = sigactionProcess;
	act.sa_flags = 0;
 
	//sigaction(SIGINT,&act,&oldact);
	sigaction(SIG_MY_DEFINE_TEST,&act,&oldact);
}
 
 
 
int main()
{
  int ret ;
  pid_t pid;/*pid 进程id号*/
 
  pid=fork();/*创建一个新进程*/
 
  if(pid==0) /*返回0为子进程*/
  {
      printf("son get Return pid is %d\n",pid);
      printf("This is son process!  pid is:%d\n",getpid());

      //信号处理函数注册
      sigactionReg();

      //raise(SIGSTOP) ;//子进程暂停

      // while(1){}
      // sleep(10);
      while(count <= 10) {

      }

      printf("son process end !pid is:%d\n", getpid()) ;
      exit(0) ;
  }
  else if(pid>0)/*返回大于0为父进程*/
  {
	  printf("parent get Return pid is %d\n",pid);
    printf("This is parent process!  pid is:%d\n",getpid());

	  sleep(10);

	  //获取到pid子进程没有退出,指定WNOHANG不会阻塞，没有退出会返回0
    while ((waitpid(pid, NULL, WNOHANG)) == 0) {



          //if ((ret = kill(pid, SIGINT)) == 0)//向子进程发出SIGKILL信号
          if ((ret = kill(pid, SIG_MY_DEFINE_TEST)) == 0)//向子进程发出SIG_MY_DEFINE_TEST信号
          {
               printf("parent kill %d\n", pid) ;
          }
      }

      waitpid(pid,NULL,0);/*等待子进程退出*/
      printf("parent process end ! pid is:%d\n", getpid()) ;
      exit(0) ;
  }
  else
  {
       perror("fork() error!");
       exit(-1) ;
  }
}