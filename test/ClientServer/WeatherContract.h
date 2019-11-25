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
constexpr const maf::messaging::ServiceIDConstant SID_WeatherService = "weather_service";
constexpr const int SERVER_UPDATE_CYCLE = 10;
constexpr const int SERVER_TOTAL_UPDATES_PER_REQUEST = 10000000;

#endif

#define MAF_ENABLE_DUMP
#include <maf/messaging/client-server/CSContractDefinesBegin.mc.h>

SERVICE(weather)

    REQUEST(today_weather)
        ALIAS(using StringList = std::vector<std::string>);
        OUTPUT
        (
            (int, place_id, -1),
            (std::string, the_status, "It is going to rain now!"),
            (StringList, list_of_places),
            (StringList, shared_list_of_places),
            (uint32_t, your_command, 0)
        )
        INPUT
        (
            (std::string, client_name, "This s client"),
            (uint32_t, command, 0)
        )

    ENDREQUEST(today_weather)

    VOID_REQUEST(clear_all_status_request)

    VOID_REQUEST(update_status_request)

    VOID_REQUEST(broad_cast_signal_request)

    VOID_REQUEST(shutdown)


    REQUEST(today_weather1)
        ALIAS(using StringList = std::vector<std::string>);
        OUTPUT
            (
                (int, place_id, -1),
                (std::string, the_status, "It is going to rain now!"),
                (StringList, list_of_places),
                (StringList, shared_list_of_places)
                )
        INPUT
            (
                (std::string, client_name, "This s client"),
                (uint32_t, command, 0)
                )

    ENDREQUEST(today_weather1)


    PROPERTY(simple)
        using CustomHeader = std::map<std::string, std::string>;
        using String = std::string;
        STATUS
            (
                (bool,              compliant,           true),
                (bool,              critical,            false),
                (CustomHeader,      headers                   ),
                (String,            cloud_message,       "don't care")
            )
    ENDPROPERTY(simple)

    PROPERTY(compliance)
        STATUS
        (
            (bool, compliant, true),
            (bool, critical, false),
            (int, updated_count, 0),
            (std::string, cloud_message, "This device is compliant!")
        )
    ENDPROPERTY(compliance)

    PROPERTY(compliance1)
    STATUS
        (
            (bool, compliant, true),
            (bool, critical, false),
            (int, updated_count, 0),
            (std::string, cloud_message, "This device is compliant1!")
            )
    ENDPROPERTY(compliance1)

    PROPERTY(compliance2)
    STATUS
        (
            (bool, compliant, true),
            (bool, critical, false),
            (int, updated_count, 0),
            (std::string, cloud_message, "This device is compliant2!")
            )
    ENDPROPERTY(compliance2)
    PROPERTY(compliance3)
    STATUS
        (
            (bool, compliant, true),
            (bool, critical, false),
            (int, updated_count, 0),
            (std::string, cloud_message, "This device is compliant3!")
        )
    ENDPROPERTY(compliance3)

    PROPERTY(compliance4)
    STATUS
        (
            (bool, compliant, true),
            (bool, critical, false),
            (int, updated_count, 0),
            (std::string, cloud_message, "This device is compliant4!")
        )
    ENDPROPERTY(compliance4)


    PROPERTY(compliance5)
        STATUS
            (
                (bool, compliant, true),
                (bool, critical, false),
                (int, updated_count, 0),
                (std::string, cloud_message, "This device is compliant5sdfds!")
            )
    ENDPROPERTY(compliance5)

    PROPERTY(boot_time)
        STATUS((uint64_t, seconds, 0))
    ENDPROPERTY(boot_time)

    VOID_SIGNAL(server_arbittrary_request)

    SIGNAL(client_info_request)
        ATTRIBUTES
        (
            (std::string, user_name)
        )
    ENDSIGNAL(client_info_request)

ENDSERVICE(weather);


#include <maf/messaging/client-server/CSContractDefinesEnd.mc.h>

