/*
 * Copyright (c) 2021,2025 BlackBerry Limited.
 *
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 *     http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 */

#ifndef _LOG_H_INCLUDED
#define _LOG_H_INCLUDED

#include <sys/slog2.h>


#define SMBUS_LOG(_level_, _format_, args...) \
    do { \
        const char *const prefix = "%s[%u]: "; \
        const size_t fsize = min(strlen(_format_) + strlen(prefix) + 1, 1024); \
        char *const fmt = alloca(fsize); \
        strlcpy(fmt, prefix, fsize); \
        strlcat(fmt, (_format_), fsize); \
        smbus_log((_level_), fmt, __FUNCTION__, __LINE__, ##args); \
    } while(0)

#define SMBUS_LOG_CRITICAL(_format_, args...)   do { SMBUS_LOG(SLOG2_CRITICAL, (_format_), ##args); } while(0)
#define SMBUS_LOG_ERROR(_format_, args...)      do { SMBUS_LOG(SLOG2_ERROR, (_format_), ##args); } while(0)
#define SMBUS_LOG_WARNING(_format_, args...)    do { SMBUS_LOG(SLOG2_WARNING, (_format_), ##args); } while(0)
#define SMBUS_LOG_INFO(_format_, args...)       do { SMBUS_LOG(SLOG2_INFO, (_format_), ##args); } while(0)
#define SMBUS_LOG_DEBUG(_format_, args...)      do { SMBUS_LOG(SLOG2_DEBUG1, (_format_), ##args); } while(0)

extern char *__progname;

void smbus_log_init(int *verbosity);
void smbus_log(const uint8_t severity, const char *const fmt, ...) __attribute__((__format__(__printf__, 2, 3)));

#endif /*_LOG_H_INCLUDED*/

