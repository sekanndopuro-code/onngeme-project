#include "rapidjson/document.h"
#include "MainScene.h"  
#include "axmol/axmol.h"
#include "Chart/ChartLoader.h"

using namespace rapidjson;

namespace chart {

static NoteType parseType(const std::string& s) {
    if (s == "tap") return NoteType::Tap;
    // 将来ここに追加
    return NoteType::Tap;                   // 未知は暫定でTap扱い
}

std::optional<Chart> loadFromFile(const std::string& filepath) {
    // 1) ファイルを文字列として読む (Axmol流)
    std::string raw = ax::FileUtils::getInstance()->getStringFromFile(filepath);
    if (raw.empty()) {
        AXLOGD("[ChartLoader] file not found or empty: {}", filepath);
        return std::nullopt;
    }

    // 2) パース
    Document doc;
    doc.Parse(raw.c_str());
    if (doc.HasParseError() || !doc.IsObject()) {
        AXLOGD("[ChartLoader] JSON parse error");
        return std::nullopt;
    }

    Chart c;

    // 3) formatVersion
    if (doc.HasMember("formatVersion") && doc["formatVersion"].IsInt()) {
        c.formatVersion = doc["formatVersion"].GetInt();
    }

    // 4) meta
    if (doc.HasMember("meta") && doc["meta"].IsObject()) {
        const auto& m = doc["meta"];
        if (m.HasMember("title")  && m["title"].IsString())  c.meta.title  = m["title"].GetString();
        if (m.HasMember("artist") && m["artist"].IsString()) c.meta.artist = m["artist"].GetString();
        if (m.HasMember("audio")  && m["audio"].IsString())  c.meta.audio  = m["audio"].GetString();
    }

    // 5) bpm / offsetMs
    if (doc.HasMember("bpm")      && doc["bpm"].IsNumber())      c.bpm      = doc["bpm"].GetDouble();
    if (doc.HasMember("offsetMs") && doc["offsetMs"].IsNumber()) c.offsetMs = doc["offsetMs"].GetInt();

    // 6) notes 配列
    if (doc.HasMember("notes") && doc["notes"].IsArray()) {
        for (const auto& n : doc["notes"].GetArray()) {
            if (!n.IsObject()) continue;

            Note note;

            // beat: [bar, num, den]
            if (n.HasMember("beat") && n["beat"].IsArray() && n["beat"].Size() == 3) {
                note.beat.bar = n["beat"][0].GetInt();
                note.beat.num = n["beat"][1].GetInt();
                note.beat.den = n["beat"][2].GetInt();
                if (note.beat.den == 0) note.beat.den = 1;      // 0除算防止
            }

            // x
            if (n.HasMember("x") && n["x"].IsNumber()) {
                note.x = n["x"].GetDouble();
            }

            // type
            if (n.HasMember("type") && n["type"].IsString()) {
                note.type = parseType(n["type"].GetString());
            }

            c.notes.push_back(note);
        }
    }

    return c;
}

} // namespace chart
