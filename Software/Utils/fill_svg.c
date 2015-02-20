#include <math.h>
#include <stdio.h>
#include <stdlib.h>

#define false 0
#define true 1

#define max(a,b) (((a)>(b))?(a):(b))
#define min(a,b) (((a)<(b))?(a):(b))

typedef int bool;

static FILE* svgFile = NULL;


bool getSvgData(int point, float *x, float* y, int* pen);

bool getDataInternal(char* svgName, int point, float *x, float* y, int* pen)
{  
  if(svgFile == NULL) {
    svgFile = fopen(svgName, "r");
    if(svgFile) {
      //      printf("found svg file\n"); 
    }
    else {
      printf("Could not find svg file: %s\n", svgName); 
      exit(1);
    }

  }
  
  if(svgFile) {
    return getSvgData(point, x, y, pen);    
  }
  else
  {
    return false;
  }
}

static int lastReadPoint = -1;

bool
seekTo(char* pattern)
{
  //yes, this is flawed string matching
  char* tmp = pattern;

  while(*tmp != '\0') {
      char c = fgetc(svgFile);
      if(c < 0) {
         return false;
      }
      if(c == *tmp) {
        tmp++;
      }
      else {
        tmp = pattern;        
      }      
  }
  return true;  
}

bool
seekToPathStart() {
    if(!seekTo("<path")) {
      //      printf("No <path> tag");    
      return false;
    }
    
    if(!seekTo(" d=\"")) {
      //      printf("No d=\" in path tag");    
      return false;
    }
  return true;  
}

bool
readFloat(float* ret) {
  char tmp[20];
  float f=0;
  bool pastPoint=false;
  long div = 1;
  
  for(int i=0 ; i<20 ; i++) {
    tmp[i] = fgetc(svgFile);
    if(tmp[i]<0) {
      return false;
    } 
    else if((tmp[i] >= '0') && (tmp[i] <= '9')) {
      f = f*10+(tmp[i]-'0');      
      if(pastPoint) {
         div = div*10;
      }
    }
    else if(tmp[i] == '.') {
      pastPoint=true;
    }
    else {
      break;
    }   
  }  
  *ret = f/div;
  
  return true;
}

bool
getNextPathSegment(float *x, float *y, int *line)
{
  char c = fgetc(svgFile);
  
  if(c == 'M') {
    *line = 0;     
  }
  else if(c == 'L') {
    *line = 1;
  }
  else {
    return false;
  }      
  
  if(!readFloat(x)) {
    return false;    
  } 
  if(!readFloat(y)) {
    return false;    
  } 
  //rewind one byte that was eaten by last float read  
  fseek(svgFile, -1, SEEK_CUR);
    
  return true; 
}

static float min_x = 100000000.0;
static float max_x = -100000000.0;
static float min_y = 100000000.0;
static float max_y = -100000000.0;
static float scaleFactor = 1.0;

static float lastX,lastY;
static int lastPen;
static long pathPosition = -1;
static long pathEndPosition = -1;

bool getSvgData(int point, float *x, float* y, int* pen)
{
  if(point == 0) {
    long segments = 0;
    //first read, get dimensions

    if(!seekToPathStart()) {
      //      printf("No path found in svg file!");    
      return false;
    }    
    //    printf("Woho found <path> in svg file!");    
    pathPosition = ftell(svgFile);

    while(true) {
      if(getNextPathSegment(x, y, pen)) {
        segments++;        
        min_x = min(min_x, *x);
        max_x = max(max_x, *x);
        min_y = min(min_y, *y);
        max_y = max(max_y, *y);
      }
      else {
	pathEndPosition = ftell(svgFile);
        break;
      }
    }
    scaleFactor = 200.0 / max( (max_x-min_x) , (max_y-min_y) );
    
    //    printf("Segments=%d\n", (int)segments);    
    // printf("Scale factor=%f\n", scaleFactor);    
    //printf("Max_x=%f\n", max_x);    

    fseek(svgFile, pathPosition, SEEK_SET);    
  }

  if(point != lastReadPoint) {
    if(getNextPathSegment(x, y, pen)) {
      *x = (*x-min_x)*scaleFactor;   
      *y = (*y-min_y)*scaleFactor;
   
      lastReadPoint = point;
      lastX = *x;
      lastY = *y;
      lastPen = *pen;   
    }
    else {
      // rewind the file:
      rewind(svgFile);
      
      return false;
    }    
  }
  else {
    *x = lastX;    
    *y = lastY;    
    *pen = lastPen;    
  }
  return true;
}

typedef struct {
  float x;
  float y;    
} Point;

typedef struct {
  Point p1;
  Point p2;    
} Line;

static int
cmpPointX(const void *a, const void *b)
{
  return ((Point*)a)->x > ((Point*)b)->x;
}

static int
cmpLineY(const void *a, const void *b)
{
  return ((Line*)a)->p1.y > ((Line*)b)->p1.y;
}


int main(int argc, char **argv)
{
  int point = 0;
  float x,y;
  int pen;
  
  float prevX=0, prevY=0;
  int vecsize = 100;
  Line* lines = malloc(vecsize*sizeof(Line));
  int nLines = 0;

  while(getDataInternal("/home/fredrik/stringent/Designs/hello_world2.svg", point, &x, &y, &pen)) {
    //    printf("."); fflush(stdout);

    if(pen > 0) {
      if(point >= vecsize) {
	vecsize *= 2;
	lines = realloc(lines, vecsize*sizeof(Line));
      }
      Point p1 = {x,y};
      Point p2 = {prevX,prevY};
      Line l = {p1,p2};
      if(p1.y > p2.y) {
	l.p1 = p2;
	l.p2 = p1;
      }
      lines[nLines++] = l;    
    }

    prevX = x;
    prevY = y;
    point++;
  }    
  //  printf("points=%d lines=%d\n", point, nLines);

  qsort(lines, point, sizeof(Line), cmpLineY);

  //  for(int i=0 ; i<point ; i++) {
  //  printf("%f2.2\n", lines[i].p1.y);
  //}

  rewind(svgFile);
  while(pathPosition-- > 0) {
    putchar(fgetc(svgFile));
  }

  for(int i=0 ; i<nLines ; i++) {
    printf("M%f %fL%f %f", lines[i].p1.x, lines[i].p1.y, lines[i].p2.x, lines[i].p2.y);
  }

  Point* intersections = malloc(nLines*sizeof(Point));
  for(int y=0 ; y<200 ; y+=5) {
    int nIntersections = 0;
    for(int l=0 ; l<nLines ; l++) {
      Line line = lines[l];
      if(line.p1.y <= y && line.p2.y > y) {
	float x = line.p1.x+ (line.p2.x-line.p1.x)*((y-line.p1.y)/(line.p2.y-line.p1.y));
	Point p = {x,y};
	intersections[nIntersections++] = p;
      }
    }
    //    printf("nInter=%d\n", nIntersections);
    qsort(intersections, nIntersections, sizeof(Point), cmpPointX);
    for(int i=0 ; i<nIntersections ; i+=2) {
      printf("M%f %fL%f %f", intersections[i].x, intersections[i].y, intersections[i+1].x, intersections[i+1].y);
    }
  }
  putchar('\"');
  fseek(svgFile, pathEndPosition, SEEK_SET);
  char c;
  while((c=fgetc(svgFile)) > 0) {
    putchar(c);
  }

  fclose(svgFile);
  free(lines);
  return 0;
}
