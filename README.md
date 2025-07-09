# 🛡️ Modified Additive Veto Protocol (AVP)

This repository contains a **Modified Additive Veto Protocol (AVP)** implemented in C++ using Boost's shared memory. The protocol is designed for **privacy-preserving voting**, where any participant can veto a decision without revealing their vote.

This version improves over traditional AVP designs by incorporating:
- Ring-LWE-style polynomial arithmetic
- Adaptive noise scaling for better error handling
- Scalable shared memory coordination

---

## 🔍 What is AVP?

**AVP** is a lightweight protocol to securely evaluate a single-bit vote (yes = 0, veto = 1) among `n` parties:

- If all vote `0`, the result is **clearly `0`**
- If even one party votes `1`, the result is **noisy**, interpreted as veto (`1`)
- Individual votes are never revealed

---

## 🔧 What's New in This Modified AVP?

Compared to the original AVP, this modified version:

✅ Uses **polynomials over ℤ<sub>q</sub>[x]/(xⁿ + 1)**  
✅ Adds **structured noise** computed from theoretical `zeta` bounds  
✅ Leverages **infinity norm** for precise vote decoding  
✅ Scales efficiently up to 100+ parties using shared memory  
✅ Is based on **RLWE** (Ring Learning With Errors), providing **post-quantum security**

---

## 🧠 Protocol Summary

1. **Each party generates a secret `sᵢ` and noise `eᵢ`**
2. Computes a public value `bᵢ = a ⋅ sᵢ + eᵢ`
3. Computes `yᵢ = sum(bⱼ) - sum(bₖ)` for balancing
4. Encodes their vote:
   - If vote = `0`: `zᵢ = sᵢ ⋅ yᵢ + eᵢ` (low noise)
   - If vote = `1`: `zᵢ = high_noise_poly()` (adds noise)
5. Final `z = Σ zᵢ` is tallied
   - If ∞-norm is low → vote = `0`
   - If ∞-norm is high → at least one `1` (veto)

---

## 📁 Files Overview

| File               | Role                                             |
|--------------------|--------------------------------------------------|
| `avpmodclean.cpp`  | Clears all shared memory segments                |
| `avpmodinit.cpp`   | Initializes public polynomial `a(x)`             |
| `avpgenb.cpp`      | Generates `bᵢ = a ⋅ sᵢ + eᵢ` and stores `sᵢ`, `bᵢ` |
| `avpgeny.cpp`      | Computes balancing vector `yᵢ`                   |
| `avpencode.cpp`    | Encodes vote using `zᵢ = sᵢ ⋅ yᵢ + eᵢ` or noise   |
| `avpmodtally.cpp`  | Tallies votes and computes final decision        |
| `find_max_m.py`    | Benchmarks how many parties the system can handle |

---

## ⚙️ Build Instructions

All C++ files require Boost (for shared memory).

### 🔧 Compile All C++ Programs

```bash
g++ avpmodclean.cpp -o avpmodclean -lrt -lpthread
g++ avpmodinit.cpp -o avpmodinit -lrt -lpthread
g++ avpgenb.cpp -o avpgenb -lrt -lpthread
g++ avpgeny.cpp -o avpgeny -lrt -lpthread
g++ avpencode.cpp -o avpencode -lrt -lpthread
g++ avpmodtally.cpp -o avpmodtally -lrt -lpthread
