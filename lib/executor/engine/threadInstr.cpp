// SPDX-License-Identifier: Apache-2.0
// SPDX-FileCopyrightText: 2019-2022 Second State INC

#include "executor/executor.h"

namespace WasmEdge {
namespace Executor {

Expect<void>
Executor::runAtomicNotifyOp(Runtime::StackManager &StackMgr,
                            Runtime::Instance::MemoryInstance &MemInst,
                            const AST::Instruction &Instr) {
  ValVariant RawAddress = StackMgr.pop();
  ValVariant &RawCount = StackMgr.getTop();

  uint32_t Address = RawAddress.get<uint32_t>();

  if (Address >
      std::numeric_limits<uint32_t>::max() - Instr.getMemoryOffset()) {
    spdlog::error(ErrCode::MemoryOutOfBounds);
    spdlog::error(ErrInfo::InfoBoundary(
        Address + static_cast<uint64_t>(Instr.getMemoryOffset()),
        sizeof(uint32_t), MemInst.getBoundIdx()));
    spdlog::error(
        ErrInfo::InfoInstruction(Instr.getOpCode(), Instr.getOffset()));
    return Unexpect(ErrCode::MemoryOutOfBounds);
  }
  Address += Instr.getMemoryOffset();

  uint32_t Count = RawCount.get<uint32_t>();

  if (auto Res = atomicNotify(MemInst, Address, Count); unlikely(!Res)) {
    spdlog::error(
        ErrInfo::InfoInstruction(Instr.getOpCode(), Instr.getOffset()));
    return Unexpect(Res);
  } else {
    RawCount.emplace<uint32_t>(*Res);
  }
  return {};
}

Expect<void> Executor::runMemoryFenceOp() {
  std::atomic_thread_fence(std::memory_order_release);
  return {};
}

Expect<uint32_t>
Executor::atomicNotify(Runtime::Instance::MemoryInstance &MemInst,
                       uint32_t Address, uint32_t Count) noexcept {
  if (!MemInst.isShared()) {
    return UINT32_C(0);
  }

  std::unique_lock<decltype(WaiterMapMutex)> Locker(WaiterMapMutex);
  uint32_t Total = 0;
  auto Range = WaiterMap.equal_range(Address);
  for (auto Iterator = Range.first; Total < Count && Iterator != Range.second;
       ++Iterator) {
    if (likely(&MemInst == Iterator->second.MemInst)) {
      Iterator->second.Cond.notify_all();
      ++Total;
    }
  }
  return Total;
}

void Executor::atomicNotifyAll() noexcept {
  std::unique_lock<decltype(WaiterMapMutex)> Locker(WaiterMapMutex);
  for (auto Iterator = WaiterMap.begin(); Iterator != WaiterMap.end();
       ++Iterator) {
    Iterator->second.Cond.notify_all();
  }
}

} // namespace Executor
} // namespace WasmEdge
