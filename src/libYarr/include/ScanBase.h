#ifndef SCANBASE_H
#define SCANBASE_H

// #################################
// # Author: Timon Heim
// # Email: timon.heim at cern.ch
// # Project: Yarr
// # Description: Scan Base Class
// # Comment:
// ################################

#include <vector>
#include <memory>
#include <map>
#include <typeindex>

#include "Fei4.h"
#include "Fe65p2.h"
#include "TxCore.h"
#include "RxCore.h"
#include "LoopEngine.h"
#include "LoopActionBase.h"
#include "ClipBoard.h"
#include "RawData.h"
#include "json.hpp"

#include "Bookkeeper.h"

using json = nlohmann::json;

class ScanBase {
    public:
        ScanBase(Fei4 *fe, TxCore *tx, RxCore *rx, ClipBoard<RawDataContainer> *data);
        ScanBase(Bookkeeper *k);
        virtual ~ScanBase() {}

        virtual void init() {}
        virtual void preScan() {}
        virtual void postScan() {}
        void run();

        std::shared_ptr<LoopActionBase> getLoop(unsigned n);
        std::shared_ptr<LoopActionBase> operator[](unsigned n);
        std::shared_ptr<LoopActionBase> operator[](std::type_index t);
        unsigned size();
        
        virtual void loadConfig(json &cfg) {}

    protected:
        LoopEngine engine;
        void addLoop(std::shared_ptr<LoopActionBase> l);
        Bookkeeper *b;

        Fei4 *g_fe;
        Fe65p2 *g_fe65p2;
        TxCore *g_tx;
        RxCore *g_rx;
        ClipBoard<RawDataContainer> *g_data;

    private:
        std::vector<std::shared_ptr<LoopActionBase> > loops;
        std::map<std::type_index, std::shared_ptr<LoopActionBase> > loopMap;
};

#endif
