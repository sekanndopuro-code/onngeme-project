// ============================================================
// CoordinateConverter.h
//
// タスクID: P0-06 座標変換ルールの決定 (v4.0)
//
// 【これは何のファイルか】
// ゲーム座標(X = 0〜10080, Y = 0〜1080)と、実際の端末の画面座標(px)を
// お互いに変換するための関数を1か所にまとめたものです。
// 今後すべてのノーツ・判定線・入力判定領域(yCenter/height)・誘導面の
// 座標計算は、必ずこのファイルの関数を経由してください
// (直接pxで計算するとスマホ/タブレットで座標がズレます)。
//
// 【v4.0での変更点】
// 旧版はX軸(0〜256)のみの変換だったが、v4.0で追加された「入力判定領域の
// Y方向移動」「誘導面」を実装するにはY軸(0〜1080)の変換も必須なので追加した。
// X_MAXも旧仕様の256から、24/30/32/48/60等で割り切れる魔法数10080に変更。
//
// 【採用した方針:画面幅フィット(黒帯なし)】
// X=0 は常に画面の左端、X=10080 は常に画面の右端に対応します。
// 端末の縦横比が変わっても、左右に黒帯(レターボックス)は出ません。
//
// 【Y軸のフィット方針:確定(2026-08)】
// X軸と同じスケール(画面幅 ÷ 10080)をY軸にもそのまま使う
// (=アスペクト比を維持したまま拡大する。Y軸だけ別比率で
// 画面高さいっぱいに引き伸ばしたりはしない)。
//
// この方式にすると、Arcaeaのような見え方に自然になる:
//   ・16:9の横長端末 → 縦方向に見える範囲(可視ゲームY座標)が狭くなり、
//     レーン/入力判定領域は画面下半分〜2/3くらいに収まって見える
//   ・iPadのような相対的に縦長の端末 → 見える範囲が広がり、
//     上部の余白(黒帯的なスペース)が自然と増える
// 実際にどこまでノーツ/領域を配置するか(半分〜2/3を超えない等)は
// 譜面側のスクロール速度で調整するのが基本方針。
//
// 加えて、この自然な余白を「あえて画面いっぱいに使わせる/使わせない」を
// プレイヤーが選べる「表示範囲(黒帯)モード」をアクセシビリティ設定に
// 追加する予定(P3-01)。これは本ファイルの範囲外(UI設定側)なので
// ここでは実装しない。getVisibleGameHeight() が土台として使える。
//
// 【これを機能させるための前提条件】
// AppDelegate.cpp 側で下記のように
//     ResolutionPolicy::FIXED_WIDTH
// を指定していることが前提です。詳しくは同封の設計メモを参照してください。
// ============================================================
#pragma once

#include "axmol/axmol.h"

namespace CoordSystem
{
    // ---- ゲーム座標系の範囲 (v4.0で確定。仕様確定事項シート参照) ----
    constexpr float GAME_X_MIN = 0.0f;
    constexpr float GAME_X_MAX = 10080.0f;

    constexpr float GAME_Y_MIN = 0.0f;
    constexpr float GAME_Y_MAX = 1080.0f;

    // ------------------------------------------------------------
    // gameToScreenX
    //   ゲーム座標X (0〜10080) → 画面座標X (px, axmolのデザイン座標系)
    //
    //   例:
    //     gameToScreenX(0.0f)     → 画面の左端のx座標
    //     gameToScreenX(5040.0f)  → 画面の水平方向のちょうど中央
    //     gameToScreenX(10080.0f) → 画面の右端のx座標
    // ------------------------------------------------------------
    inline float gameToScreenX(float gameX)
    {
        auto visibleSize = ax::Director::getInstance()->getVisibleSize();
        auto origin       = ax::Director::getInstance()->getVisibleOrigin();

        // 0.0(左端) 〜 1.0(右端) の比率に変換
        float ratio = (gameX - GAME_X_MIN) / (GAME_X_MAX - GAME_X_MIN);

        return origin.x + ratio * visibleSize.width;
    }

