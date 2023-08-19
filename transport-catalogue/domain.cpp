#include "domain.h"

#include <algorithm>

/*
 * В этом файле вы можете разместить классы/структуры, которые являются частью предметной области
 * (domain) вашего приложения и не зависят от транспортного справочника. Например Автобусные
 * маршруты и Остановки.
 *
 * Их можно было бы разместить и в transport_catalogue.h, однако вынесение их в отдельный
 * заголовочный файл может оказаться полезным, когда дело дойдёт до визуализации карты маршрутов:
 * визуализатор карты (map_renderer) можно будет сделать независящим от транспортного справочника.
 *
 * Если структура вашего приложения не позволяет так сделать, просто оставьте этот файл пустым.
 *
 */

namespace domain {
namespace detail {
std::size_t StopPairHasher::operator()(const std::pair<const Stop*, const Stop*>& stop_pair) const {
    return std::hash<const void*>{}(stop_pair.first) + 37 * std::hash<const void*>{}(stop_pair.second);
}
    
bool BusPtrCompare::operator()(const Bus* lhs, const Bus* rhs) const {
    return lhs->name < rhs->name;
}
    
bool StopPtrCompare::operator()(const Stop* lhs, const Stop* rhs) const {
    return lhs->name < rhs->name;
}
}
    
Stop::Stop(std::string name, double lat, double lng)
    : name(name)
    , coordinates{lat, lng} {
}
}
