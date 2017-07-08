#include <SpecController.h>
#include <TxCore.h>
#include <RxCore.h>
#include <RawData.h>
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
	RxCore myRx(&mySpec);
	
	// Enable all channels
	myTx.setCmdEnable(chan);
	myRx.setRxEnable(chan);

	// Write data to register
	uint32_t header = 0b1011111000000;
	uint32_t reg = 0x10;
	uint32_t abcAddr = 0b11111;
	uint32_t rwBit = 1;
	uint32_t data = (1<<8)|(1<<12);

	uint32_t writeCmd = header;
	writeCmd = (writeCmd << 5) | abcAddr;
	writeCmd = (writeCmd << 7) | 0x20;
	writeCmd = (writeCmd << 1) | rwBit;
	
	std::cout << "Configuring register." << std::endl;
	myTx.writeFifo(writeCmd);
	myTx.writeFifo(data);
	usleep(100);

	std::cout << "Scanning registers." << std::endl;
	

	for(int r=0; r<0x7f; r++){
		writeCmd = header;
		writeCmd = (writeCmd << 5) | abcAddr;
		writeCmd = (writeCmd << 7) | r;
		//writeCmd = (writeCmd << 1) | rwBit;

		myTx.writeFifo(writeCmd);
		myTx.writeFifo(0);

		RawData*rx_data = myRx.readData();
		usleep(100);
		if(rx_data == NULL){
			printf("Nothing received from r:%i.\n",r);
			continue;
		}

		std::cout << " register:" << r << "##" << std::endl;
		for(int b=0; b<sizeof(rx_data->buf)/sizeof(rx_data->buf[0]); b++)
			std::cout << b << ":" << std::hex << rx_data->buf[b] << std::dec << std::endl;
	}

	std::cout << "Done." << std::endl;

	return 0;
}

