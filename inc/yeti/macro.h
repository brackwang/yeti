/**
 * @file macro.h
 * @brief Macro definitions for logging.
 */

// Copyright (c) 2014, Dmitry Senin (seninds@gmail.com)
// All rights reserved.
//
// Redistribution and use in source and binary forms, with or without
// modification, are permitted provided that the following conditions are met:
//
//   1. Redistributions of source code must retain the above copyright notice,
//      this list of conditions and the following disclaimer.
//   2. Redistributions in binary form must reproduce the above copyright
//      notice, this list of conditions and the following disclaimer in the
//      documentation and/or other materials provided with the distribution.
//
// THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS"
// AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE
// IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE
// ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT OWNER OR CONTRIBUTORS BE
// LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR
// CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF
// SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS
// INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN
// CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE)
// ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE
// POSSIBILITY OF SUCH DAMAGE.
//
// yeti - C++ lightweight threadsafe logging
// URL: https://github.com/seninds/yeti.git

#ifndef INC_YETI_MACRO_H_
#define INC_YETI_MACRO_H_

#include <unistd.h>
#include <cstdio>
#include <map>
#include <memory>
#include <sstream>
#include <string>
#include <thread>
#include <yeti/yeti.h>

#if defined(__clang__)
#  pragma clang diagnostic ignored "-Wformat-security"
#elif defined(__GNUC__) || defined(__GNUG__)
#  pragma GCC diagnostic ignored "-Wformat-security"
#endif  // ignored "-Wformat-security"


#define MAX_MSG_LENGTH 512

/// @cond

#define YETI_BALCK   "\033[0;30m"
#define YETI_ORANGE  "\033[0;33m"
#define YETI_BLUE    "\033[0;34m"
#define YETI_GREEN   "\033[0;32m"
#define YETI_CYAN    "\033[0;36m"
#define YETI_RED     "\033[0;31m"
#define YETI_PURPLE  "\033[0;35m"
#define YETI_BROWN   "\033[0;33m"
#define YETI_GRAY    "\033[0;37m"

#define YETI_DGRAY   "\033[1;30m"
#define YETI_LBLUE   "\033[1;34m"
#define YETI_LGREEN  "\033[1;32m"
#define YETI_LCYAN   "\033[1;36m"
#define YETI_LRED    "\033[1;31m"
#define YETI_LPURPLE "\033[1;35m"
#define YETI_YELLOW  "\033[1;33m"
#define YETI_WHITE   "\033[1;37m"

#define YETI_HIGH    "\033[1m"
#define YETI_RESET   "\033[0m"

namespace yeti {

struct LogData {
  std::string log_format;
  std::string level;
  std::string color;
  std::string filename;
  std::string funcname;
  pid_t pid;
  std::thread::id tid;
  int line;
  FILE* fd;
  std::size_t msg_id;
  std::string msg;
  std::chrono::high_resolution_clock::time_point time;
};

// ------------ auxiliary functions ------------
void _EnqueueLogTask(std::shared_ptr<yeti::LogData> log_data);
std::size_t _GetMsgId();
void _IncMsgId();
// ---------------------------------------------

}  // namespace yeti

// @endcond


#ifdef YETI_DISABLE_LOGGING

#define CRT(fmt, ...) ((void) 0)
#define ERR(fmt, ...) ((void) 0)
#define WRN(fmt, ...) ((void) 0)
#define INF(fmt, ...) ((void) 0)
#define DBG(fmt, ...) ((void) 0)
#define TRC(fmt, ...) ((void) 0)

#else  // YETI_DISABLE_LOGGING

/**
 * @brief Logs critical error message using specified printf-like format.
 */
#define CRT(fmt, ...) { \
  auto __yeti_data__ = std::make_shared<yeti::LogData>(); \
  __yeti_data__->level = "CRT"; \
  __yeti_data__->color = YETI_LRED; \
  __yeti_data__->filename = __FILE__; \
  __yeti_data__->funcname = __func__; \
  __yeti_data__->line = __LINE__; \
  __yeti_data__->msg_id = yeti::_GetMsgId(); \
  yeti::_IncMsgId(); \
  \
  char __yeti_msg__[MAX_MSG_LENGTH] = { 0 }; \
  std::snprintf(__yeti_msg__, sizeof(__yeti_msg__), fmt, ##__VA_ARGS__); \
  __yeti_data__->msg = __yeti_msg__; \
  \
  yeti::_EnqueueLogTask(__yeti_data__); \
}

/**
 * @brief Logs error message using specified printf-like format.
 */
#define ERR(fmt, ...) { \
  std::size_t __yeti_msg_id__ = yeti::_GetMsgId(); \
  yeti::_IncMsgId(); \
  if (yeti::GetLogLevel() >= yeti::LOG_LEVEL_ERROR) { \
    auto __yeti_data__ = std::make_shared<yeti::LogData>(); \
    __yeti_data__->level = "ERR"; \
    __yeti_data__->color = YETI_LPURPLE; \
    __yeti_data__->filename = __FILE__; \
    __yeti_data__->funcname = __func__; \
    __yeti_data__->line = __LINE__; \
    __yeti_data__->msg_id = __yeti_msg_id__; \
    \
    char __yeti_msg__[MAX_MSG_LENGTH] = { 0 }; \
    std::snprintf(__yeti_msg__, sizeof(__yeti_msg__), fmt, ##__VA_ARGS__); \
    __yeti_data__->msg = __yeti_msg__; \
    \
    yeti::_EnqueueLogTask(__yeti_data__); \
  } \
}

