// Copyright (c) 2023-2024 Advanced Micro Devices, Inc. All rights reserved.
// AUTO GENERATED FILE. DO NOT EDIT DIRECTLY.
#ifndef INST_EXECUTION_HPP_H_
#define INST_EXECUTION_HPP_H_

// C++ libraries.
#include <array>
#include <cinttypes>
#include <memory>
#include <unordered_map>
#include <vector>

// Local libraries.
#include "generic.h"
#include "wave_state.h"

namespace amdisa
{
    enum class ExecutionType
    {
        kUndefined,
        kScalar,
        kVector,
        kBranch
    };

    struct IStateChangeRecipe
    {
        virtual void ComputeChange(WaveState& wave_state) = 0;
        virtual bool SetSource(const uint32_t index, const std::vector<uint32_t>& data) = 0;
        virtual bool GetDestination(const uint32_t index, std::vector<uint32_t>& data) = 0;
        virtual ExecutionType GetExecutionType() const = 0;
        virtual bool UpdatesScc() const = 0;
        virtual bool UpdatesExec() const = 0;
        virtual bool UpdatesPc() const = 0;
        virtual bool HasSideEffects() const = 0;
        virtual ~IStateChangeRecipe() {};
    };

    class s_mov_b32 : public IStateChangeRecipe
    {
    public:
        void ComputeChange(WaveState& wave_state) override
        {
            D0 = S0.b32();
        }

        bool SetSource(const uint32_t index, const std::vector<uint32_t>& data) override
        {
            if (index >= kSrcNum)
                return false;
            srcs_[index]->SetData(data);
            return true;
        }

        bool GetDestination(const uint32_t index, std::vector<uint32_t>& data) override
        {
            if (index >= kDstNum)
                return false;
            dsts_[index]->GetData(data);
            return true;
        }

        ExecutionType GetExecutionType() const override { return ExecutionType::kScalar; }
        bool UpdatesScc() const override { return false; }
        bool UpdatesExec() const override { return false; }
        bool UpdatesPc() const override { return false; }
        bool HasSideEffects() const override { return false; }

    private:
        const uint32_t kSrcNum = 1;
        const uint32_t kDstNum = 1;
        const ExecutionType kExecutionType = ExecutionType::kScalar;
        Generic S0, D0;
        std::array<Generic*, 1> srcs_ = { &S0 };
        std::array<Generic*, 1> dsts_ = { &D0 };
    };

    class s_getpc_b64 : public IStateChangeRecipe
    {
    public:
        void ComputeChange(WaveState& wave_state) override
        {
            D0 = (I64(wave_state.PC()) + I64(4));
        }

        bool SetSource(const uint32_t index, const std::vector<uint32_t>& data) override
        {
            return false;
        }

        bool GetDestination(const uint32_t index, std::vector<uint32_t>& data) override
        {
            if (index >= kDstNum)
                return false;
            dsts_[index]->GetData(data);
            return true;
        }

        ExecutionType GetExecutionType() const override { return ExecutionType::kScalar; }
        bool UpdatesScc() const override { return false; }
        bool UpdatesExec() const override { return false; }
        bool UpdatesPc() const override { return false; }
        bool HasSideEffects() const override { return false; }

    private:
        const uint32_t kSrcNum = 0;
        const uint32_t kDstNum = 1;
        const ExecutionType kExecutionType = ExecutionType::kScalar;
        Generic D0;
        std::array<Generic*, 1> dsts_ = { &D0 };
    };

    class s_setpc_b64 : public IStateChangeRecipe
    {
    public:
        void ComputeChange(WaveState& wave_state) override
        {
            wave_state.PC() = static_cast<uint64_t>(S0.i64());
        }

        bool SetSource(const uint32_t index, const std::vector<uint32_t>& data) override
        {
            if (index >= kSrcNum)
                return false;
            srcs_[index]->SetData(data);
            return true;
        }

        bool GetDestination(const uint32_t index, std::vector<uint32_t>& data) override
        {
            return false;
        }

        ExecutionType GetExecutionType() const override { return ExecutionType::kBranch; }
        bool UpdatesScc() const override { return false; }
        bool UpdatesExec() const override { return false; }
        bool UpdatesPc() const override { return true; }
        bool HasSideEffects() const override { return true; }

    private:
        const uint32_t kSrcNum = 1;
        const uint32_t kDstNum = 0;
        const ExecutionType kExecutionType = ExecutionType::kBranch;
        Generic S0;
        std::array<Generic*, 1> srcs_ = { &S0 };
    };

