// structures are used to declare new types
// structures are several variables bundled together into a single package
#include <math.h>

typedef struct
{
  float x;
  float y;
} point;

// you can use a struct as a type in a parameter
float get_len_from_point(point p)
{
  float res = sqrt(p.x * p.x + p.y * p.y);
  return res;
}

point increment_point(point p)
{
  point new_point;

  new_point.x = p.x + 1;
  new_point.y = p.y + 1;
  return new_point;
}

int main(int argc, char const *argv[])
{
  // you can use a struct like any other type, once you have defined it
  point p1;

  p1.x = 4.5;
  p1.y = 12.12;

  float p1_len = get_len_from_point(p1);

  point p2 = increment_point(p1);

  return 0;
}
