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

	// Scan the tx_channels
	std::cout << "Scanning..." << std::endl;
	for(uint32_t i=1; i<32;i++){
		// Enable ith channel
		myTx.setCmdEnable(1<<i);
		for(int j=0;j<100;j++)myTx.writeFifo(i);
	}
	std::cout << "Done." << std::endl;

	return 0;
}