    class s_swappc_b64 : public IStateChangeRecipe
    {
    public:
        void ComputeChange(WaveState& wave_state) override
        {
            jump_addr = S0.i64();
            D0 = (I64(wave_state.PC()) + I64(4));
            wave_state.PC() = static_cast<uint64_t>(jump_addr.i64());
        }

        bool SetSource(const uint32_t index, const std::vector<uint32_t>& data) override
        {
            if (index >= kSrcNum)
                return false;
            srcs_[index]->SetData(data);
            return true;
        }

        bool GetDestination(const uint32_t index, std::vector<uint32_t>& data) override
        {
            if (index >= kDstNum)
                return false;
            dsts_[index]->GetData(data);
            return true;
        }

        ExecutionType GetExecutionType() const override { return ExecutionType::kBranch; }
        bool UpdatesScc() const override { return false; }
        bool UpdatesExec() const override { return false; }
        bool UpdatesPc() const override { return true; }
        bool HasSideEffects() const override { return true; }

    private:
        const uint32_t kSrcNum = 1;
        const uint32_t kDstNum = 1;
        const ExecutionType kExecutionType = ExecutionType::kBranch;
        Generic S0, D0, jump_addr;
        std::array<Generic*, 1> srcs_ = { &S0 };
        std::array<Generic*, 1> dsts_ = { &D0 };
    };

    class s_add_u32 : public IStateChangeRecipe
    {
    public:
        void ComputeChange(WaveState& wave_state) override
        {
            tmp = (S0.u32().u64() + S1.u32().u64());
            wave_state.SCC() = ((tmp >= U64(4294967296))) ? U1(1) : U1(0);
            // unsigned overflow or carry-out for S_ADDC_U32.;
            D0 = tmp.u32();
        }

        bool SetSource(const uint32_t index, const std::vector<uint32_t>& data) override
        {
            if (index >= kSrcNum)
                return false;
            srcs_[index]->SetData(data);
            return true;
        }

        bool GetDestination(const uint32_t index, std::vector<uint32_t>& data) override
        {
            if (index >= kDstNum)
                return false;
            dsts_[index]->GetData(data);
            return true;
        }

        ExecutionType GetExecutionType() const override { return ExecutionType::kScalar; }
        bool UpdatesScc() const override { return true; }
        bool UpdatesExec() const override { return false; }
        bool UpdatesPc() const override { return false; }
        bool HasSideEffects() const override { return true; }

    private:
        const uint32_t kSrcNum = 2;
        const uint32_t kDstNum = 1;
        const ExecutionType kExecutionType = ExecutionType::kScalar;
        Generic S0, S1, D0, tmp;
        std::array<Generic*, 2> srcs_ = { &S0, &S1 };
        std::array<Generic*, 1> dsts_ = { &D0 };
    };

    class s_sub_u32 : public IStateChangeRecipe
    {
    public:
        void ComputeChange(WaveState& wave_state) override
        {
            tmp = (S0.u32() - S1.u32());
            wave_state.SCC() = ((S1.u32() > S0.u32())) ? U1(1) : U1(0);
            // unsigned overflow or carry-out for S_SUBB_U32.;
            D0 = tmp.u32();
        }

        bool SetSource(const uint32_t index, const std::vector<uint32_t>& data) override
        {
            if (index >= kSrcNum)
                return false;
            srcs_[index]->SetData(data);
            return true;
        }

        bool GetDestination(const uint32_t index, std::vector<uint32_t>& data) override
        {
            if (index >= kDstNum)
                return false;
            dsts_[index]->GetData(data);
            return true;
        }

        ExecutionType GetExecutionType() const override { return ExecutionType::kScalar; }
        bool UpdatesScc() const override { return true; }
        bool UpdatesExec() const override { return false; }
        bool UpdatesPc() const override { return false; }
        bool HasSideEffects() const override { return true; }

    private:
        const uint32_t kSrcNum = 2;
        const uint32_t kDstNum = 1;
        const ExecutionType kExecutionType = ExecutionType::kScalar;
        Generic S0, S1, D0, tmp;
        std::array<Generic*, 2> srcs_ = { &S0, &S1 };
        std::array<Generic*, 1> dsts_ = { &D0 };
    };

    class s_add_i32 : public IStateChangeRecipe
    {
    public:
        void ComputeChange(WaveState& wave_state) override
        {
            tmp = (S0.i32() + S1.i32());
            wave_state.SCC() = ((S0.u32()[I32(31)] == S1.u32()[I32(31)]) && (S0.u32()[I32(31)] != tmp.u32()[I32(31)]));
            // signed overflow.;
            D0 = tmp.i32();
        }

        bool SetSource(const uint32_t index, const std::vector<uint32_t>& data) override
        {
            if (index >= kSrcNum)
                return false;
            srcs_[index]->SetData(data);
            return true;
        }

