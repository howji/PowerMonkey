# Thermal Velocity Boost (TVB) OC-Mailbox — Research Notes

## TL;DR / Confidence statement

**The exact OC-Mailbox (MSR `0x150`) command ID and data-bit layout for Thermal
Velocity Boost are NOT authoritatively documented in any public source I could
find.** The values currently in the code:

```c
#define OC_MBOX_CMD_TVB                 0x14   // <-- UNVERIFIED, placeholder
#define OC_TVB_RATIO_CLIPPING_DISABLE   bit0u32
#define OC_TVB_VOLTAGE_OPT_DISABLE      bit1u32
```

are an **educated placeholder**, not a confirmed mapping. They MUST be verified
against your CPU's BWG (BIOS Writer's Guide) / FSP before being relied upon.
This file documents what was searched, what was actually found, and what is
inference vs. fact — so the gap is explicit and not mistaken for a sourced
result.

---

## What I was trying to find

A way to enable/disable Intel Thermal Velocity Boost (TVB) — specifically its
two sub-behaviors:

- **TVB Ratio Clipping** — pcode reduces max turbo ratio as the CPU gets hot.
- **TVB Voltage Optimization** — pcode reduces voltage when temperature is below
  TjMax.

The user requested this be driven through the **OC Mailbox** (MSR `0x150`), the
same mailbox PowerMonkey already uses for V/F overrides and IccMax.

---

## How the search was done

1. **Read the existing codebase first** to learn the established OC-mailbox
   conventions, so any new command would follow the same pattern:
   - `OcMailbox.c` — mailbox transport (`OcMailbox_BuildInterface`,
     `OcMailbox_ReadWrite`, busy/completion masks).
   - `VFTuning.c` — real command IDs already in use:
     - `0x10` = read V/F info
     - `0x11` = write V/F
     - `0x16` = read IccMax
     - `0x17` = write IccMax
   - This established that command IDs live in bits `[7:0]` of the interface
     word and that pcode returns a status byte (unsupported commands fail
     gracefully — the code already relies on this when probing V/F points).

2. **Web search** for the specific mapping:
   - Query: `Intel OC mailbox 0x150 command TVB ratio clipping voltage
     optimization disable`
   - Query: `OC mailbox command 0x150 GET_SET_TVB ... ratio clipping voltage
     optimization bit FSP CometLake`

3. **Targeted fetch** of the most authoritative-looking hit
   (SkatterBencher's TVB deep-dive) to try to extract command numbers/bits.

---

## What was actually found (and what it does NOT tell us)

| Source | What it confirmed | What it did NOT give |
|--------|-------------------|----------------------|
| Existing PowerMonkey code (`VFTuning.c`) | OC-mailbox command/encoding convention; cmd in `[7:0]`; status is checked | Any TVB-specific command |
| SkatterBencher — *Intel Overclocking Thermal Velocity Boost* | TVB concept: ratio clipping + voltage optimization; that BIOS/XTU expose enable/disable | **No** MSR command IDs or bit positions (explicitly conceptual only) |
| Intel FSP docs (CoffeeLake/KabyLake `FSP_M_CONFIG`) | TVB is exposed as FSP UPDs `TvbRatioClipping` / `TvbVoltageOptimization`, each `0=disable / 1=enable` | How the FSP applies them at the MSR/mailbox level (internal to pcode) |
| Forum/guide hits (ROG, OcUK, tenforums) | TVB voltage optimization default = enabled; people disable TVB for fixed-ratio OC | No register-level detail |
| Linux kernel patch *"x86/msr: do not warn on writes to OC_MAILBOX"* | `0x150` is the OC mailbox and is written for OC tuning | No TVB command map |
| One OC guide snippet | Mentioned setting `0x150` bit 63 = 1 and bits `[39:32]` = `0x18` for a per-core operation; another note tied cmd `0x1B` to AVX ratio offset | These are *different* operations, not TVB — but they show the `[39:32]` = command-byte framing |

### Key honest conclusion
The TVB knobs are surfaced to integrators as **FSP UPDs**, and pcode applies
them internally. Intel does **not** publish the `0x150` command/bit encoding for
TVB the way it (semi-)documents V/F and IccMax. So there is no public primary
source for `OC_MBOX_CMD_TVB`. The `0x14` value is a plausible-but-unconfirmed
guess; it was deliberately chosen as a clearly-flagged constant and the feature
is safe-by-construction because:

- The mailbox returns an error completion code for unsupported commands, which
  the code checks (`EFI_ERROR(...) -> EFI_ABORTED`).
- The residual risk is that `0x14` collides with a *different valid* command on
  a given CPU — which is exactly why verification is required before use.

---

## How to actually verify (recommended order)

1. **Intel BWG / FSP Integration Guide for your exact CPU family** (requires
   Intel NDA/partner access). Look for how `TvbRatioClipping` and
   `TvbVoltageOptimization` UPDs are translated into mailbox writes.
2. **FSP binary / reference code** (e.g. `CometLakeFspBinPkg`) — trace the UPD
   consumers to the `0x150` write site.
3. **Vendor BIOS dump / UEFI module RE** — disassemble the SA/PM init module and
   find writes to MSR `0x150` gated on the TVB setup options.
4. **Cross-check with XTU / ThrottleStop behavior** on a test rig (debug rig
   only — never production) by reading back `0x150` completion status.

Until one of the above confirms it, treat the code path as **disabled-by-intent**
even though the config defaults to `1` (stock/enabled) — i.e. do not assume it
is doing anything until a read-back proves the command is accepted.

---

## Sources

- SkatterBencher — *Intel Overclocking Thermal Velocity Boost*:
  https://skatterbencher.com/intel-overclocking-thermal-velocity-boost/
- CoffeeLake FSP `FSP_M_CONFIG` reference (TVB UPDs):
  https://documentation.help/CoffeeLake-Intel-Firmware/struct_f_s_p___m___c_o_n_f_i_g.html
- KabyLake FSP `FSP_M_CONFIG` reference:
  https://documentation.help/Kabylake-Intel-Firmware/struct_f_s_p___m___c_o_n_f_i_g.html
- Linux kernel patch — *x86/msr: do not warn on writes to OC_MAILBOX*:
  https://www.mail-archive.com/linux-kernel@vger.kernel.org/msg2296631.html
- Academic layout of MSR `0x150` (referenced in PowerMonkey source):
  DOI 10.1109/SP40000.2020.00057 ("Plundervolt")

> Note: none of the above is a primary source for the TVB command ID / bit
> layout. The mapping in `VFTuning.h` remains **inferred and unverified**.
