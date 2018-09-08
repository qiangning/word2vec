//
// Created by qning2 on 8/15/18.
//

#include <stdio.h>
#include <string.h>
#include <math.h>

#include <stdlib.h>
#include <stdbool.h>

#define MAX_STRING 100
const long long max_size = 2000;         // max length of strings
const long long max_w = 50;              // max length of vocabulary entries


typedef float real;
char output_file[MAX_STRING];
char vocabOfInterest_file[MAX_STRING];
char *vocabOfInterest;

bool isKeep(char* keepVocab, int keepVocabSize, char* word){
    int i;
    for(i=0;i<keepVocabSize;i++){
        if(strcmp(word,&keepVocab[i*max_w])==0) return true;
    }
    return false;
}
int main(int argc, char **argv) {
    FILE *f;
    char file_name[max_size];
    float len;
    long long words, size, a, b, c, d;
    float *M;
    char *vocab;
    strcpy(file_name, argv[1]);
    strcpy(output_file,argv[2]);
    strcpy(vocabOfInterest_file,argv[3]);
    const long long classes = atoi(argv[4]);
    const int iter = atoi(argv[5]);

    // filter vocabulary
    char line[max_size], *keepVocab;
    printf("KeepVocabFile=%s\n",vocabOfInterest_file);
    f = fopen(vocabOfInterest_file,"r");
    long long keepVocabSize = 0;
    while(!feof(f)){
        fscanf(f,"%[^\n]\n",line);
        keepVocabSize++;
    }
    printf("keepVocabSize=%lld\n",keepVocabSize);
    rewind(f);
    keepVocab = (char *) malloc((long long) keepVocabSize * max_w * sizeof(char));
    for(b=0;b<keepVocabSize;b++){
        a = 0;
        while (1) {
            keepVocab[b * max_w + a] = fgetc(f);
            if (keepVocab[b * max_w + a] == ' ') break;
            if (a < max_w) a++;
        }
        keepVocab[b * max_w + a] = 0;
        printf("%s\n",&keepVocab[b*max_w]);
        fscanf(f,"%[^\n]\n",line);
    }
    fclose(f);
    // read embeddings
    f = fopen(file_name, "rb");
    if (f == NULL) {
        printf("Input file not found\n");
        return -1;
    }
    fscanf(f, "%lld", &words);
    fscanf(f, "%lld", &size);
    //vocab = (char *) malloc((long long) words * max_w * sizeof(char));
    //M = (float *) malloc((long long) words * (long long) size * sizeof(float));
    vocab = (char *) malloc((long long) keepVocabSize * max_w * sizeof(char));
    M = (float *) malloc((long long) keepVocabSize * (long long) size * sizeof(float));
    if (M == NULL) {
        printf("Cannot allocate memory: %lld MB    %lld  %lld\n", (long long) keepVocabSize * size * sizeof(float) / 1048576,
               keepVocabSize, size);
        return -1;
    }
    printf("Loading...\n");
    c = 0;
    for (b = 0; b < words; b++) {
        if((b+1)%30000==0) printf("%lld/%lld\n",b+1,words);
        a = 0;
        while (1) {
            vocab[c * max_w + a] = fgetc(f);
            if (feof(f) || (vocab[c * max_w + a] == ' ')) break;
            if ((a < max_w) && (vocab[c * max_w + a] != '\n')) a++;
        }
        vocab[c * max_w + a] = 0;
        for (a = 0; a < size; a++) fread(&M[a + c * size], sizeof(float), 1, f);
        len = 0;
        for (a = 0; a < size; a++) len += M[a + c * size] * M[a + c * size];
        len = sqrt(len);
        for (a = 0; a < size; a++) M[a + c * size] /= len;
        if(isKeep(keepVocab,keepVocabSize,&vocab[c*max_w])){
            c++;
        }
    }
    fclose(f);

    words = keepVocabSize;
    // Run K-means on the word vectors
    f = fopen(output_file, "wb");
    printf("K-means...\n");
    int clcn = classes, closeid;
    int *centcn = (int *) malloc(classes * sizeof(int));
    int *cl = (int *) calloc(words, sizeof(int));
    real closev, x;
    real *cent = (real *) calloc(classes * size, sizeof(real));
    for (a = 0; a < words; a++) cl[a] = a % clcn;
    for (a = 0; a < iter; a++) {
        printf("Iter:%lld\n",a);
        for (b = 0; b < clcn * size; b++) cent[b] = 0;
        for (b = 0; b < clcn; b++) centcn[b] = 1;
        for (c = 0; c < words; c++) {
            for (d = 0; d < size; d++) cent[size * cl[c] + d] += M[c * size + d];
            centcn[cl[c]]++;
        }
        for (b = 0; b < clcn; b++) {
            closev = 0;
            for (c = 0; c < size; c++) {
                cent[size * b + c] /= centcn[b];
                closev += cent[size * b + c] * cent[size * b + c];
            }
            closev = sqrt(closev);
            for (c = 0; c < size; c++) cent[size * b + c] /= closev;
        }
        for (c = 0; c < words; c++) {
            closev = -10;
            closeid = 0;
            for (d = 0; d < clcn; d++) {
                x = 0;
                for (b = 0; b < size; b++) x += cent[size * d + b] * M[c * size + b];
                if (x > closev) {
                    closev = x;
                    closeid = d;
                }
            }
            cl[c] = closeid;
        }
    }
    // Save the K-means classes
    for (a = 0; a < words; a++) fprintf(f, "%s %d\n", &vocab[a*max_w], cl[a]);
    fclose(f);
    free(centcn);
    free(cent);
    free(cl);

    return 0;
}
