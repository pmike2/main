#include <iostream>

using namespace std;

int main(int argc, char **argv) {
	unsigned char * buffer= new unsigned char[9000000000];
	if (buffer== 0) {
		cout << "ERROR\n";
	}
	delete[] buffer;
	
	return 0;
}
