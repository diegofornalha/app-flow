/*******************************************************************************
*   (c) 2019 Zondax GmbH
*
*  Licensed under the Apache License, Version 2.0 (the "License");
*  you may not use this file except in compliance with the License.
*  You may obtain a copy of the License at
*
*      http://www.apache.org/licenses/LICENSE-2.0
*
*  Unless required by applicable law or agreed to in writing, software
*  distributed under the License is distributed on an "AS IS" BASIS,
*  WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
*  See the License for the specific language governing permissions and
*  limitations under the License.
********************************************************************************/
#include <fmt/core.h>
#include <coin.h>
#include <crypto.h>
#include <parser_txdef.h>
#include <parser_impl.h>
#include <iomanip>
#include "testcases.h"
#include "zxmacros.h"

bool TestcaseIsValid(const Json::Value &) {
    return true;
}

std::string formatString(const std::string &data, uint8_t idx, uint8_t *pageCount) {
    char outBuffer[40];
    pageString(outBuffer, sizeof(outBuffer), data.c_str(), idx, pageCount);
    return std::string(outBuffer);
}

std::vector<std::string> formatStringParts(const Json::Value &v) {
    std::vector<std::string> answer;
    std::stringstream s;
    s << v.asString();

    uint8_t pageIdx = 0;
    uint8_t pageCount = 1;
    char outBuffer[40];

    while (pageIdx < pageCount) {
        pageString(outBuffer, sizeof(outBuffer), s.str().c_str(), pageIdx, &pageCount);
        answer.emplace_back(outBuffer);
        pageIdx++;
    }

    return answer;
}

template<typename S, typename... Args>
void addTo(std::vector<std::string> &answer, const S &format_str, Args &&... args) {
    answer.push_back(fmt::format(format_str, args...));
}

void addMultiStringArgumentTo(std::vector<std::string> &answer, const std::string &name, uint16_t item, const Json::Value &v) {
    auto chunks = formatStringParts(v);

    if (chunks.size() == 1) {
        addTo(answer, "{} | {} : {}", item, name, chunks[0]);
        return;
    }

    for (uint16_t j = 0; j < (uint16_t) chunks.size(); j++) {
        addTo(answer, "{} | {} [{}/{}] : {}", item, name, j + 1, chunks.size(), chunks[j]);
    }
}

