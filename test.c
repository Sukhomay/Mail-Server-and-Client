#include<stdio.h>                                                                
#include<time.h>                                                            
                                                                                
void main()                                                                     
{                                                                               
  time_t t = time(NULL);                                                        
  struct tm time = *localtime(&t);                                              
                                                                                
  printf("date - %d:%d:%d\n", time.tm_mday,  time.tm_mon + 1, time.tm_year + 1900);
  printf("time- %d:%d %d\n", time.tm_hour, time.tm_min, time.tm_sec);           
}  