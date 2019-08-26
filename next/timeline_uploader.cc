// Copyright 2014 Toggl Desktop developers.

#include "timeline_uploader.h"

#include <sstream>
#include <string>

#include "./formatter.h"
#include "./https_client.h"
#include "./urls.h"

#include "Poco/Foundation.h"
#include "Poco/Thread.h"
#include "Poco/Util/Application.h"

#include <json/json.h>  // NOLINT

namespace toggl {

Poco::Logger &TimelineUploader::logger() const {
    return Poco::Logger::get("timeline_uploader");
}

void TimelineUploader::execute() {
    {
        std::stringstream out;
        out << "upload_loop_activity (current interval "
            << current_upload_interval_seconds_ << "s)";
        logger().debug(out.str());
    }

    TimelineBatch batch;
    error err = timeline_datasource_->CreateCompressedTimelineBatchForUpload(&batch);

    if (err != noError) {
        logger().warning(err);
        return;
    }

    if (!batch.Events().size()) {
        return;
    }

    err = upload(&batch);
    if (err != noError) {
        backoff();
        logger().warning(err);
        return;
    }

    {
        std::stringstream out;
        out << "Sync of " << batch.Events().size()
            << " event(s) was successful.";
        logger().debug(out.str());
    }

    reset_backoff();

    err = timeline_datasource_->MarkTimelineBatchAsUploaded(batch.Events());
    if (err != noError) {
        logger().warning(err);
    }

    schedule(current_upload_interval_seconds_);
}

error TimelineUploader::upload(TimelineBatch *batch) {
    TogglClient client;

    std::stringstream ss;
    ss << "Uploading " << batch->Events().size()
       << " event(s) of user " << batch->UserID();
    logger().debug(ss.str());

    std::string json = convertTimelineToJSON(
        batch->Events(),
        batch->DesktopID());
    logger().trace(json);

    // Not implemented in v9 as of 12.05.2017
    HTTPSRequest req;
    req.host = urls::TimelineUpload();
    req.relative_url = "/api/v8/timeline";
    req.payload = json;
    req.basic_auth_username = batch->APIToken();
    req.basic_auth_password = "api_token";

    return client.Post(req).err;
}

std::string convertTimelineToJSON(
    const std::vector<TimelineEvent> &timeline_events,
    const std::string &desktop_id) {

    Json::Value root;

    for (std::vector<TimelineEvent>::const_iterator i = timeline_events.begin();
            i != timeline_events.end();
            ++i) {
        TimelineEvent event = *i;
        Json::Value n = event.SaveToJSON();
        n["desktop_id"] = desktop_id;
        root.append(n);
    }

    Json::StyledWriter writer;
    return writer.write(root);
}

void TimelineUploader::backoff() {
    logger().warning("backoff");
    current_upload_interval_seconds_ *= 2;
    if (current_upload_interval_seconds_ > kTimelineUploadMaxBackoffSeconds) {
        logger().warning("Max upload interval reached.");
        current_upload_interval_seconds_ = kTimelineUploadMaxBackoffSeconds;
    }
    std::stringstream out;
    out << "Upload interval set to " << current_upload_interval_seconds_ << "s";
    logger().debug(out.str());
}

void TimelineUploader::reset_backoff() {
    logger().debug("reset_backoff");
    current_upload_interval_seconds_ = kTimelineUploadIntervalSeconds;
}

error TimelineUploader::start() {
    schedule(0);
}

error TimelineUploader::Shutdown() {
    unschedule();
}

}  // namespace toggl