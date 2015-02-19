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

#include <yeti/yeti.h>

#include <csignal>
#include <cstdio>
#include <ctime>

#include <iostream>

#include <map>
#include <mutex>
#include <string>
#include <src/logger.h>

namespace yeti {

const std::map<int, std::string> SIGNAME = {
    { SIGABRT, "SIGABRT" },
    { SIGFPE, "SIGFPE" },
    { SIGILL, "SIGILL" },
    { SIGINT, "SIGINT" },
    { SIGSEGV, "SIGSEGV" },
    { SIGTERM, "SIGTERM" }
};

typedef void (*__sighandler_t)(int);

std::map<int, __sighandler_t> g_old_handlers = {
    { SIGABRT, nullptr },
    { SIGFPE, nullptr },
    { SIGILL, nullptr },
    { SIGINT, nullptr },
    { SIGSEGV, nullptr },
    { SIGTERM, nullptr }
};


void SetLogLevel(LogLevel level) noexcept {
  Logger::instance().SetLevel(level);
}

int GetLogLevel() noexcept {
  return Logger::instance().GetLevel();
}

void SetLogColored(bool is_colored) noexcept {
  Logger::instance().SetColored(is_colored);
}

bool IsLogColored() noexcept {
  return Logger::instance().IsColored();
}

void SetLogFileDesc(FILE* fd) noexcept {
  Logger::instance().SetFileDesc(fd);
}

FILE* GetLogFileDesc() noexcept {
  return Logger::instance().GetFileDesc();
}

void CloseLogFileDesc(FILE* fd) {
  Logger::instance().CloseFileDesc(fd);
}

void SetLogFormatStr(const std::string& format_str) noexcept {
  Logger::instance().SetFormatStr(format_str);
}

std::string GetLogFormatStr() noexcept {
  return Logger::instance().GetFormatStr();
}

void ShutdownLog() {
  yeti::Logger::instance().Shutdown();
}

void SimpleSignalHandler(int sig_num) {
  DEBUG("caught %s: start flushing log...\n", SIGNAME.at(sig_num).c_str());
  yeti::Logger::instance().Flush();
  if (g_old_handlers[sig_num]) g_old_handlers[sig_num](sig_num);
}

void RegSignal(int sig_num) {
  // logger should be instantiated before signals will be registered
  auto old_handler = signal(sig_num, SimpleSignalHandler);
  if (old_handler) g_old_handlers[sig_num] = old_handler;
}

void RegAllSignals() {
  for (const auto& entry : SIGNAME) {
    RegSignal(entry.first);
  }
}

void FlushLog() {
  yeti::Logger::instance().Flush();
}

std::size_t _GetMsgId() {
  return yeti::Logger::instance().GetMsgId();
}

void _IncMsgId() {
  yeti::Logger::instance().IncMsgId();
}

std::string _CreateLogStr(std::shared_ptr<const yeti::LogData> log_data) {
  std::map<std::string, std::string> subs;
  subs["%(LEVEL)"] = log_data->level;
  subs["%(FILENAME)"] = log_data->filename;
  subs["%(FUNCNAME)"] = log_data->funcname;
  subs["%(MSG)"] = log_data->msg;

  if (log_data->log_format.find("%(PID)") != std::string::npos) {
    subs["%(PID)"] = std::to_string(log_data->pid);
  }

  if (log_data->log_format.find("%(TID)") != std::string::npos) {
    std::hash<std::thread::id> hash_fn;
    std::ostringstream oss;
    oss << std::hex << std::uppercase << hash_fn(log_data->tid);
    subs["%(TID)"] = oss.str();
  }

  if (log_data->log_format.find("%(DATE)") != std::string::npos) {
    using namespace std::chrono;
    char date_buf[16] = { 0 };
    auto sec = duration_cast<seconds>(log_data->time.time_since_epoch());
    std::time_t t = sec.count();
    std::strftime(date_buf, sizeof(date_buf), "%F", std::localtime(&t));
    subs["%(DATE)"] = date_buf;
  }

  if (log_data->log_format.find("%(TIME)") != std::string::npos) {
    using namespace std::chrono;
    char time_buf[32] = { 0 };
    auto nanos = duration_cast<nanoseconds>(log_data->time.time_since_epoch());
    auto sec = duration_cast<seconds>(log_data->time.time_since_epoch());
    std::time_t t = sec.count();
    std::size_t frac = nanos.count() % 1000000000;
    std::strftime(time_buf, sizeof(time_buf), "%T", std::localtime(&t));
    std::string time_str = std::string(time_buf) + "." + std::to_string(frac);
    subs["%(TIME)"] = time_str;
  }

  if (log_data->log_format.find("%(LINE)") != std::string::npos) {
    subs["%(LINE)"] = std::to_string(log_data->line);
  }

  if (log_data->log_format.find("%(MSG_ID)") != std::string::npos) {
    subs["%(MSG_ID)"] = std::to_string(log_data->msg_id);
  }

  std::string result = log_data->log_format;
  size_t pos = 0;
  for (const auto& entry : subs) {
    while ((pos = result.find(entry.first)) != std::string::npos) {
      result.replace(pos, entry.first.length(), entry.second);
    }
  }
  return result;
}

void _EnqueueLogTask(std::shared_ptr<LogData> log_data) {
  log_data->log_format = yeti::Logger::instance().GetFormatStr();
  log_data->time = std::chrono::high_resolution_clock::now();
  log_data->pid = getpid();
  log_data->tid = std::this_thread::get_id();
  log_data->fd = yeti::Logger::instance().GetFileDesc();
  bool is_colored = yeti::Logger::instance().IsColored();

  auto print_func = [log_data, is_colored] {
    std::string log_str = _CreateLogStr(log_data) + "\n";

// To colorize stdout and stderr in Windows cmd.exe it is necessary
// to include windows.h and use SetConsoleTextAttribute().
// It is terrible, so I decided to disable coloring on WIN32 platform.
#ifndef _WIN32
    if (isatty(fileno(log_data->fd)) != 0 && is_colored) {
      log_str = log_data->color + log_str + std::string(YETI_RESET);
    }
#endif  // _WIN32

    std::fprintf(log_data->fd, log_str.c_str());
  };

  yeti::Logger::instance().EnqueueTask(print_func);
}

}  // namespace yeti
