// Copyright (c) 2019 The Unit-e developers
// Distributed under the MIT software license, see the accompanying
// file COPYING or http://www.opensource.org/licenses/mit-license.php.

#ifndef UNIT_E_TEST_UNITE_MOCKS_H
#define UNIT_E_TEST_UNITE_MOCKS_H

#include <blockdb.h>
#include <coins.h>
#include <finalization/state_db.h>
#include <staking/active_chain.h>
#include <staking/block_index_map.h>
#include <staking/network.h>
#include <staking/stake_validator.h>

#include <atomic>
#include <cstdint>
#include <functional>

namespace mocks {

class NetworkMock : public staking::Network {
 public:
  mutable std::atomic<std::uint32_t> invocations_GetTime;
  mutable std::atomic<std::uint32_t> invocations_GetNodeCount;
  mutable std::atomic<std::uint32_t> invocations_GetInboundNodeCount;
  mutable std::atomic<std::uint32_t> invocations_GetOutboundNodeCount;

  std::int64_t time = 0;
  size_t node_count = 0;
  size_t inbound_node_count = 0;
  size_t outbound_node_count = 0;

  int64_t GetTime() const override {
    ++invocations_GetTime;
    return time;
  }
  size_t GetNodeCount() const override {
    ++invocations_GetNodeCount;
    return node_count;
  }
  size_t GetInboundNodeCount() const override {
    ++invocations_GetInboundNodeCount;
    return inbound_node_count;
  }
  size_t GetOutboundNodeCount() const override {
    ++invocations_GetOutboundNodeCount;
    return outbound_node_count;
  }
};

class BlockIndexMapMock : public staking::BlockIndexMap {
public:
  bool reverse = false;

  CCriticalSection &GetLock() const override { return cs; }
  CBlockIndex *Insert(const uint256 &block_hash) {
    const auto result = indexes.emplace(block_hash, new CBlockIndex());
    CBlockIndex *index = result.first->second;
    const uint256 &hash = result.first->first;
    if (!result.second) {
      return index;
    }
    index->phashBlock = &hash;
    return index;
  }
  CBlockIndex *Lookup(const uint256 &block_hash) const override {
    const auto it = indexes.find(block_hash);
    if (it == indexes.end()) {
      return nullptr;
    }
    return it->second;
  }
  void ForEach(std::function<bool(const uint256 &, const CBlockIndex &)> &&f) const override {
    if (!reverse) {
      for (auto it = indexes.begin(); it != indexes.end(); ++it) {
        if (!f(it->first, *it->second)) {
          return;
        }
      }
    } else {
      for (auto it = indexes.rbegin(); it != indexes.rend(); ++it) {
        if (!f(it->first, *it->second)) {
          return;
        }
      }
    }
  }
  ~BlockIndexMapMock() {
    for (auto &i : indexes) {
      delete i.second;
    }
  }
private:
  mutable CCriticalSection cs;
  std::map<uint256, CBlockIndex*> indexes;
};

class ActiveChainMock : public staking::ActiveChain {
  mutable CCriticalSection lock;

 public:
  mutable std::atomic<std::uint32_t> invocations_GetLock{0};
  mutable std::atomic<std::uint32_t> invocations_GetSize{0};
  mutable std::atomic<std::uint32_t> invocations_GetHeight{0};
  mutable std::atomic<std::uint32_t> invocations_GetTip{0};
  mutable std::atomic<std::uint32_t> invocations_GetGenesis{0};
  mutable std::atomic<std::uint32_t> invocations_Contains{0};
  mutable std::atomic<std::uint32_t> invocations_FindForkOrigin{0};
  mutable std::atomic<std::uint32_t> invocations_GetNext{0};
  mutable std::atomic<std::uint32_t> invocations_AtDepth{0};
  mutable std::atomic<std::uint32_t> invocations_AtHeight{0};
  mutable std::atomic<std::uint32_t> invocations_GetDepth{0};
  mutable std::atomic<std::uint32_t> invocations_GetBlockIndex{0};
  mutable std::atomic<std::uint32_t> invocations_ComputeSnapshotHash{0};
  mutable std::atomic<std::uint32_t> invocations_ProcessNewBlock{0};
  mutable std::atomic<std::uint32_t> invocations_GetUTXO{0};
  mutable std::atomic<std::uint32_t> invocations_GetInitialBlockDownloadStatus{0};

