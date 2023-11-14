/* map.c
 * ----------------------------------------------------------
 *  CS350
 *  Assignment 1
 *  Question 1
 *
 *  Purpose:  Gain experience with threads and basic
 *  synchronization.
 *
 *  YOU MAY ADD WHATEVER YOU LIKE TO THIS FILE.
 *  YOU CANNOT CHANGE THE SIGNATURE OF MultithreadedWordCount.
 * ----------------------------------------------------------
 */
#include "data.h"

#include <stdlib.h>
#include <string.h>
#include <pthread.h>

/* --------------------------------------------------------------------
 * MultithreadedWordCount
 * --------------------------------------------------------------------
 * Takes a Library of articles containing words and a word.
 * Returns the total number of times that the word appears in the
 * Library.
 *
 * For example, "There the thesis sits on the theatre bench.", contains
 * 2 occurences of the word "the".
 * --------------------------------------------------------------------
 */

struct args {
    struct Library * lib;
    char * word;
    unsigned int start, end;
};

void countMatch(struct args* a) {
//    size_t wordCount = 0;
    size_t * wordCount = malloc(sizeof(size_t*));
    (*wordCount) = 0;
    for(unsigned int i = a->start; i <= a->end; i++) {
        struct Article * art = a->lib->articles[i];
        for(unsigned int j = 0; j < art->numWords; j++) {
            size_t len = strnlen( art->words[j], MAXWORDSIZE );
            if(!strncmp(art->words[j], a->word, len))
                (*wordCount) += 1;
        }
    }
    pthread_exit(wordCount);
    return;
}

size_t MultithreadedWordCount( struct  Library * lib, char * word)
{
    size_t wordCount = 0;
    pthread_t threads[NUMTHREADS];
    struct args a[NUMTHREADS];
    int* ans[NUMTHREADS];
  printf("Parallelizing with %d threads...\n",NUMTHREADS);
    /* XXX FILLMEIN
     * Also feel free to remove the printf statement
     * to improve time */
    unsigned int quad = lib->numArticles / NUMTHREADS;
    for(int i = 0; i < NUMTHREADS - 1; i++) {
        a[i].lib = lib;
        a[i].word = word;
        a[i].start = i*quad;
        a[i].end = (i+1)*quad - 1;
        pthread_create(&threads[i], NULL, &countMatch, &a[i]);
    }
    a[NUMTHREADS-1].lib = lib;
    a[NUMTHREADS-1].word = word;
    a[NUMTHREADS-1].start = (NUMTHREADS-1)*quad;
    a[NUMTHREADS-1].end = lib->numArticles - 1;
    pthread_create(&threads[NUMTHREADS-1], NULL, &countMatch, &a[NUMTHREADS-1]);

    for(int i = 0; i < NUMTHREADS; i++) {
        pthread_join(threads[i], &ans[i]);
        wordCount += *ans[i];
        free(ans[i]);
    }

    return wordCount;
}
