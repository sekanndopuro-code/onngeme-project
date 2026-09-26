#include "TouchTestScene.h"
#include "CoordinateConverter.h"
USING_NS_AX;

Scene* TouchTestScene::createScene() {
    return TouchTestScene::create();
}

bool TouchTestScene::init() {
    if (!Scene::init()) return false;

    // ---- P0-06 検証用: 座標変換の端点・中央チェック ----
    // 赤=X軸(左端/中央/右端)、青=Y軸(下端/中央/上端)
    // ウィンドウサイズを変えても、赤は常に画面の左端・水平中央・右端に、
    // 青は常に画面の下端・(見えている範囲の)垂直中央付近・上方に来ればOK。
    auto addDot = [this](const ax::Vec2& pos, const ax::Color4F& color) {
        auto dot = DrawNode::create();
        dot->drawSolidCircle(Vec2::ZERO, 12.0f, 0.0f, 24, color);
        dot->setPosition(pos);
        this->addChild(dot, 10);
    };

    // X軸: 左端 / 中央 / 右端 (Yは仮に画面下から100pxの高さに固定)
    float testScreenY = CoordSystem::gameToScreenY(0.0f) + 100.0f;
    addDot(CoordSystem::gameToScreenPoint(CoordSystem::GAME_X_MIN, testScreenY), Color4F::RED);
    addDot(CoordSystem::gameToScreenPoint((CoordSystem::GAME_X_MIN + CoordSystem::GAME_X_MAX) / 2.0f, testScreenY), Color4F::RED);
    addDot(CoordSystem::gameToScreenPoint(CoordSystem::GAME_X_MAX, testScreenY), Color4F::RED);

    // Y軸: 下端 / 中央 / 上端 (Xは画面中央に固定)
    float centerGameX = (CoordSystem::GAME_X_MIN + CoordSystem::GAME_X_MAX) / 2.0f;
    addDot(CoordSystem::gameToScreenPoint2D(centerGameX, CoordSystem::GAME_Y_MIN), Color4F::BLUE);
    addDot(CoordSystem::gameToScreenPoint2D(centerGameX, (CoordSystem::GAME_Y_MIN + CoordSystem::GAME_Y_MAX) / 2.0f), Color4F::BLUE);
    addDot(CoordSystem::gameToScreenPoint2D(centerGameX, CoordSystem::GAME_Y_MAX), Color4F::BLUE);
    // ----------------------------------------------------

    auto listener = EventListenerTouchAllAtOnce::create();

    listener->onTouchesBegan = [](const std::vector<Touch*>& touches, Event* event) {
        AXLOG("touches began: %d", (int)touches.size());
        for (auto touch : touches) {
            AXLOG("  id=%d x=%.1f y=%.1f", touch->getID(), touch->getLocation().x, touch->getLocation().y);
        }
    };

    listener->onTouchesMoved = [](const std::vector<Touch*>& touches, Event* event) {
        // 今は何もしなくてOK
    };

    listener->onTouchesEnded = [](const std::vector<Touch*>& touches, Event* event) {
        AXLOG("touches ended: %d", (int)touches.size());
    };

    _eventDispatcher->addEventListenerWithSceneGraphPriority(listener, this);

    return true;
}