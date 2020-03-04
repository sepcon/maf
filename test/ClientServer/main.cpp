#include <maf/logging/Logger.h>

#ifdef WIN32 // should be fixed for unix??
#include "maf/utils/TimeMeasurement.h"
#include "ControllableInterface.h"
#include <iostream>
#include <Windows.h>

#include <maf/messaging/ExtensibleComponent.h>

class MainComponent
    : public maf::messaging::ExtensibleComponent
{
    std::vector<HMODULE> libHandles;
    std::vector<std::string> modules = {"Client", "Server"};
    std::vector<maf::test::ControllableInterface*> testables;

    void onExit() override
    {
        maf::logging::Logger::debug("Component exits!");
    }

    void onEntry() override
    {
        using maf::logging::Logger;

        for(auto& m : modules)
        {
            auto lib = "Test" + m + ".dll";

            if(auto h = LoadLibraryA(lib.c_str()))
            {
                libHandles.push_back(h);
                maf::test::ControllableDetails* details =
                    reinterpret_cast<maf::test::ControllableDetails*>(
                        GetProcAddress(h, "exports")
                        );

                if(details)
                {
                    Logger::debug(
                        "Loaded a plugin, details:",
                        "\n \tfile name        = ", details->fileName,
                        "\n \tclass name       = ", details->className,
                        "\n \tapi version      = ", details->apiVersion,
                        "\n \tobject name      = ", details->objectName,
                        "\n \tobject version   = ", details->objectVersion
                        );
                    if(auto plg = details->initializeFunc())
                    {
                        testables.push_back(plg);
                    }
                }
            }
            else
            {
                Logger::debug("Failed to load ", lib);
            }
        }
        if(testables.size() == modules.size())
        {
            for(auto& testable : testables)
            {
                testable->init();
            }

            for(auto& testable : testables)
            {
                testable->start();
            }

            for(auto& testable : testables)
            {
                testable->deinit();
            }
        }
        stop();
    }
public:
    using maf::messaging::ExtensibleComponent::ExtensibleComponent;
};

int main()
{
    using maf::logging::Logger;
    Logger::init(maf::logging::LOG_LEVEL_DEBUG
            /*| maf::logging::LOG_LEVEL_FROM_INFO*/,
        [](const std::string& msg) {
            std::cout << msg << std::endl;
        },
        [](const std::string& msg) {
            std::cerr << msg << std::endl;
        });
    maf::util::TimeMeasurement tm([](auto t){
        Logger::debug("Total time is: ",
                          static_cast<double>(t.count()) / 1000,
                          "ms");
    });


    MainComponent maincomp{"This is main component"};
    maincomp.run(maf::messaging::LaunchMode::AttachToCurrentThread);

    return 0;
}

#else

int main()
{
    maf::logging::Logger::error("Do not test non-windows yet");
}

#endif


