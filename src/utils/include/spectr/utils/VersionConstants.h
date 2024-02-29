#pragma once

namespace spectr::utils {
#define SPECTR_VERSION_MAJOR 0
#define SPECTR_VERSION_MINOR 0
#define SPECTR_VERSION_PATCH 1
#define BUILD_COMMIT_HASH "10000"

#define STR(x) #x
#define STR_EVAL(x) STR(x)

#define VERSION(major, minor, patch) major ## . ## minor ## . ## patch
#define VERSION_EVAL(x,y,z) VERSION(x,y,z)

#define SPECTR_VERSION STR_EVAL(VERSION_EVAL(SPECTR_VERSION_MAJOR, SPECTR_VERSION_MINOR, SPECTR_VERSION_PATCH))

}
