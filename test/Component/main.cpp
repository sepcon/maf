#include <maf/messaging/Component.h>
#include <iostream>

using namespace maf::messaging;


struct GreetMsg
{
    std::string message;
    std::weak_ptr<Component> sender;
};

struct IntegerListRequestMsg{
    std::weak_ptr<Component> sender;
};

struct ShutdownRequestMsg{};

using IntegerList = std::vector<int>;


void registerServerMessageHandling(std::shared_ptr<Component> server)
{
    server->onMessage<GreetMsg>([](GreetMsg msg){
        std::cout << "[Server] received greet msg: " << msg.message << std::endl;
        if(auto sender = msg.sender.lock())
        {
            sender->post( GreetMsg{"This is server response!", RunningComponent::weak()} );
        }
        else
        {
            std::cout << "The sender who sent messag to server is no longer existed!" << std::endl;
        }
    })
    ->onMessage<IntegerListRequestMsg>([](IntegerListRequestMsg msg){
        if(auto sender = msg.sender.lock())
        {
            std::cout << "[Server] responds to IntegerListRequestMsg..." << std::endl;
            sender->post(IntegerList{1, 2, 3, 4, 5});
        }
    })
    ->onMessage<ShutdownRequestMsg>([](auto){
        std::cout << "[Server] Received shutdown request from client then stops!" << std::endl;
        RunningComponent::stop();
    })
            ->onMessage<std::string>([](std::string msg){
        std::cout << "[Server] Received string: " << msg << std::endl;
    });
}

void registerClientMessageHandling(std::shared_ptr<Component> client, std::shared_ptr<Component> server)
{
    client->onMessage<GreetMsg>([](GreetMsg msg){
        std::cout << "[Client] Received message from server " << msg.message << std::endl;
        msg.sender.lock()->post(std::string{"Thanks server for response!"});

        std::cout << "[Client] then sends request to server to get a list of integers" << std::endl;
        //We can use template function to post message as well
        msg.sender.lock()->post(IntegerListRequestMsg{RunningComponent::weak()});
    })
    ->onMessage<IntegerList>([server](IntegerList integers){
        std::cout << "[Client] received integer list from server: " << std::endl;
        for(auto i : integers)
        {
            std::cout << i << ", ";
        }
        std::cout << std::endl;

        std::cout << "[Client] request server to shutdown and then client stops" << std::endl;
        server->post<ShutdownRequestMsg>();
        RunningComponent::stop();
    });
}

int main()
{
    auto server = Component::create();
    auto client = Component::create();

    registerClientMessageHandling(client, server);
    registerServerMessageHandling(server);

    // start server in dedicated thread
    auto serverFuture = server->runAsync();

    server->post(GreetMsg{"Client greets server!", client});
    // start client in current thread
    client->run();
    // Wait for server to completely stop
    serverFuture.wait();

    std::cout << "Program exits!" << std::endl;
    return 0;
}
