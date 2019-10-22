#pragma once


#include <maf/messaging/client-server/CSTypes.h>
#include <maf/messaging/client-server/Address.h>

#define MAF_ENABLE_DUMP
#include <maf/messaging/client-server/CSContractDefinesBegin.mc.h>

constexpr maf::messaging::Address::Port REQUESTS_PER_CLIENT = 100;
constexpr maf::messaging::Address::Port WEATHER_SERVER_PORT  = 0 ;

constexpr const char* const SERVER_ADDRESS = "nocpes.github.com";
constexpr const maf::messaging::ServiceID SID_WeatherService = 0;

FUNCTION(WeatherStatus)
    enum StatusType{
        Rainy,
        Suny,
        Windy
    };

    RESULT_MESSAGE
    (
            (StatusType, status),
            (std::string, sStatus),
            (std::vector<std::string>, extra_information),
			(int, index)
    )

    REQUEST_MESSAGE
    (
        (int, place_id),
        (std::string, sid),
        (std::vector<std::string>, extra_information)
    )
ENDFUNC()

FUNCTION(PolicyStatus)
        RESULT_MESSAGE((int, compliant_status))
ENDFUNC()

FUNCTION(NotificationMessageUpdate)
    RESULT_MESSAGE((std::string, message, "nothing to show"))
ENDFUNC()

FUNCTION(ShutDownServerRequest)
        EMPTY_EVENT_MESSAGE()
ENDFUNC()

#include <maf/messaging/client-server/CSContractDefinesEnd.mc.h>

