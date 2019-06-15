#include <iostream>
#include "InfInt.h"

using namespace std;

InfInt repeatSquare(InfInt x, InfInt e, InfInt n) {

	InfInt y=1;//initialize y to 1, very important
	while (e >  0) {
		if (( e % 2 ) == 0) {
			x = (x*x) % n;
			e = e/2;
		}
		else {
			y = (x*y) % n;
			e = e-1;
		}
	}
	return y; //the result is stored in y
}


int main(){
	//int p = 11;
	//int q = 13;
	char c;
	while(true){
		cout << "Enter letter PRESS ENTER" << endl;
		cin >> c;
		InfInt ch = c;
		cout << "Value before encryption: " << ch.toString() << endl;
		InfInt cipher = repeatSquare(ch, 7, 143);
		cout << "Encrypted Value: " << cipher.toString() << endl;
		ch = repeatSquare(cipher, 103, 143);
		cout << "Decrypted final value: " << ch.toString() << endl;
		c = ch.toInt();
		cout << "Character is: " << c << endl;
	}
}