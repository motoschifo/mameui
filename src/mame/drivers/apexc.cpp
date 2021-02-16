// license:GPL-2.0+
// copyright-holders:Raphael Nabet, Robbbert
/*
    drivers/apexc.c : APEXC driver

    By Raphael Nabet

    see cpu/apexc.c for background and tech info
*/

#include "emu.h"
#include "includes/apexc.h"

/*static*/ const device_timer_id apexc_state::TIMER_POLL_INPUTS = 1;

void apexc_state::machine_start()
{
	teletyper_init();

	m_input_timer = timer_alloc(TIMER_POLL_INPUTS);
	m_input_timer->adjust(attotime::from_hz(60), 0, attotime::from_hz(60));
}

/*
    Punch a tape character
*/

void apexc_state::tape_write(uint8_t data)
{
	m_tape_puncher->write(data);
	teletyper_putchar(data & 0x1f); /* display on 'screen' */
}

/*
    APEXC control panel

    I know really little about the details, although the big picture is obvious.

    Things I know :
    * "As well as starting and stopping the machine, [it] enables information to be inserted
    manually and provides for the inspection of the contents of the memory via various
    storage registers." (Booth, p. 2)
    * "Data can be inserted manually from the control panel [into the control register]".
    (Booth, p. 3)
    * The contents of the R register can be edited, too.  A button allows to clear
    a complete X (or Y ???) field.  (forgot the reference, but must be somewhere in Booth)
    * There is no trace mode (Booth, p. 213)

    Since the control panel is necessary for the operation of the APEXC, I tried to
    implement a commonplace control panel.  I cannot tell how close the feature set and
    operation of this control panel is to the original APEXC control panel, but it
    cannot be too different in the basic principles.
*/


#if 0
/* defines for input port numbers */
enum
{
	panel_control = 0,
	panel_edit1,
	panel_edit2
};
#endif


/* defines for each bit and mask in input port panel_control */
enum
{
	/* bit numbers */
	panel_run_bit = 0,
	panel_CR_bit,
	panel_A_bit,
	panel_R_bit,
	panel_HB_bit,
	panel_ML_bit,
	panel_mem_bit,
	panel_write_bit,

	/* masks */
	panel_run = (1 << panel_run_bit),
	panel_CR  = (1 << panel_CR_bit),
	panel_A   = (1 << panel_A_bit),
	panel_R   = (1 << panel_R_bit),
	panel_HB  = (1 << panel_HB_bit),
	panel_ML  = (1 << panel_ML_bit),
	panel_mem = (1 << panel_mem_bit),
	panel_write = (1 << panel_write_bit)
};

