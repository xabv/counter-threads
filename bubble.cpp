//alex vega
#include <iostream>
#include <sched.h>
#include <time.h>
#include <pthread.h>
#include "sem.h"
#define NUM_CELLS 32

using namespace std;

extern sim_semaphore create_sim_sem(int);
extern void wait_sem (sim_semaphore);
extern void signal_sem (sim_semaphore);
extern long random(void);

pthread_mutex_t stdoutLock;
sim_semaphore reading; //for reading of cell data
sim_semaphore org[NUM_CELLS]; //for comparisons of cell data
int  cell[NUM_CELLS];
pthread_t thr[NUM_CELLS];
int checking = 0;

struct threadIdType
{
  int id;
};

void init()
{
  if ( 0!=pthread_mutex_init(&stdoutLock, NULL) )
  {  cout << "MUTEX INITIALIZATION FAILURE!" << endl;
   exit(-1) ;}
  for (int i = 0; i < NUM_CELLS; i++)
    org[i] = create_sim_sem(1); //cells available for comparisons
  reading = create_sim_sem(1); //reading is active
  srandom(time((time_t *) 0));
}

void * child (void * idPtr)
{
  int m_delay, j;
  int me = ((threadIdType *) (idPtr))->id, temp;
  do
  {
    wait_sem(org[me]);
    wait_sem(org[me-1]);
    wait_sem(reading);
    if (cell[me-1] > cell[me])
    {
      temp = cell[me-1];
      cell[me-1] = cell[me];
      m_delay = (int) random()%100;
      for (j=0; j<m_delay; j++) sched_yield();
      cell[me] = temp;
    }
    signal_sem(reading);
    signal_sem(org[me-1]);
    signal_sem(org[me]); //last semaphore layer
  } while (true);
  pthread_exit ((void *)0);
}

void mother()
{
  int i;
  threadIdType * idPtr;
  for (i = 1; i < NUM_CELLS ; i++)
  {
    idPtr = new threadIdType;
    idPtr->id = i;
    if ( 0!=pthread_create(&thr[i], NULL, child, (void *) idPtr) )
    {  cout << "THREAD CREATION FAILURE!" << endl;
     exit(-1) ; }
    if (0!=pthread_detach(thr[i]))
    {  cout << "THREAD DETACHMENT FAILURE!" << endl;
     exit(-1) ;}
  }
  bool sorted;
  int m_delay, j;
  do
  {
    sorted = true;
    wait_sem(reading); //wait in case comparison is being made
    for (i=1; i<NUM_CELLS; i++)
    {
      if (cell[i-1] > cell[i]) sorted = false;
      m_delay = (int) random()%100;
      for (j=0; j<m_delay; j++) sched_yield();
      signal_sem(reading); //resume comparison if needed
    }
  } while (!sorted);
  for (i=0; i<NUM_CELLS; i++) cout << endl << cell[i];
}

int main()
{
  init();
  int idx;
  for (idx=0; idx<NUM_CELLS; idx++)  cin >> cell[idx];
  for (idx=0; idx<NUM_CELLS; idx++)  cout << endl << cell[idx];
  cout << endl; 
  mother();
  cout << endl << endl;
  return 0;
}
