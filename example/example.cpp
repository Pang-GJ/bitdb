#include "bitdb/db.h"
#include "bitdb/options.h"
#include "bitdb/common/logger.h"

int main(int argc, char* argv[]) {
  auto options = bitdb::DefaultOptions();
  options.dir_path = "/tmp/bitdb";
  bitdb::DB* db = nullptr;
  auto status = bitdb::DB::Open(options, &db);
  if (!status.IsOk()) {
    LOG_ERROR("Open db failed. reason: {}", status.ToString());
    exit(-1);
  }
  status = db->Put("name", "bitdb-example");
  if (!status.IsOk()) {
    LOG_ERROR("DB put failed. reason: {}", status.ToString());
    exit(-1);
  }

  std::string value;
  status = db->Get("name", &value);
  if (!status.IsOk()) {
    LOG_ERROR("DB get failed. reason: {}", status.ToString());
    exit(-1);
  }
  LOG_INFO("value: {}", value);

  status = db->Delete("name");
  if (!status.IsOk()) {
    LOG_ERROR("DB delete failed. reason: {}", status.ToString());
    exit(-1);
  }
  status = db->Get("name", &value);
  if (!status.IsNotFound()) {
    LOG_ERROR("DB item still can be founded after delete. reason: {}",
              status.ToString());
    exit(-1);
  }
}