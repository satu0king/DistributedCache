#pragma once
#include <istream>

enum class DataConnector {DEFAULT, POSTGRES};

std::istream& operator>>(std::istream& in, DataConnector& dataConnector)
{
    std::string token;
    in >> token;
    if (token == "DEFAULT")
        dataConnector = DataConnector::DEFAULT;
    else if (token == "POSTGRES")
        dataConnector = DataConnector::POSTGRES;
    else 
        in.setstate(std::ios_base::failbit);
    return in;
}