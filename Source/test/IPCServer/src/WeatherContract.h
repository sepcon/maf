#pragma once

#include "thaf/Messaging/IPC/cscmbk.h"
#include "thaf/Messaging/IPC/CSContractMC.h"

#define REQUESTS_PER_CLIENT 100
#define SERVER_ADDRESS "\\\\.\\pipe\\mynamedpipe" 
#define WEATHER_SERVER_PORT 0

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

#include "thaf/Messaging/IPC/cscmrs.h"
