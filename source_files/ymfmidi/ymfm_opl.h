// BSD 3-Clause License
//
// Copyright (c) 2021, Aaron Giles
// All rights reserved.
//
// Redistribution and use in source and binary forms, with or without
// modification, are permitted provided that the following conditions are met:
//
// 1. Redistributions of source code must retain the above copyright notice, this
//    list of conditions and the following disclaimer.
//
// 2. Redistributions in binary form must reproduce the above copyright notice,
//    this list of conditions and the following disclaimer in the documentation
//    and/or other materials provided with the distribution.
//
// 3. Neither the name of the copyright holder nor the names of its
//    contributors may be used to endorse or promote products derived from
//    this software without specific prior written permission.
//
// THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS"
// AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE
// IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE
// DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT HOLDER OR CONTRIBUTORS BE LIABLE
// FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL
// DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR
// SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER
// CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY,
// OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE
// OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.

#ifndef YMFM_OPL_H
#define YMFM_OPL_H

#pragma once

#include "ymfm.h"
#include "ymfm_fm.h"

namespace ymfm
{

//*********************************************************
//  REGISTER CLASSES
//*********************************************************

// ======================> opl_registers_base

//
// OPL/OPL2/OPL3/OPL4 register map:
//
//      System-wide registers:
//           01 xxxxxxxx Test register
//              --x----- Enable OPL compatibility mode [OPL2 only] (1 = enable)
//           02 xxxxxxxx Timer A value (4 * OPN)
//           03 xxxxxxxx Timer B value
//           04 x------- RST
//              -x------ Mask timer A
//              --x----- Mask timer B
//              ------x- Load timer B
//              -------x Load timer A
//           08 x------- CSM mode [OPL/OPL2 only]
//              -x------ Note select
//           BD x------- AM depth
//              -x------ PM depth
//              --x----- Rhythm enable
//              ---x---- Bass drum key on
//              ----x--- Snare drum key on
//              -----x-- Tom key on
//              ------x- Top cymbal key on
//              -------x High hat key on
//          101 --xxxxxx Test register 2 [OPL3 only]
//          104 --x----- Channel 6 4-operator mode [OPL3 only]
//              ---x---- Channel 5 4-operator mode [OPL3 only]
//              ----x--- Channel 4 4-operator mode [OPL3 only]
//              -----x-- Channel 3 4-operator mode [OPL3 only]
//              ------x- Channel 2 4-operator mode [OPL3 only]
//              -------x Channel 1 4-operator mode [OPL3 only]
//          105 -------x New [OPL3 only]
//              ------x- New2 [OPL4 only]
//
//     Per-channel registers (channel in address bits 0-3)
//     Note that all these apply to address+100 as well on OPL3+
//        A0-A8 xxxxxxxx F-number (low 8 bits)
//        B0-B8 --x----- Key on
//              ---xxx-- Block (octvate, 0-7)
//              ------xx F-number (high two bits)
//        C0-C8 x------- CHD output (to DO0 pin) [OPL3+ only]
//              -x------ CHC output (to DO0 pin) [OPL3+ only]
//              --x----- CHB output (mixed right, to DO2 pin) [OPL3+ only]
//              ---x---- CHA output (mixed left, to DO2 pin) [OPL3+ only]
//              ----xxx- Feedback level for operator 1 (0-7)
//              -------x Operator connection algorithm
//
//     Per-operator registers (operator in bits 0-5)
//     Note that all these apply to address+100 as well on OPL3+
//        20-35 x------- AM enable
//              -x------ PM enable (VIB)
//              --x----- EG type
//              ---x---- Key scale rate
//              ----xxxx Multiple value (0-15)
//        40-55 xx------ Key scale level (0-3)
//              --xxxxxx Total level (0-63)
//        60-75 xxxx---- Attack rate (0-15)
//              ----xxxx Decay rate (0-15)
//        80-95 xxxx---- Sustain level (0-15)
//              ----xxxx Release rate (0-15)
//        E0-F5 ------xx Wave select (0-3) [OPL2 only]
//              -----xxx Wave select (0-7) [OPL3+ only]
//

template<int Revision>
class opl_registers_base : public fm_registers_base
{
	static constexpr bool IsOpl2 = (Revision == 2);
	static constexpr bool IsOpl2Plus = (Revision >= 2);
	static constexpr bool IsOpl3Plus = (Revision >= 3);
	static constexpr bool IsOpl4Plus = (Revision >= 4);

public:
	// constants
	static constexpr uint32_t OUTPUTS = IsOpl3Plus ? 4 : 1;
	static constexpr uint32_t CHANNELS = IsOpl3Plus ? 18 : 9;
	static constexpr uint32_t ALL_CHANNELS = (1 << CHANNELS) - 1;
	static constexpr uint32_t OPERATORS = CHANNELS * 2;
	static constexpr uint32_t WAVEFORMS = IsOpl3Plus ? 8 : (IsOpl2Plus ? 4 : 1);
	static constexpr uint32_t REGISTERS = IsOpl3Plus ? 0x200 : 0x100;
	static constexpr uint32_t REG_MODE = 0x04;
	static constexpr uint32_t DEFAULT_PRESCALE = IsOpl4Plus ? 19 : (IsOpl3Plus ? 8 : 4);
	static constexpr uint32_t EG_CLOCK_DIVIDER = 1;
	static constexpr uint32_t CSM_TRIGGER_MASK = ALL_CHANNELS;
	static constexpr bool DYNAMIC_OPS = IsOpl3Plus;
	static constexpr bool MODULATOR_DELAY = !IsOpl3Plus;
	static constexpr uint8_t STATUS_TIMERA = 0x40;
	static constexpr uint8_t STATUS_TIMERB = 0x20;
	static constexpr uint8_t STATUS_BUSY = 0;
	static constexpr uint8_t STATUS_IRQ = 0x80;

