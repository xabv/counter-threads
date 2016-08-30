#include <iostream>
#include <pthread.h>
#include "sem.h"
using namespace std ;
   
extern int checking ;  
extern pthread_mutex_t stdoutLock ;
sim_semaphore create_sim_sem(int  init_val) 
{
    sim_semaphore ptr ;
    ptr = new sim_sem_data ;
    if   (ptr == NULL)
                cout << "ERROR: Out of space for semaphore." << endl ;
    else
    {    if ( 0!=pthread_mutex_init(&(ptr->lock), NULL) )
        {  cout << "MUTEX INITIALIZATION FAILURE!" << endl;
                exit(-1) ;}
                
                if ( 0!=pthread_cond_init(&(ptr->cond), NULL) )
                {  cout << "CONDITION INITIALIZATION FAILURE!" << endl ;
            exit(-1) ;}
                
                (ptr->value)     =  init_val ;
                (ptr->Qfront)    =  NULL ;
                (ptr->Qrear)     =  NULL ;
    }
        
    return(ptr) ;
}
int empty_sem_Q ( sim_sem_data sem_d)
{
        return (sem_d.Qfront == NULL) ;
}
void print_sem_Q (sim_sem_data sem_d)
{
        sim_PCB  *temp ;
        temp = sem_d.Qfront ;
        cout << "Printing Q .... " << endl ;
        if (temp == NULL) cout << "Q is empty." << endl ;
        while (temp != NULL)
        {
                cout << "Thread #" << (long) temp->id_num << endl ;
                temp = temp->next ;
        }
}
void enq_sem_Q (sim_semaphore sem, sim_PCB * proc)
{    
        
        (proc->id_num)   = (int*) pthread_self() ;
        (proc->next)     = NULL ;
        (proc->in_Q)     = 1 ;
        if   (empty_sem_Q(*sem)) sem->Qfront = proc ;
        else sem->Qrear->next = proc ;
        sem->Qrear = proc ;
}
void serve_sem_Q(sim_semaphore sem)
{
        sim_PCB  *temp ;
        
        if (checking)
        { pthread_mutex_lock (&stdoutLock) ;
        
                cout << "State of Q in semaphore #"<< (long) (int*) sem 
                << " prior to Q serve: " << endl ;
                print_sem_Q (*sem) ;
                pthread_mutex_unlock(&stdoutLock) ; }
        
        temp              =    sem->Qfront ;
        sem->Qfront        =   sem->Qfront->next ;
        
        
        
        (temp->in_Q)   =   0 ; 
        if (sem->Qfront == NULL)  sem->Qrear = NULL ;
        
        if (checking) 
        { pthread_mutex_lock(&stdoutLock) ;
                cout << "State of Q in semaphore #"<< (long) (int*) sem 
                << " after Q serve: " << endl ;
                print_sem_Q (*sem) ;
                pthread_mutex_unlock(&stdoutLock) ; }
}
void wait_sem (sim_semaphore sem ) 
{ 
        pthread_mutex_lock( &(sem->lock) ) ;
        sem->value = sem->value - 1 ;
        
        if (checking)
        { pthread_mutex_lock(&stdoutLock) ;
                cout << "Thread #" << (long) pthread_self() 
                << " begins a wait and"
                << " decrements the value of semaphore #" 
                << (long) (int*) sem << " to: " << sem->value << endl ;
                pthread_mutex_unlock(&stdoutLock) ; }
        
        if (sem->value < 0)
        
        {
                sim_PCB * proc = new sim_PCB ;
                if    (proc == NULL) 
                        cout << "ERROR: Out of space for sim_PCB." << endl ;
                else               
                
                {
                        if (checking)
                        { pthread_mutex_lock(&stdoutLock) ;
                                cout << "Thread #" << (long) pthread_self() 
                                << " queues up on semaphore #" << (long) (int*) sem
                                << "." << endl ;
                                pthread_mutex_unlock(&stdoutLock) ;  }
                        
                        
                        
                        enq_sem_Q(sem, proc) ;  
                        
                        
                        
                        do
                        {
                                if (checking)
                                {  pthread_mutex_lock(&stdoutLock) ;
                                        cout << "Thread #" << (long) pthread_self()
                                        << " now BLOCKS on semaphore #" << (long) (int*) sem
                                        << " by doing a pthread_cond_wait." << endl ;
                                        pthread_mutex_unlock(&stdoutLock) ; }
                                
                                if ( 0!= pthread_cond_wait (&(sem->cond),&(sem->lock) ) )
                                {  cout << "COND BROADCAST FAILURE!" << endl ;
                                        exit(-1) ;}
                                
                                if (checking)
                                {  pthread_mutex_lock(&stdoutLock) ;
                                        cout << "Thread #" << (long) pthread_self()
                                        << " on semaphore #" << (long) (int*) sem
                                        << " wakes up, and checks to" << endl
                                        << "\t" << "see if a signal_sem OP has "
                                        << "taken it off the queue." << endl 
                                        << "\t" << "It IS" ;
                                        if (!proc->in_Q)  cout << " NOT" ;
                                        cout << " in the queue." << endl ;
                                        pthread_mutex_unlock(&stdoutLock) ; }
                        } 
                        while (proc->in_Q) ;
                        delete proc ;
                }  
        } 
        
        if (checking)
        {  pthread_mutex_lock(&stdoutLock) ;
        cout << "Thread #" << (long) pthread_self()
        << " completes a wait on semaphore #" 
        << (long) (int*) sem << "." << endl ;
        pthread_mutex_unlock(&stdoutLock) ; }
        
        pthread_mutex_unlock( &(sem->lock) ) ;
}
void signal_sem (sim_semaphore sem)  
{
        pthread_mutex_lock(&(sem->lock)) ;
        sem->value = sem->value + 1 ;
        
        if (checking)
        { pthread_mutex_lock(&stdoutLock) ;
                cout << "Thread #" << (long) pthread_self() 
                << " begins a signal and"
                << " increments the value of semaphore #" 
                << (long) (int*) sem << " to: " << sem->value << endl ;
                pthread_mutex_unlock(&stdoutLock) ; }
        
        if (sem->value <= 0)
        {
                if (checking)
                {  pthread_mutex_lock(&stdoutLock) ;
                        cout << "Thread #" << (long) pthread_self() 
                        << " removes a PCB from the queue of"
                        << " semaphore #" << (long) (int*) sem << "." << endl ;
                        pthread_mutex_unlock(&stdoutLock) ; }
                
                serve_sem_Q(sem) ;  
                
                
                
                if (checking)
                {  pthread_mutex_lock(&stdoutLock) ;
                        cout << "Thread #" << (long) pthread_self()
                        << " broadcasts to the thread(s) waiting on semaphore #"
                        << (long) (int*) sem << "." << endl ;
                        pthread_mutex_unlock(&stdoutLock) ;  }
                
                if ( 0!=pthread_cond_broadcast(&(sem->cond)) )
                {  cout << "COND BROADCAST FAILURE!" << endl ;
                        exit(-1) ;}
        }
        
        if (checking)
        {  pthread_mutex_lock(&stdoutLock) ;
                cout << "Thread #" << (long) pthread_self()
                << " completes a signal on semaphore #" 
                << (long) (int*) sem << "." << endl ;
                pthread_mutex_unlock(&stdoutLock) ; }
        
        
        pthread_mutex_unlock( &(sem->lock) ) ;
}
