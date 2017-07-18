#include <SpecController.h>
#include <TxCore.h>
#include <RxCore.h>
#include <RawData.h>
#include <iostream>
#include <stdint.h>
#include <string.h>
#include <unistd.h>

#define nWait (10)

#define HCC_adr (0x0)
#define ABC_adr (0x1)

#define INPUT_adr_start (0x18)
#define INPUT_adr_stop (0x1f)

void sendCommand2(TxCore&tx, uint8_t hcc_adr, uint8_t abc_adr, uint8_t reg_adr, uint8_t rwBit,uint32_t data,bool verbose=false){
        uint32_t header = 0x0;
        header += (0xbe << 18);
        header += ((0x1F & hcc_adr) << 13);
        header += ((0x1F & abc_adr) << 8);
        header += ((0x7F & reg_adr) << 1);
        header += ((0x1 & rwBit));
	
	if(verbose)printf("Sending 0x%08x%08x \n",header,data);
	tx.writeFifo(header);
	tx.writeFifo(data);

	tx.writeFifo(0);
	tx.writeFifo(0);
	while(!tx.isCmdEmpty());
}

void sendCommand(TxCore&tx, uint8_t hcc_adr, uint8_t abc_adr, uint8_t reg_adr, uint8_t rwBit,uint32_t data,bool verbose=false){
	uint8_t hccAddr, abcAddr, writeNotRead, addr;
	hccAddr = hcc_adr;
	abcAddr = abc_adr;
	addr = reg_adr;
	writeNotRead = rwBit;
	
//	1011 111 0 hhhhh aaa|aa rrrrrrr w dddddd|dddddddddddddddd|dddddddddd 000000
	uint16_t cb_data[4];
	cb_data[0] = 0xbe00 | ((hccAddr&0x1f) << 3) | ((abcAddr & 0x1c) >> 2);
	cb_data[1] = ((abcAddr & 0x3) << 14) | (addr<<7) | (writeNotRead?0x40:0) | ((data & 0xfc000000) >> 26);
	cb_data[2] = (data & 0x3fffc00) >> 10;
	cb_data[3] = (data & 0x3ff) << 6;

        uint32_t header = (cb_data[0]<<16)|cb_data[1];
        data = (cb_data[2]<<16)|cb_data[3];

	if(verbose)printf("Sending 0x%08x%08x \n",header,data);
	tx.writeFifo(header);
	tx.writeFifo(data);

	tx.writeFifo(0);
	tx.writeFifo(0);
	while(!tx.isCmdEmpty());
}

