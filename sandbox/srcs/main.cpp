
#include <iostream>
#include <fstream>
#include <iostream>
#include <sstream>
#include <vector>
#include <thread>


int number = 0;


void increment(){
	std::mutex mtx;
	mtx.lock();
	for(int i=0; i<1000000; i++){
		number++;
	}
	mtx.unlock();
}


int main() {
	std::thread t1(increment);
	std::thread t2(increment);

	t1.join();
	t2.join();

	std::cout << "Number after execution of t1 and t2 is " << number << "\n";

	return 0;
}
