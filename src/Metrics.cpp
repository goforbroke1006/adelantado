//
// Created by goforbroke on 25.11.2020.
//

#include "Metrics.h"

prometheus::Counter *Metrics::mDownloadPageCount = nullptr;
prometheus::Counter *Metrics::mFailedPageCount = nullptr;
prometheus::Gauge *Metrics::mProcessingPageTotalDuration = nullptr;
prometheus::Gauge *Metrics::mDownloadPageDuration = nullptr;
prometheus::Gauge *Metrics::mParsePageDuration = nullptr;

prometheus::Gauge *Metrics::mRegisterLinkDuration = nullptr;
prometheus::Counter *Metrics::mRegisterLinkCount = nullptr;

void Metrics::init(const std::string &serviceName, prometheus::Registry *registry) {
    std::string prefix = serviceName + "_";

    mDownloadPageCount = &prometheus::BuildCounter()
            .Name(prefix + "download_page_count")
            .Help("How many page were downloaded")
//                .Labels({{"label", "value"}})
            .Register(*registry).Add({});

    mFailedPageCount = &prometheus::BuildCounter()
            .Name(prefix + "failed_page_count")
            .Help("How many page were failed")
//                .Labels({{"label", "value"}})
            .Register(*registry).Add({});

    mProcessingPageTotalDuration = &prometheus::BuildGauge()
            .Name(prefix + "processing_page_total_duration")
            .Help("How many time spend to download page")
//                .Labels({{"label", "value"}})
            .Register(*registry).Add({});

    mDownloadPageDuration = &prometheus::BuildGauge()
            .Name(prefix + "download_page_duration")
            .Help("How many time spend to download page")
//                .Labels({{"label", "value"}})
            .Register(*registry).Add({});

    mParsePageDuration = &prometheus::BuildGauge()
            .Name(prefix + "parse_page_duration")
            .Help("How many time spend to parse page")
//                .Labels({{"label", "value"}})
            .Register(*registry).Add({});

    mRegisterLinkDuration = &prometheus::BuildGauge()
            .Name(prefix + "register_link_duration")
            .Help("How many time spend to register new link")
            .Register(*registry).Add({});
    mRegisterLinkCount = &prometheus::BuildCounter()
            .Name(prefix + "register_link_count")
            .Help("How many links were registered")
            .Register(*registry).Add({});
}

prometheus::Counter *Metrics::getDownloadPageCount() { return mDownloadPageCount; }

prometheus::Counter *Metrics::getFailedPageCount() { return mFailedPageCount; }

prometheus::Gauge *Metrics::getProcessingPageTotalDuration() { return mProcessingPageTotalDuration; }

prometheus::Gauge *Metrics::getDownloadPageDuration() { return mDownloadPageDuration; }

prometheus::Gauge *Metrics::getParsePageDuration() { return mParsePageDuration; }

prometheus::Gauge *Metrics::getRegisterLinkDuration() { return mRegisterLinkDuration; }

prometheus::Counter *Metrics::getRegisterLinkCount() { return mRegisterLinkCount; }

std::chrono::milliseconds Metrics::now() {
    return std::chrono::duration_cast<std::chrono::milliseconds>(
            std::chrono::system_clock::now().time_since_epoch()
    );
}

long long Metrics::since(std::chrono::milliseconds start) {
    return (now() - start).count();
}
