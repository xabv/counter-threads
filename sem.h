typedef int* id_num_type ;
typedef struct sim_PCB
{
    id_num_type       id_num ;  
    int               in_Q ;     
    struct sim_PCB *next ; 
} sim_PCB ;
typedef struct sim_sem_data
{
        
     
     
    pthread_mutex_t   lock ;    
    pthread_cond_t    cond ;    
    int value ; 
    sim_PCB           *Qfront;
    sim_PCB           *Qrear ;
} sim_sem_data ;
typedef sim_sem_data *sim_semaphore ;
sim_semaphore create_sim_sem(int  init_val) ;
void wait_sem (sim_semaphore sem ) ;
void signal_sem (sim_semaphore sem) ;
