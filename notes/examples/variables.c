

int main(int argc, char const *argv[])
{
  // we can assign a value immediately
  int number = 10;

  // or we can create a variable then assign it later
  int counter;

  counter = 1;
  // we can then reassign a variable to a new value, as long as it is the same type
  counter = 100;

  // we cannot reassign a variable to a value that is of a different type

  counter = 'abc';
  return 0;
}
