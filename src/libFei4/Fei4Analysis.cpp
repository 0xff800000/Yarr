// #################################
// # Author: Timon Heim
// # Email: timon.heim at cern.ch
// # Project: Yarr
// # Description: Analysis Base class
// # Comment: 
// ################################

#include "Fei4Analysis.h"

Fei4Analysis::Fei4Analysis() {

}

Fei4Analysis::~Fei4Analysis() {

}

void Fei4Analysis::init() {
    for (unsigned i=0; i<algorithms.size(); i++) {
        algorithms[i]->connect(output);
        algorithms[i]->init(scan);
    }
}

void Fei4Analysis::process() {
    while(!input->empty()) {
        HistogramBase *h = input->popData();
        if (h != NULL) {
            for (unsigned i=0; i<algorithms.size(); i++) {
                algorithms[i]->processHistogram(h);
            }
            delete h;
        }
    }
}

void Fei4Analysis::end() {
    for (unsigned i=0; i<algorithms.size(); i++) {
        algorithms[i]->end();
    }
}

void Fei4Analysis::addAlgorithm(AnalysisAlgorithm *a) {
    algorithms.push_back(a);
}

void Fei4Analysis::plot(std::string basename) {
    if (output->empty())
        return;
    for (std::deque<HistogramBase*>::iterator it = output->begin(); it != output->end(); ++it) {
        std::cout << "Plotting : " << (*it)->getName() << std::endl;
        (*it)->plot(basename);
    }
}


void OccupancyAnalysis::init(ScanBase *s) {
    n_count = 1;
    for (unsigned n=0; n<s->size(); n++) {
        std::shared_ptr<LoopActionBase> l = s->getLoop(n);
        if ((l->type() != typeid(Fei4TriggerLoop*) &&
                    l->type() != typeid(Fei4MaskLoop*) &&
                    l->type() != typeid(StdDataLoop*) &&
                    l->type() != typeid(Fei4DcLoop*))) {
            loops.push_back(n);
            loopMax.push_back((unsigned)l->getMax());
        } else {
            unsigned cnt = (l->getMax() - l->getMin())/l->getStep();
            if (cnt == 0)
                cnt = 1;
            n_count = n_count*cnt;
        }
    }
}

void OccupancyAnalysis::processHistogram(HistogramBase *h) {
    // Check if right Histogram
    if (h->getType() != typeid(OccupancyMap*))
        return;

    // Select correct output container
    unsigned ident = 0;
    unsigned offset = 0;

    // Determine identifier
    std::string name = "OccupancyMap";
    for (unsigned n=0; n<loops.size(); n++) {
        ident += h->getStat().get(loops[n])+offset;
        offset += loopMax[n];
        name += "-" + std::to_string(h->getStat().get(loops[n]));
    }

    // Check if Histogram exists
    if (occMaps[ident] == NULL) {
        Histo2d *hh = new Histo2d(name, 80, 0.5, 80.5, 336, 0.5, 336.5, typeid(this));
        hh->setXaxisTitle("Column");
        hh->setYaxisTitle("Row");
        hh->setZaxisTitle("Hits");
        occMaps[ident] = hh;
        innerCnt[ident] = 0;
    }

    // Add up Histograms
    occMaps[ident]->add(*(Histo2d*)h);
    innerCnt[ident]++;

    // Got all data, finish up Analysis
    if (innerCnt[ident] == n_count) {
        output->pushData(occMaps[ident]);
        //delete occMaps[ident];
        //occMaps[ident] = NULL;
    }
}

void ScurveFitter::init(ScanBase *s) {
    std::shared_ptr<LoopActionBase> tmpVcalLoop(Fei4ParameterLoopBuilder(&Fei4::PlsrDAC));
    scan = s;
    n_count = 26880;
    vcalLoop = 0;
    injections = 50;
    for (unsigned n=0; n<s->size(); n++) {
        std::shared_ptr<LoopActionBase> l = s->getLoop(n);
        if ((l->type() != typeid(Fei4TriggerLoop*) &&
                    l->type() != typeid(Fei4MaskLoop*) &&
                    l->type() != typeid(StdDataLoop*) &&
                    l->type() != typeid(Fei4DcLoop*)) &&
                l->type() != tmpVcalLoop->type()) {
            loops.push_back(n);
            loopMax.push_back((unsigned)l->getMax());
        } else {
            unsigned cnt = (l->getMax() - l->getMin())/l->getStep();
            if (cnt == 0)
                cnt = 1;
            n_count = n_count*cnt;
        }
        // Vcal Loop
        if (l->type() == tmpVcalLoop->type()) {
            vcalLoop = n;
            vcalMax = l->getMax();
            vcalMin = l->getMin();
            vcalStep = l->getStep();
            vcalBins = (vcalMax-vcalMin)/vcalStep;
        }

        if (l->type() == typeid(Fei4TriggerLoop*)) {
            Fei4TriggerLoop *trigLoop = (Fei4TriggerLoop*) l.get();
            injections = trigLoop->getTrigCnt();
        }
    }
    
    for (unsigned i=vcalMin; i<=vcalMax; i+=vcalStep) {
        x.push_back(i);
    }
    cnt = 0;
}