  //! The tip to be returned by GetTip()
  CBlockIndex *tip = nullptr;

  //! The genesis to be returned by GetGenesis()
  CBlockIndex *genesis = nullptr;

  //! The sync states to be returned by GetIBDStatus()
  ::SyncStatus sync_status = SyncStatus::SYNCED;

  //! The height to be returned by GetHeight() (GetSize = GetHeight + 1)
  blockchain::Height height = 0;

  //! The snapshot hash to be returned by ComputeSnapshotHash()
  uint256 snapshot_hash = uint256();

  //! Function to retrieve the block at the given depth
  std::function<CBlockIndex *(blockchain::Depth)> block_at_depth = [](blockchain::Depth) {
    return nullptr;
  };

  //! Function to retrieve the block at the given height
  std::function<CBlockIndex *(blockchain::Height)> block_at_height = [](blockchain::Height) {
    return nullptr;
  };

  //! Function to retrieve the block at the given index
  std::function<CBlockIndex *(const uint256 &)> get_block_index = [](const uint256 &) {
    return nullptr;
  };

  //! Function to retrieve the block at the given index
  std::function<boost::optional<staking::Coin>(const COutPoint &)> get_utxo = [](const COutPoint &) {
    return boost::none;
  };

  CCriticalSection &GetLock() const override {
    ++invocations_GetLock;
    return lock;
  }
  blockchain::Height GetSize() const override {
    ++invocations_GetSize;
    return height + 1;
  }
  blockchain::Height GetHeight() const override {
    ++invocations_GetHeight;
    return height;
  }
  const CBlockIndex *GetTip() const override {
    ++invocations_GetTip;
    return tip;
  }
  const CBlockIndex *GetGenesis() const override {
    ++invocations_GetGenesis;
    return genesis;
  }
  bool Contains(const CBlockIndex &block_index) const override {
    ++invocations_Contains;
    return block_at_height(block_index.nHeight) == &block_index;
  }
  const CBlockIndex *FindForkOrigin(const CBlockIndex &block_index) const override {
    ++invocations_FindForkOrigin;
    const CBlockIndex *walk = &block_index;
    while (walk != nullptr && block_at_height(walk->nHeight) != walk) {
      walk = walk->pprev;
    }
    return walk;
  }
  const CBlockIndex *GetNext(const CBlockIndex &block_index) const override {
    ++invocations_GetNext;
    if (block_at_height(block_index.nHeight) == &block_index) {
      return block_at_height(block_index.nHeight + 1);
    }
    return nullptr;
  }
  const CBlockIndex *AtDepth(blockchain::Depth depth) const override {
    ++invocations_AtDepth;
    return block_at_depth(depth);
  }
  const CBlockIndex *AtHeight(blockchain::Height height) const override {
    ++invocations_AtHeight;
    return block_at_height(height);
  }
  blockchain::Depth GetDepth(const blockchain::Height height) const override {
    ++invocations_GetDepth;
    return GetHeight() - height + 1;
  }
  const CBlockIndex *GetBlockIndex(const uint256 &hash) const override {
    ++invocations_GetBlockIndex;
    return get_block_index(hash);
  }
  const uint256 ComputeSnapshotHash() const override {
    ++invocations_ComputeSnapshotHash;
    return snapshot_hash;
  }
  bool ProcessNewBlock(std::shared_ptr<const CBlock> pblock) override {
    ++invocations_ProcessNewBlock;
    return false;
  }
  boost::optional<staking::Coin> GetUTXO(const COutPoint &outpoint) const override {
    ++invocations_GetUTXO;
    return get_utxo(outpoint);
  }
  ::SyncStatus GetInitialBlockDownloadStatus() const override {
    ++invocations_GetInitialBlockDownloadStatus;
    return sync_status;
  }
};

class StakeValidatorMock : public staking::StakeValidator {
  mutable CCriticalSection lock;

