
#ifndef _REENTRANT
#define _REENTRANT
#endif
#include <ctype.h>
#include <pthread.h>
#include <stdlib.h>
#include <stdio.h>
#include <stdbool.h>
#include <string.h>
#include <time.h>
#include <sys/time.h>
#include <omp.h>


#define MAXWORDS 25145
#define MAXWORDLENGTH 256

#define MAXWORKERS 10   /* maximum number of workers if W was not specified in command line */



//define char array pointer
char **words = NULL;

//Helper arrays and global variables
int Palresults[MAXWORDS]; //marked with 1 if corresponding i is a palindrome
int PalCount[MAXWORKERS] = {0}; //holds how many palindromes each thread found

int Semoresults[MAXWORDS];
int SemorCount[MAXWORKERS] = {0};

int Paltotal = 0;
int Semototal = 0;
int word_count = 0; //How many words actually in file
int numWorkers; //Actual number of workers

int cmpstr(const void* str1,const void* str2){
  char *a1 = *(char**)str1;
  char *a2 = *(char**)str2;
  return strcmp(a1, a2);
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

bool binary_search(char* words[], char* target) {
  int left = 0, right = word_count - 1;
  int mid;

  while(left <= right) {
    mid = (left+right)/ 2;

    int cmp = strcmp(words[mid], target);
    if (cmp == 0) {
      return true;
    }
    if (cmp > 0) {
      right = mid - 1;
    }
    else {
      left = mid + 1;
    }

  }
  return false;
}


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
  if (isPalindrome(word) == true) {
    return false;
  }
  int i;
  int wordLength = strlen(word);
  char reversed[wordLength + 1];
  for (i = 0; i < wordLength; i++) {
    reversed[i] = word[wordLength - i - 1];
  }
  reversed[wordLength] = '\0';

  if (binary_search(words, reversed) == true) {
      return true;

  }
  return false;
}

void *Palindrome(void *word) {
  int  i,j;

#pragma omp parallel for shared(words)

  for (i= 0; i <= word_count; i++) {
    long myid = omp_get_thread_num();
    if (words[i] == NULL) {continue;}

    //Check for palindroms
    if (i < MAXWORDS && isPalindrome(words[i])) {
      Palresults[i] = 1;
      PalCount[myid]++;
#pragma omp atomic
      Paltotal++;
    }
    else {Palresults[i] = 0;}
    //Check for semordnilaps
    if (i<MAXWORDS && isSemor(words[i])) {
      Semoresults[i] = 1;
      SemorCount[myid]++;
#pragma omp atomic
      Semototal++;
    }
    else{Semoresults[i] = 0;}
  }
}


/* read command line, initialize, and create threads */
int main(int argc, char *argv[]) {
  int i, j;

  //FILE READING
  FILE *fp_in;
  const char *filename = "wordFile";
  char lineBuff[MAXWORDLENGTH]; //Buffer

  fp_in = fopen(filename, "r");
  if (fp_in == NULL) {
    perror("Error opening file");
    return 1;
  }
  words = (char **)malloc(MAXWORDS * sizeof(char *));

  while (fgets(lineBuff, MAXWORDLENGTH, fp_in) != NULL && word_count < MAXWORDS) {
    size_t wordLength = strlen(lineBuff);
    if (wordLength > 0 && lineBuff[wordLength - 1] == '\n') {
      lineBuff[wordLength- 1] = '\0';
    }

    for (j = 0; j < wordLength; j++) {
      lineBuff[j] = tolower(lineBuff[j]);
    }
    //Allocate space for the word to be stored
    words[word_count] = (char *) malloc((strlen(lineBuff) + 1) * sizeof(char));
    //Move the word in buffer to allocated space
    strcpy(words[word_count], lineBuff);
    word_count++;
  }
  fclose(fp_in);
  printf("There are %d words in the file.\n", word_count);
  qsort(words, word_count -1 ,  sizeof(char*), cmpstr);


  /* read command line args if any */
  numWorkers = (argc > 1) ? atoi(argv[1]) : MAXWORKERS; //Gets number of threads from command line. If not, use MAXWORKERS
  if (numWorkers > MAXWORKERS) numWorkers = MAXWORKERS;
  stripSize = (word_count + numWorkers - 1) / numWorkers;


  start_time = omp_get_wtime();
  double start2 = read_timer();

  omp_set_num_threads(numWorkers);
  Palindrome(words);

  double end2 = read_timer();
  end_time = omp_get_wtime();


  //WRITE TO OUTPUT
  FILE *fp_out;
  const char *outfile = "OutputFile";
  fp_out = fopen(outfile, "w");

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

    }
  }
  fprintf(fp_out, "----------------------------------------------------\nSemordnilaps:\n");

  for (int i = 0; i < word_count; i++) {
    if (Semoresults[i] == 1) {
      fprintf(fp_out, "%s\n", words[i]);

    }
  }
  fclose(fp_out);

  for (int i = 0; i < numWorkers; i++) {
    printf("Worker %d discovered %d palindromes and %d Semordnilaps\n", i, PalCount[i], SemorCount[i]);

  }
  printf("Total palindromes: %d\n", Paltotal);
  printf("Total semordnilaps words: %d\n", Semototal);


printf("total time: %f\n", end_time - start_time);
  printf("total time 2: %f\n", end2 - start2);

  for (i = 0; i < word_count; i++) {
    free(words[i]);
  }
  free(words);

}



