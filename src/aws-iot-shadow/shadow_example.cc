#include <iostream>
#include <string>
#include <thread>
#include <chrono>
#include <json/json.h>
#include <aws/core/Aws.h>
#include <aws/iotshadow/IoTShadowClient.h>
#include <aws/crt/mqtt/MqttClient.h>
#include <aws/crt/Api.h>
#include <exception>

// Define an enum for the update source
enum class DeviceUpdateSource {
    STARTUP,
    UPDATED_BY_CLOUD_SHADOW,
    UPDATED_BY_LOCAL
};

void ShadowExample::PrintHelp() {
    std::cout << "Usage: shadow_handler [command] [options]\n"
              << "Commands:\n"
              << "  get <shadow_name>          Get the specified shadow\n"
              << "  update <shadow_name> <payload>  Update the specified shadow with payload\n"
              << "  list                       List all named shadows\n"
              << "  help                       Show this help message\n";
}

sv::Status ShadowExample::GetThingShadow(const Aws::Crt::String& shadow_name) {
    LOG(INFO) << "GetThingShadowRequest Getting shadow: " << thing_name_ << "/" << shadow_name;
    GetThingShadowRequest request;
    request.SetThingName(thing_name_);
    request.SetShadowName(shadow_name);
  
    LOG(INFO) << "NewGetThingShadow";
    auto get_shadow_operation = ipc_client_->NewGetThingShadow();
    auto activate = get_shadow_operation->Activate(request, nullptr);
    activate.wait();
  
    LOG(INFO) << "get_shadow_operation->GetResult";
    auto responseFuture = get_shadow_operation->GetResult();
    if (responseFuture.wait_for(std::chrono::seconds(10)) == std::future_status::timeout) {
      LOG(ERROR) << "Operation timed out while getting shadow: " << thing_name_ << "/" << shadow_name;
      return sv::Status::kFailure;
    }
    LOG(INFO) << "responseFuture.get()";
    if (auto response = responseFuture.get(); !response) {
      if (auto errorType = response.GetResultType(); errorType == OPERATION_ERROR) {
        auto* error = response.GetOperationError();
        LOG(ERROR) << "Operation error while getting shadow: " << error->GetMessage().value();
        return sv::Status::kFailure;
      } else {
        LOG(ERROR) << "RPC error while getting shadow: " << response.GetRpcError();
        return sv::Status::kFailure;
      }
    } else {
      LOG(INFO) << "response.GetOperationResponse()";
      auto result = response.GetOperationResponse();
      if (result->GetPayload().has_value()) {
        std::string shadowPayload(result->GetPayload().value().begin(), result->GetPayload().value().end());
        LOG(INFO) << "Successfully got shadow: " << thing_name_ << "/" << shadow_name << ": " << shadowPayload;
      } else {
        LOG(ERROR) << "Shadow payload is empty";
        return sv::Status::kFailure;
      }
    }
    LOG(INFO) << "Successfully got shadow: " << thing_name_ << "/" << shadow_name;
    return sv::Status::kSuccess;
  }
  
  sv::Status ShadowExample::UpdateThingShadow(const Aws::Crt::String& shadow_name, const std::string& shadow_payload) {
    UpdateThingShadowRequest request;
    request.SetThingName(thing_name_);
    request.SetShadowName(shadow_name);
  
    Aws::Crt::Vector<uint8_t> payload(shadow_payload.begin(), shadow_payload.end());
    request.SetPayload(payload);
  
    auto update_shadow_operation = ipc_client_->NewUpdateThingShadow();
    auto activate = update_shadow_operation->Activate(request, nullptr);
    activate.wait();
  
    auto responseFuture = update_shadow_operation->GetResult();
    if (responseFuture.wait_for(std::chrono::seconds(10)) == std::future_status::timeout) {
      LOG(ERROR) << "Operation timed out while updating shadow: " << thing_name_ << "/" << shadow_name;
      return sv::Status::kFailure;
    }
  
    if (auto response = responseFuture.get(); !response) {
      if (auto errorType = response.GetResultType(); errorType == OPERATION_ERROR) {
        auto* error = response.GetOperationError();
        LOG(ERROR) << "Operation error while updating shadow: " << error->GetMessage().value();
        return sv::Status::kFailure;
      } else {
        LOG(ERROR) << "RPC error while updating shadow: " << response.GetRpcError();
        return sv::Status::kFailure;
      }
    } else {
      LOG(INFO) << "Successfully updated shadow: " << thing_name_ << "/" << shadow_name;
    }
  
    return sv::Status::kSuccess;
  }
  
  sv::Status ShadowExample::ListNamedShadowsForThing() {
    Aws::Crt::String next_token;
    Aws::Crt::Vector<Aws::Crt::String> named_shadows;
    int timeout_seconds = 10;
  
    do {
      LOG(INFO) << "ListNamedShadowsForThingRequest";
  
      ListNamedShadowsForThingRequest request;
      request.SetThingName(thing_name_);
      if (!next_token.empty()) {
        request.SetNextToken(next_token);
      }
      request.SetPageSize(25);
  
      LOG(INFO) << "NewListNamedShadowsForThing";
      auto list_shadows_operation = ipc_client_->NewListNamedShadowsForThing();
      auto activate = list_shadows_operation->Activate(request, nullptr);
      activate.wait();
      LOG(INFO) << "list_shadows_operation->GetResult";
  
      auto responseFuture = list_shadows_operation->GetResult();
      if (responseFuture.wait_for(std::chrono::seconds(timeout_seconds)) == std::future_status::timeout) {
        LOG(ERROR) << "Operation timed out while listing named shadows for thing: " << thing_name_;
        return sv::Status::kFailure;
      }
      LOG(INFO) << "responseFuture.get()";
  
      if (auto response = responseFuture.get(); !response) {
        if (auto errorType = response.GetResultType(); errorType == OPERATION_ERROR) {
          auto* error = response.GetOperationError();
          LOG(ERROR) << "Operation error while listing named shadows: " << error->GetMessage().value();
          return sv::Status::kFailure;
        } else {
          LOG(ERROR) << "RPC error while listing named shadows: " << response.GetRpcError();
          return sv::Status::kFailure;
        }
      } else {
        LOG(INFO) << "response.GetOperationResponse()";
  
        auto result = response.GetOperationResponse();
        LOG(INFO) << "result->GetResults" << std::endl;
        if (result->GetResults().has_value()) {
          LOG(INFO) << "result->GetResults().value" << std::endl;
          // const auto& shadows = result->GetResults().value();
          Aws::Crt::Vector<Aws::Crt::String> shadows = result->GetResults().value();
          LOG(INFO) << "vector size: " << shadows.size() << std::endl;
          named_shadows = shadows;
        }
        if (result->GetNextToken().has_value()) {
          LOG(INFO) << "NextToken: " << result->GetNextToken().value().c_str() << std::endl;
        } else {
          LOG(INFO) << "NextToken is empty or not present." << std::endl;
        }
      }
    } while (!next_token.empty());
  
    LOG(INFO) << std::endl;
    LOG(INFO) << "Successfully got named shadows for thing " << thing_name_ << ": " << named_shadows.size();
  
    for (const auto& shadow : named_shadows) {
      LOG(INFO) << "Shadow: " << shadow;
    }
  
    return sv::Status::kSuccess;
  }

  
