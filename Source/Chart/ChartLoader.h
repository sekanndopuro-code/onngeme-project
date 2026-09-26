#pragma once
#include "ChartData.h"
#include <string>
#include <optional>

namespace chart {

// 成功なら Chart を、失敗なら空を返す
std::optional<Chart> loadFromFile(const std::string& filepath);

} // namespace chart