 public:
  std::function<bool(uint256)> checkkernelfunc =
      [](uint256 kernel) { return false; };
  std::function<uint256(const CBlockIndex *, const staking::Coin &, blockchain::Time)> computekernelfunc =
      [](const CBlockIndex *, const staking::Coin &, blockchain::Time) { return uint256(); };

  CCriticalSection &GetLock() override {
    return lock;
  }
  bool CheckKernel(CAmount, const uint256 &kernel, blockchain::Difficulty) const override {
    return checkkernelfunc(kernel);
  }
  uint256 ComputeKernelHash(const CBlockIndex *blockindex, const staking::Coin &coin, blockchain::Time time) const override {
    return computekernelfunc(blockindex, coin, time);
  }
  staking::BlockValidationResult CheckStake(const CBlock &, staking::BlockValidationInfo *) const override {
    return staking::BlockValidationResult();
  }
  uint256 ComputeStakeModifier(const CBlockIndex *, const staking::Coin &) const override { return uint256(); }
  bool IsPieceOfStakeKnown(const COutPoint &) const override { return false; }
  void RememberPieceOfStake(const COutPoint &) override {}
  void ForgetPieceOfStake(const COutPoint &) override {}
};

class CoinsViewMock : public AccessibleCoinsView {

 public:
  Coin default_coin;
  bool default_have_inputs = true;

  mutable std::atomic<std::uint32_t> invocations_AccessCoins{0};
  mutable std::atomic<std::uint32_t> invocations_HaveInputs{0};

  mutable std::function<const Coin &(const COutPoint &)> access_coin = [this](const COutPoint &op) -> const Coin & {
    return default_coin;
  };

  mutable std::function<bool(const CTransaction &)> have_inputs = [this](const CTransaction &tx) -> bool {
    return default_have_inputs;
  };

  const Coin &AccessCoin(const COutPoint &output) const override {
    ++invocations_AccessCoins;
    return access_coin(output);
  }

  bool HaveInputs(const CTransaction &tx) const override {
    ++invocations_HaveInputs;
    return have_inputs(tx);
  }
};

class StateDBMock : public finalization::StateDB {
  using FinalizationState = finalization::FinalizationState;
public:
  mutable std::atomic<std::uint32_t> invocations_Save{0};
  mutable std::atomic<std::uint32_t> invocations_Load{0};
  mutable std::atomic<std::uint32_t> invocations_LoadParticular{0};
  mutable std::atomic<std::uint32_t> invocations_FindLastFinalizedEpoch{0};
  mutable std::atomic<std::uint32_t> invocations_LoadStatesHigherThan{0};

  bool Save(const std::map<const CBlockIndex *, FinalizationState> &states) override {
    ++invocations_Save;
    return false;
  }

  bool Load(const esperanza::FinalizationParams &fin_params,
            const esperanza::AdminParams &admin_params,
            std::map<const CBlockIndex *, FinalizationState> *states) override {
    ++invocations_Load;
    return false;
  }

  bool Load(const CBlockIndex &index,
            const esperanza::FinalizationParams &fin_params,
            const esperanza::AdminParams &admin_params,
            std::map<const CBlockIndex *, FinalizationState> *states) const override {
    ++invocations_LoadParticular;
    return false;
  }

  boost::optional<uint32_t> FindLastFinalizedEpoch(
      const esperanza::FinalizationParams &fin_params,
      const esperanza::AdminParams &admin_params) const override {
    ++invocations_FindLastFinalizedEpoch;
    return boost::none;
  }

  void LoadStatesHigherThan(
      blockchain::Height height,
      const esperanza::FinalizationParams &fin_params,
      const esperanza::AdminParams &admin_params,
      std::map<const CBlockIndex *, FinalizationState> *states) const override {
    ++invocations_LoadStatesHigherThan;
  }
};

class BlockDBMock : public ::BlockDB {
public:
  mutable std::atomic<std::uint32_t> invocations_ReadBlock{0};

  boost::optional<CBlock> ReadBlock(const CBlockIndex &index) override {
    ++invocations_ReadBlock;
    return boost::none;
  }
};

}  // namespace mocks

#endif  //UNIT_E_TEST_UNITE_MOCKS_H