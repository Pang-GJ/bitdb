#include <cassert>
#include <cstdint>
#define ANKERL_NANOBENCH_IMPLEMENT
#include <nanobench.h>

#include "bitdb/ds/hashmap.h"
#include "bitdb/ds/skiplist.h"
#include "bitdb/ds/treemap.h"
#include "bitdb/utils/random.h"

int main(int argc, char* argv[]) {
  auto* s = new bitdb::ds::SkipList<int, int>();
  auto* h = new bitdb::ds::HashMap<int, int>();
  auto* t = new bitdb::ds::TreeMap<int, int>();
  bitdb::Random rand;
  const int max_val = 10000000;
  const int num_epochs = 10;

  ankerl::nanobench::Bench().epochs(num_epochs).run("hashmap put", [&] {
    for (int i = 1; i < max_val; ++i) {
      h->emplace(i, i);
    }
  });
  ankerl::nanobench::Bench().epochs(num_epochs).run("hashmap get", [&] {
    for (int i = 1; i < max_val; ++i) {
      auto val = h->at(i);
      if (val != i) {
        printf("hashmap get error");
      }
    }
  });
  ankerl::nanobench::Bench()
      .epochs(num_epochs)
      .run("hashmap get reverse", [&] {
        for (int i = max_val - 1; i >= 1; --i) {
          auto val = h->at(i);
          if (val != i) {
            printf("hashmap get error");
          }
        }
      });
  ankerl::nanobench::Bench().epochs(num_epochs).run("hashmap put random", [&] {
    for (int i = 1; i < max_val; ++i) {
      const auto randval = rand.Uniform(i);
      h->emplace(randval, randval);
    }
  });
  ankerl::nanobench::Bench().epochs(num_epochs).run("hashmap get random", [&] {
    for (int i = 1; i < max_val; ++i) {
      const auto randval = rand.Uniform(i);
      auto val = h->at(randval);
      if (val != randval) {
        printf("hashmap get error");
      }
    }
  });

  ankerl::nanobench::Bench().epochs(num_epochs).run("treemap put", [&] {
    for (int i = 1; i < max_val; ++i) {
      t->emplace(i, i);
    }
  });
  ankerl::nanobench::Bench().epochs(num_epochs).run("treemap get", [&] {
    for (int i = 1; i < max_val; ++i) {
      auto val = t->at(i);
      if (val != i) {
        printf("treemap get error");
      }
    }
  });
  ankerl::nanobench::Bench()
      .epochs(num_epochs)
      .run("treemap get reverse", [&] {
        for (int i = max_val - 1; i >= 1; --i) {
          auto val = t->at(i);
          if (val != i) {
            printf("treemap get error");
          }
        }
      });
  ankerl::nanobench::Bench().epochs(num_epochs).run("treemap put random", [&] {
    for (int i = 1; i < max_val; ++i) {
      const auto randval = rand.Uniform(i);
      t->emplace(randval, randval);
    }
  });
  ankerl::nanobench::Bench().epochs(num_epochs).run("treemap get random", [&] {
    for (int i = 1; i < max_val; ++i) {
      const auto randval = rand.Uniform(i);
      auto val = t->at(randval);
      if (val != randval) {
        printf("treemap get error");
      }
    }
  });

  ankerl::nanobench::Bench().epochs(num_epochs).run("skiplist put", [&] {
    for (int i = 1; i < max_val; ++i) {
      s->Insert(i, i);
    }
  });
  ankerl::nanobench::Bench().epochs(num_epochs).run("skiplist get", [&] {
    for (int i = 1; i < max_val; ++i) {
      auto val = *(s->Find(i));
      if (val != i) {
        printf("skiplist get error");
      }
    }
  });
  ankerl::nanobench::Bench()
      .epochs(num_epochs)
      .run("skiplist get reverse", [&] {
        for (int i = max_val - 1; i >= 1; --i) {
          auto val = *(s->Find(i));
          if (val != i) {
            printf("skiplist get error");
          }
        }
      });
  ankerl::nanobench::Bench()
      .epochs(num_epochs)
      .run("skiplist put random", [&] {
        for (int i = 1; i < max_val; ++i) {
          const auto randval = rand.Uniform(i);
          s->Insert(randval, randval);
        }
      });
  ankerl::nanobench::Bench()
      .epochs(num_epochs)
      .run("skiplist get random", [&] {
        for (int i = 1; i < max_val; ++i) {
          const auto randval = rand.Uniform(i);
          auto val = *(s->Find(randval));
          if (val != randval) {
            printf("skiplist get error");
          }
        }
      });

  return 0;
}