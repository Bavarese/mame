// license:BSD-3-Clause
// copyright-holders:hap
/*

  Hughes HLCD 0538(A)/0539(A) LCD Driver

*/

#ifndef MAME_VIDEO_HLCD0538_H
#define MAME_VIDEO_HLCD0538_H

#pragma once

// pinout reference

/*
               ____   ____
        +V  1 |*   \_/    | 40 R 1
   DATA IN  2 |           | 39 R 2
       CLK  3 |           | 38 R 3
       LCD  4 |           | 37 R 4
       GND  5 |           | 36 R 5
 INTERRUPT  6 |           | 35 R 6
      C 26  7 |           | 34 R 7
      C 25  8 |           | 33 R 8
      C 24  9 |           | 32 C 1
      C 23 10 | HLCD 0538 | 31 C 2
      C 22 11 |           | 30 C 3
      C 21 12 |           | 29 C 4
      C 20 13 |           | 28 C 5
      C 19 14 |           | 27 C 6
      C 18 15 |           | 26 C 7
      C 17 16 |           | 25 C 8
      C 16 17 |           | 24 C 9
      C 15 18 |           | 23 C 10
      C 14 19 |           | 22 C 11
      C 13 20 |___________| 21 C 12

    HLCD 0539 has 8 more C pins(1-8) in place of R pins.
*/


// C/R pins (0538: d0-d7 for rows)
#define MCFG_HLCD0538_WRITE_COLS_CB(_devcb) \
	devcb = &downcast<hlcd0538_device &>(*device).set_write_cols_callback(DEVCB_##_devcb);

// INTERRUPT pin
#define MCFG_HLCD0538_INTERRUPT_CB(_devcb) \
	devcb = &downcast<hlcd0538_device &>(*device).set_write_interrupt_callback(DEVCB_##_devcb);


class hlcd0538_device : public device_t
{
public:
	hlcd0538_device(const machine_config &mconfig, const char *tag, device_t *owner, u32 clock);

	// configuration helpers
	template <typename Object> devcb_base &set_write_cols_callback(Object &&cb) { return m_write_cols.set_callback(std::forward<Object>(cb)); }
	template <typename Object> devcb_base &set_write_interrupt_callback(Object &&cb) { return m_write_interrupt.set_callback(std::forward<Object>(cb)); }

	DECLARE_WRITE_LINE_MEMBER(write_clk);
	DECLARE_WRITE_LINE_MEMBER(write_lcd);
	DECLARE_WRITE_LINE_MEMBER(write_data) { m_data = (state) ? 1 : 0; }

protected:
	hlcd0538_device(const machine_config &mconfig, device_type type, const char *tag, device_t *owner, u32 clock);

	// device-level overrides
	virtual void device_start() override;

	int m_lcd;      // input pin state
	int m_clk;      // "
	int m_data;     // "
	u64 m_shift;

	// callbacks
	devcb_write64 m_write_cols;
	devcb_write_line m_write_interrupt;
};


class hlcd0539_device : public hlcd0538_device
{
public:
	hlcd0539_device(const machine_config &mconfig, const char *tag, device_t *owner, u32 clock);
};



DECLARE_DEVICE_TYPE(HLCD0538, hlcd0538_device)
DECLARE_DEVICE_TYPE(HLCD0539, hlcd0539_device)

#endif // MAME_VIDEO_HLCD0538_H
