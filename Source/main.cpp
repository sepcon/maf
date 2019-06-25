void thaf_fwapp_runTest();
void thaf_srz_runTest();

#include <stdint.h>
#include <iostream>
#include <tuple>


#define REMOVE_LAST_ARG(a,b,c) a,b
void print(int a, int b) {
	std::cout << "Hello" << std::endl;
}
int main()
{
    thaf_srz_runTest();
	std::tuple<REMOVE_LAST_ARG(int, int,)> tp;
	std::cin.get();
    return 0;
}