/* fake input ports with keyboard keys */
static INPUT_PORTS_START(apexc)

	PORT_START("panel") /* 0 : panel control */
	PORT_BIT(panel_run, IP_ACTIVE_HIGH, IPT_OTHER) PORT_NAME("Run/Stop")                PORT_CODE(KEYCODE_ENTER)
	PORT_BIT(panel_CR,  IP_ACTIVE_HIGH, IPT_OTHER) PORT_NAME("Read CR")                 PORT_CODE(KEYCODE_1_PAD)
	PORT_BIT(panel_A,   IP_ACTIVE_HIGH, IPT_OTHER) PORT_NAME("Read A")                  PORT_CODE(KEYCODE_2_PAD)
	PORT_BIT(panel_R,   IP_ACTIVE_HIGH, IPT_OTHER) PORT_NAME("Read R")                  PORT_CODE(KEYCODE_3_PAD)
	PORT_BIT(panel_HB,  IP_ACTIVE_HIGH, IPT_OTHER) PORT_NAME("Read HB")                 PORT_CODE(KEYCODE_4_PAD)
	PORT_BIT(panel_ML,  IP_ACTIVE_HIGH, IPT_OTHER) PORT_NAME("Read ML")                 PORT_CODE(KEYCODE_5_PAD)
	PORT_BIT(panel_mem, IP_ACTIVE_HIGH, IPT_OTHER) PORT_NAME("Read mem")                PORT_CODE(KEYCODE_6_PAD)
	PORT_BIT(panel_write, IP_ACTIVE_HIGH, IPT_OTHER) PORT_NAME("Write instead of read") PORT_CODE(KEYCODE_LSHIFT)

	PORT_START("data")  /* data edit */
	PORT_BIT(0x80000000, IP_ACTIVE_HIGH, IPT_OTHER) PORT_NAME("Toggle bit #1")              PORT_CODE(KEYCODE_1)
	PORT_BIT(0x40000000, IP_ACTIVE_HIGH, IPT_OTHER) PORT_NAME("Toggle bit #2")              PORT_CODE(KEYCODE_2)
	PORT_BIT(0x20000000, IP_ACTIVE_HIGH, IPT_OTHER) PORT_NAME("Toggle bit #3")              PORT_CODE(KEYCODE_3)
	PORT_BIT(0x10000000, IP_ACTIVE_HIGH, IPT_OTHER) PORT_NAME("Toggle bit #4")              PORT_CODE(KEYCODE_4)
	PORT_BIT(0x08000000, IP_ACTIVE_HIGH, IPT_OTHER) PORT_NAME("Toggle bit #5")              PORT_CODE(KEYCODE_5)
	PORT_BIT(0x04000000, IP_ACTIVE_HIGH, IPT_OTHER) PORT_NAME("Toggle bit #6")              PORT_CODE(KEYCODE_6)
	PORT_BIT(0x02000000, IP_ACTIVE_HIGH, IPT_OTHER) PORT_NAME("Toggle bit #7")              PORT_CODE(KEYCODE_7)
	PORT_BIT(0x01000000, IP_ACTIVE_HIGH, IPT_OTHER) PORT_NAME("Toggle bit #8")              PORT_CODE(KEYCODE_8)
	PORT_BIT(0x00800000, IP_ACTIVE_HIGH, IPT_OTHER) PORT_NAME("Toggle bit #9")              PORT_CODE(KEYCODE_9)
	PORT_BIT(0x00400000, IP_ACTIVE_HIGH, IPT_OTHER) PORT_NAME("Toggle bit #10")             PORT_CODE(KEYCODE_0)
	PORT_BIT(0x00200000, IP_ACTIVE_HIGH, IPT_OTHER) PORT_NAME("Toggle bit #11")             PORT_CODE(KEYCODE_Q)
	PORT_BIT(0x00100000, IP_ACTIVE_HIGH, IPT_OTHER) PORT_NAME("Toggle bit #12")             PORT_CODE(KEYCODE_W)
	PORT_BIT(0x00080000, IP_ACTIVE_HIGH, IPT_OTHER) PORT_NAME("Toggle bit #13")             PORT_CODE(KEYCODE_E)
	PORT_BIT(0x00040000, IP_ACTIVE_HIGH, IPT_OTHER) PORT_NAME("Toggle bit #14")             PORT_CODE(KEYCODE_R)
	PORT_BIT(0x00020000, IP_ACTIVE_HIGH, IPT_OTHER) PORT_NAME("Toggle bit #15")             PORT_CODE(KEYCODE_T)
	PORT_BIT(0x00010000, IP_ACTIVE_HIGH, IPT_OTHER) PORT_NAME("Toggle bit #16")             PORT_CODE(KEYCODE_Y)

	PORT_BIT(0x00008000, IP_ACTIVE_HIGH, IPT_OTHER) PORT_NAME("Toggle bit #17")             PORT_CODE(KEYCODE_U)
	PORT_BIT(0x00004000, IP_ACTIVE_HIGH, IPT_OTHER) PORT_NAME("Toggle bit #18")             PORT_CODE(KEYCODE_I)
	PORT_BIT(0x00002000, IP_ACTIVE_HIGH, IPT_OTHER) PORT_NAME("Toggle bit #19")             PORT_CODE(KEYCODE_O)
	PORT_BIT(0x00001000, IP_ACTIVE_HIGH, IPT_OTHER) PORT_NAME("Toggle bit #20")             PORT_CODE(KEYCODE_OPENBRACE)
	PORT_BIT(0x00000800, IP_ACTIVE_HIGH, IPT_OTHER) PORT_NAME("Toggle bit #21")             PORT_CODE(KEYCODE_A)
	PORT_BIT(0x00000400, IP_ACTIVE_HIGH, IPT_OTHER) PORT_NAME("Toggle bit #22")             PORT_CODE(KEYCODE_S)
	PORT_BIT(0x00000200, IP_ACTIVE_HIGH, IPT_OTHER) PORT_NAME("Toggle bit #23")             PORT_CODE(KEYCODE_D)
	PORT_BIT(0x00000100, IP_ACTIVE_HIGH, IPT_OTHER) PORT_NAME("Toggle bit #24")             PORT_CODE(KEYCODE_F)
	PORT_BIT(0x00000080, IP_ACTIVE_HIGH, IPT_OTHER) PORT_NAME("Toggle bit #25")             PORT_CODE(KEYCODE_G)
	PORT_BIT(0x00000040, IP_ACTIVE_HIGH, IPT_OTHER) PORT_NAME("Toggle bit #26")             PORT_CODE(KEYCODE_H)
	PORT_BIT(0x00000020, IP_ACTIVE_HIGH, IPT_OTHER) PORT_NAME("Toggle bit #27")             PORT_CODE(KEYCODE_J)
	PORT_BIT(0x00000010, IP_ACTIVE_HIGH, IPT_OTHER) PORT_NAME("Toggle bit #28")             PORT_CODE(KEYCODE_K)
	PORT_BIT(0x00000008, IP_ACTIVE_HIGH, IPT_OTHER) PORT_NAME("Toggle bit #29")             PORT_CODE(KEYCODE_L)
	PORT_BIT(0x00000004, IP_ACTIVE_HIGH, IPT_OTHER) PORT_NAME("Toggle bit #30")             PORT_CODE(KEYCODE_Z)
	PORT_BIT(0x00000002, IP_ACTIVE_HIGH, IPT_OTHER) PORT_NAME("Toggle bit #31")             PORT_CODE(KEYCODE_X)
	PORT_BIT(0x00000001, IP_ACTIVE_HIGH, IPT_OTHER) PORT_NAME("Toggle bit #32")             PORT_CODE(KEYCODE_C)
