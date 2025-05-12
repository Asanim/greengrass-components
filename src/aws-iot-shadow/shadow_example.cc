#include <iostream>
#include <string>
#include <thread>
#include <chrono>
#include <aws/common/json.h>
#include <aws/greengrass/GreengrassCoreIpcModel.h>
#include <aws/crt/mqtt/MqttClient.h>
#include <aws/crt/Api.h>
#include <exception>
#include "shadow_example.h" // Added include for the header file

// Define an enum for the update source
enum class DeviceUpdateSource
{
  STARTUP,
  UPDATED_BY_CLOUD_SHADOW,
  UPDATED_BY_LOCAL
};

void ShadowExample::PrintHelp()
{
  std::cout << "Usage: shadow_handler [command] [options]\n"
            << "Commands:\n"
            << "  get <shadow_name>          Get the specified shadow\n"
            << "  update <shadow_name> <payload>  Update the specified shadow with payload\n"
            << "  list                       List all named shadows\n"
            << "  help                       Show this help message\n";
}

ResultCode ShadowExample::GetThingShadow(const Aws::Crt::String &shadow_name)
{
  std::cout << "GetThingShadowRequest Getting shadow: " << thing_name_ << "/" << shadow_name;
  Aws::Greengrass::GetThingShadowRequest request;

  request.SetThingName(thing_name_);
  request.SetShadowName(shadow_name);

  auto get_shadow_operation = ipc_client_->NewGetThingShadow();
  auto activate = get_shadow_operation->Activate(request, nullptr);
  activate.wait();

  auto responseFuture = get_shadow_operation->GetResult();
  if (responseFuture.wait_for(std::chrono::seconds(10)) == std::future_status::timeout)
  {
    std::cerr << "Operation timed out while getting shadow: " << thing_name_ << "/" << shadow_name;
    return ResultCode::kFailure;
  }
  if (auto response = responseFuture.get(); !response)
  {
    if (auto errorType = response.GetResultType(); errorType == OPERATION_ERROR)
    {
      auto *error = response.GetOperationError();
      std::cerr << "Operation error while getting shadow: " << error->GetMessage().value();
      return ResultCode::kFailure;
    }
    else
    {
      std::cerr << "RPC error while getting shadow: " << response.GetRpcError();
      return ResultCode::kFailure;
    }
  }
  else
  {
    auto result = response.GetOperationResponse();
    if (result->GetPayload().has_value())
    {
      std::string shadowPayload(result->GetPayload().value().begin(), result->GetPayload().value().end());
      std::cout << "Successfully got shadow: " << thing_name_ << "/" << shadow_name << ": " << shadowPayload;
    }
    else
    {
      std::cerr << "Shadow payload is empty";
      return ResultCode::kFailure;
    }
  }
  std::cout << "Successfully got shadow: " << thing_name_ << "/" << shadow_name;
  return ResultCode::kSuccess;
}

ResultCode ShadowExample::UpdateThingShadow(const Aws::Crt::String &shadow_name, const std::string &shadow_payload)
{
  Aws::Greengrass::UpdateThingShadowRequest request;
  request.SetThingName(thing_name_);
  request.SetShadowName(shadow_name);

  Aws::Crt::Vector<uint8_t> payload(shadow_payload.begin(), shadow_payload.end());
  request.SetPayload(payload);

  auto update_shadow_operation = ipc_client_->NewUpdateThingShadow();
  auto activate = update_shadow_operation->Activate(request, nullptr);
  activate.wait();

  auto responseFuture = update_shadow_operation->GetResult();
  if (responseFuture.wait_for(std::chrono::seconds(10)) == std::future_status::timeout)
  {
    std::cerr << "Operation timed out while updating shadow: " << thing_name_ << "/" << shadow_name;
    return ResultCode::kFailure;
  }

  if (auto response = responseFuture.get(); !response)
  {
    if (auto errorType = response.GetResultType(); errorType == OPERATION_ERROR)
    {
      auto *error = response.GetOperationError();
      std::cerr << "Operation error while updating shadow: " << error->GetMessage().value();
      return ResultCode::kFailure;
    }
    else
    {
      std::cerr << "RPC error while updating shadow: " << response.GetRpcError();
      return ResultCode::kFailure;
    }
  }
  else
  {
    std::cout << "Successfully updated shadow: " << thing_name_ << "/" << shadow_name;
  }

  return ResultCode::kSuccess;
}

ResultCode ShadowExample::ListNamedShadowsForThing()
{
  Aws::Crt::String next_token;
  Aws::Crt::Vector<Aws::Crt::String> named_shadows;
  int timeout_seconds = 10;

  do
  {
    Aws::Greengrass::ListNamedShadowsForThingRequest request;
    request.SetThingName(thing_name_);
    if (!next_token.empty())
    {
      request.SetNextToken(next_token);
    }
    request.SetPageSize(25);

    auto list_shadows_operation = ipc_client_->NewListNamedShadowsForThing();
    auto activate = list_shadows_operation->Activate(request, nullptr);
    activate.wait();

    auto responseFuture = list_shadows_operation->GetResult();
    if (responseFuture.wait_for(std::chrono::seconds(timeout_seconds)) == std::future_status::timeout)
    {
      std::cerr << "Operation timed out while listing named shadows for thing: " << thing_name_;
      return ResultCode::kFailure;
    }

    if (auto response = responseFuture.get(); !response)
    {
      if (auto errorType = response.GetResultType(); errorType == OPERATION_ERROR)
      {
        auto *error = response.GetOperationError();
        std::cerr << "Operation error while listing named shadows: " << error->GetMessage().value();
        return ResultCode::kFailure;
      }
      else
      {
        std::cerr << "RPC error while listing named shadows: " << response.GetRpcError();
        return ResultCode::kFailure;
      }
    }
    else
    {

      auto result = response.GetOperationResponse();
      std::cout << "result->GetResults" << std::endl;
      if (result->GetResults().has_value())
      {
        std::cout << "result->GetResults().value" << std::endl;
        // const auto& shadows = result->GetResults().value();
        Aws::Crt::Vector<Aws::Crt::String> shadows = result->GetResults().value();
        std::cout << "vector size: " << shadows.size() << std::endl;
        named_shadows = shadows;
      }
      if (result->GetNextToken().has_value())
      {
        std::cout << "NextToken: " << result->GetNextToken().value().c_str() << std::endl;
      }
      else
      {
        std::cout << "NextToken is empty or not present." << std::endl;
      }
    }
  } while (!next_token.empty());

  std::cout << std::endl;
  std::cout << "Successfully received named shadows for thing " << thing_name_ << ": " << named_shadows.size();

  for (const auto &shadow : named_shadows)
  {
    std::cout << "Shadow: " << shadow;
  }

  return ResultCode::kSuccess;
}

ResultCode ShadowExample::Initialize()
{
  // Initialize ApiHandle
  apiHandle_ = std::make_unique<Aws::Crt::ApiHandle>(Aws::Crt::DefaultAllocator());

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
  auto connectionStatus = ipc_client_->Connect(*ipc_lifecycle_handler_).get();
  if (!connectionStatus)
  {
    std::cerr << "Failed to establish IPC connection: " << connectionStatus.StatusToString();
    return ResultCode::kFailure;
  }
  else
  {
    // MQTT setup successful, mark as connected
    is_connected_ = true;
    is_running_ = true;
  }

  return ResultCode::kSuccess;
}