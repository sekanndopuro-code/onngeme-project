#pragma once

#include "axmol/audio/AudioEngine.h"
#include <chrono>
#include <string>

// ============================================================
// GameClock
// 「曲の再生」と「今が曲の何ms地点か」を正確に扱うための土台クラス。
// 判定・ノーツの落下位置は、必ず GetSongTimeMs() を基準にすること。
// 「毎フレームの経過時間(dt)を自分で足し算したタイマー」を
// 正としてはいけない。ここがズレの温床になる。
// ============================================================
class GameClock
{
public:
    enum class State
    {
        Stopped,
        Playing,
        Paused
    };

    // 曲を最初から再生する
    void Play(const std::string& filePath, float volume = 1.0f);

    void Pause();
    void Resume();
    void Stop();

    // 毎フレーム、シーンのupdate(dt)から呼ぶこと
    void Update();

    // 「今、曲の何ms地点を再生しているか」。判定・ノーツ移動計算はこれを使う。
    double GetSongTimeMs() const;

    State GetState() const { return _state; }

private:
    ax::AudioId _audioId = -1; // -1 = 再生していない状態

    State _state = State::Stopped;

    // アンカー:「システム時計でこの瞬間、曲は何ms地点だったか」の記録
    std::chrono::steady_clock::time_point _anchorSystemTime{};
    double _anchorSongTimeMs = 0.0;

    // pause中に固定表示しておく時刻
    double _pausedSongTimeMs = 0.0;

    // 直近に観測したgetCurrentTime()の生値(秒)。値の変化を検出するために保持
    float _lastRawTimeSec = -1.0f;
};