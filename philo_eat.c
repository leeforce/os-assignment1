#include <stdio.h>
#include <pthread.h>
#include <unistd.h>
#include <stdlib.h>
#include <time.h>

//死锁问题补救措施为只有两根筷子可用时，才能拿起它们

pthread_cond_t self[5];           //条件变量
pthread_mutex_t lock;             
pthread_t philo[5];
enum { thinking, hungry, eating } state[5];

void pickup_forks(int philo_num);
void return_forks(int philo_num);
void test(int philo_num);

 
//开始进行
void *think_eat (void * philo_num)
{
	while(1){
		sleep(rand()%3);             //随机分配一段思考时间     
		pickup_forks((int) philo_num);	
		sleep(rand()%3);             //随机分配一段吃饭时间
		return_forks((int) philo_num);
	}
}


//更新状态为hungry，并尝试eating
void pickup_forks(int philo_num)
{
	state[philo_num] = hungry;
	printf("哲学家 %d 感到饥饿  想要吃饭\n",(int) philo_num);
	test(philo_num);                                         //查看是否满足吃饭条件	
	pthread_mutex_lock(&lock);                        //若不满足条件，则进入阻塞等待状态，等待满足
	while(state[philo_num] != eating)
		pthread_cond_wait(&self[philo_num], &lock);
	pthread_mutex_unlock(&lock);
}


//将eating转为thinking，同时询问相邻两人是否hungry
void return_forks(int philo_num)
{
	state[philo_num] = thinking;
	printf("哲学家 %d 结束吃饭  开始思考\n", philo_num );
	test((philo_num + 4) % 5);
	test((philo_num + 1) % 5);
}


//判断左右是否为eating，并决定是否eating
void test(int philo_num)
{
    if(((state[( philo_num + 4 ) % 5] == eating) || (state[(philo_num + 1) % 5] == eating) && (state[philo_num] ==hungry ))){
        printf("哲学家 %d 想要吃饭  没有筷子\n",philo_num);           //判 断哲学家的状态与是否满足条件
    }
	if((state[( philo_num + 4 ) % 5] != eating) && (state[(philo_num + 1) % 5] != eating) && (state[philo_num] ==hungry )){
		state[philo_num] = eating;
		printf("哲学家 %d 开始吃饭\n",philo_num );                   //满足条件就开始吃饭，并更新状态
		
		pthread_cond_signal(&self[philo_num]);                      //释放信号，退出阻塞状态
	}
	//pthread_mutex_unlock(&lock);
}


//主函数
int main(void)
{
	int i;
    srand((int)time(0));
	for(i = 0; i < 5; i++)
		pthread_create(&philo[i], NULL, think_eat,(void*)i);  //生成5个线程，代表5个哲学家并开始活动

	for(i = 0; i < 5; i++)
		pthread_join(philo[i], NULL);	
	
	pthread_mutex_init(&lock, NULL);                        //初始化互斥锁和条件变量
	for (int i = 0; i < 5; i++) 
		pthread_cond_init(&self[i], NULL);                  //条件变量初始化

	return 0;
}