// license:BSD-3-Clause
// copyright-holders:Nicola Salmoria
/***************************************************************************

    Sega Zaxxon hardware

****************************************************************************

    Sound interface is provided by an 8255. The 8255 is a parallel peripheral
    interface, used also in Scramble. It has three 8-bit outputs.
    All sounds are generated by discrete circuits. Each sound is triggered by
    an output pin of the 8255.

    Zaxxon Sound Information: (from the schematics)
    by Frank Palazzolo

    There are four registers in the 8255. they are mapped to
    (111x xxxx 0011 11pp) by Zaxxon.  Zaxxon writes to these
    at FF3C-FF3F.

    There are three modes of the 8255, but by the schematics I
    can see that Zaxxon is using "Mode 0", which is very simple.

    Important Note:
    These are all Active-Low outputs.
    A 1 De-activates the sound, while a 0 Activates/Triggers it

    Port A Output:
    FF3C bit7 Battleship
         bit6 Laser
         bit5 Base Missle
         bit4 Homing Missle
         bit3 Player Ship D
         bit2 Player Ship C
         bit1 Player Ship B
         bit0 Player Ship A

    Port B Output:
    FF3D bit7 Cannon
         bit6 N/C
         bit5 M-Exp
         bit4 S-Exp
         bit3 N/C
         bit2 N/C
         bit1 N/C
         bit0 N/C

    Port C Output:
    FF3E bit7 N/C
         bit6 N/C
         bit5 N/C
         bit4 N/C
         bit3 Alarm 3
         bit2 Alarm 2
         bit1 N/C
         bit0 Shot

    Control Byte:
    FF3F Should be written an 0x80 for Mode 0
         (Very Simple) operation of the 8255

***************************************************************************/

#include "emu.h"
#include "sound/samples.h"
#include "includes/zaxxon.h"



/*************************************
 *
 *  Zaxxon sound hardware description
 *
 *************************************/

static const char *const zaxxon_sample_names[] =
{
	"*zaxxon",
	"03",   /* 0 - Homing Missile */
	"02",   /* 1 - Base Missile */
	"01",   /* 2 - Laser (force field) */
	"00",   /* 3 - Battleship (end of level boss) */
	"11",   /* 4 - S-Exp (enemy explosion) */
	"10",   /* 5 - M-Exp (ship explosion) */
	"08",   /* 6 - Cannon (ship fire) */
	"23",   /* 7 - Shot (enemy fire) */
	"21",   /* 8 - Alarm 2 (target lock) */
	"20",   /* 9 - Alarm 3 (low fuel) */
	"05",   /* 10 - initial background noise */
	"04",   /* 11 - looped asteroid noise */
	nullptr
};


void zaxxon_state::zaxxon_samples(machine_config &config)
{
	SAMPLES(config, m_samples);
	m_samples->set_channels(12);
	m_samples->set_samples_names(zaxxon_sample_names);
	m_samples->add_route(ALL_OUTPUTS, "speaker", 0.25);
}



/*************************************
 *
 *  Zaxxon PPI write handlers
 *
 *************************************/

WRITE8_MEMBER(zaxxon_state::zaxxon_sound_a_w)
{
	uint8_t diff = data ^ m_sound_state[0];
	m_sound_state[0] = data;

	/* PLAYER SHIP A/B: volume */
	m_samples->set_volume(10, 0.5 + 0.157 * (data & 0x03));
	m_samples->set_volume(11, 0.5 + 0.157 * (data & 0x03));

	/* PLAYER SHIP C: channel 10 */
	if ((diff & 0x04) && !(data & 0x04)) m_samples->start(10, 10, true);
	if ((diff & 0x04) &&  (data & 0x04)) m_samples->stop(10);

	/* PLAYER SHIP D: channel 11 */
	if ((diff & 0x08) && !(data & 0x08)) m_samples->start(11, 11, true);
	if ((diff & 0x08) &&  (data & 0x08)) m_samples->stop(11);

	/* HOMING MISSILE: channel 0 */
	if ((diff & 0x10) && !(data & 0x10)) m_samples->start(0, 0, true);
	if ((diff & 0x10) &&  (data & 0x10)) m_samples->stop(0);

	/* BASE MISSILE: channel 1 */
	if ((diff & 0x20) && !(data & 0x20)) m_samples->start(1, 1);

	/* LASER: channel 2 */
	if ((diff & 0x40) && !(data & 0x40)) m_samples->start(2, 2, true);
	if ((diff & 0x40) &&  (data & 0x40)) m_samples->stop(2);

	/* BATTLESHIP: channel 3 */
	if ((diff & 0x80) && !(data & 0x80)) m_samples->start(3, 3, true);
	if ((diff & 0x80) &&  (data & 0x80)) m_samples->stop(3);
}


