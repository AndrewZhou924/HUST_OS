#include <stdio.h>
#include <unistd.h>
#include <stdlib.h>
#include <time.h>
#include <fcntl.h>
#include <string.h>
#include <sys/stat.h>
#include <dirent.h>
#include <memory.h>
#include <string.h>    
#include <iostream>
#include <string>
#include <sstream>

using namespace std;

#define ERR_EXIT(m) \
do\
{\
    perror(m);\
    exit(EXIT_FAILURE);\
}\
while (0);\

typedef struct{
    pid_t pid;
    char name[256];//进程名称
    char user[256];//进程名称
    char endTimebuff[1024];
    char startTimebuff[1024];
}proc_info_st;//保存读取的进程信息

#define PROC_NAME_LINE 1//名称所在行
#define PROC_PID_LINE 4//pid所在行
#define BUFF_LEN 1024 //行缓冲区的长度

#ifndef TRUE
# define TRUE 1
#endif

#ifndef FALSE
# define FALSE 0
#endif

void read_proc(proc_info_st* info,const char* c_pid);//读取进程信息
int read_line(FILE* fp,char* buff,int b_l,int l);//读取一行
void getEndTime(int pid, char endTimebuff2[]);
void getStartTime(int pid, char startTimebuff[]);
void getUser(int pid, char userBuff[]);
void creat_daemon(void);



int main(void)
{
    time_t t;
    int fd;
    creat_daemon();
    while(1){
        fd = open("daemon.log",O_WRONLY|O_CREAT|O_TRUNC,0644);
        if(fd == -1)
            ERR_EXIT("open error");
        t = time(0);
        char *buf = asctime(localtime(&t));
        write(fd,buf,strlen(buf));

            //打开目录
        DIR *dir;
        struct dirent *ptr;
        if (!(dir = opendir("/proc")))
            return 0;
     //读取目录
        while (ptr = readdir(dir)) {//循环读取出所有的进程文件
    
        if (ptr->d_name[0] > '0' && ptr->d_name[0] <= '9') {
            //获取进程信息
            proc_info_st info;
            read_proc(&info,ptr->d_name);//读取信息
            // printf("pid:%d\npname:%s\nlastTime: %s\nStartTime: %s\nUser:%s\n",info.pid,info.name,info.endTimebuff,info.startTimebuff,info.user);

            stringstream ss;
            ss << "pid:";
            char pid_str[25];
            sprintf(pid_str, "%d", info.pid);
            ss << pid_str;

            ss << " ---user:";
            ss << info.user;

            ss << " ---process name:";
            ss << info.name;

            ss << " ---start time:";
            ss << info.startTimebuff;

            ss << " ---last time:";
            ss << info.endTimebuff;
            ss << "\n";

            // printf("%s",ss.str().c_str());
            write(fd,ss.str().c_str(),strlen(ss.str().c_str()));
            }
        }



        close(fd);
        sleep(60);
            
    }
    return 0;
}


void creat_daemon(void)
{
    pid_t pid;
    pid = fork();
    if( pid == -1)
        ERR_EXIT("fork error");
    if(pid > 0 )
        exit(EXIT_SUCCESS);
    if(setsid() == -1)
        ERR_EXIT("SETSID ERROR");
    chdir("/");
    int i;
    for( i = 0; i < 3; ++i)
    {
        close(i);
        open("/dev/null", O_RDWR);
        dup(0);
        dup(0);
    }
    umask(0);
    return;
}

/**************************************************
 **说明:根据进程pid获取进程信息,存放在proc_info_st结构体中
 **
 **输入:
 **        /proc_info_st* info  返回进程信息
 **        /char* c_pid  进程pid的字符串形式
 **
 **
 **
 *************************************************/
void read_proc(proc_info_st* info,const char* c_pid)
{
    FILE* fp = NULL;
    char file[512] = {0};
    char line_buff[BUFF_LEN] = {0};//读取行的缓冲区
    
    sprintf(file,"/proc/%s/status",c_pid);//读取status文件
    if (!(fp = fopen(file,"r")))
    {
        printf("read %s file fail!\n",file);
        return;
    }

    char name[32];
    //先读取进程名称
    if (read_line(fp,line_buff,BUFF_LEN,PROC_NAME_LINE))
    {
        sscanf(line_buff,"%s %s",name,(info->name));
    }
   
    fseek(fp,0,SEEK_SET);//回到文件头部
    //读取进程pid
    if (read_line(fp,line_buff,BUFF_LEN,PROC_PID_LINE))
    {
        sscanf(line_buff,"%s %d",name,&(info->pid));
    }
    fclose(fp);

    //读取进程运行时间
    char endTimebuff[1024];    
    getEndTime((int)(info->pid),endTimebuff);
    strcpy(info->endTimebuff,endTimebuff);

    //读取进程创建时间
    char startTimebuff[1024];    
    getStartTime((int)(info->pid),startTimebuff);
    strcpy(info->startTimebuff,startTimebuff);

    char userBuff[1024];    
    getUser((int)(info->pid),userBuff);
    strcpy(info->user,userBuff);
    
    
}


