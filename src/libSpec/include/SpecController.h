#ifndef SPECCONTROLLER_H
#define SPECCONTROLLER_H

// #################################
// # Author: Timon Heim
// # Email: timon.heim at cern.ch
// # Project: Yarr
// # Description: Spec Controller class
// # Comment:
// # Data: Feb 2017
// ################################

#include "HwController.h"
#include "SpecTxCore.h"
#include "SpecRxCore.h"
#include "json.hpp"

class SpecController : public HwController, public SpecTxCore, public SpecRxCore {
    public:

        void loadConfig(nlohmann::json &j) {
            unsigned id = 0;
            if (!j["specNum"].empty())
                id = j["specNum"];
            
            SpecCom::init(id);
        }
};

#endif
