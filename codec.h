/*
 * Copyright (C) 2017 Rockchip Electronics Co., Ltd.
 * author: Hertz Wang wangh@rock-chips.com
 *
 * SPDX-License-Identifier: LGPL-2.1-or-later
 */

#ifndef RKMEDIA_CODEC_H_
#define RKMEDIA_CODEC_H_

#include <string.h>

#include <condition_variable>
#include <list>
#include <memory>
#include <mutex>
#include <string>
#include <thread>

#include "media_config.h"

namespace rkmedia {

class MediaBuffer;
class Codec {
public:
  Codec() : extra_data(nullptr), extra_data_size(0) {
    memset(&config, 0, sizeof(config));
  }
  virtual ~Codec() = 0;
  static const char *GetCodecName() { return nullptr; }
  MediaConfig &GetConfig() { return config; }
  void SetConfig(const MediaConfig &cfg) { config = cfg; }
  bool SetExtraData(void *data, size_t size, bool realloc = true);
  void GetExtraData(void *&data, size_t &size) {
    data = extra_data;
    size = extra_data_size;
  }

  virtual bool Init() = 0;
  // sync call
  virtual int Process(std::shared_ptr<MediaBuffer> input,
                      std::shared_ptr<MediaBuffer> output,
                      std::shared_ptr<MediaBuffer> extra_output) = 0;
  virtual std::shared_ptr<MediaBuffer> GenEmptyOutPutBuffer();

private:
  MediaConfig config;
  void *extra_data;
  size_t extra_data_size;
};

class ThreadCodec : public Codec {
public:
  ThreadCodec(const char *thread_name_prefix = NULL) : quit(false) {
    if (thread_name_prefix)
      th_name_prefix = thread_name_prefix;
    else
      th_name_prefix = "";
  }
  virtual ~ThreadCodec();
  virtual bool Init();
  virtual int Process(std::shared_ptr<MediaBuffer> input,
                      std::shared_ptr<MediaBuffer> output,
                      std::shared_ptr<MediaBuffer> extra_output) final;
  bool IsQuit() { return quit; }
  void SetQuit(bool q) { quit = q; }
  virtual bool SendInput(std::shared_ptr<MediaBuffer> input);
  virtual std::shared_ptr<MediaBuffer> GetOutPut(bool wait = false);
  virtual std::shared_ptr<MediaBuffer> GetExtraPut();

  virtual int ProcessInput(std::shared_ptr<MediaBuffer> input) = 0;
  virtual int ProcessOutput(std::shared_ptr<MediaBuffer> output,
                            std::shared_ptr<MediaBuffer> extra_output) = 0;
  virtual void InputRun();
  virtual void OutputRun();

protected:
  std::shared_ptr<MediaBuffer>
  GetElement(std::list<std::shared_ptr<MediaBuffer>> &list, std::mutex &mtx,
             std::condition_variable &cond, bool wait);

private:
  volatile bool quit;
  std::string th_name_prefix;
  std::mutex input_mtx;
  std::condition_variable input_cond;
  std::list<std::shared_ptr<MediaBuffer>> input_list;
  std::thread *input_th;
  std::mutex output_mtx;
  std::condition_variable output_cond;
  std::list<std::shared_ptr<MediaBuffer>> output_list;
  std::list<std::shared_ptr<MediaBuffer>> extra_output_list;
  std::thread *output_th;
};

inline void msleep(int ms) {
  std::this_thread::sleep_for(std::chrono::milliseconds(ms));
}

} // namespace rkmedia

#endif // #ifndef RKMEDIA_CODEC_H_
