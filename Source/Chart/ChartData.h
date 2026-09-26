#pragma once
#include <string>
#include <vector>

namespace chart {

// 拍位置: [整数拍, 分子, 分母]  例) [3,1,4] = 3.25拍
struct Beat {
    int bar = 0;   // 整数拍(小節ではなく、4拍子なら1小節=4)
    int num = 0;   // 分子
    int den = 1;   // 分母 (0にならないよう1で初期化)

    // 拍を小数(double)に変換するユーティリティ
    double toDouble() const {
        return static_cast<double>(bar) + (den == 0 ? 0.0 : static_cast<double>(num) / static_cast<double>(den));
    }
};

enum class NoteType {
    Tap,
    // 将来ここに Hold, Slide, Flick, Wipe, Trace を足していく
};

struct Note {
    Beat   beat;
    double x = 128.0;     // 0〜256座標
    NoteType type = NoteType::Tap;
};

struct ChartMeta {
    std::string title;
    std::string artist;
    std::string audio;    // 音源ファイル名
};

struct Chart {
    int         formatVersion = 1;
    ChartMeta   meta;
    double      bpm = 120.0;
    int         offsetMs = 0;
    std::vector<Note> notes;
};

} // namespace chart
