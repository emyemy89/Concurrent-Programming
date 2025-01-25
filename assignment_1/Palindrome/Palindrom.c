/* matrix summation using pthreads

   features: uses a barrier; the Worker[0] computes
             the total sum from partial sums computed by Workers
             and prints the total sum to the standard output

   usage under Linux:
     gcc matrixSum.c -lpthread
     a.out size numWorkers

*/
#ifndef _REENTRANT
#define _REENTRANT
#endif
#include <pthread.h>
#include <stdlib.h>
#include <stdio.h>
#include <stdbool.h>
#include <string.h>
#include <time.h>
#include <sys/time.h>


#define MAXWORDS 25145
#define MAXWORDLENGTH 256



#define MAXWORKERS 10   /* maximum number of workers */

//define locks

pthread_mutex_t lockPal;
pthread_mutex_t lockSemo;



//int words[MAXWORDS];

char **words = NULL;

int Palresults[MAXWORDS]; //marked with 1 if corresponding i is a palindrome
int PalCount[MAXWORKERS] = {0}; //holds how many palindromes each thread found



int Semoresults[MAXWORDS];
int SemorCount[MAXWORKERS] = {0};


int Paltotal = 0;
int Semototal = 0;

int word_count = 0;


pthread_mutex_t barrier;  /* mutex lock for the barrier */
pthread_cond_t go;        /* condition variable for leaving */
int numWorkers;           /* number of workers */
int numArrived = 0;       /* number who have arrived */

/* a reusable counter barrier */
void Barrier() {
  pthread_mutex_lock(&barrier);
  numArrived++;
  if (numArrived == numWorkers) {
    numArrived = 0;
    pthread_cond_broadcast(&go);
  } else
    pthread_cond_wait(&go, &barrier);
  pthread_mutex_unlock(&barrier);
}

/* timer */
double read_timer() {
    static bool initialized = false;
    static struct timeval start;
    struct timeval end;
    if( !initialized )
    {
        gettimeofday( &start, NULL );
        initialized = true;
    }
    gettimeofday( &end, NULL );
    return (end.tv_sec - start.tv_sec) + 1.0e-6 * (end.tv_usec - start.tv_usec);
}

double start_time, end_time; /* start and end times */
int size, stripSize;  /* assume size is multiple of numWorkers */





bool isPalindrome(char *word) {
  int i;
  int wordLength = strlen(word);
  char reversed[wordLength + 1];
  for (i = 0; i < wordLength; i++) {
    reversed[i] = word[wordLength - i - 1];
  }
  reversed[wordLength] = '\0';
  if (strcmp(word, reversed) != 0) {
    return false;
  }
  return true;
}

bool isSemor(char *word) {
  int i;
  int wordLength = strlen(word);
  char reversed[wordLength + 1];
  for (i = 0; i < wordLength; i++) {
    reversed[i] = word[wordLength - i - 1];
  }
  reversed[wordLength] = '\0';
  for (i = 0; i < word_count; i++) {
    if (strcmp(reversed, words[i]) == 0) {
      return true;
    }
  }
  return false;
}


void *Palindrome(void *word){

  int  i, j;

  long myid = (long)word;
  int first = myid * stripSize;
  int last = (myid == numWorkers - 1) ? (word_count - 1) : (first + stripSize - 1);



  for (i= first; i <= last; i++) {
    if (words[i] == NULL) {continue;}

    if (i < MAXWORDS && isPalindrome(words[i])) {
      pthread_mutex_lock(&lockPal);
      Palresults[i] = 1;
      PalCount[myid]++;
      pthread_mutex_unlock(&lockPal);
    }
    else {Palresults[i] = 0;}

    if (i<MAXWORDS && isSemor(words[i])) {
      pthread_mutex_lock(&lockSemo);
      Semoresults[i] = 1;
      SemorCount[myid]++;
      pthread_mutex_unlock(&lockSemo);
    }
    else{Semoresults[i] = 0;}
  }

  pthread_exit(NULL);
}



