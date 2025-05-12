#include "mqtt_example.h"
#include "subscription_example.h"
#include <iostream>
#include <fstream>
#include <sstream>
#include <string>

int main(int argc, char *argv[])
{
    if (argc < 2)
    {
        std::cerr << "Usage: " << argv[0] << " <topic_to_subscribe>\n";
        return 1;
    }

    std::string topic_to_subscribe = argv[1];

    // Initialize MqttExample
    MqttExample mqtt_handler;
    if (mqtt_handler.Setup() != ResultCode::kSuccess)
    {
        std::cerr << "Failed to initialize MqttExample\n";
        return 1;
    }

    // Initialize SubscriptionHandler and subscribe to the provided topic
    auto subscription_handler = std::make_shared<SubscriptionHandler>();
    if (mqtt_handler.SubscribeToTopic(topic_to_subscribe, subscription_handler) != ResultCode::kSuccess)
    {
        std::cerr << "Failed to subscribe to topic: " << topic_to_subscribe << "\n";
        return 1;
    }

    while (mqtt_handler.IsRunning())
    {
        std::this_thread::sleep_for(std::chrono::seconds(1));
    }

    return 0;
}