void init(TxCore&tx){
	// Resets
	tx.writeFifo(0xa3); // Sys
	tx.writeFifo(0);
	tx.writeFifo(0);
	while(!tx.isCmdEmpty());
	usleep(1);
	tx.writeFifo(0xa9); // Soft
	tx.writeFifo(0);
	tx.writeFifo(0);
	while(!tx.isCmdEmpty());
	usleep(1);
        tx.writeFifo(0xa5); // BC
	tx.writeFifo(0);
	tx.writeFifo(0);
	while(!tx.isCmdEmpty());
	usleep(1);

	// Init special register
	sendCommand2(tx,HCC_adr,ABC_adr,0x00,1,0x0,false);


	// Enable FIFOs
	uint32_t data = 0x00001f02;
	sendCommand2(tx,HCC_adr,ABC_adr,0x20,1,data,false);


	
        // Set min power for DATA and XOFF
	data = 0x00000000;
	sendCommand2(tx,HCC_adr,ABC_adr,0x21,1,data,false);


        // Set max power for DATA and XOFF
	data = 0x00000077;
	sendCommand2(tx,HCC_adr,ABC_adr,0x21,1,data,false);

	
/*
	// Set latency
	data = 0x00000001;
	sendCommand2(tx,HCC_adr,ABC_adr,0x22,1,data,false);
	tx.writeFifo(0);
	tx.writeFifo(0);
	while(!tx.isCmdEmpty());
	// Disable packet limit
	data = 0x00000000;
	sendCommand2(tx,HCC_adr,ABC_adr,0x23,1,data,false);
	tx.writeFifo(0);
	tx.writeFifo(0);
	while(!tx.isCmdEmpty());
	
	// Disable write
//	sendCommand2(tx,HCC_adr,ABC_adr,0x00,1,(1<<4),false);
	tx.writeFifo(0);
	tx.writeFifo(0);
	while(!tx.isCmdEmpty());
	usleep(1);
*/
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
	myTx.setCmdEnable(1<<3);

	// Soft Reset
	init(myTx);

	// Initialise input registers
/*	for(int r=0x10; r<0x18; r++){
		uint32_t d = 0xdeadc0de;
		sendCommand(myTx,HCC_adr,ABC_adr,r,1,d);
		myTx.writeFifo(0);
		myTx.writeFifo(0);
		myTx.writeFifo(0);
	}
	
	// Send L0 readout
	myTx.setCmdEnable(0);
	myTx.setCmdEnable(1<<2);
	myTx.writeFifo(1);
	myTx.writeFifo(0);
	myTx.writeFifo(0);
	myTx.setCmdEnable(1<<3);
*/	
	
	std::cout << "Scanning inputs" << std::endl;
	
	init(myTx);


	while(true){
		//for(int r=INPUT_adr_start; r<INPUT_adr_stop+1; r++){
		
		sendCommand2(myTx,HCC_adr,ABC_adr,0x20,1,0x00001f11, false);
		//sendCommand2(myTx,HCC_adr,ABC_adr,0x21,1,0x11,false);
		sendCommand2(myTx,HCC_adr,ABC_adr,0x21,1,0x55,false);
		sendCommand2(myTx,HCC_adr,ABC_adr,0x21,0,0x00, false);
	usleep(30);	
		sendCommand2(myTx,HCC_adr,ABC_adr,0x21,0,0x00, false);
	usleep(30);	
		sendCommand2(myTx,HCC_adr,ABC_adr,0x21,0,0x00, false);
	usleep(100);	
		//sendCommand2(myTx,HCC_adr,ABC_adr,0x21,0,0x77,false);		
		/*sendCommand2(myTx,HCC_adr,ABC_adr,0x21,1,0x11,false);
		sendCommand2(myTx,HCC_adr,ABC_adr,0x21,0,0x11,false);
		sendCommand2(myTx,HCC_adr,ABC_adr,0x21,1,0x22,false);
		sendCommand2(myTx,HCC_adr,ABC_adr,0x21,0,0x22,false);
		sendCommand2(myTx,HCC_adr,ABC_adr,0x21,1,0x33,false);
		sendCommand2(myTx,HCC_adr,ABC_adr,0x21,0,0x33,false);
		sendCommand2(myTx,HCC_adr,ABC_adr,0x21,1,0x44,false);
		sendCommand2(myTx,HCC_adr,ABC_adr,0x21,0,0x44,false);
		sendCommand2(myTx,HCC_adr,ABC_adr,0x21,1,0x55,false);
		sendCommand2(myTx,HCC_adr,ABC_adr,0x21,0,0x55,false);
		sendCommand2(myTx,HCC_adr,ABC_adr,0x21,1,0x66,false);
		sendCommand2(myTx,HCC_adr,ABC_adr,0x21,0,0x66,false);
		sendCommand2(myTx,HCC_adr,ABC_adr,0x21,1,0x77,false);
		sendCommand2(myTx,HCC_adr,ABC_adr,0x21,0,0x77,false);
	        */	
        //sendCommand2(myTx,HCC_adr,ABC_adr,0x21,0,0x77,false);
usleep(30);
		/*
		for(int r=0x0; r<0x7f; r++){
//			std::cout <<std::hex<< r << std::endl;
			init(myTx);
			
			sendCommand2(myTx,HCC_adr,ABC_adr,r,0,0x77);
			while(!myTx.isCmdEmpty())std::cout<<"FlushFifo";
			int a;
//			std::cin >> a;
			
                        usleep(1000);
                        // Wait
//			for(int n=0; n<nWait; n++)myTx.writeFifo(0);
		}
		*/
	}

	std::cout << "Done." << std::endl;

	return 0;
}
