#include <SpecController.h>
#include <TxCore.h>
#include <iostream>
#include <stdint.h>
#include <string.h>

void sendStr(TxCore&tx, char*str){
	for(unsigned i=0; str[i]!='\0'; i++){
		tx.writeFifo(str[i]);
	}
}

int main(int argc, char **argv) {
	// Init the spec controller
	int ch=0;
	if (argc == 2)
		ch = atoi(argv[1]);
	SpecController mySpec(0);
	TxCore myTx(&mySpec);

	// Scan the tx_channels
	std::cout << "Sending strings..." << std::endl;
	myTx.setCmdEnable(1<<ch);
	for(int j=0;j<100;j++)sendStr(myTx,"HELLO");
	std::cout << "Done." << std::endl;
	return 0;
}
