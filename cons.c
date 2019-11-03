#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/shm.h>
#include <unistd.h>
#include <semaphore.h>
#include <sys/types.h>
#include <sys/ipc.h>
#include <fcntl.h>
#include <time.h>
#include <math.h>

#define Buffer_Size 20           //缓冲区大小


pthread_t consumer[3];
double my_lanmada_c;
//缓冲池结构
struct shared_use_st  
{  
    int Index[Buffer_Size];     //20个缓冲池,为0表示对应的缓冲区未被生产者使用，可分配但不可消费；为1表示对应的缓冲区已被生产者使用，不可分配但可消费
    int Buffer[Buffer_Size];    //20个缓冲区
    sem_t sem;                  //信号量，同步功能
};

struct shared_use_st *shared = NULL; 

double negtive_index(double lanmada,int id)       //负指数函数，随时间变化
{
    double rnd;
    double r,x;
    while(1)
    {
        rnd = rand();
        if(rnd != 0 && rnd != RAND_MAX)
        break;
    }
    r = rnd / RAND_MAX;
    x = (-1 / lanmada) * log(r) * 3;
    printf("消费者 %d 间隔%f秒\n",id,x);
    return x;
}

void *consume(void *id)
{
    int j;
    
    while(1)
    {
        sleep(negtive_index(my_lanmada_c,id));
        if(sem_wait(&(shared->sem)) == -1)                                      //sem_wait为P操作，减少信号量的值
        {
            printf("P操作 ERROR!\n");
            exit(EXIT_FAILURE);
        }
        for(j = 0; j < Buffer_Size && shared->Index[j] == 0; j++)
            ;
        if(j == Buffer_Size)                                                    //五个缓冲区没有都被生产者占用
        {   
            printf("缓冲区为空\n");
            sem_wait(&(shared->sem));
            sleep(1);
                                                         
        }
        else
        {
            printf("消费者 %d:id(%d) 取得数据: %d\n", id,pthread_self(),shared->Buffer[j]);           //打印出生产者写入的字符
            shared->Index[j] = 0;                                   //为0时，表示已被消费者使用
            sem_post(&shared->sem);                                         //sem_post为V操作
        }                          
    }       
}

int main(int argc,char *argv[])
{
    srand((int)time(0));
    my_lanmada_c = atoi(argv[1]);
    int i = 0; 
    void *shm = NULL;                                                           //共享存储段连接的实际地址
    int shmid;                    //声明共享内存标识符   
                                               
    shmid = shmget((key_t)1121, sizeof(struct shared_use_st), 0666|IPC_CREAT);  //获得或创建一个共享内存标识符
    if(shmid == -1)                                                             //获取或创建一个共享内存标识符失败
    {  
        exit(EXIT_FAILURE);
    }
    shm = shmat(shmid, 0, 0);                                                   //返回共享存储段连接的实际地址    
    if(shm == (void*)-1)                                                        //失败
    {
        exit(EXIT_FAILURE);
    }
    printf("分配内存连接到： %ld\n", (intptr_t)shm);
    shared = (struct shared_use_st*)shm;  //缓冲池为共享存储段连接

    // if(sem_wait(&(shared->sem)) == -1)                                      //sem_wait为P操作，减少信号量的值
    // {
    //     printf("P操作 ERROR!\n");
    //     exit(EXIT_FAILURE);
    // }
    // for(i = 0; i < Buffer_Size && shared->Index[i] == 0; i++)
    //     ;              
    // if(i != Buffer_Size)                                                    //五个缓冲区没有都被生产者占用
    // {   
    //     sem_post(&shared->sem);                                             //sem_post为V操作
    //     sleep(1); 
    // }
    while(1)                                                               //制造一个循环
    {       
        sem_post(&shared->sem);                                          //V 操作增加信号量
        for(i = 0; i < 3; i++)
    	    pthread_create(&consumer[i], NULL, consume,(void *)i);                       //读取stdin字符流最多BUFSIZ-1个，并存在buffer数组中 其中stdin是键盘输入到缓冲区的字符
            // strncpy(shared->Buffer[i%Buffer_Size], buffer,TEXT_SZ);        //读取的字符串存入缓冲区shared->Buffer中
            // shared->Index[i%Buffer_Size] = 1;                              //表示该缓冲区被生产者使用了
        for(i=0;i<3;i++)
        {
            pthread_join(consumer[i],NULL);
        }
        while(1)
        {
            ;
        }
    
    }
    if(shmdt(shm) == -1)                                                      //失败
    {     
        exit(EXIT_FAILURE);
    }
    if(shmctl(shmid, IPC_RMID, 0) == -1)        //失败
    {
        exit(EXIT_FAILURE);
    }  
    exit(EXIT_SUCCESS);
}