#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/ipc.h>
#include <sys/shm.h>
#include <semaphore.h>
#include <fcntl.h>
#include <time.h>
#include <math.h>

#define Buffer_Size 20           //缓冲区大小


// pthread_cond_t self[3];           //条件变量
// pthread_mutex_t lock;             
pthread_t producer[3];
double my_lanmada_p;
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
    printf("生产者 %d:id(%d) 间隔%lf秒\n",id,pthread_self(),x);
    return x;
}


void *produce(void * id)
{
    int j;
    while(1)
    {
        sleep(negtive_index(my_lanmada_p,(int)id));
        if(sem_wait(&(shared->sem)) == -1)                                    //sem_wait为P操作，减少信号量的值
        {
            printf("P操作 ERROR!\n");
            exit(EXIT_FAILURE);
        }
        for(j = 0; j < Buffer_Size && shared->Index[j] == 1; j++)             //i用于判断缓冲池数据个数
            ;
        if(j==Buffer_Size)
        {
            printf("缓冲区满了\n");
            sem_post(&shared->sem);
            sleep(1);
        }
        else
        {
            sem_post(&shared->sem);
            shared->Index[j] = 1;
            shared->Buffer[j] = rand();
            printf("生产者 %d 生产数据：%d\n",id,shared->Buffer[j]);
            sleep(1);
        }
        
    }
}


int main(int argc,char *argv[])
{
    int x;
    srand((int)time(0));
    x = atoi(argv[1]);
    my_lanmada_p = (double)x;
    int i = 0;   
    void *shm = NULL;                                                         //共享存储段连接的实际地址
    char buffer[BUFSIZ + 1];                                                  //缓冲区存放字符
    int shmid;                                                                //共享内存标识符   
    shmid = shmget((key_t)1121, sizeof(struct shared_use_st), 0666|IPC_CREAT);//获得或创建一个共享内存标识符
    if(shmid == -1)                                                           //判断是否创建成功
    {   
      exit(EXIT_FAILURE);
    }
    shm = shmat(shmid, (void*)0, 0);                                          //返回共享存储段连接的实际地址
    if(shm == (void*)-1)                                                      //判断是否得到共享内存地址
    {
        exit(EXIT_FAILURE);
    }
    printf("分配内存连接到： %ld\n", (intptr_t)shm);
    shared = (struct shared_use_st*)shm;                                      //缓冲池为共享存储段连接地址
    for( ; i < Buffer_Size; i++ )
    {
        shared->Index[i] = 0;                                                 //对缓冲池初始化，Index为0表示可以生产
    }
    sem_init(&(shared->sem),1,1);                                             //信号量化初始化，且信号量初始值为第二个1
    i = 0;

    while(1)                                                               //制造一个循环
    {
        sem_post(&shared->sem);                                           //V 操作增加信号量
        for(i = 0; i < 3; i++)
        	pthread_create(&producer[i], NULL, produce,(void *)i);                       //读取stdin字符流最多BUFSIZ-1个，并存在buffer数组中 其中stdin是键盘输入到缓冲区的字符
            // strncpy(shared->Buffer[i%Buffer_Size], buffer,TEXT_SZ);        //读取的字符串存入缓冲区shared->Buffer中
            // shared->Index[i%Buffer_Size] = 1;                              //表示该缓冲区被生产者使用了
        for(i=0;i<3;i++)
        {
            pthread_join(producer[i],NULL);
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