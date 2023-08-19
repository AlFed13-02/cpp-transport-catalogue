#include "json.h"
#include "json_reader.h"
#include "request_handler.h"

#include <iostream>

using namespace std::literals;

int main() {
    /*
     * Примерная структура программы:
     *
     * Считать JSON из stdin
     * Построить на его основе JSON базу данных транспортного справочника
     * Выполнить запросы к справочнику, находящиеся в массиве "stat_requests", построив JSON-массив
     * с ответами.
     * Вывести в stdout ответы в виде JSON
     */
    transport_catalogue::TransportCatalogue transport_catalogue;
    RequestHandler request_handler(transport_catalogue);
    JsonReader json_reader(request_handler);
    json_reader.LoadJson(std::cin);
    json_reader.ProcessBaseRequests();
    auto json_doc = json_reader.ProcessStatRequests();
    json::Print(json_doc, std::cout);
}