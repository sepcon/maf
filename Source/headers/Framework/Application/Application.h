#ifndef APPLICATION_H
#define APPLICATION_H

#include "AppComponent.h"
#include "Messages.h"
#include "headers/Libs/Patterns/Patterns.h"

namespace thaf {
namespace app {

class Application final : public AppComponent, public pattern::SingletonObject<Application>
{
public:
    Application(Invisible) : AppComponent(false)
    {
        onSignal<ShutdownMessage>([this]{ this->shutdown(); });
    }

};
}
}
#endif // APPLICATION_H
