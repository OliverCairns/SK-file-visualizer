
#include "displayfull.h"
#include "sketch.h"
#include <stdio.h>
#include <stdlib.h>
#include <stdbool.h>


state *newState() {
  state*  s  = (state*)malloc(sizeof(state));
  s->x=0;
  s->y=0;
  s->tx=0;
  s->ty=0;
  s->start=0;
  s->end=false;
  s->tool=LINE;
  s->data=0;
  return s;
}

void freeState(state *s) {
  free(s);
}

int getOpcode(byte b) {
  int a;
  a= b>>6;
  return a;
}

int getOperand(byte b) {
  if ((b & 0x20)==0x20){
    signed char d= (signed char)b;
    d= d | 0xe0;
    signed int e= (signed char)d;
    return e  ;
  }
  else {
    unsigned int c= b&0x1f;
    return c;
  }
}

void action(display *d, state *s){
  if (s->tool==LINE)line(d, s->x,s->y,s->tx,s->ty);
  else if (s->tool==BLOCK)   block(d, s->x,s->y,(s->tx)-(s->x),(s->ty)-(s->y));
  s->x = s->tx;
  s->y = s->ty;
}

void obey(display *d, state *s, byte op) {
  unsigned int b= getOperand(op);
  switch(getOpcode(op))
  {
    case 0:
      s->tx+=getOperand(op);
      break;
    case 1:
      s->ty+=getOperand(op);
      action(d,s);
      break;
    case 2:
      switch(getOperand(op))
      {
        case 0:
          s->tool=NONE;
          break;
        case 1:
          s->tool=LINE;
          break;
        case 2:
          s->tool=BLOCK;
          break;
        case 3:
          colour(d, s->data);
          break;
        case 4:
          s->tx=s->data;
          break;
        case 5:
          s->ty=s->data;
          break;
        case 6:
          show(d);
          break;
        case 7:
          pause(d, s->data);
          break;
        case 8:
          s->end=true;
          break;
      }
      s->data=0; 
      break;
    case 3:
      s->data=(s->data)<<6;
      s->data= s->data | (b & 0x3f);
      break;
  }
}

int findSize(char *name){
	FILE *f=fopen(name, "rb");
	if (f == NULL) return -1;
	fseek(f, 0, SEEK_END);
	int size = ftell(f);
	fclose(f);
	return size;
}
//Global variable to keep track of s->start
int i;

bool processSketch(display *d, void *data, const char pressedKey) {
    if (data != NULL) {
      FILE *in= fopen(getName(d),"rb");
      int size=findSize(getName(d));
      unsigned char op [size];
      fread(&op,sizeof(op),size,in);
      state *s= newState();
      s->start=i; 
      while (s->end==false && i<size){
        obey(d, s, op[i]);
        i++;
      }      
      if (s->end==false) {
        i=0; 
      }
      show(d);
      freeState(s);
    }
    else return (pressedKey == 27);
  return (pressedKey == 27);
}

void view(char *filename) {
  display *d = newDisplay(filename, 200, 200);
  state *s = newState();
  run(d, s, processSketch);
  freeState(s);
  freeDisplay(d);
}

#ifndef TESTING
int main(int n, char *args[n]) {
  if (n != 2) { 
    byte b=00000000;
    getOpcode(b);
    printf("Use ./sketch file\n");
    exit(1);
  } else view(args[1]); 
  return 0;
}
#endif
