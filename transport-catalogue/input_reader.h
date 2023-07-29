#pragma once

#include "transport_catalogue.h"

#include <string>
#include <vector>
#include <tuple>
#include <unordered_map>

namespace transport_catalogue {
namespace detail {
int ReadLineWithNumber();
std::string Strip(std::string str);

struct StringPairHasher {
    std::size_t operator()(const std::pair<std::string, std::string>& string_pair) const;
};
}

namespace input {
std::tuple<double, double, std::unordered_map<std::string, int>> ParseStopData(const std::string& raw_data);
std::vector<std::string> ParseBusData(const std::string& raw_data);

void FillTransportCatalogue(TransportCatalogue& transport_catalogue);
}
}