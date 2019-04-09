#include <stdio.h>    
#include <string.h>    
#include <stdlib.h>
#include <iostream>
#include <string>
#include <sys/time.h>
#include <sstream>

using namespace std;

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
    printf("\nEndTime in getEndTime==>%s",endTimebuff);
    strncpy(endTimebuff2,endTimebuff,5);
}


int main(int argc,char*argv[])
{    
    // FILE *fstream = NULL;      
    // char buff[1024];    
    // char endTimebuff[1024];    

    // memset(buff, 0, sizeof(buff));   

    // if(NULL == (fstream = popen("ps -p 12 -o etime","r")))      
    // {     
    //     fprintf(stderr,"execute command failed");      
    //     return -1;      
    // }   

    // while(NULL != fgets(buff, sizeof(buff), fstream)) {  
    //         printf("%s",buff);    
    // }  
    // pclose(fstream);    

    // int j = 0;
    // for(int i = 0; i<1024;  i++) {
    //     if((buff[i]  >= '0' && buff[i]  <= '9')|| (buff[i] == ':')) {
    //         endTimebuff[j++] = buff[i];
    //     }
    // }
    // printf("/nEndTime==>%s",endTimebuff);

    // OK
    // char endTimebuff[1024];    
    // getEndTime(12,endTimebuff);
    // printf("\nEndTime==>%s",endTimebuff);



    // int pid1 = 12;
    // char str[25];
    // sprintf(str, "%d", pid1);

    // char buff1[] = "ps -p ";
    // char buff2[] = " -o etime";

    // stringstream ss;
    // ss << buff1;
    // ss << str;
    // ss << buff2;
    // cout << ss.str() << endl;

    // char commandButff[1024];

    // printf("%s",str);

    char endTimebuff[1024];    
    getEndTime(12,endTimebuff);
    printf("\nEndTime==>%s",endTimebuff);


    return 0;     
}  


