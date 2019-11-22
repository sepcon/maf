#pragma once
#ifndef WEATHER_CONTRACT_H
#define WEATHER_CONTRACT_H
#include <maf/messaging/client-server/CSTypes.h>
#include <maf/messaging/client-server/Address.h>

#include <vector>
#include <string>
#include <map>

constexpr maf::messaging::Address::Port REQUESTS_PER_CLIENT = 100;
constexpr maf::messaging::Address::Port WEATHER_SERVER_PORT  = 0 ;

constexpr const char* const SERVER_ADDRESS = "nocpes.github.com";
constexpr const maf::messaging::ServiceID SID_WeatherService = 0;
constexpr const int SERVER_UPDATE_CYCLE = 10;
constexpr const int SERVER_TOTAL_UPDATES_PER_REQUEST = 10000000;

#endif

#define MAF_ENABLE_DUMP
#include <maf/messaging/client-server/CSContractDefinesBegin.mc.h>

SERVICE(weather)
    FUNCTION(today_weather)
        ALIAS(using StringList = std::vector<std::string>);
        RESULT
        (
            (int, place_id, -1),
            (std::string, the_status, "It is going to rain now!"),
            (StringList, list_of_places),
            (StringList, shared_list_of_places),
            (uint32_t, your_command, 0)
        )
        REQUEST
        (
            (std::string, client_name, "This s client"),
            (uint32_t, command, 0)
        )

    ENDFUNC(today_weather)

    FUNCTION(today_weather1)
    ALIAS(using StringList = std::vector<std::string>);
    RESULT
        (
            (int, place_id, -1),
            (std::string, the_status, "It is going to rain now!"),
            (StringList, list_of_places),
            (StringList, shared_list_of_places)
            )
    REQUEST
        (
            (std::string, client_name, "This s client"),
            (uint32_t, command, 0)
            )

    ENDFUNC(today_weather1)

    FUNCTION(simple_status)
        using CustomHeader = std::map<std::string, std::string>;
        using String = std::string;
        STATUS
        (
            (bool,              compliant,           true),
            (bool,              critical,            false),
            (CustomHeader,      headers                   ),
            (String,            cloud_message,       "don't care")
        )
    ENDFUNC(simple_status)

    FUNCTION(compliance)
        STATUS
        (
            (bool, compliant, true),
            (bool, critical, false),
            (int, updated_count, 0),
            (std::string, cloud_message, "This device is compliant!")
        )
    ENDFUNC(compliance)
    FUNCTION(compliance1)
    STATUS
        (
            (bool, compliant, true),
            (bool, critical, false),
            (std::string, cloud_message, "This device is compliant1!")
            )
    ENDFUNC(compliance1)
    FUNCTION(compliance2)
    STATUS
        (
            (bool, compliant, true),
            (bool, critical, false),
            (std::string, cloud_message, "This device is compliant2!")
            )
    ENDFUNC(compliance2)
    FUNCTION(compliance3)
    STATUS
        (
            (bool, compliant, true),
            (bool, critical, false),
            (std::string, cloud_message, "This device is compliant3!")
            )
    ENDFUNC(compliance3)
    FUNCTION(compliance4)
    STATUS
        (
            (bool, compliant, true),
            (bool, critical, false),
            (std::string, cloud_message, "This device is compliant4!")
            )
    ENDFUNC(compliance4)
    FUNCTION(compliance5)
    STATUS
        (
            (bool, compliant, true),
            (bool, critical, false),
            (std::string, cloud_message, "This device is compliant5sdfds!")
            )
    ENDFUNC(compliance5)

    FUNCTION(boot_time)
        STATUS((uint64_t, seconds, 0))
        REQUEST((uint64_t, seconds, 0))
    ENDFUNC(boot_time)

    FUNCTION(shutdown)
        EMPTY_REQUEST()
    ENDFUNC(shutdown)
ENDSERVICE(weather);


#include <maf/messaging/client-server/CSContractDefinesEnd.mc.h>

