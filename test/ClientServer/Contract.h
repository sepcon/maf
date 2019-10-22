#pragma once

#include <vector>
#include <string>
#include <map>

#define MAF_ENABLE_DUMP
#include "maf/messaging/client-server/CSContractDefinesBegin.mc.h"

FUNCTION(WeatherStatus)
using StringList = std::vector<std::string>;
RESULT_MESSAGE
    (
        (std::string, the_status, "It is going to rain now!"),
        (StringList, list_of_places),
        (StringList, shared_list_of_places)
        )
REQUEST_MESSAGE
    (
        (std::string, client_name, "This s client"),
        (uint32_t, command, 0)
        )
ENDFUNC(WeatherStatus);

FUNCTION(UpdateSignal)
using CustomHeader = std::map<std::string, std::string>;
using String = std::string;
EVENT_MESSAGE
    (
        (bool,              compliant,           true),
        (bool,              critical,            false),
        (CustomHeader,      headers                   ),
        (String,            cloud_message,       "don't care")
        )
MESSAGE
    (
        Status,
        (bool, compliant, false),
        (bool, critical, false)
        )
ENDFUNC()

FUNCTION(ComplianceStatus)
RESULT_MESSAGE
    (
        (bool, compliant, true),
        (bool, critical, false),
        (std::string, cloud_message, "This device is compliant!")
        )
ENDFUNC()

#include <maf/messaging/client-server/CSContractDefinesEnd.mc.h>
