#include "util.h"

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

void vector_init(vector *v){
  v->data = NULL;
  v->size = 0;
  v->count = 0;
}
int vector_count(vector* v){
  return v->count;
}
void vector_add(vector* v, void *e){
  if(v->size == 0){
    v->size = 10;
    v->data = malloc(sizeof(void*)*v->size);
    memset(v->data, '\0', sizeof(void) * v->size);
  }
  if(v->size == v->count){
    v->size *= 2;
    v->data = realloc(v->data, sizeof(void*) * v->size);
  }
  v->data[v->count] = e;
  v->count++;
}

void vector_set(vector* v, unsigned i, void* e){
  if(i >= v->count){
    return;
  }
  v->data[i] = e;
}

void * vector_get(vector* v, unsigned i){
  if(i >= v->count){
    return NULL;
  }
  return v->data[i];
}

void vector_delete(vector* v, unsigned index){
  if(index >= v->count){
    return;
  }
  v->data[index] = NULL;
  int i, j;
  void ** new_arr = (void**)malloc(sizeof(void*) * v->count * 2);
  for(i = 0, j =0; i < v->count; i++){
    if(v->data[i] != NULL){
      new_arr[j] = v->data[i];
      j++;
    }
  }
  free(v->data);
  v->data = new_arr;
  v->count--;
}

void vector_free(vector* v){
  free(v->data);
}
