#include "transport_catalogue.h"
#include "input_reader.h"
#include "stat_reader.h"

#include <string>
#include <sstream>
#include <iostream>
#include <string_view>
#include <utility>

using namespace std::literals;

int main() {
    using namespace transport_catalogue;
    
    TransportCatalogue transport_catalogue;
    
    
    input::FillTransportCatalogue(transport_catalogue);
    output::ProcessRequests(transport_catalogue);
}