INPUT_PORTS_END

void apexc_state::device_timer(emu_timer &timer, device_timer_id id, int param, void *ptr)
{
	if (id == TIMER_POLL_INPUTS)
	{
		check_inputs();
	}
}

void apexc_state::check_inputs()
{
	/* read new state of edit keys */
	uint32_t edit_keys = m_data_port->read();

	/* toggle data reg according to transitions */
	m_panel_data_reg ^= edit_keys & (~m_old_edit_keys);

	/* remember new state of edit keys */
	m_old_edit_keys = edit_keys;

	/* read new state of control keys */
	int control_keys = m_panel_port->read();

	/* compute transitions */
	int control_transitions = control_keys & (~m_old_control_keys);

	/* process commands */

	if (control_transitions & panel_run)
	{   /* toggle run/stop state */
		m_maincpu->set_state_int(APEXC_STATE, ! m_maincpu->state_int(APEXC_STATE));
	}

	while (control_transitions & (panel_CR | panel_A | panel_R | panel_ML | panel_HB))
	{   /* read/write a register */
		/* note that we must take into account the possibility of simulteanous keypresses
		(which would be a goofy thing to do when reading, but a normal one when writing,
		if the user wants to clear several registers at once) */
		printf("crarmlhb\n");
		int reg_id = -1;

		/* determinate value of reg_id */
		if (control_transitions & panel_CR)
		{   /* CR register selected ? */
			control_transitions &= ~panel_CR;   /* clear so that it is ignored on next iteration */
			reg_id = APEXC_CR;          /* matching register ID */
		}
		else if (control_transitions & panel_A)
		{
			control_transitions &= ~panel_A;
			reg_id = APEXC_A;
		}
		else if (control_transitions & panel_R)
		{
			control_transitions &= ~panel_R;
			reg_id = APEXC_R;
		}
		else if (control_transitions & panel_HB)
		{
			control_transitions &= ~panel_HB;
			reg_id = APEXC_WS;
		}
		else if (control_transitions & panel_ML)
		{
			control_transitions &= ~panel_ML;
			reg_id = APEXC_ML;
		}

		if (-1 != reg_id)
		{
			/* read/write register #reg_id */
			if (control_keys & panel_write)
				/* write reg */
				m_maincpu->set_state_int(reg_id, m_panel_data_reg);
			else
				/* read reg */
				m_panel_data_reg = m_maincpu->state_int(reg_id);
		}
	}

	if (control_transitions & panel_mem)
	{   /* read/write memory */
		if (control_keys & panel_write) /* write memory */
			m_maincpu->space(AS_PROGRAM).write_dword(m_maincpu->pc(), m_panel_data_reg);
		else                            /* read memory */
			m_panel_data_reg = m_maincpu->space(AS_PROGRAM).read_dword(m_maincpu->pc());
	}

	/* remember new state of control keys */
	m_old_control_keys = control_keys;
}