// Errorfunction
// par[0] = Mean
// par[1] = Sigma
// par[2] = Normlization
#define SQRT2 1.414213562
double scurveFct(double x, const double *par) {
    return 0.5*(2-erfc((x-par[0])/(par[1]*SQRT2)))*par[2];
}

void ScurveFitter::processHistogram(HistogramBase *h) {
    cnt++;
    //std::cout << "--> Processing : " << h->getName() << " " << cnt << std::endl;
    // Check if right Histogram
    if (h->getType() != typeid(OccupancyMap*))
        return;

    Histo2d *hh = (Histo2d*) h;
    for(unsigned col=1; col<=80; col++) {
        for (unsigned row=1; row<=336; row++) {
            unsigned bin = hh->binNum(col, row);
            if (hh->getBin(bin) != 0) {
                // Select correct output container
                unsigned ident = bin;
                unsigned outerIdent = 0;
                unsigned offset = 26880;
                unsigned outerOffset = 0;
                unsigned vcal = hh->getStat().get(vcalLoop);
                // Determine identifier
                std::string name = "Scurve";
                name += "-" + std::to_string(bin);
                // Check for other loops
                for (unsigned n=0; n<loops.size(); n++) {
                    ident += hh->getStat().get(loops[n])+offset;
                    outerIdent += hh->getStat().get(loops[n])+offset;
                    offset += loopMax[n];
                    outerOffset += loopMax[n];
                    name += "-" + std::to_string(hh->getStat().get(loops[n]));
                }

                // Check if Histogram exists
                if (histos[ident] == NULL) {
                    //std::cout << "NEW " << name << std::endl;
                    Histo1d *hhh = new Histo1d(name, vcalBins+1, vcalMin-((double)vcalStep/2.0), vcalMax+((double)vcalStep/2.0), typeid(this));
                    hhh->setXaxisTitle("Vcal");
                    hhh->setYaxisTitle("Occupancy");
                    histos[ident] = hhh;
                    innerCnt[ident] = 0;
                }

                // Add up Histograms
                histos[ident]->fill(vcal, hh->getBin(bin));
                innerCnt[ident]++;
                //std::cout << innerCnt[ident] << std::endl;

                // Got all data, finish up Analysis
                // TODO This requires the loop to run from low to high and a hit in the last bin
                if (vcal == vcalMax) {
                    //std::cout << " Done with scurve : " << histos[ident]->getName() << std::endl;
                    // Scale histos
                    //histos[ident]->scale(1.0/(double)injections);

                    lm_status_struct status;
                    lm_control_struct control;
                    control = lm_control_float;
                    control.verbosity = 0;
                    const unsigned n_par = 3;
                    double par[n_par] = {50, 5, (double) injections};
                    std::chrono::high_resolution_clock::time_point start;
                    std::chrono::high_resolution_clock::time_point end;
                    start = std::chrono::high_resolution_clock::now();
                    lmcurve(n_par, par, vcalBins, &x[0], histos[ident]->getData(), scurveFct, &control, &status);
                    end = std::chrono::high_resolution_clock::now();
                    std::chrono::microseconds fitTime = std::chrono::duration_cast<std::chrono::microseconds>(end-start);
                    if (thrMap[outerIdent] == NULL) {
                        //std::cout << " NEW ThresholdMap: " << outerIdent << std::endl;
                        Histo2d *hh2 = new Histo2d("ThresholdMap", 80, 0.5, 80.5, 336, 0.5, 336.5, typeid(this));
                        hh2->setXaxisTitle("Column");
                        hh2->setYaxisTitle("Row");
                        hh2->setZaxisTitle("Threshold [Vcal]");
                        thrMap[outerIdent] = hh2;
                        hh2 = new Histo2d("NoiseMap", 80, 0.5, 80.5, 336, 0.5, 336.5, typeid(this));
                        hh2->setXaxisTitle("Column");
                        hh2->setYaxisTitle("Row");
                        hh2->setZaxisTitle("Noise [Vcal]");
                        sigMap[outerIdent] = hh2;
                        //std::cout << " NEW ThresholdDist: " << outerIdent << std::endl;
                        Histo1d *hh1 = new Histo1d("ThresholdDist", 51, -1, 101, typeid(this));
                        hh1->setXaxisTitle("Threshold [Vcal]");
                        hh1->setYaxisTitle("Number of Pixels");
                        thrDist[outerIdent] = hh1;
                        hh1 = new Histo1d("NoiseDist", 51, -0.1, 10.1, typeid(this));
                        hh1->setXaxisTitle("Threshold [Vcal]");
                        hh1->setYaxisTitle("Number of Pixels");
                        sigDist[outerIdent] = hh1;
                        hh1 = new Histo1d("Chi2Dist", 51, -0.025, 2.525, typeid(this));
                        hh1->setXaxisTitle("Fit Chi/ndf");
                        hh1->setYaxisTitle("Number of Pixels");
                        chiDist[outerIdent] = hh1;
                        hh1 = new Histo1d("TimePerFitDist", 201, -1, 401, typeid(this));
                        hh1->setXaxisTitle("Fit Time [us]");
                        hh1->setYaxisTitle("Number of Pixels");
                        timeDist[outerIdent] = hh1;
                    }
                    if (par[0] > vcalMin && par[0] < vcalMax) {
                        thrMap[outerIdent]->fill(col, row, par[0]);
                        thrDist[outerIdent]->fill(par[0]);
                        sigMap[outerIdent]->fill(col, row, par[1]);
                        sigDist[outerIdent]->fill(par[1]);
                        chiDist[outerIdent]->fill(status.fnorm/(double)status.nfev);
                        timeDist[outerIdent]->fill(fitTime.count());
                    }

                    // Delete s-curve
                    if (bin%2680==0) {
                        output->pushData(histos[ident]);
                    } else {
                        delete histos[ident];
                        histos[ident] = NULL;
                    }
                }
            }
        }
    }
}