/**
 * @brief Logs warning message using specified printf-like format.
 */
#define WRN(fmt, ...) { \
  std::size_t __yeti_msg_id__ = yeti::_GetMsgId(); \
  yeti::_IncMsgId(); \
  if (yeti::GetLogLevel() >= yeti::LOG_LEVEL_WARNING) { \
    auto __yeti_data__ = std::make_shared<yeti::LogData>(); \
    __yeti_data__->level = "WRN"; \
    __yeti_data__->color = YETI_YELLOW; \
    __yeti_data__->filename = __FILE__; \
    __yeti_data__->funcname = __func__; \
    __yeti_data__->line = __LINE__; \
    __yeti_data__->msg_id = __yeti_msg_id__; \
    \
    char __yeti_msg__[MAX_MSG_LENGTH] = { 0 }; \
    std::snprintf(__yeti_msg__, sizeof(__yeti_msg__), fmt, ##__VA_ARGS__); \
    __yeti_data__->msg = __yeti_msg__; \
    \
    yeti::_EnqueueLogTask(__yeti_data__); \
  } \
}

/**
 * @brief Logs informational message using specified printf-like format.
 */
#define INF(fmt, ...) { \
  std::size_t __yeti_msg_id__ = yeti::_GetMsgId(); \
  yeti::_IncMsgId(); \
  if (yeti::GetLogLevel() >= yeti::LOG_LEVEL_INFO) { \
    auto __yeti_data__ = std::make_shared<yeti::LogData>(); \
    __yeti_data__->level = "INF"; \
    __yeti_data__->color = YETI_LGREEN; \
    __yeti_data__->filename = __FILE__; \
    __yeti_data__->funcname = __func__; \
    __yeti_data__->line = __LINE__; \
    __yeti_data__->msg_id = __yeti_msg_id__; \
    \
    char __yeti_msg__[MAX_MSG_LENGTH] = { 0 }; \
    std::snprintf(__yeti_msg__, sizeof(__yeti_msg__), fmt, ##__VA_ARGS__); \
    __yeti_data__->msg = __yeti_msg__; \
    \
    yeti::_EnqueueLogTask(__yeti_data__); \
  } \
}

/**
 * @brief Logs debug message using specified printf-like format.
 */
#define DBG(fmt, ...) { \
  std::size_t __yeti_msg_id__ = yeti::_GetMsgId(); \
  yeti::_IncMsgId(); \
  if (yeti::GetLogLevel() >= yeti::LOG_LEVEL_DEBUG) { \
    auto __yeti_data__ = std::make_shared<yeti::LogData>(); \
    __yeti_data__->level = "DBG"; \
    __yeti_data__->color = YETI_WHITE; \
    __yeti_data__->filename = __FILE__; \
    __yeti_data__->funcname = __func__; \
    __yeti_data__->line = __LINE__; \
    __yeti_data__->msg_id = __yeti_msg_id__; \
    \
    char __yeti_msg__[MAX_MSG_LENGTH] = { 0 }; \
    std::snprintf(__yeti_msg__, sizeof(__yeti_msg__), fmt, ##__VA_ARGS__); \
    __yeti_data__->msg = __yeti_msg__; \
    \
    yeti::_EnqueueLogTask(__yeti_data__); \
  } \
}

/**
 * @brief Logs trace message using specified printf-like format.
 */
#define TRC(fmt, ...) { \
  std::size_t __yeti_msg_id__ = yeti::_GetMsgId(); \
  yeti::_IncMsgId(); \
  if (yeti::GetLogLevel() >= yeti::LOG_LEVEL_TRACE) { \
    auto __yeti_data__ = std::make_shared<yeti::LogData>(); \
    __yeti_data__->level = "TRC"; \
    __yeti_data__->color = ""; \
    __yeti_data__->filename = __FILE__; \
    __yeti_data__->funcname = __func__; \
    __yeti_data__->line = __LINE__; \
    __yeti_data__->msg_id = __yeti_msg_id__; \
    \
    char __yeti_msg__[MAX_MSG_LENGTH] = { 0 }; \
    std::snprintf(__yeti_msg__, sizeof(__yeti_msg__), fmt, ##__VA_ARGS__); \
    __yeti_data__->msg = __yeti_msg__; \
    \
    yeti::_EnqueueLogTask(__yeti_data__); \
  } \
}

#endif  // YETI_DISABLE_LOGGING

#define CRITICAL(fmt, ...) CRT(fmt, ##__VA_ARGS__)
#define CRIT(fmt, ...)     CRT(fmt, ##__VA_ARGS__)
#define ERROR(fmt, ...)    ERR(fmt, ##__VA_ARGS__)
#define WARN(fmt, ...)     WRN(fmt, ##__VA_ARGS__)
#define WARNING(fmt, ...)  WRN(fmt, ##__VA_ARGS__)
#define INFO(fmt, ...)     INF(fmt, ##__VA_ARGS__)
#define DEBUG(fmt, ...)    DBG(fmt, ##__VA_ARGS__)
#define TRACE(fmt, ...)    TRC(fmt, ##__VA_ARGS__)

#endif  // INC_YETI_MACRO_H_
