#pragma once
#include "axmol/axmol.h"

class TouchTestScene : public ax::Scene {
public:
    static ax::Scene* createScene();
    bool init() override;
};