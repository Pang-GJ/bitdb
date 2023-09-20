#define DOCTEST_CONFIG_IMPLEMENT_WITH_MAIN
#include <doctest/doctest.h>
#include "tests/raft/raft_test_utils.h"

using ListRaftGroup = RaftGroup<ListStateMachine, ListCommand>;

TEST_CASE("Initial election") {
  int num_nodes = 3;
  ListRaftGroup* group = new ListRaftGroup(num_nodes);

  // 1. check whether there is exact one leader in the group
  mssleep(300);  // sleep 300 ms
  group->check_exact_one_leader();

  // 2. check whether every one has the same term
  mssleep(2000);  // sleep 2s
  int term1 = group->check_same_term();

  // 3. wait for a few seconds and check whether the term is not changed.
  mssleep(2000);  // sleep 2s
  int term2 = group->check_same_term();

  CHECK_MESSAGE(term1 == term2,
                "inconsistent term: " << term1 << ", " << term2);
  group->check_exact_one_leader();
  delete group;
}

TEST_CASE("Election after network failure") {
  int num_nodes = 5;
  ListRaftGroup* group = new ListRaftGroup(num_nodes);

  // 1. check whether there is exact one leader in the group
  mssleep(300);  // sleep 300ms
  int leader1 = group->check_exact_one_leader();

  // 2. kill the leader
  group->disable_node(leader1);
  mssleep(1000);  // sleep 1s
  int leader2 = group->check_exact_one_leader();

  CHECK_MESSAGE(
      leader1 != leader2,
      "node " << leader2 << "is killed, which should not be the new leader!");

  // 3. kill the second leader
  group->disable_node(leader2);
  mssleep(1000);  // sleep 1s
  int leader3 = group->check_exact_one_leader();
  bool check_expr = leader3 != leader1 && leader3 != leader2;
  CHECK_MESSAGE(
      check_expr,
      "node " << leader3 << "is killed, which should not be the new leader!");

  // 4. kill the third leader
  group->disable_node(leader3);
  mssleep(1000);  // sleep 1s

  // 5. now, there are only 2 nodes left with no leader.
  group->check_no_leader();

  // 6. resume a node
  group->enable_node(leader1);
  mssleep(1000);
  group->check_exact_one_leader();

  delete group;
}
