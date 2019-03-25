#include <stdio.h>  
#include <unistd.h>   

void ProgressBar()  
{  
    char arr[102] = {'\0'};  
    char *imag = "|/-\\";  
    int n = 60, i = 0;  
    while (i <= n)  
    {  
        arr[i] = '='; 
        printf("[%-60s][%d%%][%c]\r", arr, 100 * i / 60, imag[i % 4]);
        fflush(stdout);  
        i++;  
        usleep(1000000);  
    }  
    printf("\n");  
}  

int main()  
{  
    ProgressBar();  

    return 0;  
}
