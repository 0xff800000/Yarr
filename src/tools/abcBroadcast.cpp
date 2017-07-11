#include <SpecController.h>
#include <TxCore.h>
#include <RxCore.h>
#include <RawData.h>
#include <iostream>
#include <stdint.h>
#include <string.h>
#include <unistd.h>

void sendCommand(TxCore&tx, uint8_t hcc_adr, uint8_t abc_adr, uint8_t reg_adr, uint8_t rwBit,uint32_t data,bool verbose=false){
        uint32_t header = 0x0;
        header += (0xbe << 18);
        header += ((0x1F & hcc_adr) << 13);
        header += ((0x1F & abc_adr) << 8);
        header += ((0x7F & reg_adr) << 1);
        header += ((0x1 & rwBit));
	
//	if(verbose)std::cout<<"Sending 0x"<<std::hex<<header<<data<<std::endl;
	if(verbose)printf("Sending 0x%08x%08x \n",header,data);
	tx.writeFifo(header);
	tx.writeFifo(data);
}

void reset(TxCore&tx){
	// Reset
	tx.writeFifo(0xa3);
	tx.writeFifo(0);
	tx.writeFifo(0);
	tx.writeFifo(0xa9);
	tx.writeFifo(0);
	tx.writeFifo(0);

	// Write disable
	sendCommand(tx,0,0x1f,0x00,1,(1<<4),false);
	tx.writeFifo(0);
	tx.writeFifo(0);

	// Enable FIFOs
	uint32_t data = (1<<8)|(1<<12);
	sendCommand(tx,0,0x1f,0x00,1,(data),false);
	tx.writeFifo(0);
	tx.writeFifo(0);

	// Write disable
	sendCommand(tx,0,0x1f,0x00,1,7,false);
	tx.writeFifo(0);
	tx.writeFifo(0);
}

int main(int argc, char **argv) {
	int chan = ~0;
	if (argc == 2)
		chan = atoi(argv[1]);

	// Init the spec controller
	SpecController mySpec(0);
	TxCore myTx(&mySpec);

	// Enable channels
	myTx.setCmdEnable(chan);

	// Reset
	reset(myTx);
	
	std::cout << "Scanning registers." << std::endl;
	
	while(true){
		for(int r=0; r<0x7f; r++){
			sendCommand(myTx,0,0x1f,r,0,0);
			usleep(100);
		}
	}

	std::cout << "Done." << std::endl;

	return 0;
}

