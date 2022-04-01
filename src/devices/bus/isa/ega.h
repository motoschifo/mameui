// license:BSD-3-Clause
// copyright-holders:Wilbert Pol
#ifndef MAME_BUS_ISA_EGA_H
#define MAME_BUS_ISA_EGA_H

#pragma once

#include "isa.h"
#include "video/crtc_ega.h"
#include "emupal.h"

//**************************************************************************
//  TYPE DEFINITIONS
//**************************************************************************

// ======================> isa8_ega_device

class isa8_ega_device :
		public device_t,
		public device_isa8_card_interface
{
public:
	// construction/destruction
	isa8_ega_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock);

	uint8_t read(offs_t offset);
	void write(offs_t offset, uint8_t data);
	uint8_t pc_ega8_3b0_r(offs_t offset);
	void pc_ega8_3b0_w(offs_t offset, uint8_t data);
	uint8_t pc_ega8_3c0_r(offs_t offset);
	void pc_ega8_3c0_w(offs_t offset, uint8_t data);
	uint8_t pc_ega8_3d0_r(offs_t offset);
	void pc_ega8_3d0_w(offs_t offset, uint8_t data);

	CRTC_EGA_ROW_UPDATE(pc_ega_graphics);
	CRTC_EGA_ROW_UPDATE(pc_ega_text);

protected:
	isa8_ega_device(const machine_config &mconfig, device_type type, const char *tag, device_t *owner, uint32_t clock);

	// device-level overrides
	virtual void device_start() override;
	virtual void device_reset() override;

	// optional information overrides
	virtual void device_add_mconfig(machine_config &config) override;
	virtual const tiny_rom_entry *device_rom_region() const override;
	virtual ioport_constructor device_input_ports() const override;

	uint8_t alu_op( uint8_t data, uint8_t latch_data );

private:
	DECLARE_WRITE_LINE_MEMBER(de_changed);
	DECLARE_WRITE_LINE_MEMBER(hsync_changed);
	DECLARE_WRITE_LINE_MEMBER(vsync_changed);
	DECLARE_WRITE_LINE_MEMBER(vblank_changed);

	CRTC_EGA_ROW_UPDATE(ega_update_row);

public:
	required_device<crtc_ega_device> m_crtc_ega;

	void install_banks();
	void change_mode();
	void pc_ega8_3X0_w(offs_t offset, uint8_t data);
	uint8_t pc_ega8_3X0_r(offs_t offset);

	/* Video memory and related variables */
	std::unique_ptr<uint8_t[]> m_vram;
	uint8_t   *m_plane[4]{};
	uint8_t   m_read_latch[4]{};
	uint8_t   *m_videoram = nullptr;
	uint8_t   *m_charA = nullptr;
	uint8_t   *m_charB = nullptr;

	/* Registers */
	uint8_t   m_misc_output = 0;
	uint8_t   m_feature_control = 0;

	/* Attribute registers AR00 - AR14
	*/
	struct {
		uint8_t   index = 0;
		uint8_t   data[32]{};
		uint8_t   index_write = 0;
	} m_attribute;

	/* Sequencer registers SR00 - SR04
	*/
	struct {
		uint8_t   index = 0;
		uint8_t   data[8]{};
	} m_sequencer;

	/* Graphics controller registers GR00 - GR08
	*/
	struct {
		uint8_t   index = 0;
		uint8_t   data[16]{};
	} m_graphics_controller;

	uint8_t   m_frame_cnt = 0;
	uint8_t   m_hsync = 0;
	uint8_t   m_vsync = 0;
	uint8_t   m_vblank = 0;
	uint8_t   m_display_enable = 0;
	int     m_video_mode = 0;
	required_device<palette_device> m_palette;
};


// device type definition
DECLARE_DEVICE_TYPE(ISA8_EGA, isa8_ega_device)

#endif // MAME_BUS_ISA_EGA_H
