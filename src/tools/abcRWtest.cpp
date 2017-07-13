#include <SpecController.h>
#include <TxCore.h>
#include <RxCore.h>
#include <RawData.h>
#include <iostream>
#include <stdint.h>
#include <string.h>
#include <unistd.h>

#define nWait (10)

#define HCC_adr (0x1f)
#define ABC_adr (0x00)

void sendCommand(TxCore&tx, uint8_t hcc_adr, uint8_t abc_adr, uint8_t reg_adr, uint8_t rwBit,uint32_t data,bool verbose=false){
        uint32_t header = 0x0;
        header += (0xbe << 18);
        header += ((0x1F & hcc_adr) << 13);
        header += ((0x1F & abc_adr) << 8);
        header += ((0x7F & reg_adr) << 1);
        header += ((0x1 & rwBit));
	
	if(verbose)printf("Sending 0x%08x%08x \n",header,data);
	tx.writeFifo(header);
	tx.writeFifo(data);
}

void init(TxCore&tx){
	// Resets
	tx.writeFifo(0xa3); // Sys
	tx.writeFifo(0);
	tx.writeFifo(0);
	tx.writeFifo(0xa9); // Soft
	tx.writeFifo(0);
	tx.writeFifo(0);
        tx.writeFifo(0xa5); // BC
	tx.writeFifo(0);
	tx.writeFifo(0);

	// Init special register
	sendCommand(tx,HCC_adr,ABC_adr,0x00,1,0x0,false);
	tx.writeFifo(0);
	tx.writeFifo(0);

	// Enable FIFOs
	uint32_t data = (1<<8)|(1<<12);
	sendCommand(tx,HCC_adr,ABC_adr,0x20,1,data,false);
	tx.writeFifo(0);
	tx.writeFifo(0);

        // Set max power for DATA and XOFF
	data = 0x77;
	sendCommand(tx,HCC_adr,ABC_adr,0x21,1,data,false);
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

        // Hard reset
        myTx.setCmdEnable(1<<4);
        for(int i=0; i<100; i++)myTx.writeFifo(~0);
        while(!myTx.isCmdEmpty());
        myTx.setCmdEnable(0);


        usleep(1000);
	// Enable channels
	myTx.setCmdEnable(chan);

	// Soft Reset
	init(myTx);
	
	std::cout << "Scanning registers." << std::endl;
	
	while(true){
		for(int r=0; r<0x7f; r++){
			sendCommand(myTx,HCC_adr,ABC_adr,r,0,0);

                        usleep(1000);

                        // Wait
                        for(int n=0; n<nWait; n++)myTx.writeFifo(0);
		}
	}

	std::cout << "Done." << std::endl;

	return 0;
}

