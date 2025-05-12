#include "mqtt_example.h"
#include <iostream>
#include <fstream>
#include <sstream>
#include <string>

int main(int argc, char *argv[])
{
  // Initialize MqttExample
  MqttExample mqtt_handler;
  if (mqtt_handler.Setup() != ResultCode::kSuccess)
  {
    std::cerr << "Failed to initialize MqttExample\n";
    return 1;
  }

  // Initialize SubscriptionHandler and subscribe to the provided topic
  if (mqtt_handler.SubscribeToCompponentUpdates() != ResultCode::kSuccess)
  {
    std::cerr << "Failed to subscribe to component updates \n";
    return 1;
  }

  while (true)
  {
    std::this_thread::sleep_for(std::chrono::seconds(1));
  }

  return 0;
}