std::vector<std::string> GenerateExpectedUIOutput(const testcaseData_t &tcd) {
    auto answer = std::vector<std::string>();

    if (!tcd.valid) {
        answer.emplace_back("Test case is not valid!");
        return answer;
    }

    uint8_t scriptHash[32];
    script_type_e scriptType = SCRIPT_UNKNOWN;
    sha256((const uint8_t *) tcd.script.c_str(), tcd.script.length(), scriptHash);
    _matchScriptType(scriptHash, &scriptType);

    uint16_t item = 0;
    uint8_t dummy;

    switch (scriptType) {
        case SCRIPT_UNKNOWN:
            addTo(answer, "{} | Type :Unknown", item++);
            break;
        case SCRIPT_TOKEN_TRANSFER: {
            addTo(answer, "{} | Type : Token Transfer", item++);
            addTo(answer, "{} | ChainID : {}", item++, tcd.chainID);
            addTo(answer, "{} | Amount : {}", item++, tcd.arguments[0]["value"].asString());
            addTo(answer, "{} | Destination : {}", item++, tcd.arguments[1]["value"].asString());
            break;
        }
        case SCRIPT_CREATE_ACCOUNT: {
            addTo(answer, "{} | Type : Create Account", item++);
            addTo(answer, "{} | ChainID : {}", item++, tcd.chainID);
            const auto pks = tcd.arguments[0]["value"];

            for (uint16_t i = 0; i < (uint16_t) pks.size(); i++) {
                addMultiStringArgumentTo(answer, fmt::format("Pub key {}", i + 1), item++, pks[i]["value"]);
            }
            break;
        }
        case SCRIPT_ADD_NEW_KEY: {
            addTo(answer, "{} | Type : Add New Key", item++);
            addTo(answer, "{} | ChainID : {}", item++, tcd.chainID);
            addMultiStringArgumentTo(answer, "Pub key", item++, tcd.arguments[0]["value"]);
            break;
        }
        case SCRIPT_TH01_WITHDRAW_UNLOCKED_TOKENS: {
            addTo(answer, "{} | Type : Withdraw FLOW from Lockbox", item++);
            addTo(answer, "{} | ChainID : {}", item++, tcd.chainID);
            addTo(answer, "{} | Amount : {}", item++, tcd.arguments[0]["value"].asString());
            break;
        }
        case SCRIPT_TH02_DEPOSIT_UNLOCKED_TOKENS: {
            addTo(answer, "{} | Type : Deposit FLOW to Lockbox", item++);
            addTo(answer, "{} | ChainID : {}", item++, tcd.chainID);
            addTo(answer, "{} | Amount : {}", item++, tcd.arguments[0]["value"].asString());
            break;
        }
        case SCRIPT_TH06_REGISTER_NODE: {
            addTo(answer, "{} | Type : Register Staked Node", item++);
            addTo(answer, "{} | ChainID : {}", item++, tcd.chainID);
            addMultiStringArgumentTo(answer, "Node ID", item++, tcd.arguments[0]["value"]);
            addTo(answer, "{} | Node Role : {}", item++, tcd.arguments[1]["value"].asString());
            addMultiStringArgumentTo(answer, "Networking Address", item++, tcd.arguments[2]["value"]);
            addMultiStringArgumentTo(answer, "Networking Key", item++, tcd.arguments[3]["value"]);
            addMultiStringArgumentTo(answer, "Staking Key", item++, tcd.arguments[4]["value"]);
            addTo(answer, "{} | Amount : {}", item++, tcd.arguments[5]["value"].asString());
            break;
        }
        case SCRIPT_TH08_STAKE_NEW_TOKENS: {
            addTo(answer, "{} | Type : Stake FLOW from Lockbox", item++);
            addTo(answer, "{} | ChainID : {}", item++, tcd.chainID);
            addTo(answer, "{} | Amount : {}", item++, tcd.arguments[0]["value"].asString());
            break;
        }
        case SCRIPT_TH09_RESTAKE_UNSTAKED_TOKENS: {
            addTo(answer, "{} | Type : Restake Unstaked FLOW", item++);
            addTo(answer, "{} | ChainID : {}", item++, tcd.chainID);
            addTo(answer, "{} | Amount : {}", item++, tcd.arguments[0]["value"].asString());
            break;
        }
        case SCRIPT_TH10_RESTAKE_REWARDED_TOKENS: {
            addTo(answer, "{} | Type : Restake Rewarded FLOW", item++);
            addTo(answer, "{} | ChainID : {}", item++, tcd.chainID);
            addTo(answer, "{} | Amount : {}", item++, tcd.arguments[0]["value"].asString());
            break;
        }
        case SCRIPT_TH11_UNSTAKE_TOKENS: {
            addTo(answer, "{} | Type : Unstake FLOW", item++);
            addTo(answer, "{} | ChainID : {}", item++, tcd.chainID);
            addTo(answer, "{} | Amount : {}", item++, tcd.arguments[0]["value"].asString());
            break;
        }
        case SCRIPT_TH12_UNSTAKE_ALL_TOKENS: {
            addTo(answer, "{} | Type : Unstake All FLOW", item++);
            addTo(answer, "{} | ChainID : {}", item++, tcd.chainID);
            break;
        }
        case SCRIPT_TH13_WITHDRAW_UNSTAKED_TOKENS: {
            addTo(answer, "{} | Type : Withdraw Unstaked FLOW to Lockbox", item++);
            addTo(answer, "{} | ChainID : {}", item++, tcd.chainID);
            addTo(answer, "{} | Amount : {}", item++, tcd.arguments[0]["value"].asString());
            break;
        }
        case SCRIPT_TH14_WITHDRAW_REWARDED_TOKENS: {
            addTo(answer, "{} | Type : Withdraw Rewarded FLOW to Lockbox", item++);
            addTo(answer, "{} | ChainID : {}", item++, tcd.chainID);
            addTo(answer, "{} | Amount : {}", item++, tcd.arguments[0]["value"].asString());
            break;
        }
        case SCRIPT_TH16_REGISTER_OPERATOR_NODE: {
            addTo(answer, "{} | Type : Register Operator Node", item++);
            addTo(answer, "{} | ChainID : {}", item++, tcd.chainID);
            addTo(answer, "{} | Operator Address : {}", item++, tcd.arguments[0]["value"].asString());
            addMultiStringArgumentTo(answer, "Node ID", item++, tcd.arguments[1]["value"]);
            addTo(answer, "{} | Amount : {}", item++, tcd.arguments[2]["value"].asString());
            break;
        }
        case SCRIPT_TH17_REGISTER_DELEGATOR: {
            addTo(answer, "{} | Type : Register Delegator", item++);
            addTo(answer, "{} | ChainID : {}", item++, tcd.chainID);
            addMultiStringArgumentTo(answer, "Node ID", item++, tcd.arguments[0]["value"]);
            addTo(answer, "{} | Amount : {}", item++, tcd.arguments[1]["value"].asString());
            break;
        }
        case SCRIPT_TH19_DELEGATE_NEW_TOKENS: {
            addTo(answer, "{} | Type : Delegate FLOW from Lockbox", item++);
            addTo(answer, "{} | ChainID : {}", item++, tcd.chainID);
            addTo(answer, "{} | Amount : {}", item++, tcd.arguments[0]["value"].asString());
            break;
        }
        case SCRIPT_TH20_RESTAKE_UNSTAKED_DELEGATED_TOKENS: {
            addTo(answer, "{} | Type : Re-delegate Unstaked FLOW", item++);
            addTo(answer, "{} | ChainID : {}", item++, tcd.chainID);
            addTo(answer, "{} | Amount : {}", item++, tcd.arguments[0]["value"].asString());
            break;
        }
        case SCRIPT_TH21_RESTAKE_REWARDED_DELEGATED_TOKENS: {
            addTo(answer, "{} | Type : Re-delegate Rewarded FLOW", item++);
            addTo(answer, "{} | ChainID : {}", item++, tcd.chainID);
            addTo(answer, "{} | Amount : {}", item++, tcd.arguments[0]["value"].asString());
            break;
        }
        case SCRIPT_TH22_UNSTAKE_DELEGATED_TOKENS: {
            addTo(answer, "{} | Type : Unstake Delegated FLOW", item++);
            addTo(answer, "{} | ChainID : {}", item++, tcd.chainID);
            addTo(answer, "{} | Amount : {}", item++, tcd.arguments[0]["value"].asString());
            break;
        }
        case SCRIPT_TH23_WITHDRAW_UNSTAKED_DELEGATED_TOKENS: {
            addTo(answer, "{} | Type : Withdraw Undelegated FLOW to Lockbox", item++);
            addTo(answer, "{} | ChainID : {}", item++, tcd.chainID);
            addTo(answer, "{} | Amount : {}", item++, tcd.arguments[0]["value"].asString());
            break;
        }
        case SCRIPT_TH24_WITHDRAW_REWARDED_DELEGATED_TOKENS: {
            addTo(answer, "{} | Type : Withdraw Delegate Rewards to Lockbox", item++);
            addTo(answer, "{} | ChainID : {}", item++, tcd.chainID);
            addTo(answer, "{} | Amount : {}", item++, tcd.arguments[0]["value"].asString());
            break;
        }
        case SCRIPT_TH25_UPDATE_NETWORKING_ADDRESS: {
            addTo(answer, "{} | Type : Update Networking Address", item++);
            addTo(answer, "{} | ChainID : {}", item++, tcd.chainID);
            addMultiStringArgumentTo(answer, "Address", item++, tcd.arguments[0]["value"]);
            break;
        }
        case SCRIPT_SCO01_SETUP_STAKING_COLLECTION: {
            addTo(answer, "{} | Type : Setup Staking Collection", item++);
            addTo(answer, "{} | ChainID : {}", item++, tcd.chainID);
            break;
        }
        case SCRIPT_SCO02_REGISTER_DELEGATOR: {
            addTo(answer, "{} | Type : Register Delegator", item++);
            addTo(answer, "{} | ChainID : {}", item++, tcd.chainID);
            addMultiStringArgumentTo(answer, "Node ID", item++, tcd.arguments[0]["value"]);
            addTo(answer, "{} | Amount : {}", item++, tcd.arguments[1]["value"].asString());
            break;
        }
        case SCRIPT_SCO03_REGISTER_NODE: {
            addTo(answer, "{} | Type : Register Node", item++);
            addTo(answer, "{} | ChainID : {}", item++, tcd.chainID);
            addMultiStringArgumentTo(answer, "Node ID", item++, tcd.arguments[0]["value"]);
            addTo(answer, "{} | Node Role : {}", item++, tcd.arguments[1]["value"].asString());
            addMultiStringArgumentTo(answer, "Netw. Addr.", item++, tcd.arguments[2]["value"]);
            addMultiStringArgumentTo(answer, "Netw. Key", item++, tcd.arguments[3]["value"]);
            addMultiStringArgumentTo(answer, "Staking Key", item++, tcd.arguments[4]["value"]);
            addTo(answer, "{} | Amount : {}", item++, tcd.arguments[5]["value"].asString());
            if (tcd.arguments[6]["value"].isObject()) {
                for (uint16_t i = 0; i < (uint16_t) tcd.arguments[6]["value"]["value"].size(); i++) {
                    addMultiStringArgumentTo(answer, fmt::format("Pub key {}", i + 1), item++, tcd.arguments[6]["value"]["value"][i]["value"]);
                }
            }
            else {
                addTo(answer, "{} | Pub key 1 : None", item++);
            }
            break;
        }
        case SCRIPT_SCO04_CREATE_MACHINE_ACCOUNT: {
            addTo(answer, "{} | Type : Create Machine Account", item++);
            addTo(answer, "{} | ChainID : {}", item++, tcd.chainID);
            addMultiStringArgumentTo(answer, "Node ID", item++, tcd.arguments[0]["value"]);
            for (uint16_t i = 0; i < (uint16_t) tcd.arguments[1]["value"].size(); i++) {
                addMultiStringArgumentTo(answer, fmt::format("Pub key {}", i + 1), item++, tcd.arguments[1]["value"][i]["value"]);
            }
            break;
        }
        case SCRIPT_SCO05_REQUEST_UNSTAKING: {
            addTo(answer, "{} | Type : Request Unstaking", item++);
            addTo(answer, "{} | ChainID : {}", item++, tcd.chainID);
            addMultiStringArgumentTo(answer, "Node ID", item++, tcd.arguments[0]["value"]);
            if (tcd.arguments[1]["value"].isObject())
                addTo(answer, "{} | Delegator ID : {}", item++, tcd.arguments[1]["value"]["value"].asString());
            else
                addTo(answer, "{} | Delegator ID : None", item++);
            addTo(answer, "{} | Amount : {}", item++, tcd.arguments[2]["value"].asString());
            break;
        }
        case SCRIPT_SCO06_STAKE_NEW_TOKENS: {
            addTo(answer, "{} | Type : Stake New Tokens", item++);
            addTo(answer, "{} | ChainID : {}", item++, tcd.chainID);
            addMultiStringArgumentTo(answer, "Node ID", item++, tcd.arguments[0]["value"]);
            if (tcd.arguments[1]["value"].isObject())
                addTo(answer, "{} | Delegator ID : {}", item++, tcd.arguments[1]["value"]["value"].asString());
            else
                addTo(answer, "{} | Delegator ID : None", item++);
            addTo(answer, "{} | Amount : {}", item++, tcd.arguments[2]["value"].asString());
            break;
        }
        case SCRIPT_SCO07_STAKE_REWARD_TOKENS: {
            addTo(answer, "{} | Type : Stake Reward Tokens", item++);
            addTo(answer, "{} | ChainID : {}", item++, tcd.chainID);
            addMultiStringArgumentTo(answer, "Node ID", item++, tcd.arguments[0]["value"]);
            if (tcd.arguments[1]["value"].isObject())
                addTo(answer, "{} | Delegator ID : {}", item++, tcd.arguments[1]["value"]["value"].asString());
            else
                addTo(answer, "{} | Delegator ID : None", item++);
            addTo(answer, "{} | Amount : {}", item++, tcd.arguments[2]["value"].asString());
            break;
        }
        case SCRIPT_SCO08_STAKE_UNSTAKED_TOKENS: {
            addTo(answer, "{} | Type : Stake Unstaked Tokens", item++);
            addTo(answer, "{} | ChainID : {}", item++, tcd.chainID);
            addMultiStringArgumentTo(answer, "Node ID", item++, tcd.arguments[0]["value"]);
            if (tcd.arguments[1]["value"].isObject())
                addTo(answer, "{} | Delegator ID : {}", item++, tcd.arguments[1]["value"]["value"].asString());
            else
                addTo(answer, "{} | Delegator ID : None", item++);
            addTo(answer, "{} | Amount : {}", item++, tcd.arguments[2]["value"].asString());
            break;
        }
        case SCRIPT_SCO09_UNSTAKE_ALL: {
            addTo(answer, "{} | Type : Unstake All", item++);
            addTo(answer, "{} | ChainID : {}", item++, tcd.chainID);
            addMultiStringArgumentTo(answer, "Node ID", item++, tcd.arguments[0]["value"]);
            break;
        }
        case SCRIPT_SCO10_WITHDRAW_REWARD_TOKENS: {
            addTo(answer, "{} | Type : Withdraw Reward Tokens", item++);
            addTo(answer, "{} | ChainID : {}", item++, tcd.chainID);
            addMultiStringArgumentTo(answer, "Node ID", item++, tcd.arguments[0]["value"]);
            if (tcd.arguments[1]["value"].isObject())
                addTo(answer, "{} | Delegator ID : {}", item++, tcd.arguments[1]["value"]["value"].asString());
            else
                addTo(answer, "{} | Delegator ID : None", item++);
            addTo(answer, "{} | Amount : {}", item++, tcd.arguments[2]["value"].asString());
            break;
        }
        case SCRIPT_SCO11_WITHDRAW_UNSTAKED_TOKENS: {
            addTo(answer, "{} | Type : Withdraw Unstaked Tokens", item++);
            addTo(answer, "{} | ChainID : {}", item++, tcd.chainID);
            addMultiStringArgumentTo(answer, "Node ID", item++, tcd.arguments[0]["value"]);
            if (tcd.arguments[1]["value"].isObject())
                addTo(answer, "{} | Delegator ID : {}", item++, tcd.arguments[1]["value"]["value"].asString());
            else
                addTo(answer, "{} | Delegator ID : None", item++);
            addTo(answer, "{} | Amount : {}", item++, tcd.arguments[2]["value"].asString());
            break;
        }
        case SCRIPT_SCO12_CLOSE_STAKE: {
            addTo(answer, "{} | Type : Close Stake", item++);
            addTo(answer, "{} | ChainID : {}", item++, tcd.chainID);
            addMultiStringArgumentTo(answer, "Node ID", item++, tcd.arguments[0]["value"]);
            if (tcd.arguments[1]["value"].isObject())
                addTo(answer, "{} | Delegator ID : {}", item++, tcd.arguments[1]["value"]["value"].asString());
            else
                addTo(answer, "{} | Delegator ID : None", item++);
            break;
        }
        case SCRIPT_SCO13_TRANSFER_NODE: {
            addTo(answer, "{} | Type : Transfer Node", item++);
            addTo(answer, "{} | ChainID : {}", item++, tcd.chainID);
            addMultiStringArgumentTo(answer, "Node ID", item++, tcd.arguments[0]["value"]);
            addMultiStringArgumentTo(answer, "Address", item++, tcd.arguments[1]["value"]);
            break;
        }
        case SCRIPT_SCO14_TRANSFER_DELEGATOR: {
            addTo(answer, "{} | Type : Transfer Delegator", item++);
            addTo(answer, "{} | ChainID : {}", item++, tcd.chainID);
            addMultiStringArgumentTo(answer, "Node ID", item++, tcd.arguments[0]["value"]);
            addTo(answer, "{} | Delegator ID : {}", item++, tcd.arguments[1]["value"].asString());
            addMultiStringArgumentTo(answer, "Address", item++, tcd.arguments[2]["value"]);
            break;
        }
        case SCRIPT_SCO15_WITHDRAW_FROM_MACHINE_ACCOUNT: {
            addTo(answer, "{} | Type : Withdraw From Machine Account", item++);
            addTo(answer, "{} | ChainID : {}", item++, tcd.chainID);
            addMultiStringArgumentTo(answer, "Node ID", item++, tcd.arguments[0]["value"]);
            addTo(answer, "{} | Amount : {}", item++, tcd.arguments[1]["value"].asString());
            break;
        }
        case SCRIPT_SCO16_UPDATE_NETWORKING_ADDRESS: {
            addTo(answer, "{} | Type : Update Networking Address", item++);
            addTo(answer, "{} | ChainID : {}", item++, tcd.chainID);
            addMultiStringArgumentTo(answer, "Node ID", item++, tcd.arguments[0]["value"]);
            addMultiStringArgumentTo(answer, "Address", item++, tcd.arguments[1]["value"]);
            break;
        }
        case SCRIPT_FUSD01_SETUP_FUSD_VAULT: {
            addTo(answer, "{} | Type : Setup FUSD Vault", item++);
            addTo(answer, "{} | ChainID : {}", item++, tcd.chainID);
            break;
        }
        case SCRIPT_FUSD02_TRANSFER_FUSD: {
            addTo(answer, "{} | Type : Transfer FUSD", item++);
            addTo(answer, "{} | ChainID : {}", item++, tcd.chainID);
            addTo(answer, "{} | Amount : {}", item++, tcd.arguments[0]["value"].asString());
            addMultiStringArgumentTo(answer, "Recipient", item++, tcd.arguments[1]["value"]);
            break;
        }
        case SCRIPT_TS01_SET_UP_TOPSHOT_COLLECTION: {
            addTo(answer, "{} | Type : Set Up Top Shot Collection", item++);
            addTo(answer, "{} | ChainID : {}", item++, tcd.chainID);
            break;
        }
        case SCRIPT_TS02_TRANSFER_TOP_SHOT_MOMENT: {
            addTo(answer, "{} | Type : Transfer Top Shot Moment", item++);
            addTo(answer, "{} | ChainID : {}", item++, tcd.chainID);
            addTo(answer, "{} | Moment ID : {}", item++, tcd.arguments[0]["value"].asString());
            addMultiStringArgumentTo(answer, "Address", item++, tcd.arguments[1]["value"]);
            break;
        }
        default:
            addTo(answer, "{} | Type : ERROR", item++);
            break;
    }

    addTo(answer, "{} | Ref Block [1/2] : {}", item, formatString(tcd.refBlock, 0, &dummy));
    addTo(answer, "{} | Ref Block [2/2] : {}", item++, formatString(tcd.refBlock, 1, &dummy));
    addTo(answer, "{} | Gas Limit : {}", item++, tcd.gasLimit);
    addTo(answer, "{} | Prop Key Addr : {}", item++, formatString(tcd.proposalKeyAddress, 0, &dummy));
    addTo(answer, "{} | Prop Key Id : {}", item++, tcd.proposalKeyId);
    addTo(answer, "{} | Prop Key Seq Num : {}", item++, tcd.proposalKeySequenceNumber);
    addTo(answer, "{} | Payer : {}", item++, formatString(tcd.payer, 0, &dummy));

    for (uint16_t i = 0; i < (uint16_t) tcd.authorizers.size(); i++) {
        addTo(answer, "{} | Authorizer {} : {}", item++, i + 1, tcd.authorizers[i]);
    }

    return answer;
}
