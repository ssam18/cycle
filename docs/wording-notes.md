<!--
SPDX-License-Identifier: Apache-2.0 WITH LLVM-exception
-->

# P3806R0 wording findings

Issues found while implementing `views::cycle` against the proposed wording in
[P3806R0](https://www.open-std.org/jtc1/sc22/wg21/docs/papers/2025/p3806r0.html).
These are intended for the paper author / LEWG and tracked here so they travel
with the implementation.

## 1. `operator+=` mis-handles negative `n` that crosses a cycle boundary (substantive)

§[range.cycle.iterator] p13 specifies:

```cpp
const auto first = ranges::begin(parent_->base_);
const auto dist = ranges::distance(parent_->base_);
const auto offset = current_ - first;
const auto new_n = n + offset;
const auto new_offset = new_n % dist;
n_ += new_n / dist;
current_ = first + range_difference_t<Base>(new_offset >= 0 ? new_offset : new_offset + dist);
return *this;
```

C++ `/` and `%` are *truncated* (round toward zero). For negative `new_n` whose
remainder is non-zero, this leaves `n_` off by one lap because the offset gets
shifted up by `dist` (to make it non-negative) without a compensating
decrement of the lap counter.

**Concrete repro.** Let `dist = 3`, current state `n_ = 5`, `offset = 1`
(logical index = 16). Execute `*this += -5`:

| Step                                              | Paper's wording      | What's needed         |
|---------------------------------------------------|----------------------|-----------------------|
| `new_n = n + offset`                              | `-5 + 1 = -4`        | same                  |
| `new_offset = new_n % dist`                       | `-4 % 3 = -1`        | same                  |
| `n_ += new_n / dist`                              | `n_ += -4/3 = -1` → `n_ = 4` | `n_ += floor(-4/3) = -2` → `n_ = 3` |
| `current_ = first + (new_offset>=0 ? … : … + dist)` | `first + 2`        | same                  |
| **logical index = n_·dist + (current_ - first)**  | `4·3 + 2 = **14**`   | `3·3 + 2 = **11**` ✓  |

Expected post-state: `16 + (-5) = 11`. Paper produces 14 — off by one full
lap forward.

This also makes the `(x.n_ - y.n_) * dist + x.current_ - y.current_` formula
in p22 (`operator-`) inconsistent when one operand was produced by a
negative-`+=` that crossed a boundary, because that formula assumes the
invariant `0 ≤ (current_ - first) < dist` on both sides.

### Suggested wording fix

The least-invasive fix is to keep p13 textually identical except for one extra
line correcting the lap counter when the offset is shifted up by `dist`:

```cpp
n_ += new_n / dist;
if (new_offset < 0) --n_;                                         // <-- added
current_ = first + range_difference_t<Base>(new_offset >= 0 ? new_offset : new_offset + dist);
```

A clearer alternative restructures the body to use floor-division semantics
explicitly (this is what the Beman implementation does):

```cpp
const auto first  = ranges::begin(parent_->base_);
const auto dist   = ranges::distance(parent_->base_);
const auto offset = current_ - first;
const auto new_n  = n + offset;
auto quot = new_n / dist;
auto rem  = new_n % dist;
if (rem < 0) { rem += dist; --quot; }   // floor-division correction
n_       += quot;
current_  = first + range_difference_t<Base>(rem);
return *this;
```

Either form preserves the invariant `0 ≤ current_ - first < dist` and keeps
`operator-` correct.

## 2. `operator==` typo (editorial)

§[range.cycle.iterator] p15 reads:

> *Returns:* `x.n_ == y.n_ && x.current_ == x.current_`.

The second `x.current_` should be `y.current_`.

## 3. `operator--(int)` constraint has `requires requires` (editorial)

§[range.cycle.iterator] p12 declares:

```cpp
constexpr iterator& operator--(int)
  requires requires bidirectional-common<Base> || sized-random-access-range<Base>;
//        ^^^^^^^^^
```

The duplicated `requires` keyword should be removed (compare with `operator--()`
in p11, which has the single-`requires` form).

## 4. `iterator(Parent&, iterator_t<Base>)` ctor body (editorial)

§[range.cycle.iterator] p5 reads:

> *Effects:* Initializes `current_` with `std::move(current_)` and `parent_`
> with `addressof(parent)`.

`std::move(current_)` should be `std::move(current)` — i.e. move from the
parameter, not from the (still-uninitialized) member.
