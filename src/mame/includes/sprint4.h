// license:BSD-3-Clause
// copyright-holders:Stefan Jokisch
#ifndef MAME_INCLUDES_SPRINT4_H
#define MAME_INCLUDES_SPRINT4_H

#pragma once

#include "machine/watchdog.h"
#include "sound/discrete.h"
#include "emupal.h"
#include "screen.h"
#include "tilemap.h"

class sprint4_state : public driver_device
{
public:
	enum
	{
		TIMER_NMI
	};

	sprint4_state(const machine_config &mconfig, device_type type, const char *tag) :
		driver_device(mconfig, type, tag),
		m_maincpu(*this, "maincpu"),
		m_watchdog(*this, "watchdog"),
		m_discrete(*this, "discrete"),
		m_gfxdecode(*this, "gfxdecode"),
		m_screen(*this, "screen"),
		m_palette(*this, "palette"),
		m_videoram(*this, "videoram")
	{ }

	void sprint4(machine_config &config);

	template <int N> DECLARE_READ_LINE_MEMBER(lever_r);
	template <int N> DECLARE_READ_LINE_MEMBER(wheel_r);
	template <int N> DECLARE_READ_LINE_MEMBER(collision_flipflop_r);

private:
	uint8_t wram_r(offs_t offset);
	uint8_t analog_r(offs_t offset);
	uint8_t coin_r(offs_t offset);
	uint8_t collision_r(offs_t offset);
	uint8_t options_r(offs_t offset);
	void wram_w(offs_t offset, uint8_t data);
	void collision_reset_w(offs_t offset, uint8_t data);
	void da_latch_w(uint8_t data);
	void video_ram_w(offs_t offset, uint8_t data);
	void bang_w(uint8_t data);
	void attract_w(uint8_t data);

	virtual void machine_start() override;
	virtual void machine_reset() override;
	virtual void video_start() override;
	virtual void device_timer(emu_timer &timer, device_timer_id id, int param, void *ptr) override;
	void sprint4_palette(palette_device &palette) const;

	TILE_GET_INFO_MEMBER(tile_info);
	uint32_t screen_update(screen_device &screen, bitmap_ind16 &bitmap, const rectangle &cliprect);
	DECLARE_WRITE_LINE_MEMBER(screen_vblank);
	TIMER_CALLBACK_MEMBER(nmi_callback);

	void sprint4_cpu_map(address_map &map);

	required_device<cpu_device> m_maincpu;
	required_device<watchdog_timer_device> m_watchdog;
	required_device<discrete_sound_device> m_discrete;
	required_device<gfxdecode_device> m_gfxdecode;
	required_device<screen_device> m_screen;
	required_device<palette_device> m_palette;

	required_shared_ptr<uint8_t> m_videoram;

	int m_da_latch;
	int m_steer_FF1[4];
	int m_steer_FF2[4];
	int m_gear[4];
	uint8_t m_last_wheel[4];
	int m_collision[4];
	tilemap_t* m_playfield;
	bitmap_ind16 m_helper;
	emu_timer *m_nmi_timer;
};

#endif // MAME_INCLUDES_SPRINT4_H
