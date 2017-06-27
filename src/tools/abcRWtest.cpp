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
	uint32_t data = 0xdeadbeef;
	
	std::cout << "Scanning registers." << std::endl;
	
	for(int a=0; a<abcAddr; a++){
		for(int r=0; r<0x7f; r++){
			uint32_t writeCmd = header;
			writeCmd = (writeCmd << 5) | a;
			writeCmd = (writeCmd << 7) | r;
			writeCmd = (writeCmd << 1) | rwBit;

			uint32_t readCmd = writeCmd-1;

			myTx.writeFifo(writeCmd);
			myTx.writeFifo(data);
			
			usleep(100);
			myTx.writeFifo(readCmd);
			myTx.writeFifo(0);
			RawData*rx_data = myRx.readData();
			usleep(100);
			if(rx_data == NULL){
				printf("Nothing received from a:%d;r:%i.\n",a,r);
				continue;
			}

			std::cout << "## Address:" << a << " register:" << r << "##" << std::endl;
			for(int b=0; b<sizeof(rx_data->buf)/sizeof(rx_data->buf[0]); b++)
				std::cout << b << ":" << std::hex << rx_data->buf[b] << std::dec << std::endl;
		}
	}

	std::cout << "Done." << std::endl;

	return 0;
}

