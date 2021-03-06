//
// Created by goforbroke on 25.11.2020.
//

#ifndef ADELANTADO_METRICS_H
#define ADELANTADO_METRICS_H

#include <string>
#include <chrono>
#include <prometheus/family.h>
#include <prometheus/counter.h>

class Metrics {
public:
    static void init(const std::string &serviceName, prometheus::Registry *registry);

    static prometheus::Counter *getDownloadPageCount();

    static prometheus::Counter *getFailedPageCount();

    static prometheus::Gauge *getProcessingPageTotalDuration();

    static prometheus::Gauge *getDownloadPageDuration();

    static prometheus::Gauge *getParsePageDuration();

    static prometheus::Gauge *getRegisterLinkDuration();

    static prometheus::Counter *getRegisterLinkCount();

    static std::chrono::milliseconds now();

    static long long since(std::chrono::milliseconds start);

private:
    static prometheus::Counter *mDownloadPageCount;
    static prometheus::Counter *mFailedPageCount;
    static prometheus::Gauge *mProcessingPageTotalDuration;
    static prometheus::Gauge *mDownloadPageDuration;
    static prometheus::Gauge *mParsePageDuration;

    static prometheus::Gauge *mRegisterLinkDuration;
    static prometheus::Counter *mRegisterLinkCount;
};

#endif //ADELANTADO_METRICS_H