sv::Status ShadowExample::Initalize() {
    // Initialize ApiHandle
    apiHandle_ = std::make_unique<Aws::Crt::ApiHandle>(DefaultAllocator());
  
    // Initialize EventLoopGroup
    event_loop_group_ = std::make_unique<Aws::Crt::Io::EventLoopGroup>(1);
  
    // Initialize DefaultHostResolver
    socket_resolver_ = std::make_unique<Aws::Crt::Io::DefaultHostResolver>(*event_loop_group_, 64, 30);
  
    // Initialize ClientBootstrap
    client_bootstrap_ = std::make_unique<Aws::Crt::Io::ClientBootstrap>(*event_loop_group_, *socket_resolver_);
  
    // Initialize IpcClientLifecycleHandler
    ipc_lifecycle_handler_ = std::make_unique<sv::IpcClientLifecycleHandler>();
  
    // Initialize GreengrassCoreIpcClient
    ipc_client_ = std::make_shared<Aws::Greengrass::GreengrassCoreIpcClient>(*client_bootstrap_);
  
    // Connect to IPC
    // Check IPC connection status
    if (auto connectionStatus = ipc_client_->Connect(*ipc_lifecycle_handler_).get(); !connectionStatus) {
      LOG(ERROR) << "Failed to establish IPC connection: " << connectionStatus.StatusToString();
      return sv::Status::kFailure;
    } else {
      // MQTT setup successful, mark as connected
      is_connected_ = true;
      is_running_ = true;
    }
  
    return sv::Status::kSuccess;
  }