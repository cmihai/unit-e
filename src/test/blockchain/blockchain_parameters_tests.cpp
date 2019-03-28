// Copyright (c) 2018-2019 The Unit-e developers
// Distributed under the MIT software license, see the accompanying
// file COPYING or http://www.opensource.org/licenses/mit-license.php.

#include <blockchain/blockchain_parameters.h>
#include <test/test_unite.h>
#include <boost/test/unit_test.hpp>

using namespace blockchain;

BOOST_FIXTURE_TEST_SUITE(blockchain_parameters_tests, ReducedTestingSetup)

BOOST_AUTO_TEST_CASE(total_supply_test) {

  Parameters params = blockchain::Parameters::TestNet();

  constexpr CAmount initial_supply = 150000000000000000;
  constexpr CAmount max_supply = initial_supply + (3750000000 + 1700000000 + 550000000 + 150000000 + 31000000) * 1971000 * 10;
  BOOST_CHECK_EQUAL(max_supply, params.maximum_supply);

  BOOST_CHECK(MoneyRange(max_supply));
  BOOST_CHECK(!MoneyRange(max_supply + 1));

  constexpr CAmount theoretic_supply = 2718281828 * UNIT;  // e billion tokens
  constexpr CAmount expected_delta = 6728 * UNIT;
  BOOST_CHECK_EQUAL(params.maximum_supply, theoretic_supply - expected_delta);
}

BOOST_AUTO_TEST_CASE(reward_function_test) {

  Parameters params = blockchain::Parameters::TestNet();

  CAmount block_reward = params.reward_function(params, 0);
  BOOST_CHECK_EQUAL(375000000, block_reward);

  BOOST_CHECK_EQUAL(375000000, block_reward);

  block_reward = params.reward_function(params, params.period_blocks - 1);
  BOOST_CHECK_EQUAL(375000000, block_reward);

  block_reward = params.reward_function(params, params.period_blocks);
  BOOST_CHECK_EQUAL(170000000, block_reward);

  block_reward = params.reward_function(params, params.period_blocks * 2);
  BOOST_CHECK_EQUAL(55000000, block_reward);

  block_reward = params.reward_function(params, params.period_blocks * 3);
  BOOST_CHECK_EQUAL(15000000, block_reward);

  block_reward = params.reward_function(params, params.period_blocks * 4);
  BOOST_CHECK_EQUAL(3100000, block_reward);

  block_reward = params.reward_function(params, params.period_blocks * 5);
  BOOST_CHECK_EQUAL(0, block_reward);

  block_reward = params.reward_function(params, params.period_blocks * 500);
  BOOST_CHECK_EQUAL(0, block_reward);
}

BOOST_AUTO_TEST_SUITE_END()