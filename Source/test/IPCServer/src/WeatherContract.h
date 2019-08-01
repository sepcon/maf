#pragma once

#include <thaf/messaging/client-server/CSTypes.h>
#include <thaf/messaging/client-server/CSContractDefines.mc.h>
#include <thaf/messaging/client-server/CSContractBegin.mc.h>

constexpr thaf::messaging::Address::Port REQUESTS_PER_CLIENT = 100;
constexpr thaf::messaging::Address::Port WEATHER_SERVER_PORT  = 0 ;

constexpr const char* const SERVER_ADDRESS = "nocpes.github.com";
constexpr const thaf::messaging::ServiceID SID_WeatherService = 0;

result_object_s(WeatherStatus)
    enum StatusType{
        Rainy,
        Suny,
        Windy
    };
    properties
        (
            (StatusType, status),
            (std::string, sStatus),
            (std::vector<std::string>, extra_information)
        )
result_object_e(WeatherStatus)

request_object_s(WeatherStatus)
    properties
(
    (int, place_id),
	(std::string, sid),
    (std::vector<std::string>, extra_information)
    )
request_object_e(WeatherStatus)


result_object_s(PolicyStatus)
        properties((int, compliant_status))
result_object_e(PolicyStatus)

#include <thaf/messaging/client-server/CSContractEnd.mc.h>