	// constructor
	opl_registers_base();

	// reset to initial state
	void reset();

	// save/restore
	void save_restore(ymfm_saved_state &state);

	// map channel number to register offset
	static constexpr uint32_t channel_offset(uint32_t chnum)
	{
		assert(chnum < CHANNELS);
		if (!IsOpl3Plus)
			return chnum;
		else
			return (chnum % 9) + 0x100 * (chnum / 9);
	}

	// map operator number to register offset
	static constexpr uint32_t operator_offset(uint32_t opnum)
	{
		assert(opnum < OPERATORS);
		if (!IsOpl3Plus)
			return opnum + 2 * (opnum / 6);
		else
			return (opnum % 18) + 2 * ((opnum % 18) / 6) + 0x100 * (opnum / 18);
	}

	// return an array of operator indices for each channel
	struct operator_mapping { uint32_t chan[CHANNELS]; };
	void operator_map(operator_mapping &dest) const;

	// OPL4 apparently can read back FM registers?
	uint8_t read(uint16_t index) const { return m_regdata[index]; }

	// handle writes to the register array
	bool write(uint16_t index, uint8_t data, uint32_t &chan, uint32_t &opmask);

	// clock the noise and LFO, if present, returning LFO PM value
	int32_t clock_noise_and_lfo();

	// reset the LFO
	void reset_lfo() { m_lfo_am_counter = m_lfo_pm_counter = 0; }

	// return the AM offset from LFO for the given channel
	// on OPL this is just a fixed value
	uint32_t lfo_am_offset(uint32_t choffs) const { return m_lfo_am; }

	// return LFO/noise states
	uint32_t noise_state() const { return m_noise_lfsr >> 23; }

	// caching helpers
	void cache_operator_data(uint32_t choffs, uint32_t opoffs, opdata_cache &cache);

	// compute the phase step, given a PM value
	uint32_t compute_phase_step(uint32_t choffs, uint32_t opoffs, opdata_cache const &cache, int32_t lfo_raw_pm);