enum
{
	apexc_charnum = /*96+4*/128,    /* ASCII set + 4 special characters */
									/* for whatever reason, 96+4 breaks greek characters */

	apexcfontdata_size = 8 * apexc_charnum
};

/* apexc driver init : builds a font for use by the teletyper */
void apexc_state::init_apexc()
{
	static const unsigned char fontdata6x8[apexcfontdata_size] =
	{   /* ASCII characters */
		0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x20,0x20,0x20,0x20,0x20,0x00,0x20,0x00,
		0x50,0x50,0x50,0x00,0x00,0x00,0x00,0x00,0x00,0x50,0xf8,0x50,0xf8,0x50,0x00,0x00,
		0x20,0x70,0xc0,0x70,0x18,0xf0,0x20,0x00,0x40,0xa4,0x48,0x10,0x20,0x48,0x94,0x08,
		0x60,0x90,0xa0,0x40,0xa8,0x90,0x68,0x00,0x10,0x20,0x40,0x00,0x00,0x00,0x00,0x00,
		0x20,0x40,0x40,0x40,0x40,0x40,0x20,0x00,0x10,0x08,0x08,0x08,0x08,0x08,0x10,0x00,

		0x20,0xa8,0x70,0xf8,0x70,0xa8,0x20,0x00,0x00,0x20,0x20,0xf8,0x20,0x20,0x00,0x00,
		0x00,0x00,0x00,0x00,0x00,0x30,0x30,0x60,0x00,0x00,0x00,0xf8,0x00,0x00,0x00,0x00,
		0x00,0x00,0x00,0x00,0x00,0x30,0x30,0x00,0x00,0x08,0x10,0x20,0x40,0x80,0x00,0x00,
		0x70,0x88,0x88,0x88,0x88,0x88,0x70,0x00,0x10,0x30,0x10,0x10,0x10,0x10,0x10,0x00,
		0x70,0x88,0x08,0x10,0x20,0x40,0xf8,0x00,0x70,0x88,0x08,0x30,0x08,0x88,0x70,0x00,
		0x10,0x30,0x50,0x90,0xf8,0x10,0x10,0x00,0xf8,0x80,0xf0,0x08,0x08,0x88,0x70,0x00,
		0x70,0x80,0xf0,0x88,0x88,0x88,0x70,0x00,0xf8,0x08,0x08,0x10,0x20,0x20,0x20,0x00,
		0x70,0x88,0x88,0x70,0x88,0x88,0x70,0x00,0x70,0x88,0x88,0x88,0x78,0x08,0x70,0x00,
		0x00,0x00,0x30,0x30,0x00,0x30,0x30,0x00,0x00,0x00,0x30,0x30,0x00,0x30,0x30,0x60,
		0x10,0x20,0x40,0x80,0x40,0x20,0x10,0x00,0x00,0x00,0xf8,0x00,0xf8,0x00,0x00,0x00,
		0x40,0x20,0x10,0x08,0x10,0x20,0x40,0x00,0x70,0x88,0x08,0x10,0x20,0x00,0x20,0x00,
		0x70,0x88,0xb8,0xa8,0xb8,0x80,0x70,0x00,0x70,0x88,0x88,0xf8,0x88,0x88,0x88,0x00,
		0xf0,0x88,0x88,0xf0,0x88,0x88,0xf0,0x00,0x70,0x88,0x80,0x80,0x80,0x88,0x70,0x00,
		0xf0,0x88,0x88,0x88,0x88,0x88,0xf0,0x00,0xf8,0x80,0x80,0xf0,0x80,0x80,0xf8,0x00,
		0xf8,0x80,0x80,0xf0,0x80,0x80,0x80,0x00,0x70,0x88,0x80,0x98,0x88,0x88,0x70,0x00,
		0x88,0x88,0x88,0xf8,0x88,0x88,0x88,0x00,0x20,0x20,0x20,0x20,0x20,0x20,0x20,0x00,
		0x08,0x08,0x08,0x08,0x88,0x88,0x70,0x00,0x88,0x90,0xa0,0xc0,0xa0,0x90,0x88,0x00,
		0x80,0x80,0x80,0x80,0x80,0x80,0xf8,0x00,0x88,0xd8,0xa8,0x88,0x88,0x88,0x88,0x00,
		0x88,0xc8,0xa8,0x98,0x88,0x88,0x88,0x00,0x70,0x88,0x88,0x88,0x88,0x88,0x70,0x00,
		0xf0,0x88,0x88,0xf0,0x80,0x80,0x80,0x00,0x70,0x88,0x88,0x88,0x88,0x88,0x70,0x08,
		0xf0,0x88,0x88,0xf0,0x88,0x88,0x88,0x00,0x70,0x88,0x80,0x70,0x08,0x88,0x70,0x00,
		0xf8,0x20,0x20,0x20,0x20,0x20,0x20,0x00,0x88,0x88,0x88,0x88,0x88,0x88,0x70,0x00,
		0x88,0x88,0x88,0x88,0x88,0x50,0x20,0x00,0x88,0x88,0x88,0x88,0xa8,0xd8,0x88,0x00,
		0x88,0x50,0x20,0x20,0x20,0x50,0x88,0x00,0x88,0x88,0x88,0x50,0x20,0x20,0x20,0x00,
		0xf8,0x08,0x10,0x20,0x40,0x80,0xf8,0x00,0x30,0x20,0x20,0x20,0x20,0x20,0x30,0x00,
		0x40,0x40,0x20,0x20,0x10,0x10,0x08,0x08,0x30,0x10,0x10,0x10,0x10,0x10,0x30,0x00,
		0x20,0x50,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0xfc,
		0x40,0x20,0x10,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x70,0x08,0x78,0x88,0x78,0x00,
		0x80,0x80,0xf0,0x88,0x88,0x88,0xf0,0x00,0x00,0x00,0x70,0x88,0x80,0x80,0x78,0x00,
		0x08,0x08,0x78,0x88,0x88,0x88,0x78,0x00,0x00,0x00,0x70,0x88,0xf8,0x80,0x78,0x00,
		0x18,0x20,0x70,0x20,0x20,0x20,0x20,0x00,0x00,0x00,0x78,0x88,0x88,0x78,0x08,0x70,
		0x80,0x80,0xf0,0x88,0x88,0x88,0x88,0x00,0x20,0x00,0x20,0x20,0x20,0x20,0x20,0x00,
		0x20,0x00,0x20,0x20,0x20,0x20,0x20,0xc0,0x80,0x80,0x90,0xa0,0xe0,0x90,0x88,0x00,
		0x20,0x20,0x20,0x20,0x20,0x20,0x20,0x00,0x00,0x00,0xf0,0xa8,0xa8,0xa8,0xa8,0x00,
		0x00,0x00,0xb0,0xc8,0x88,0x88,0x88,0x00,0x00,0x00,0x70,0x88,0x88,0x88,0x70,0x00,
		0x00,0x00,0xf0,0x88,0x88,0xf0,0x80,0x80,0x00,0x00,0x78,0x88,0x88,0x78,0x08,0x08,
		0x00,0x00,0xb0,0xc8,0x80,0x80,0x80,0x00,0x00,0x00,0x78,0x80,0x70,0x08,0xf0,0x00,
		0x20,0x20,0x70,0x20,0x20,0x20,0x18,0x00,0x00,0x00,0x88,0x88,0x88,0x98,0x68,0x00,
		0x00,0x00,0x88,0x88,0x88,0x50,0x20,0x00,0x00,0x00,0xa8,0xa8,0xa8,0xa8,0x50,0x00,
		0x00,0x00,0x88,0x50,0x20,0x50,0x88,0x00,0x00,0x00,0x88,0x88,0x88,0x78,0x08,0x70,
		0x00,0x00,0xf8,0x10,0x20,0x40,0xf8,0x00,0x08,0x10,0x10,0x20,0x10,0x10,0x08,0x00,
		0x20,0x20,0x20,0x20,0x20,0x20,0x20,0x20,0x40,0x20,0x20,0x10,0x20,0x20,0x40,0x00,
		0x00,0x68,0xb0,0x00,0x00,0x00,0x00,0x00,0x20,0x50,0x20,0x50,0xa8,0x50,0x00,0x00,

		/* theta */
		0x70,
		0x88,
		0x88,
		0xF8,
		0x88,
		0x88,
		0x70,
		0x00,

		/* Sigma */
		0xf8,
		0x40,
		0x20,
		0x10,
		0x20,
		0x40,
		0xf8,
		0x00,

		/* Phi */
		0x20,
		0x70,
		0xA8,
		0xA8,
		0xA8,
		0xA8,
		0x70,
		0x20,

		/* pi */
		0x00,
		0x00,
		0xF8,
		0x50,
		0x50,
		0x50,
		0x50,
		0x00
	};

	memcpy(m_chargen_region->base(), fontdata6x8, apexcfontdata_size);
}

