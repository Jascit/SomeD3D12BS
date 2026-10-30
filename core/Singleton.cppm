/* SPDX-License-Identifier: Apache-2.0 */

/**
 * @file    Singleton.cppm
 * @brief
 *
 * @date    11/10/2026
 *
 * @copyright Copyright (c) 2026 The Project Contributors
 */

export module core.singleton;

export namespace core {
template<typename Derivered>
class Singleton {
  friend typename Derivered;
  using base = Singleton<Derivered>;
public:
  Singleton(const Singleton &) = delete;
  Singleton(Singleton &&) noexcept = delete;
  Singleton &operator=(const Singleton &) = delete;
  Singleton &operator=(Singleton &&) noexcept = delete;

  static Derivered &instance() noexcept {
    static Derivered instance;
    return instance;
  }

private:
  Singleton() noexcept = default;
  virtual ~Singleton() = default;
};
} // namespace core
