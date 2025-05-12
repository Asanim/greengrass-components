///
/// \brief
///
/// \file mqtt_example.cc
///

#include <aws/crt/Api.h>
#include <aws/greengrass/GreengrassCoreIpcClient.h>
#include "mqtt_example.h"
#include "component_update_example.h"
#include <chrono>
#include <iostream>
#include <queue>
#include <string>
#include <thread>
#include <vector>

using namespace Aws::Crt;
using namespace Aws::Greengrass;

MqttExample::MqttExample() { Setup(); }

MqttExample::~MqttExample() { Stop(); }

ResultCode MqttExample::Stop()
{
  return ResultCode::kSuccess;
}

ResultCode MqttExample::Setup()
{
  // Retrieve environment variables
  thing_name_ = getenv("AWS_IOT_THING_NAME");
  ggc_version_ = getenv("GGC_VERSION");

  // Check if required environment variables are not set
  if (!thing_name_ || !ggc_version_)
  {
    // Log an error and disable greengrass-examples
    std::cerr << "AWS Environment variables 'AWS_IOT_THING_NAME' 'GGC_VERSION' not set. You must run this as a Greengrass component";
    return ResultCode::kFailure;
  }
  // Log the retrieved thing name and GGC version
  std::cout << "Thing name: " << thing_name_ << "\nGGC version: " << ggc_version_;

  // Check if MQTT setup is successful
  if (MqttExample::SetupMQTT() != ResultCode::kSuccess)
  {
    // Log an error and disable greengrass-examples
    std::cerr << "Could not initialize IoT Core. MQTT publish is disabled";
    return ResultCode::kFailure;
  }

  return ResultCode::kSuccess;
}

ResultCode MqttExample::SetupMQTT()
{
  // Initialize ApiHandle
  apiHandle_ = std::make_unique<Aws::Crt::ApiHandle>(DefaultAllocator());

  // Initialize EventLoopGroup
  event_loop_group_ = std::make_unique<Aws::Crt::Io::EventLoopGroup>(1);

  // Initialize DefaultHostResolver
  socket_resolver_ = std::make_unique<Aws::Crt::Io::DefaultHostResolver>(*event_loop_group_, 64, 30);

  // Initialize ClientBootstrap
  client_bootstrap_ = std::make_unique<Aws::Crt::Io::ClientBootstrap>(*event_loop_group_, *socket_resolver_);

  // Initialize IpcClientLifecycleHandler
  ipc_lifecycle_handler_ = std::make_unique<IpcClientLifecycleHandler>();

  // Initialize GreengrassCoreIpcClient
  ipc_client_ = std::make_shared<Aws::Greengrass::GreengrassCoreIpcClient>(*client_bootstrap_);

  // Connect to IPC
  // Check IPC connection status
  if (auto connectionStatus = ipc_client_->Connect(*ipc_lifecycle_handler_).get(); !connectionStatus)
  {
    std::cerr << "Failed to establish IPC connection: " << connectionStatus.StatusToString();
    return ResultCode::kFailure;
  }

  component_update_handler_ = std::make_shared<ComponentUpdateExample>();
  component_update_handler_->SetIPCClient(ipc_client_);

  is_running_ = true;

  return ResultCode::kSuccess;
}

ResultCode MqttExample::SubscribeToCompponentUpdates()
{
  // Subscribe to updates
  std::this_thread::sleep_for(std::chrono::milliseconds(100));

  // SubscribeToConfigurationUpdateRequest
  Aws::Greengrass::SubscribeToComponentUpdatesRequest request;

  auto ipc_update_operation = ipc_client_->NewSubscribeToComponentUpdates(component_update_handler_);
  auto activate = ipc_update_operation->Activate(request, nullptr);
  activate.wait();

  auto responseFuture = ipc_update_operation->GetResult();
  if (responseFuture.wait_for(std::chrono::seconds(10)) == std::future_status::timeout)
  {
    std::cerr << "Operation timed out while waiting for response from Greengrass Core." << std::endl;
  }

  auto response = responseFuture.get();
  if (response)
  {
    std::cout << "Successfully subscribed component updates: " << std::endl;
  }
  else
  {
    // An error occurred.
    std::cerr << "Failed to subscribe component updates: " << std::endl;
    auto errorType = response.GetResultType();
    if (errorType == OPERATION_ERROR)
    {
      auto *error = response.GetOperationError();
      std::cerr << "Operation error: " << error->GetMessage().value() << std::endl;
    }
    else
    {
      std::cerr << "RPC error: " << response.GetRpcError() << std::endl;
    }
  }

  int i = 0;
  while (true)
  {
    std::cout << "sleep: " << i++ << std::endl;
    std::this_thread::sleep_for(std::chrono::seconds(60));
  }
}