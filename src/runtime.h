//
// Created by goforbroke on 10.11.2020.
//

#ifndef ADELANTADO_RUNTIME_H
#define ADELANTADO_RUNTIME_H

#include <thread>
#include <unistd.h>

unsigned int getCPUCount() {
//    return sysconf(_SC_NPROCESSORS_ONLN);
    return std::thread::hardware_concurrency();
}

#endif //ADELANTADO_RUNTIME_H