static const gfx_layout fontlayout =
{
	6, 8,           /* 6*8 characters */
	apexc_charnum,  /* 96+4 characters */
	1,              /* 1 bit per pixel */
	{ 0 },
	{ 0, 1, 2, 3, 4, 5, 6, 7 }, /* straightforward layout */
	{ 0*8, 1*8, 2*8, 3*8, 4*8, 5*8, 6*8, 7*8 },
	8*8 /* every char takes 8 consecutive bytes */
};

static GFXDECODE_START( gfx_apexc )
	GFXDECODE_ENTRY( "chargen", 0, fontlayout, 0, 2 )
GFXDECODE_END


void apexc_state::mem(address_map &map)
{
	map(0x0000, 0x0fff).ram().share("maincpu");
	map(0x1000, 0x7fff).noprw();
}

void apexc_state::apexc(machine_config &config)
{
	/* basic machine hardware */
	/* APEXC CPU @ 2.0 kHz (memory word clock frequency) */
	APEXC(config, m_maincpu, 2000);
	m_maincpu->set_addrmap(AS_PROGRAM, &apexc_state::mem);
	m_maincpu->tape_read().set(m_tape_reader, FUNC(apexc_tape_reader_image_device::read));
	m_maincpu->tape_punch().set(FUNC(apexc_state::tape_write));

	/* video hardware does not exist, but we display a control panel and the typewriter output */
	SCREEN(config, m_screen, SCREEN_TYPE_RASTER);
	m_screen->set_refresh_hz(60);
	m_screen->set_vblank_time(ATTOSECONDS_IN_USEC(2500)); /* not accurate */
	m_screen->set_size(256, 192);
	m_screen->set_visarea(0, 256-1, 0, 192-1);
	m_screen->set_palette(m_palette);
	m_screen->set_screen_update(FUNC(apexc_state::screen_update_apexc));

	GFXDECODE(config, m_gfxdecode, m_palette, gfx_apexc);

	PALETTE(config, m_palette, FUNC(apexc_state::apexc_palette), std::size(palette_table));

	APEXC_CYLINDER(config, m_cylinder);
	APEXC_TAPE_PUNCHER(config, m_tape_puncher);
	APEXC_TAPE_READER(config, m_tape_reader);
}

ROM_START(apexc)
	/*CPU memory space*/
	/* Note this computer has no ROM... */

	ROM_REGION(apexcfontdata_size, "chargen", ROMREGION_ERASEFF)
		/* space filled with our font */
ROM_END

//    YEAR  NAME     PARENT  COMPAT  MACHINE  INPUT  CLASS        INIT        COMPANY                FULLNAME */
//COMP( 1951, apexc53, 0,      0,      apexc53, apexc, apexc_state, init_apexc, "Andrew Donald Booth", "All Purpose Electronic X-ray Computer (as described in 1953)" , MACHINE_NOT_WORKING | MACHINE_NO_SOUND_HW)
COMP( 1955, apexc,   0,      0,      apexc,   apexc, apexc_state, init_apexc, "Andrew Donald Booth", "All Purpose Electronic X-ray Computer (as described in 1957)" , MACHINE_NO_SOUND_HW)
