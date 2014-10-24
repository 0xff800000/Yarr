// #################################
// # Author: Timon Heim
// # Email: timon.heim at cern.ch
// # Project: Yarr
// # Description: Fei4 Threshold Scan
// # Comment: Nothing fancy
// ################################

#include "Fei4ThresholdScan.h"

Fei4ThresholdScan::Fei4ThresholdScan(Fei4 *fe, TxCore *tx, RxCore *rx, ClipBoard<RawData> *data) : ScanBase(fe, tx, rx, data) {
    mask = MASK_16;
    dcMode = QUAD_DC;
    numOfTriggers = 100;
    triggerFrequency = 10e3;
    triggerDelay = 50;
    minVcal = 20;
    maxVcal = 100;
    stepVcal = 2;

    verbose = false;
}

// Initialize Loops
void Fei4ThresholdScan::init() {
    // Loop 1: Mask Staging
    std::shared_ptr<Fei4MaskLoop> maskStaging(new Fei4MaskLoop);
    maskStaging->setVerbose(verbose);
    maskStaging->setMaskStage(mask);
    maskStaging->setScap(true);
    maskStaging->setLcap(true);
    
    // Loop 2: Double Columns
    std::shared_ptr<Fei4DcLoop> dcLoop(new Fei4DcLoop);
    dcLoop->setVerbose(verbose);
    dcLoop->setMode(dcMode);

    // Loop 1: Parameter Loop
    std::shared_ptr<Fei4ParameterLoopBase> parLoop( Fei4ParameterLoopBuilder(&Fei4::PlsrDAC) );
    parLoop->setRange(minVcal, maxVcal, stepVcal);
    parLoop->setVerbose(true);
    parLoop->setSplitData(true);

    // Loop 3: Trigger
    std::shared_ptr<Fei4TriggerLoop> triggerLoop(new Fei4TriggerLoop);
    triggerLoop->setVerbose(verbose);
    triggerLoop->setTrigCnt(numOfTriggers);
    triggerLoop->setTrigFreq(triggerFrequency);
    triggerLoop->setTrigDelay(triggerDelay);

    // Loop 4: Data gatherer
    std::shared_ptr<StdDataLoop> dataLoop(new StdDataLoop);
    dataLoop->setVerbose(verbose);
    dataLoop->connect(g_data);

    loops.push_back(maskStaging);
    loops.push_back(dcLoop);
    loops.push_back(parLoop);
    loops.push_back(triggerLoop);
    loops.push_back(dataLoop);

    engine.addAction(parLoop);
    engine.addAction(maskStaging);
    engine.addAction(dcLoop);
    engine.addAction(triggerLoop);
    engine.addAction(dataLoop);

    engine.init();
}

// Do necessary pre-scan configuration
void Fei4ThresholdScan::preScan() {
    g_fe->writeRegister(&Fei4::Trig_Count, 8);
    g_fe->writeRegister(&Fei4::Trig_Lat, (255-triggerDelay)-2);
    g_fe->writeRegister(&Fei4::PlsrDAC, 300);
    g_fe->writeRegister(&Fei4::CalPulseWidth, 20); // Longer than max ToT 
    while(!g_tx->isCmdEmpty());
}