/**************************************************
 **说明:读取文件的一行到buff
 **
 **输入:
 **     /FILE* fp  文件指针
 **        /char* buff  缓冲区
 **     /int b_l  缓冲区的长度
 **        /l  指定行
 **
 **输出:
 **     /true 读取成功
 **     /false 读取失败
 *************************************************/
int read_line(FILE* fp,char* buff,int b_l,int l)
{
    if (!fp)
        return FALSE;
    
    char line_buff[b_l];
    int i;
    //读取指定行的前l-1行,转到指定行
    for (i = 0; i < l-1; i++)
    {
        if (!fgets (line_buff, sizeof(line_buff), fp))
        {
            return FALSE;
        }
    }

    //读取指定行
    if (!fgets (line_buff, sizeof(line_buff), fp))
    {
        return FALSE;
    }

    memcpy(buff,line_buff,b_l);

    return TRUE;
}

void getEndTime(int pid, char endTimebuff2[]) {
    FILE *fstream = NULL;      
    char buff[1024];    
    char endTimebuff[1024]; 

    char pid_str[25];
    sprintf(pid_str, "%d", pid);

    char buff1[] = "ps -p ";
    char buff2[] = " -o etime";

    stringstream ss;
    ss << buff1;
    ss << pid_str;
    ss << buff2;
    // cout << ss.str() << endl;

    if(NULL == (fstream = popen(ss.str().c_str(),"r"))) {     
        fprintf(stderr,"execute command failed");      
        return;      
    }   

    while(NULL != fgets(buff, sizeof(buff), fstream)) {  
            // printf("%s",buff);    
    }  
    pclose(fstream);    

    int j = 0;
    for(int i = 0; i<1024;  i++) {
        if((buff[i]  >= '0' && buff[i]  <= '9')|| (buff[i] == ':')) {
            endTimebuff[j++] = buff[i];
        }
    }
    // printf("\nEndTime in getEndTime==>%s",endTimebuff);
    strncpy(endTimebuff2,endTimebuff,5);
}

void getStartTime(int pid, char startTimebuff[]) {
    FILE *fstream = NULL;      
    char buff[1024];    
    char temp_startTimebuff[1024]; 

    char pid_str[25];
    sprintf(pid_str, "%d", pid);

    char buff1[] = "ps -p ";
    char buff2[] = " -o stime";

    stringstream ss;
    ss << buff1;
    ss << pid_str;
    ss << buff2;
    // cout << ss.str() << endl;

    if(NULL == (fstream = popen(ss.str().c_str(),"r"))) {     
        fprintf(stderr,"execute command failed");      
        return;      
    }   

    while(NULL != fgets(buff, sizeof(buff), fstream)) {  
            // printf("%s",buff);    
    }  
    pclose(fstream);    

    int j = 0;
    for(int i = 0; i<1024;  i++) {
        if((buff[i]  >= '0' && buff[i]  <= '9')|| (buff[i] == ':')) {
            temp_startTimebuff[j++] = buff[i];
        }
    }
    strncpy(startTimebuff,temp_startTimebuff,5);
}

void getUser(int pid, char userBuff[]) {
    FILE *fstream = NULL;      
    char buff[1024];    
    char temp_userBuff[1024] = {}; 

    char pid_str[25];
    sprintf(pid_str, "%d", pid);

    char buff1[] = "ps -p ";
    char buff2[] = " -o user";

    stringstream ss;
    ss << buff1;
    ss << pid_str;
    ss << buff2;
    // cout << ss.str() << endl;

    if(NULL == (fstream = popen(ss.str().c_str(),"r"))) {     
        fprintf(stderr,"execute command failed");      
        return;      
    }   

    while(NULL != fgets(buff, sizeof(buff), fstream)) {  
            // printf("%s",buff);    
    }  
    pclose(fstream);    

    int j = 0;
    for(int i = 0; i<10;  i++) {
        if(buff[i] >='a' && buff[i]<='z' ||  buff[i] >='A' && buff[i]<='Z') {
            temp_userBuff[j++] = buff[i];
        }
    }
    // printf("\nuser name :%s\n",temp_userBuff);
    strncpy(userBuff,temp_userBuff,10);
}