        bool GetDestination(const uint32_t index, std::vector<uint32_t>& data) override
        {
            if (index >= kDstNum)
                return false;
            dsts_[index]->GetData(data);
            return true;
        }

        ExecutionType GetExecutionType() const override { return ExecutionType::kScalar; }
        bool UpdatesScc() const override { return true; }
        bool UpdatesExec() const override { return false; }
        bool UpdatesPc() const override { return false; }
        bool HasSideEffects() const override { return true; }

    private:
        const uint32_t kSrcNum = 2;
        const uint32_t kDstNum = 1;
        const ExecutionType kExecutionType = ExecutionType::kScalar;
        Generic S0, S1, D0, tmp;
        std::array<Generic*, 2> srcs_ = { &S0, &S1 };
        std::array<Generic*, 1> dsts_ = { &D0 };
    };

    class s_addc_u32 : public IStateChangeRecipe
    {
    public:
        void ComputeChange(WaveState& wave_state) override
        {
            tmp = ((S0.u32().u64() + S1.u32().u64()) + U64(wave_state.SCC()));
            wave_state.SCC() = ((tmp >= U64(4294967296))) ? U1(1) : U1(0);
            // unsigned overflow or carry-out for S_ADDC_U32.;
            D0 = tmp.u32();
        }

        bool SetSource(const uint32_t index, const std::vector<uint32_t>& data) override
        {
            if (index >= kSrcNum)
                return false;
            srcs_[index]->SetData(data);
            return true;
        }

        bool GetDestination(const uint32_t index, std::vector<uint32_t>& data) override
        {
            if (index >= kDstNum)
                return false;
            dsts_[index]->GetData(data);
            return true;
        }

        ExecutionType GetExecutionType() const override { return ExecutionType::kScalar; }
        bool UpdatesScc() const override { return true; }
        bool UpdatesExec() const override { return false; }
        bool UpdatesPc() const override { return false; }
        bool HasSideEffects() const override { return true; }

    private:
        const uint32_t kSrcNum = 2;
        const uint32_t kDstNum = 1;
        const ExecutionType kExecutionType = ExecutionType::kScalar;
        Generic S0, S1, D0, tmp;
        std::array<Generic*, 2> srcs_ = { &S0, &S1 };
        std::array<Generic*, 1> dsts_ = { &D0 };
    };

    class s_movk_i32 : public IStateChangeRecipe
    {
    public:
        void ComputeChange(WaveState& wave_state) override
        {
            D0 = S0.i16().signext().i32();
        }

        bool SetSource(const uint32_t index, const std::vector<uint32_t>& data) override
        {
            if (index >= kSrcNum)
                return false;
            srcs_[index]->SetData(data);
            return true;
        }

        bool GetDestination(const uint32_t index, std::vector<uint32_t>& data) override
        {
            if (index >= kDstNum)
                return false;
            dsts_[index]->GetData(data);
            return true;
        }

        ExecutionType GetExecutionType() const override { return ExecutionType::kScalar; }
        bool UpdatesScc() const override { return false; }
        bool UpdatesExec() const override { return false; }
        bool UpdatesPc() const override { return false; }
        bool HasSideEffects() const override { return false; }

    private:
        const uint32_t kSrcNum = 1;
        const uint32_t kDstNum = 1;
        const ExecutionType kExecutionType = ExecutionType::kScalar;
        Generic S0, D0;
        std::array<Generic*, 1> srcs_ = { &S0 };
        std::array<Generic*, 1> dsts_ = { &D0 };
    };

    class s_addk_i32 : public IStateChangeRecipe
    {
    public:
        void ComputeChange(WaveState& wave_state) override
        {
            tmp = D0.i32();
            // save value so we can check sign bits for overflow later.;
            D0 = (D0.i32() + S0.i16().signext().i32());
            wave_state.SCC() = ((tmp[I32(31)] == S0.i16()[I32(15)]) && (tmp[I32(31)] != D0.i32()[I32(31)]));
            // signed overflow.;
        }

        bool SetSource(const uint32_t index, const std::vector<uint32_t>& data) override
        {
            if (index >= kSrcNum)
                return false;
            srcs_[index]->SetData(data);
            return true;
        }

        bool GetDestination(const uint32_t index, std::vector<uint32_t>& data) override
        {
            if (index >= kDstNum)
                return false;
            dsts_[index]->GetData(data);
            return true;
        }

        ExecutionType GetExecutionType() const override { return ExecutionType::kScalar; }
        bool UpdatesScc() const override { return true; }
        bool UpdatesExec() const override { return false; }
        bool UpdatesPc() const override { return false; }
        bool HasSideEffects() const override { return true; }

