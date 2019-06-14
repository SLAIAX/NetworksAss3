#include <iostream>
#include "InfInt.h"

using namespace std;

bool isPrime(InfInt n) 
{ 
  bool flag = true;
   for(InfInt i = 2; i <= n / 2; i++){
      if((n % i) == 0){
        flag = false;
        break;
      }
   }
   return flag;
} 

int main(){
	for(int i=0; i < 1000; i++){
		if(isPrime(i)){
			cout << i << endl;
		}
	}
}