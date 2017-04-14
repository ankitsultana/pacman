#include <iostream>
#include <cstdlib>

using namespace std;

int main(int argc, char ** argv){
	for(int i=0; i<2*atoi(argv[2]); i++){
		cout << "X";
	}
	cout << endl;
	for(int i=0; i<(atoi(argv[1])-2); i++){
		cout << "XX";
		for(int j=0; j<2*atoi(argv[2])-4; j++){
			cout << " ";
		}
		cout << "XX";
		cout << endl;
	}
	for(int i=0; i<2*atoi(argv[2]); i++){
		cout << "X";
	}
	cout << endl;
	return 0;
}
