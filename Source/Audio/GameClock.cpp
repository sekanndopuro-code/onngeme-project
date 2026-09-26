#include "GameClock.h"

using namespace ax;

void GameClock::Play(const std::string& filePath, float volume)
{
    _audioId = AudioEngine::play2d(filePath, false, volume);
    _state = State::Playing;

    // 最初のアンカーは仮置き。すぐ後のUpdate()で本物の値に上書きされる。
    _anchorSystemTime = std::chrono::steady_clock::now();
    _anchorSongTimeMs = 0.0;
    _lastRawTimeSec = -1.0f;
}

void GameClock::Pause()
{
    if (_state != State::Playing) return;

    _pausedSongTimeMs = GetSongTimeMs(); // 止まる直前の推定時刻を保存
    AudioEngine::pause(_audioId);
    _state = State::Paused;
}

void GameClock::Resume()
{
    if (_state != State::Paused) return;

    AudioEngine::resume(_audioId);
    _state = State::Playing;

    // 「resumeは一瞬で完了した」と仮でアンカーを置く。
    // 実際のレイテンシは、次にgetCurrentTime()が新しい値を返した瞬間に
    // Update()側で自動的に補正される(ここが蓄積ズレを防ぐ肝)。
    _anchorSystemTime = std::chrono::steady_clock::now();
    _anchorSongTimeMs = _pausedSongTimeMs;
    _lastRawTimeSec = -1.0f; // 次に読む値を必ず「新しい値」として扱わせる
}

void GameClock::Stop()
{
    if (_audioId == -1) return;
    AudioEngine::stop(_audioId);
    _audioId = -1;
    _state = State::Stopped;
    _pausedSongTimeMs = 0.0;
}

void GameClock::Update()
{
    if (_state != State::Playing || _audioId == -1) return;

    float raw = AudioEngine::getCurrentTime(_audioId);

    // getCurrentTime()が「新しく更新されたタイミング」だけ、
    // アンカーを本物の値で上書きする → ズレを毎回リセットする
    if (raw != _lastRawTimeSec)
    {
        _lastRawTimeSec = raw;
        _anchorSystemTime = std::chrono::steady_clock::now();
        _anchorSongTimeMs = static_cast<double>(raw) * 1000.0;
    }
}

double GameClock::GetSongTimeMs() const
{
    if (_state == State::Stopped) return 0.0;
    if (_state == State::Paused)  return _pausedSongTimeMs;

    auto elapsed = std::chrono::steady_clock::now() - _anchorSystemTime;
    double elapsedMs = std::chrono::duration<double, std::milli>(elapsed).count();
    return _anchorSongTimeMs + elapsedMs;
}