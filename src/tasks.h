// Copyright 2014 kopsik developers

#ifndef SRC_TASKS_H_
#define SRC_TASKS_H_

#include <string>

#include "Poco/Task.h"
#include "./kopsik_api.h"
#include "./kopsik_api_private.h"
#include "./websocket_client.h"
#include "./https_client.h"
#include "./context.h"

class BaseTask : public Poco::Task {
  public:
    BaseTask(Context *ctx,
             const std::string task_name,
             void *callback)
      : Task(task_name)
      , ctx_(ctx)
      , callback_(callback) {}

  protected:
    Context *context() { return ctx_; }
    void *callback() { return callback_; }
    KopsikResultCallback result_callback() {
      poco_assert(callback());
      return reinterpret_cast<KopsikResultCallback>(callback_);
    }

  private:
    Context *ctx_;
    void *callback_;
};

class SyncTask : public BaseTask {
  public:
    SyncTask(Context *ctx,
      int full_sync,
      KopsikResultCallback callback)
        : BaseTask(ctx, "sync", reinterpret_cast<void *>(callback)),
      full_sync_(full_sync) {}
    void runTask();

   private:
    int full_sync_;
};

class WebSocketStartTask : public BaseTask {
  public:
    WebSocketStartTask(Context *ctx,
                       kopsik::WebSocketMessageCallback websocket_callback)
      : BaseTask(ctx, "start_websocket",
          reinterpret_cast<void *>(websocket_callback)) {}
    void runTask();
};

class WebSocketStopTask : public BaseTask {
  public:
    explicit WebSocketStopTask(Context *ctx)
      : BaseTask(ctx, "stop_websocket", 0) {}
    void runTask();
};

// Start timeline recording on local machine
class TimelineStartTask : public BaseTask {
  public:
    TimelineStartTask(
      Context *ctx,
      KopsikResultCallback callback)
        : BaseTask(ctx, "TimelineStartTask",
            reinterpret_cast<void *>(callback)) {}
    void runTask();
};

// Stop timeline recording on local machine
class TimelineStopTask : public BaseTask {
  public:
    TimelineStopTask(
      Context *ctx,
      KopsikResultCallback callback)
        : BaseTask(ctx, "TimelineStopTask",
            reinterpret_cast<void *>(callback)) {}
    void runTask();
};

// Enable timeline recording on server side
class TimelineUpdateServerSettingsTask : public BaseTask {
  public:
    TimelineUpdateServerSettingsTask(
      Context *ctx,
      KopsikResultCallback callback)
      : BaseTask(ctx, "TimelineUpdateServerSettingsTask",
                 reinterpret_cast<void *>(callback)) {}
    void runTask();
};

// Send feedback to Toggl support
class SendFeedbackTask : public BaseTask {
 public:
  SendFeedbackTask(Context *ctx,
                   std::string subject,
                   std::string details,
                   std::string attachment_path,
                   KopsikResultCallback callback)
  : BaseTask(ctx, "SendFeedbackTask", reinterpret_cast<void *>(callback))
  , subject_(subject)
  , details_(details)
  , attachment_path_(attachment_path) {}

  void runTask();

 private:
  std::string feedbackJSON();
  std::string filename();
  std::string base64encode_attachment();

  std::string subject_;
  std::string details_;
  std::string attachment_path_;
};

// Check if new Toggl-built version is available.
class FetchUpdatesTask : public BaseTask {
  public:
    FetchUpdatesTask(Context *ctx,
                     KopsikCheckUpdateCallback updates_callback,
                     const std::string update_channel)
      : BaseTask(ctx, "check_updates",
          reinterpret_cast<void *>(updates_callback))
      , update_channel_(update_channel) {}
    void runTask();

  private:
    const std::string updateURL();
    const std::string osName();
    std::string update_channel_;
};

#endif  // SRC_TASKS_H_