void ScurveFitter::end() {
    if (thrDist[0] != NULL) {
        output->pushData(thrDist[0]);
        output->pushData(thrMap[0]);
        output->pushData(sigDist[0]);
        output->pushData(sigMap[0]);
        output->pushData(chiDist[0]);
        output->pushData(timeDist[0]);
    }
}

void OccGlobalThresholdTune::init(ScanBase *s) {
    std::shared_ptr<LoopActionBase> tmpVthinFb(Fei4GlobalFeedbackBuilder(&Fei4::Vthin_Fine));
    n_count = 1;
    for (unsigned n=0; n<s->size(); n++) {
        std::shared_ptr<LoopActionBase> l = s->getLoop(n);
        if ((l->type() != typeid(Fei4TriggerLoop*) &&
                    l->type() != typeid(Fei4MaskLoop*) &&
                    l->type() != typeid(StdDataLoop*) &&
                    l->type() != typeid(Fei4DcLoop*))) {
            loops.push_back(n);
            loopMax.push_back((unsigned)l->getMax());
        } else {
            unsigned cnt = (l->getMax() - l->getMin())/l->getStep();
            if (cnt == 0)
                cnt = 1;
            n_count = n_count*cnt;
            std::cout << "Count per Loop = " << cnt << std::endl;
        }
        
        if (l->type() == typeid(Fei4TriggerLoop*)) {
            Fei4TriggerLoop *trigLoop = (Fei4TriggerLoop*) l.get();
            injections = trigLoop->getTrigCnt();
        }

        if (l->type() == tmpVthinFb->type()) {
            std::cout << "Found Feedback Loop" << std::endl;
            fb = (Fei4GlobalFeedbackBase*) l.get();  
        }
    }

}

void OccGlobalThresholdTune::processHistogram(HistogramBase *h) {
    // Check if right Histogram
    if (h->getType() != typeid(OccupancyMap*))
        return;

    // Select correct output container
    unsigned ident = 0;
    unsigned offset = 0;

    // Determine identifier
    std::string name = "OccupancyMap";
    std::string name2 = "OccupancyDist";
    for (unsigned n=0; n<loops.size(); n++) {
        ident += h->getStat().get(loops[n])+offset;
        offset += loopMax[n];
        name += "-" + std::to_string(h->getStat().get(loops[n]));
        name2 += "-" + std::to_string(h->getStat().get(loops[n]));
    }

    // Check if Histogram exists
    if (occMaps[ident] == NULL) {
        Histo2d *hh = new Histo2d(name, 80, 0.5, 80.5, 336, 0.5, 336.5, typeid(this));
        hh->setXaxisTitle("Column");
        hh->setYaxisTitle("Row");
        hh->setZaxisTitle("Hits");
        occMaps[ident] = hh;
        Histo1d *hhh = new Histo1d(name2, injections+1, -0.5, injections+0.5, typeid(this));
        hh->setXaxisTitle("Occupancy");
        hh->setYaxisTitle("Number of Pixels");
        occDists[ident] = hhh;
        innerCnt[ident] = 0;
    }

    // Add up Histograms
    occMaps[ident]->add(*(Histo2d*)h);
    innerCnt[ident]++;

    // Got all data, finish up Analysis
    if (innerCnt[ident] == n_count) {

        for(unsigned i=0; i<occMaps[ident]->size(); i++)
            occDists[ident]->fill(occMaps[ident]->getBin(i));

        bool done = false;
        double sign = 0;
        if (fb->getStep() == 1) {
            done = true;
        }

        double meanOcc = occDists[ident]->getMean()/injections;
        std::cout << "Mean Occupancy: " << meanOcc << std::endl;

        if (meanOcc > 0.52) {
            sign = +1;
        } else if (meanOcc < 0.48) {
            sign = -1;
        } else {
            sign = 0;
            done = true;
        }
        
        std::cout << "Sending feedback: " << sign << " " << done << std::endl;
        fb->feedback(sign, done);
        output->pushData(occMaps[ident]);
        output->pushData(occDists[ident]);
        innerCnt[ident] = 0;
        //delete occMaps[ident];
        occMaps[ident] = NULL;
        //delete occDists[ident];
        occDists[ident] = NULL;
    }
    
}
