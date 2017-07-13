#include <SpecController.h>
#include <TxCore.h>
#include <RxCore.h>
#include <RawData.h>
#include <iostream>
#include <stdint.h>
#include <string.h>
#include <unistd.h>

int main(int argc, char **argv) {
	// Init the spec controller
	SpecController mySpec(0);
        TxCore rstTx(&mySpec);
        
        std::cout << "Start reset" << std::endl;

        // Hard reset
        rstTx.setCmdEnable(1<<4);
        for(int i=0; i<1000000; i++)rstTx.writeFifo(~0);
        rstTx.setCmdEnable(0);
	std::cout << "Done." << std::endl;

	return 0;
}

