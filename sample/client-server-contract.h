#pragma once

#include <maf/messaging/client-server/CSTypes.h>
#include <maf/messaging/client-server/Address.h>

#include <vector>
#include <string>
#include <map>

constexpr auto REQUESTS_PER_CLIENT = 100;
constexpr auto WEATHER_SERVER_PORT  = 0 ;

constexpr auto SERVER_NAME = "nocpes.github.com";
constexpr auto SID_WeatherService = "weather_service";
constexpr auto SERVER_UPDATE_CYCLE = 10;
constexpr auto SERVER_TOTAL_UPDATES_PER_REQUEST = 10000000;

// clang-format off
#include <maf/messaging/client-server/CSContractDefinesBegin.mc.h>

REQUEST(get_pid)
    OUTPUT((int, pid))
ENDREQUEST()

REQUEST(kill_process)
    INPUT((int, pid))
ENDREQUEST()

REQUEST(today_weather)
    using StringList = std::vector<std::string>;
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
        (int32_t, pid, -1)
    )

ENDREQUEST(today_weather)

VOID_REQUEST(clear_all_status)

VOID_REQUEST(update_status)

VOID_REQUEST(broad_cast_signal)

VOID_REQUEST(shutdown)

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

REQUEST(boot_time)
    OUTPUT((uint64_t, seconds, 0))
ENDREQUEST(boot_time)

VOID_SIGNAL(server_request)

SIGNAL(client_info_request)
    ATTRIBUTES
    (
        (std::string, user_name)
    )
ENDSIGNAL(client_info_request)

VOID_REQUEST(unhandled)

REQUEST(implicitly_response)
    INPUT
    (
        (std::string, client_name)
    )
ENDREQUEST(implicitly_response)
// clang-format on

#include <maf/messaging/client-server/CSContractDefinesEnd.mc.h>
