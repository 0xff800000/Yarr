#include <SpecController.h>
#include <TxCore.h>
#include <iostream>
#include <stdint.h>
#include <string.h>

int main(int argc, char **argv) {
	int chan = ~0;
	if (argc == 2)
		chan = atoi(argv[1]);

	// Init the spec controller
	SpecController mySpec(0);
	TxCore myTx(&mySpec);
	
	// Enable all channels
	myTx.setCmdEnable(chan);

	// Send data to the fifo
	std::cout << "Sending ones..." << std::endl;
	for(uint32_t i=0; i<100;i++){
		myTx.writeFifo(~0);
	}
	std::cout << "Done." << std::endl;

	return 0;
}