/* read command line, initialize, and create threads */
int main(int argc, char *argv[]) {
  int i, j;
  long l; /* use long in case of a 64-bit system */
  pthread_attr_t attr;
  pthread_t workerid[MAXWORKERS];

  /* set global thread attributes */
  pthread_attr_init(&attr);
  pthread_attr_setscope(&attr, PTHREAD_SCOPE_SYSTEM);




  //FILE READING
  FILE *fp_in;
  const char *filename = "/home/student/Desktop/HW1/Task2/wordFile";


  char lineBuff[MAXWORDLENGTH];


  fp_in = fopen(filename, "r");
  if (fp_in == NULL) {
    perror("Error opening file");
    return 1;
  }

  words = (char **)malloc(MAXWORDS * sizeof(char *));
  if (words == NULL) {
    perror("Memory allocation failed");
    fclose(fp_in);
    return 1;
  }

  while (fgets(lineBuff, MAXWORDLENGTH, fp_in) != NULL && word_count < MAXWORDS) {
    size_t len = strlen(lineBuff);
    if (len > 0 && lineBuff[len - 1] == '\n') {
      lineBuff[len - 1] = '\0';
    }

    words[word_count] = (char *) malloc((strlen(lineBuff) + 1) * sizeof(char));


    strcpy(words[word_count], lineBuff);
    word_count++;
  }

  fclose(fp_in);

  printf("There are %d words in the file.\n", word_count);


  //Initialize locks

  pthread_mutex_init(&lockPal, NULL);
  pthread_mutex_init(&lockSemo, NULL);

  /* read command line args if any */
  numWorkers = (argc > 1) ? atoi(argv[1]) : MAXWORKERS; //Gets number of threads from command line. If not, use MAXWORKERS
  if (numWorkers > MAXWORKERS) numWorkers = MAXWORKERS;
  stripSize = (word_count + numWorkers - 1) / numWorkers;


  /* do the parallel work: create the workers */
  start_time = read_timer();
  for (l = 0; l < numWorkers; l++) {
    pthread_create(&workerid[l], &attr, Palindrome, (void *) l);
  }

  for (l = 0; l < numWorkers; l++) {
    pthread_join(workerid[l],NULL);
  }

  //Write Palindromes onto output file

  FILE *fp_out;
  const char *outfile = "/home/student/Desktop/HW1/Task2/OutputFile";

  fp_out = fopen(outfile, "w");
  int PaloutCount = 0;
  int SemoutCount = 0;

  if (fp_out == NULL) {
    perror("Error opening output file");
    for (int i = 0; i < word_count; i++) free(words[i]);
    free(words);
    return 1;
  }

  fprintf(fp_out, "PALINDROMES:\n");
  for (int i = 0; i < word_count; i++) {
    if (Palresults[i] == 1) {
      fprintf(fp_out, "%s\n", words[i]);
      PaloutCount++;
    }
  }
  fprintf(fp_out, "----------------------------------------------------\nSemordnilaps:\n");

  for (int i = 0; i < word_count; i++) {
    if (Semoresults[i] == 1) {
      fprintf(fp_out, "%s\n", words[i]);
      SemoutCount++;
    }
  }

  fclose(fp_out);



  for (int i = 0; i < numWorkers; i++) {
    printf("Worker %d discovered %d palindromes and %d Semordnilaps\n", i, PalCount[i], SemorCount[i]);
    Paltotal += PalCount[i];
    Semototal += SemorCount[i];
  }
  printf("Total palindromes: %d\n", Paltotal);
  printf("Total semordnilaps words: %d\n", Semototal);



//Destroy locks

  pthread_mutex_destroy(&lockPal);
  pthread_mutex_destroy(&lockSemo);



  /* get end time */
  end_time = read_timer();

printf("total time: %f\n", end_time - start_time);

  for (i = 0; i < word_count; i++) {
    free(words[i]);
  }
  free(words);

}



