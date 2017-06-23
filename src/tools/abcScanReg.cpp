#include <SpecController.h>
#include <TxCore.h>
#include <iostream>
#include <stdint.h>
#include <string.h>
#include <unistd.h>

int main(int argc, char **argv) {
	int chan = ~0;
	if (argc == 2)
		chan = atoi(argv[1]);

	// Init the spec controller
	SpecController mySpec(0);
	TxCore myTx(&mySpec);
	
	// Enable all channels
	myTx.setCmdEnable(chan);

	// Write data to register
	uint32_t header = 0b1011111000000;
	uint32_t reg = 0x10;
	uint32_t abcAddr = 0b11111;
	uint32_t rwBit = 1;
	uint32_t data = 0xdeadbeef;

	// For all ABCs
	for(int i=0; i<abcAddr; i++){
		std::cout << "ABC : " << i << std::endl;
		// For all registers
		for(int r=0; r<0x7f; r++){
			std::cout << "Register : " << r << std::endl;
			uint32_t writeCmd = header;
			writeCmd = (writeCmd << 5) | i;
			writeCmd = (writeCmd << 7) | r;
			writeCmd = (writeCmd << 1) | rwBit;

			uint32_t readCmd = writeCmd-1;			
			myTx.writeFifo(readCmd);
			myTx.writeFifo(0);
			
			usleep(100);
		}
	}

	std::cout << "Done." << std::endl;

	return 0;
}
