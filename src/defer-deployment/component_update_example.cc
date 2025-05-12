#include "component_update_example.h"

#include <iostream>

void ComponentUpdateExample::SetIPCClient(
    std::shared_ptr<Aws::Greengrass::GreengrassCoreIpcClient> ipc_client)
{
  ipc_client_ = ipc_client;
}

void ComponentUpdateExample::OnStreamEvent(Aws::Greengrass::ComponentUpdatePolicyEvents *response)
{
  try
  {
    std::cout << "Received component update, GetPreUpdateEvent: " << response->GetPreUpdateEvent().has_value()
              << " GetPostUpdateEvent: " << response->GetPostUpdateEvent().has_value() << std::endl;

    if (response->GetPreUpdateEvent().has_value())
    {
      std::cout << "Received component update, event ID: " << response->GetPreUpdateEvent()->GetDeploymentId()->c_str()
                << std::endl;

      int32_t recheck_after_ms = 0;

      // defer the update
      if (!is_update_safe_)
      {
        std::cout << "Update is not safe, delaying update...";
        recheck_after_ms = component_update_recheck_ms_;
      }

      // run the update thread...
      if (update_thread_ && update_thread_->joinable())
      {
        // The update thread is currently running
        std::cout << "The update thread is currently running...";
        update_thread_->join();
      }
      // The update thread is not running
      std::cout << "The update thread is not running... Creating a new update request thread";
      update_thread_ = std::make_unique<std::thread>(&ComponentUpdateExample::DeferSystemComponentUpdate, this, response->GetPreUpdateEvent()->GetDeploymentId().value(), recheck_after_ms);
    }

    if (response->GetPostUpdateEvent().has_value())
    {
      std::cout << "Received component update complete, event ID: "
                << response->GetPostUpdateEvent()->GetDeploymentId()->c_str() << std::endl;
    }
  }
  catch (const std::exception &e)
  {
    std::cout << "Exception caught: " << e.what() << std::endl;
  }
}

bool ComponentUpdateExample::OnStreamError(RpcError rpcError)
{
  std::cout << "IPC component updates error: " << rpcError.StatusToString() << std::endl;

  // Handle IPC component updates error.
  return true;
}

bool ComponentUpdateExample::OnStreamError(Aws::Greengrass::ServiceError *operationError)
{
  std::cout << "IPC component updates error: " << operationError->GetMessage()->c_str() << std::endl;

  // Handle IPC component updates error.
  return true;
}

bool ComponentUpdateExample::OnStreamError(Aws::Greengrass::ResourceNotFoundError *operationError)
{
  std::cout << "IPC component updates error: " << operationError->GetMessage()->c_str() << std::endl;

  // Handle IPC component updates error.
  return true;
}

bool ComponentUpdateExample::OnStreamError(Aws::Eventstreamrpc::OperationError *operationError)
{
  std::cout << "IPC component updates error: " << operationError->GetMessage()->c_str() << std::endl;

  // Handle IPC component updates error.
  return true;
}

void ComponentUpdateExample::DeferSystemComponentUpdate(const Aws::Crt::String &deployment_id,
                                                        int32_t recheck_after_ms)
{
  std::cout << "Building defer deployment request: " << deployment_id << std::endl;
  auto defer_update_request = Aws::Greengrass::DeferComponentUpdateRequest();
  defer_update_request.SetDeploymentId(deployment_id);
  defer_update_request.SetRecheckAfterMs(1000);
  defer_update_request.SetMessage(Aws::Crt::String("Greengrass Examples: Update is not safe, deferring until safe."));

  std::cout << "Create defer_update_operation" << std::endl;
  auto defer_update_operation = ipc_client_->NewDeferComponentUpdate();
  std::cout << "Activate defer_update_operation" << std::endl;
  auto request_status = defer_update_operation->Activate(defer_update_request, nullptr);
  std::cout << "Wait for result" << std::endl;
  request_status.wait();
  // auto result = request_status.wait_for(std::chrono::seconds(10));
  // if (result != std::future_status::ready)
  //   std::cout << "Wait for result timed out" << std::endl;

  std::cout << "Get Result" << std::endl;
  auto publish_result_future = defer_update_operation->GetResult();
  std::cout << "Waiting for Result" << std::endl;

  if (publish_result_future.wait_for(std::chrono::seconds(10)) == std::future_status::timeout)
  {
    std::cerr << "Operation timed out while waiting for response from Greengrass Core." << std::endl;
    return;
  }

  std::cout << "Check Result" << std::endl;
  if (auto publish_result = publish_result_future.get(); publish_result)
  {
    const auto *response = publish_result.GetOperationResponse();
    (void)response;
    std::cout
        << "Defer Success" << std::endl;
  }
  else
  {
    auto error_type = publish_result.GetResultType();
    if (error_type == OPERATION_ERROR)
    {
      OperationError *error = publish_result.GetOperationError();
      if (error->GetMessage().has_value())
      {
        std::cerr << "Greengrass Core responded with an error: " << error->GetMessage().value().c_str();
        return;
      }
    }
    else
    {
      std::cerr << "Attempting to receive the response from the server failed with error code %s\n"
                << publish_result.GetRpcError().StatusToString().c_str();
      return;
    }
  }
  std::cout << "Function Complete" << std::endl;
  return;
}