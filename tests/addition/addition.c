// MiniC program to test addition
extern int print_int(int X, bool y);

int addition(int n, int m){
	int result;
	result = n + m;
  

  if(n == 4) {
    print_int(n+m);
  }
  else {
    print_int(n*m);
  }

  return result;
}