	// log a key-on event
	std::string log_keyon(uint32_t choffs, uint32_t opoffs);

	// system-wide registers
	uint32_t test() const                            { return byte(0x01, 0, 8); }
	uint32_t waveform_enable() const                 { return IsOpl2 ? byte(0x01, 5, 1) : (IsOpl3Plus ? 1 : 0); }
	uint32_t timer_a_value() const                   { return byte(0x02, 0, 8) * 4; } // 8->10 bits
	uint32_t timer_b_value() const                   { return byte(0x03, 0, 8); }
	uint32_t status_mask() const                     { return byte(0x04, 0, 8) & 0x78; }
	uint32_t irq_reset() const                       { return byte(0x04, 7, 1); }
	uint32_t reset_timer_b() const                   { return byte(0x04, 7, 1) | byte(0x04, 5, 1); }
	uint32_t reset_timer_a() const                   { return byte(0x04, 7, 1) | byte(0x04, 6, 1); }
	uint32_t enable_timer_b() const                  { return 1; }
	uint32_t enable_timer_a() const                  { return 1; }
	uint32_t load_timer_b() const                    { return byte(0x04, 1, 1); }
	uint32_t load_timer_a() const                    { return byte(0x04, 0, 1); }
	uint32_t csm() const                             { return IsOpl3Plus ? 0 : byte(0x08, 7, 1); }
	uint32_t note_select() const                     { return byte(0x08, 6, 1); }
	uint32_t lfo_am_depth() const                    { return byte(0xbd, 7, 1); }
	uint32_t lfo_pm_depth() const                    { return byte(0xbd, 6, 1); }
	uint32_t rhythm_enable() const                   { return byte(0xbd, 5, 1); }
	uint32_t rhythm_keyon() const                    { return byte(0xbd, 4, 0); }
	uint32_t newflag() const                         { return IsOpl3Plus ? byte(0x105, 0, 1) : 0; }
	uint32_t new2flag() const                        { return IsOpl4Plus ? byte(0x105, 1, 1) : 0; }
	uint32_t fourop_enable() const                   { return IsOpl3Plus ? byte(0x104, 0, 6) : 0; }

	// per-channel registers
	uint32_t ch_block_freq(uint32_t choffs) const    { return word(0xb0, 0, 5, 0xa0, 0, 8, choffs); }
	uint32_t ch_feedback(uint32_t choffs) const      { return byte(0xc0, 1, 3, choffs); }
	uint32_t ch_algorithm(uint32_t choffs) const     { return byte(0xc0, 0, 1, choffs) | (IsOpl3Plus ? (8 | (byte(0xc3, 0, 1, choffs) << 1)) : 0); }
	uint32_t ch_output_any(uint32_t choffs) const    { return newflag() ? byte(0xc0 + choffs, 4, 4) : 1; }
	uint32_t ch_output_0(uint32_t choffs) const      { return newflag() ? byte(0xc0 + choffs, 4, 1) : 1; }
	uint32_t ch_output_1(uint32_t choffs) const      { return newflag() ? byte(0xc0 + choffs, 5, 1) : (IsOpl3Plus ? 1 : 0); }
	uint32_t ch_output_2(uint32_t choffs) const      { return newflag() ? byte(0xc0 + choffs, 6, 1) : 0; }
	uint32_t ch_output_3(uint32_t choffs) const      { return newflag() ? byte(0xc0 + choffs, 7, 1) : 0; }