    // ------------------------------------------------------------
    // screenToGameX
    //   画面座標X (px) → ゲーム座標X (0〜10080)
    //
    //   タッチした位置がゲーム座標のどこに当たるかを調べたいときに使う。
    //   (例: Slideノーツの追従判定、Flickの位置判定など)
    // ------------------------------------------------------------
    inline float screenToGameX(float screenX)
    {
        auto visibleSize = ax::Director::getInstance()->getVisibleSize();
        auto origin       = ax::Director::getInstance()->getVisibleOrigin();

        float ratio = (screenX - origin.x) / visibleSize.width; // 0.0〜1.0

        return GAME_X_MIN + ratio * (GAME_X_MAX - GAME_X_MIN);
    }

    // ------------------------------------------------------------
    // gameToScreenPxPerUnit
    //   ゲーム座標1単位が何pxに相当するかの倍率。X軸基準(画面幅÷10080)。
    //   Y軸もこれと同じ倍率を使うことで、アスペクト比を維持する。
    // ------------------------------------------------------------
    inline float gameToScreenPxPerUnit()
    {
        auto visibleSize = ax::Director::getInstance()->getVisibleSize();
        return visibleSize.width / (GAME_X_MAX - GAME_X_MIN);
    }

    // ------------------------------------------------------------
    // gameToScreenY / screenToGameY  ★v4.0新規
    //   ゲーム座標Y (0〜1080) ⇔ 画面座標Y (px)
    //
    //   入力判定領域(yCenter, height)や誘導面の描画・判定に使う。
    //   Y=0 (ゲーム座標の下端 = 判定線付近) を画面下端(origin.y)に固定し、
    //   X軸と同じ倍率(gameToScreenPxPerUnit)で上方向に伸ばす。
    //   端末によっては Y=1080 が画面内に収まらない(=見えない)ことが
    //   あるが、それは意図通り(縦長端末ほどより高くまで見える)。
    // ------------------------------------------------------------
    inline float gameToScreenY(float gameY)
    {
        auto origin = ax::Director::getInstance()->getVisibleOrigin();
        return origin.y + (gameY - GAME_Y_MIN) * gameToScreenPxPerUnit();
    }

    inline float screenToGameY(float screenY)
    {
        auto origin = ax::Director::getInstance()->getVisibleOrigin();
        return GAME_Y_MIN + (screenY - origin.y) / gameToScreenPxPerUnit();
    }

    // ------------------------------------------------------------
    // getVisibleGameHeight
    //   いまの端末で実際に見えているゲームY座標の範囲(0〜この値)。
    //   16:9端末なら1080より小さい値、iPad等では1080に近い/超える値になる。
    //   「表示範囲(黒帯)モード」(P3-01)の実装時、この値を基準に
    //   ・自然な余白のまま使う
    //   ・意図的にこの値を頭打ちさせて黒帯を出す
    //   といった切り替えを乗せる想定。
    // ------------------------------------------------------------
    inline float getVisibleGameHeight()
    {
        auto visibleSize = ax::Director::getInstance()->getVisibleSize();
        return visibleSize.height / gameToScreenPxPerUnit();
    }

    // ------------------------------------------------------------
    // (参考) 動作確認用のヘルパー
    //   端点(x=0,10080 / y=0,1080)と中央にテスト用のノードを置きたいときに使う。
    //   P0-06の検証方法: このスケルトンを使って赤丸などを配置し、
    //   PC上でウィンドウサイズを変えたり実機の縦横比を変えて
    //   常に「左端・中央・右端」「上端・中央・下端」に来ているか目視確認する。
    // ------------------------------------------------------------
    inline ax::Vec2 gameToScreenPoint(float gameX, float screenY)
    {
        return ax::Vec2(gameToScreenX(gameX), screenY);
    }

    // ★v4.0新規: X・Yどちらもゲーム座標から画面座標に変換したい場合はこちら
    inline ax::Vec2 gameToScreenPoint2D(float gameX, float gameY)
    {
        return ax::Vec2(gameToScreenX(gameX), gameToScreenY(gameY));
    }
}
