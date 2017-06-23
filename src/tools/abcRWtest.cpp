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

	// Write data to register
	uint32_t header = 0b1011111000000;
	uint32_t reg = 0x10;
	uint32_t abcAddr = 0b11111;
	uint32_t rwBit = 1;
	uint32_t data = 0xdeadbeef;
	
	uint32_t writeCmd = header;
	writeCmd = (writeCmd << 5) | abcAddr;
	writeCmd = (writeCmd << 7) | reg;
	writeCmd = (writeCmd << 1) | rwBit;

	uint32_t readCmd = writeCmd-1;

	// Send data to the fifo
	std::cout << "Sending 0x" << std::hex << writeCmd << data << " to the FIFO" << std::endl;
	myTx.writeFifo(writeCmd);
	myTx.writeFifo(data);

	// Send read request
	std::cout << "Sending 0x" << std::hex << readCmd << data << " to the FIFO" << std::endl;
	myTx.writeFifo(readCmd);
	myTx.writeFifo(0);

	std::cout << "Done." << std::endl;

	return 0;
}
