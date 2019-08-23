#pragma once

#include <maf/messaging/client-server/CSTypes.h>
#include <maf/messaging/client-server/CSContractBegin.mc.h>
#include <maf/messaging/client-server/CSContractDefines.mc.h>

constexpr maf::messaging::Address::Port REQUESTS_PER_CLIENT = 100;
constexpr maf::messaging::Address::Port WEATHER_SERVER_PORT  = 0 ;

constexpr const char* const SERVER_ADDRESS = "nocpes.github.com";
constexpr const maf::messaging::ServiceID SID_WeatherService = 0;

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
            (std::vector<std::string>, extra_information),
			(int, index)
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

result_object_s(NotificationMessageUpdate)
	properties((std::string, message, "nothing to show"))
result_object_e(NotificationMessageUpdate)

result_object_no_props(ShutDownServerRequest)
	
#include <maf/messaging/client-server/CSContractEnd.mc.h>