	// per-operator registers
	uint32_t op_lfo_am_enable(uint32_t opoffs) const { return byte(0x20, 7, 1, opoffs); }
	uint32_t op_lfo_pm_enable(uint32_t opoffs) const { return byte(0x20, 6, 1, opoffs); }
	uint32_t op_eg_sustain(uint32_t opoffs) const    { return byte(0x20, 5, 1, opoffs); }
	uint32_t op_ksr(uint32_t opoffs) const           { return byte(0x20, 4, 1, opoffs); }
	uint32_t op_multiple(uint32_t opoffs) const      { return byte(0x20, 0, 4, opoffs); }
	uint32_t op_ksl(uint32_t opoffs) const           { uint32_t temp = byte(0x40, 6, 2, opoffs); return bitfield(temp, 1) | (bitfield(temp, 0) << 1); }
	uint32_t op_total_level(uint32_t opoffs) const   { return byte(0x40, 0, 6, opoffs); }
	uint32_t op_attack_rate(uint32_t opoffs) const   { return byte(0x60, 4, 4, opoffs); }
	uint32_t op_decay_rate(uint32_t opoffs) const    { return byte(0x60, 0, 4, opoffs); }
	uint32_t op_sustain_level(uint32_t opoffs) const { return byte(0x80, 4, 4, opoffs); }
	uint32_t op_release_rate(uint32_t opoffs) const  { return byte(0x80, 0, 4, opoffs); }
	uint32_t op_waveform(uint32_t opoffs) const      { return IsOpl2Plus ? byte(0xe0, 0, newflag() ? 3 : 2, opoffs) : 0; }

protected:
	// return a bitfield extracted from a byte
	uint32_t byte(uint32_t offset, uint32_t start, uint32_t count, uint32_t extra_offset = 0) const
	{
		return bitfield(m_regdata[offset + extra_offset], start, count);
	}

	// return a bitfield extracted from a pair of bytes, MSBs listed first
	uint32_t word(uint32_t offset1, uint32_t start1, uint32_t count1, uint32_t offset2, uint32_t start2, uint32_t count2, uint32_t extra_offset = 0) const
	{
		return (byte(offset1, start1, count1, extra_offset) << count2) | byte(offset2, start2, count2, extra_offset);
	}

	// helper to determine if the this channel is an active rhythm channel
	bool is_rhythm(uint32_t choffs) const
	{
		return rhythm_enable() && (choffs >= 6 && choffs <= 8);
	}

	// internal state
	uint16_t m_lfo_am_counter;            // LFO AM counter
	uint16_t m_lfo_pm_counter;            // LFO PM counter
	uint32_t m_noise_lfsr;                // noise LFSR state
	uint8_t m_lfo_am;                     // current LFO AM value
	uint8_t m_regdata[REGISTERS];         // register data
	uint16_t m_waveform[WAVEFORMS][WAVEFORM_LENGTH]; // waveforms
};

using opl_registers = opl_registers_base<1>;
using opl2_registers = opl_registers_base<2>;
using opl3_registers = opl_registers_base<3>;
using opl4_registers = opl_registers_base<4>;

//*********************************************************
//  OPL3 IMPLEMENTATION CLASSES
//*********************************************************

// ======================> ymf262

class ymf262
{
public:
	using fm_engine = fm_engine_base<opl3_registers>;
	using output_data = fm_engine::output_data;
	static constexpr uint32_t OUTPUTS = fm_engine::OUTPUTS;

	// constructor
	ymf262(ymfm_interface &intf);

	// reset
	void reset();

	// save/restore
	void save_restore(ymfm_saved_state &state);

	// pass-through helpers
	uint32_t sample_rate(uint32_t input_clock) const { return m_fm.sample_rate(input_clock); }
	void invalidate_caches() { m_fm.invalidate_caches(); }

	// read access
	uint8_t read_status();
	uint8_t read(uint32_t offset);

	// write access
	void write_address(uint8_t data);
	void write_data(uint8_t data);
	void write_address_hi(uint8_t data);
	void write(uint32_t offset, uint8_t data);

	// generate samples of sound
	void generate(output_data *output, uint32_t numsamples = 1);

protected:
	// internal state
	uint16_t m_address;              // address register
	fm_engine m_fm;                  // core FM engine
};

}

#endif // YMFM_OPL_H
