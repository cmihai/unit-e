// Copyright (c) 2018 The Unit-e developers
// Distributed under the MIT software license, see the accompanying
// file COPYING or http://www.opensource.org/licenses/mit-license.php.

#include <keystore.h>
#include <random.h>
#include <test/esperanza/finalization_utils.h>
#include <boost/test/unit_test.hpp>

CTransaction CreateBaseTransaction(const CTransaction &spendableTx,
                                   const CKey &spendableKey, CAmount amount,
                                   const TxType type,
                                   const CScript &scriptPubKey) {

  CBasicKeyStore keystore;
  keystore.AddKey(spendableKey);

  CMutableTransaction mutTx;
  mutTx.SetType(type);

  mutTx.vin.resize(1);
  mutTx.vin[0].prevout.hash = spendableTx.GetHash();
  mutTx.vin[0].prevout.n = 0;

  CTxOut out(amount, scriptPubKey);
  mutTx.vout.push_back(out);

  // Sign
  std::vector<unsigned char> vchSig;
  uint256 hash = SignatureHash(spendableTx.vout[0].scriptPubKey, mutTx, 0,
                               SIGHASH_ALL, amount, SIGVERSION_BASE);

  BOOST_CHECK(spendableKey.Sign(hash, vchSig));
  vchSig.push_back((unsigned char)SIGHASH_ALL);

  mutTx.vin[0].scriptSig = CScript() << ToByteVector(vchSig)
                                     << ToByteVector(spendableKey.GetPubKey());

  return CTransaction(mutTx);
}

uint256 GetVoteHash(esperanza::Vote vote) {

  CHashWriter ss(SER_GETHASH, 0);

  ss << vote.m_validatorAddress;
  ss << vote.m_targetHash;
  ss << vote.m_sourceEpoch;
  ss << vote.m_targetEpoch;

  return ss.GetHash();
}

CTransaction CreateVoteTx(esperanza::Vote &vote, const CKey &spendableKey) {

  CMutableTransaction mutTx;
  mutTx.SetType(TxType::VOTE);

  mutTx.vin.resize(1);
  uint256 signature = GetRandHash();

  std::vector<unsigned char> voteSig;
  BOOST_CHECK(spendableKey.Sign(GetVoteHash(vote), voteSig));

  CScript voteScript = CScript::EncodeVote(vote, voteSig);
  std::vector<unsigned char> voteVector(voteScript.begin(), voteScript.end());

  CScript scriptSig = (CScript() << ToByteVector(signature)) << voteVector;
  mutTx.vin[0] = (CTxIn(GetRandHash(), 0, scriptSig));

  uint256 keyHash = GetRandHash();
  CKey k;
  k.Set(keyHash.begin(), keyHash.end(), true);
  CScript scriptPubKey = CScript::CreatePayVoteSlashScript(k.GetPubKey());

  CTxOut out{10000, scriptPubKey};
  mutTx.vout.push_back(out);

  return CTransaction(mutTx);
}

CTransaction CreateDepositTx(const CTransaction &spendableTx,
                             const CKey &spendableKey, CAmount amount) {
  CScript scriptPubKey =
      CScript::CreatePayVoteSlashScript(spendableKey.GetPubKey());

  return CreateBaseTransaction(spendableTx, spendableKey, amount,
                               TxType::DEPOSIT, scriptPubKey);
}

CTransaction CreateLogoutTx(const CTransaction &spendableTx,
                            const CKey &spendableKey, CAmount amount) {

  CScript scriptPubKey =
      CScript::CreatePayVoteSlashScript(spendableKey.GetPubKey());

  return CreateBaseTransaction(spendableTx, spendableKey, amount,
                               TxType::LOGOUT, scriptPubKey);
}

CTransaction CreateWithdrawTx(const CTransaction &spendableTx,
                              const CKey &spendableKey, CAmount amount) {

  CScript scriptPubKey = CScript::CreateP2PKHScript(
      ToByteVector(spendableKey.GetPubKey().GetID()));

  return CreateBaseTransaction(spendableTx, spendableKey, amount,
                               TxType::WITHDRAW, scriptPubKey);
}

CTransaction CreateP2PKHTx(const CTransaction &spendableTx,
                           const CKey &spendableKey, CAmount amount) {

  CScript scriptPubKey = CScript::CreateP2PKHScript(
      ToByteVector(spendableKey.GetPubKey().GetID()));

  return CreateBaseTransaction(spendableTx, spendableKey, amount,
                               TxType::STANDARD, scriptPubKey);
}