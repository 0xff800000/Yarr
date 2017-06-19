#include <SpecController.h>
#include <TxCore.h>
#include <iostream>
#include <stdint.h>
#include <string.h>

int main(int argc, char **argv) {
	// Init the spec controller
	int specNum = 0;
	if (argc == 2)
		specNum = atoi(argv[1]);
	SpecController mySpec(specNum);
	TxCore myTx(&mySpec);
	
	// Enable all channels


	// Send data to the fifo
	std::cout << "Sending numbers..." << std::endl;
	for(uint32_t i=0; i<0xffffffff;i++){
		myTx.setCmdEnable(1<<2);
		myTx.writeFifo(i);
		myTx.setCmdEnable(1<<3);
		myTx.writeFifo(~i);
	}
	std::cout << "Done." << std::endl;

	return 0;
}