WRITE8_MEMBER(zaxxon_state::zaxxon_sound_b_w)
{
	uint8_t diff = data ^ m_sound_state[1];
	m_sound_state[1] = data;

	/* S-EXP: channel 4 */
	if ((diff & 0x10) && !(data & 0x10)) m_samples->start(4, 4);

	/* M-EXP: channel 5 */
	if ((diff & 0x20) && !(data & 0x20) && !m_samples->playing(5)) m_samples->start(5, 5);

	/* CANNON: channel 6 */
	if ((diff & 0x80) && !(data & 0x80)) m_samples->start(6, 6);
}


WRITE8_MEMBER(zaxxon_state::zaxxon_sound_c_w)
{
	uint8_t diff = data ^ m_sound_state[2];
	m_sound_state[2] = data;

	/* SHOT: channel 7 */
	if ((diff & 0x01) && !(data & 0x01)) m_samples->start(7, 7);

	/* ALARM2: channel 8 */
	if ((diff & 0x04) && !(data & 0x04)) m_samples->start(8, 8);

	/* ALARM3: channel 9 */
	if ((diff & 0x08) && !(data & 0x08) && !m_samples->playing(9)) m_samples->start(9, 9);
}



/*************************************
 *
 *  Congo sound hardware description
 *
 *************************************/

static const char *const congo_sample_names[] =
{
	"*congo",
	"gorilla",  /* 0 */
	"bass",     /* 1 */
	"congal",   /* 2 */
	"congah",   /* 3 */
	"rim",      /* 4 */
	nullptr
};


void zaxxon_state::congo_samples(machine_config &config)
{
	SAMPLES(config, m_samples);
	m_samples->set_channels(5);
	m_samples->set_samples_names(congo_sample_names);
	m_samples->add_route(ALL_OUTPUTS, "speaker", 0.25);
}



/*************************************
 *
 *  Congo PPI write handlers
 *
 *************************************/

WRITE8_MEMBER(zaxxon_state::congo_sound_b_w)
{
	uint8_t diff = data ^ m_sound_state[1];
	m_sound_state[1] = data;

	/* bit 7 = mute */

	/* GORILLA: channel 0 */
	if ((diff & 0x02) && !(data & 0x02) && !m_samples->playing(0)) m_samples->start(0, 0);
}


WRITE8_MEMBER(zaxxon_state::congo_sound_c_w)
{
	uint8_t diff = data ^ m_sound_state[2];
	m_sound_state[2] = data;

	/* BASS DRUM: channel 1 */
	if ((diff & 0x01) && !(data & 0x01)) m_samples->start(1, 1);
	if ((diff & 0x01) &&  (data & 0x01)) m_samples->stop(1);

	/* CONGA (LOW): channel 2 */
	if ((diff & 0x02) && !(data & 0x02)) m_samples->start(2, 2);
	if ((diff & 0x02) &&  (data & 0x02)) m_samples->stop(2);

	/* CONGA (HIGH): channel 3 */
	if ((diff & 0x04) && !(data & 0x04)) m_samples->start(3, 3);
	if ((diff & 0x04) &&  (data & 0x04)) m_samples->stop(3);

	/* RIM: channel 4 */
	if ((diff & 0x08) && !(data & 0x08)) m_samples->start(4, 4);
	if ((diff & 0x08) &&  (data & 0x08)) m_samples->stop(4);
}
