#include <iostream>

using namespace std;

const int N = 29893;
const int E = 27;
const int Z = 29548;

int find(int e, int d, int z){
	if(((e*d) % z) == 1){
		return d;
	} else{
		return 0;
	}
}

int main(){
	for(int i = 0; i < N; i++){
		if(find(E, i, Z) != 0){
			cout << find(E, i, Z) << endl;
		}
	}
	char ch;
	cin >> ch;
}