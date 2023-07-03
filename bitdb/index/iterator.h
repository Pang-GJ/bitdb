#pragma once

#include <string>
#include "bitdb/data/log_record.h"
#include "bitdb/utils/bytes.h"
namespace bitdb::index {

class Iterator {
 public:
  virtual ~Iterator() = default;

  virtual bool Valid() const = 0;
  virtual std::string Key() const = 0;
  virtual data::LogRecordPst* Value() const = 0;
  virtual void Next() = 0;
  virtual void Prev() = 0;
  virtual void Seek(const Bytes& target) = 0;
  virtual void SeekToFirst() = 0;
  virtual void SeekToLast() = 0;
};

}  // namespace bitdb::index