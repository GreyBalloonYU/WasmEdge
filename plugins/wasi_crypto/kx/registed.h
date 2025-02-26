// SPDX-License-Identifier: Apache-2.0
// SPDX-FileCopyrightText: 2019-2022 Second State INC

//===-- wasmedge/plugins/wasi_crypto/kx/registed.h - Registed -------------===//
//
// Part of the WasmEdge Project.
//
//===----------------------------------------------------------------------===//
///
/// \file
/// This file contains the register key exchange algorithm definitions.
///
//===----------------------------------------------------------------------===//

#pragma once

#include "kx/dh/ecdsa.h"
#include "kx/dh/x25519.h"
#include "utils/error.h"

#include <variant>

namespace WasmEdge {
namespace Host {
namespace WasiCrypto {
namespace Kx {

template <typename... T> struct Registed {
  using PkVariant = std::variant<typename T::PublicKey...>;
  using SkVariant = std::variant<typename T::SecretKey...>;
  using KpVariant = std::variant<typename T::KeyPair...>;
  using Variant = std::variant<T...>;
};

using RegistedAlg = Registed<X25519, Ecdsa>;

using Algorithm = RegistedAlg::Variant;

} // namespace Kx
} // namespace WasiCrypto
} // namespace Host
} // namespace WasmEdge