    private:
        const uint32_t kSrcNum = 2;
        const uint32_t kDstNum = 1;
        const ExecutionType kExecutionType = ExecutionType::kScalar;
        Generic D0, S0, tmp;
        std::array<Generic*, 2> srcs_ = { &D0, &S0 };
        std::array<Generic*, 1> dsts_ = { &D0 };
    };

    class s_call_b64 : public IStateChangeRecipe
    {
    public:
        void ComputeChange(WaveState& wave_state) override
        {
            D0 = (I64(wave_state.PC()) + I64(4));
            wave_state.PC() = static_cast<uint64_t>(((I64(wave_state.PC()) + (S0.i16() * I16(4)).signext()) + I64(4)));
        }

        bool SetSource(const uint32_t index, const std::vector<uint32_t>& data) override
        {
            if (index >= kSrcNum)
                return false;
            srcs_[index]->SetData(data);
            return true;
        }

        bool GetDestination(const uint32_t index, std::vector<uint32_t>& data) override
        {
            if (index >= kDstNum)
                return false;
            dsts_[index]->GetData(data);
            return true;
        }

        ExecutionType GetExecutionType() const override { return ExecutionType::kBranch; }
        bool UpdatesScc() const override { return false; }
        bool UpdatesExec() const override { return false; }
        bool UpdatesPc() const override { return true; }
        bool HasSideEffects() const override { return true; }

    private:
        const uint32_t kSrcNum = 1;
        const uint32_t kDstNum = 1;
        const ExecutionType kExecutionType = ExecutionType::kBranch;
        Generic S0, D0;
        std::array<Generic*, 1> srcs_ = { &S0 };
        std::array<Generic*, 1> dsts_ = { &D0 };
    };

    class s_branch : public IStateChangeRecipe
    {
    public:
        void ComputeChange(WaveState& wave_state) override
        {
            wave_state.PC() = static_cast<uint64_t>(((I64(wave_state.PC()) + (S0.i16() * I16(4)).signext()) + I64(4)));
            // short jump.;
        }

        bool SetSource(const uint32_t index, const std::vector<uint32_t>& data) override
        {
            if (index >= kSrcNum)
                return false;
            srcs_[index]->SetData(data);
            return true;
        }

        bool GetDestination(const uint32_t index, std::vector<uint32_t>& data) override
        {
            return false;
        }

        ExecutionType GetExecutionType() const override { return ExecutionType::kBranch; }
        bool UpdatesScc() const override { return false; }
        bool UpdatesExec() const override { return false; }
        bool UpdatesPc() const override { return true; }
        bool HasSideEffects() const override { return true; }

    private:
        const uint32_t kSrcNum = 1;
        const uint32_t kDstNum = 0;
        const ExecutionType kExecutionType = ExecutionType::kBranch;
        Generic S0;
        std::array<Generic*, 1> srcs_ = { &S0 };
    };

    // Factory for the execution models.
    class ChangeRecipeFactory
    {
    public:
        std::shared_ptr<IStateChangeRecipe> GetModel(const std::string& instruction_name)
        {
            std::shared_ptr<IStateChangeRecipe> ret = nullptr;
            if (name_to_model_.find(instruction_name) != name_to_model_.end())
            {
                ret = name_to_model_.at(instruction_name);
            }
            return ret;
        }

        void Init()
        {
            name_to_model_["s_mov_b32"] = std::make_shared<s_mov_b32>();
            name_to_model_["s_getpc_b64"] = std::make_shared<s_getpc_b64>();
            name_to_model_["s_setpc_b64"] = std::make_shared<s_setpc_b64>();
            name_to_model_["s_swappc_b64"] = std::make_shared<s_swappc_b64>();
            name_to_model_["s_add_u32"] = std::make_shared<s_add_u32>();
            name_to_model_["s_sub_u32"] = std::make_shared<s_sub_u32>();
            name_to_model_["s_add_i32"] = std::make_shared<s_add_i32>();
            name_to_model_["s_addc_u32"] = std::make_shared<s_addc_u32>();
            name_to_model_["s_movk_i32"] = std::make_shared<s_movk_i32>();
            name_to_model_["s_addk_i32"] = std::make_shared<s_addk_i32>();
            name_to_model_["s_call_b64"] = std::make_shared<s_call_b64>();
            name_to_model_["s_branch"] = std::make_shared<s_branch>();
        }

    private:
        std::unordered_map<std::string, std::shared_ptr<IStateChangeRecipe>> name_to_model_;
    };
}
#endif // INST_EXECUTION_HPP_H_
