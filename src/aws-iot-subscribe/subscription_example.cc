///
/// \brief Manages communication with AWS IoT Greengrass Nucleus
///
/// \file subscription_example.cc
///

#include "subscription_example.h"
#include <iostream>
#include <stdexcept>
#include <string>
#include <regex>

void SubscriptionHandler::OnStreamEvent(Aws::Greengrass::IoTCoreMessage *response)
{
  std::cout << "OnStreamEvent triggered: Received new message from Greengrass Core." << std::endl;
  std::cout << "Received new message from Greengrass Core." << std::endl;
  std::string file_path = "";

  try
  {
    auto message = response->GetMessage();

    if (message.has_value() && message.value().GetPayload().has_value())
    {
      auto message_bytes = message.value().GetPayload().value();
      std::string message_string(message_bytes.begin(), message_bytes.end());
      std::string topic_name = message.value().GetTopicName().value().c_str();

      // Handle message.
      std::cout << "Received message from Greengrass Core: " << message_string << " on topic: " << topic_name
                << std::endl;
    }
    else
    {
      throw std::invalid_argument("Message or payload is empty.");
    }
  }
  catch (const std::invalid_argument &e)
  {
    std::cerr << "Invalid argument: " << e.what() << std::endl;
  }
}

bool SubscriptionHandler::OnStreamError(OperationError *error)
{
  std::cout << "OnStreamError triggered: Error occurred in the stream." << std::endl;
  if (error->GetMessage().has_value())
  {
    std::cerr << "Greengrass Core responded with an error: " << error->GetMessage().value().c_str();
  }
  return false; // Return true to close stream, false to keep stream open.
}

void SubscriptionHandler::OnStreamClosed()
{
  std::cout << "OnStreamClosed triggered: Stream has been closed." << std::endl;
  std::cerr << "Disconnected from IPC service." << std::endl;
}