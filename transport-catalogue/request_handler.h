#pragma once

#include "transport_catalogue.h"
#include "domain.h"
#include "json.h"
#include "svg.h"
#include "map_renderer.h"
#include "transport_router.h"

#include <string_view>
#include <unordered_set>
#include <vector>

using namespace std::literals;
/*
 * Здесь можно было бы разместить код обработчика запросов к базе, содержащего логику, которую не
 * хотелось бы помещать ни в transport_catalogue, ни в json reader.
 *
 * В качестве источника для идей предлагаем взглянуть на нашу версию обработчика запросов.
 * Вы можете реализовать обработку запросов способом, который удобнее вам.
 *
 * Если вы затрудняетесь выбрать, что можно было бы поместить в этот файл,
 * можете оставить его пустым.
 */

// Класс RequestHandler играет роль Фасада, упрощающего взаимодействие JSON reader-а
// с другими подсистемами приложения.
// См. паттерн проектирования Фасад: https://ru.wikipedia.org/wiki/Фасад_(шаблон_проектирования)
/*
class RequestHandler {
public:
    // MapRenderer понадобится в следующей части итогового проекта
    RequestHandler(const TransportCatalogue& db, const renderer::MapRenderer& renderer);

    // Возвращает информацию о маршруте (запрос Bus)
    std::optional<BusStat> GetBusStat(const std::string_view& bus_name) const;

    // Возвращает маршруты, проходящие через
    const std::unordered_set<BusPtr>* GetBusesByStop(const std::string_view& stop_name) const;

    // Этот метод будет нужен в следующей части итогового проекта
    svg::Document RenderMap() const;

private:
    // RequestHandler использует агрегацию объектов "Транспортный Справочник" и "Визуализатор Карты"
    const TransportCatalogue& db_;
    const renderer::MapRenderer& renderer_;
};
*/
namespace detail {
struct StringPairHasher {
    std::size_t operator()(const std::pair<std::string, std::string>& string_pair) const;
};
}

class RequestHandler {
public:
    RequestHandler(transport_catalogue::TransportCatalogue& db);
    
    const std::optional<domain::BusStat> GetBusStat(std::string_view bus_name) const;
    
    const std::optional<domain::StopStat> GetStopStat(std::string_view stop_name) const;
    
    void HandleBusBaseRequests(std::vector<domain::BusBaseRequest>& requests);
    void HandleStopBaseRequests(const std::vector<domain::StopBaseRequest>& requests);
    svg::Document RenderRoutes(const RenderSettings& settings) const;
    TransportRouter GetRouter(const domain::RoutingSettings& settings) const;
    
    
private:
    transport_catalogue::TransportCatalogue& db_;
};