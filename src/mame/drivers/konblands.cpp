// license:BSD-3-Clause
// copyright-holders:Angelo Salese
/***************************************************************************

    GX455 - Konami Badlands

	driver by Angelo Salese
	
	TODO:
	- verify interrupts, service mode seems way too fast and it randomly crashes;
	- fix overlay positioning & transparency enable;
	- dipswitches;
	- add sn76496 latch mechanism (should actually be in the device itself);
	
	Notes:
	- to enter service mode hold start 1 & 2 at POST.

***************************************************************************/


#include "emu.h"
#include "cpu/m6809/m6809.h"
#include "sound/sn76496.h"
#include "machine/ldv1000.h"
#include "screen.h"
#include "speaker.h"

#define MASTER_CLOCK XTAL(18'432'000)

class konblands_state : public driver_device
{
public:
	konblands_state(const machine_config &mconfig, device_type type, const char *tag)
		: driver_device(mconfig, type, tag)
		, m_maincpu(*this, "maincpu")
		, m_laserdisc(*this, "laserdisc")
		, m_vram(*this, "vram")
   		, m_gfxdecode(*this, "gfxdecode")
	{
	}

	// screen updates
	uint32_t screen_update(screen_device &screen, bitmap_ind16 &bitmap, const rectangle &cliprect);
	DECLARE_PALETTE_INIT(konblands);
	DECLARE_READ8_MEMBER(ldp_r);
	DECLARE_WRITE8_MEMBER(ldp_w);
	DECLARE_WRITE8_MEMBER(nmi_enable_w);
	DECLARE_WRITE8_MEMBER(irq_enable_w);
	DECLARE_WRITE8_MEMBER(firq_enable_w);
	INTERRUPT_GEN_MEMBER(vblank_irq);
	INTERRUPT_GEN_MEMBER(timer_irq);
	DECLARE_WRITE_LINE_MEMBER(ld_command_strobe_cb);

	void konblands(machine_config &config);
	void konblandsh(machine_config &config);
	void konblands_map(address_map &map);
	void konblandsh_map(address_map &map);
protected:
	// driver_device overrides
	virtual void machine_start() override;
	virtual void machine_reset() override;

	virtual void video_start() override;

private:
	// devices
	required_device<cpu_device> m_maincpu;
	required_device<pioneer_ldv1000_device> m_laserdisc;
	required_shared_ptr<uint8_t> m_vram;
	required_device<gfxdecode_device> m_gfxdecode;
	
	bool m_nmi_enable, m_irq_enable, m_firq_enable;
};

PALETTE_INIT_MEMBER(konblands_state, konblands)
{
	const uint8_t *color_prom = memregion("proms")->base();
	int bit0, bit1, bit2 , r, g, b;
	int i;

	for (i = 0; i < 0x20; ++i)
	{
		bit0 = 0;
		bit1 = (color_prom[i] >> 6) & 0x01;
		bit2 = (color_prom[i] >> 7) & 0x01;
		b = 0x21 * bit0 + 0x47 * bit1 + 0x97 * bit2;
		bit0 = (color_prom[i] >> 3) & 0x01;
		bit1 = (color_prom[i] >> 4) & 0x01;
		bit2 = (color_prom[i] >> 5) & 0x01;
		g = 0x21 * bit0 + 0x47 * bit1 + 0x97 * bit2;
		bit0 = (color_prom[i] >> 0) & 0x01;
		bit1 = (color_prom[i] >> 1) & 0x01;
		bit2 = (color_prom[i] >> 2) & 0x01;
		r = 0x21 * bit0 + 0x47 * bit1 + 0x97 * bit2;

		palette.set_pen_color(i, rgb_t(r, g, b));
	}
}

void konblands_state::video_start()
{
}

uint32_t konblands_state::screen_update( screen_device &screen, bitmap_ind16 &bitmap, const rectangle &cliprect )
{
	gfx_element *gfx = m_gfxdecode->gfx(0);
	int y,x;
	int count = 0;

	for (y=0;y<32;y++)
	{
		for (x=0;x<64;x++)
		{
			uint8_t tile = m_vram[count];

			gfx->opaque(bitmap,cliprect,tile,0,0,0,x*8,y*8);

			count++;
		}
	}

	return 0;
}

READ8_MEMBER(konblands_state::ldp_r)
{
	return m_laserdisc->status_r();
}

WRITE8_MEMBER(konblands_state::ldp_w)
{
	m_laserdisc->data_w(data);
}

WRITE8_MEMBER(konblands_state::nmi_enable_w)
{
	m_nmi_enable = bool(BIT(data,0));
}

WRITE8_MEMBER(konblands_state::irq_enable_w)
{
	m_irq_enable = bool(BIT(data,0));
}

WRITE8_MEMBER(konblands_state::firq_enable_w)
{
	m_firq_enable = bool(BIT(data,0));
}

ADDRESS_MAP_START(konblands_state::konblands_map)
	AM_RANGE(0x0000, 0x0000) AM_READ_PORT("DSW1") AM_WRITENOP // sn latch
	AM_RANGE(0x0800, 0x0800) AM_READ_PORT("DSW2") AM_WRITE(ldp_w)
	AM_RANGE(0x1000, 0x1000) AM_READ(ldp_r) AM_WRITENOP // led
	AM_RANGE(0x1001, 0x1001) AM_WRITENOP // coin counter 2
	AM_RANGE(0x1002, 0x1002) AM_WRITENOP // coin counter 1
	AM_RANGE(0x1003, 0x1003) AM_WRITENOP // enable overlay transparency
	AM_RANGE(0x1004, 0x1004) AM_WRITE(nmi_enable_w)
	AM_RANGE(0x1005, 0x1005) AM_WRITENOP // enable audio
	AM_RANGE(0x1006, 0x1006) AM_WRITE(irq_enable_w)
	AM_RANGE(0x1007, 0x1007) AM_WRITE(firq_enable_w)
	AM_RANGE(0x1800, 0x1800) AM_READ_PORT("INPUTS") AM_DEVWRITE("sn", sn76496_device, write)
	AM_RANGE(0x4000, 0x47ff) AM_RAM AM_SHARE("vram")
	AM_RANGE(0x4800, 0x4bff) AM_RAM
	AM_RANGE(0x5800, 0x5800) AM_WRITENOP // watchdog
	AM_RANGE(0x8000, 0x9fff) AM_READNOP // diagnostic ROM?
	AM_RANGE(0xc000, 0xffff) AM_ROM AM_REGION("ipl",0)
ADDRESS_MAP_END

ADDRESS_MAP_START(konblands_state::konblandsh_map)
	AM_RANGE(0x0000, 0x0000) AM_READ(ldp_r)
	AM_RANGE(0x0400, 0x0400) AM_WRITE(ldp_w)
	AM_RANGE(0x0802, 0x0802) AM_WRITENOP // led
	AM_RANGE(0x0803, 0x0803) AM_WRITENOP // enable overlay transparency
	AM_RANGE(0x0806, 0x0806) AM_READNOP AM_WRITE(irq_enable_w)
	AM_RANGE(0x0807, 0x0807) AM_READNOP AM_WRITE(firq_enable_w)
	AM_RANGE(0x0c00, 0x0c00) AM_READ_PORT("INPUTS")
	AM_RANGE(0x1000, 0x1000) AM_READ_PORT("DSW1")
	AM_RANGE(0x1400, 0x1400) AM_DEVWRITE("sn", sn76496_device, write)
	AM_RANGE(0x1800, 0x1800) AM_WRITENOP // sn latch
	AM_RANGE(0x2000, 0x27ff) AM_RAM AM_SHARE("vram")
	AM_RANGE(0x2800, 0x2fff) AM_RAM
	AM_RANGE(0xc000, 0xffff) AM_ROM AM_REGION("ipl",0)
ADDRESS_MAP_END

static INPUT_PORTS_START( konblands )
	PORT_START("DSW1")
	PORT_DIPNAME( 0x01, 0x01, "DSWA" )
	PORT_DIPSETTING(    0x01, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
	PORT_DIPNAME( 0x02, 0x02, DEF_STR( Unknown ) )
	PORT_DIPSETTING(    0x02, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
	PORT_DIPNAME( 0x04, 0x04, DEF_STR( Unknown ) )
	PORT_DIPSETTING(    0x04, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
	PORT_DIPNAME( 0x08, 0x08, DEF_STR( Unknown ) )
	PORT_DIPSETTING(    0x08, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
	PORT_DIPNAME( 0x10, 0x10, DEF_STR( Unknown ) )
	PORT_DIPSETTING(    0x10, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
	PORT_DIPNAME( 0x20, 0x20, DEF_STR( Unknown ) )
	PORT_DIPSETTING(    0x20, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
	PORT_DIPNAME( 0x40, 0x40, DEF_STR( Unknown ) )
	PORT_DIPSETTING(    0x40, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
	PORT_DIPNAME( 0x80, 0x80, DEF_STR( Unknown ) )
	PORT_DIPSETTING(    0x80, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )

	PORT_START("DSW2")
	PORT_DIPNAME( 0x01, 0x01, "DSWA" )
	PORT_DIPSETTING(    0x01, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
	PORT_DIPNAME( 0x02, 0x02, DEF_STR( Unknown ) )
	PORT_DIPSETTING(    0x02, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
	PORT_DIPNAME( 0x04, 0x04, DEF_STR( Unknown ) )
	PORT_DIPSETTING(    0x04, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
	PORT_DIPNAME( 0x08, 0x08, DEF_STR( Unknown ) )
	PORT_DIPSETTING(    0x08, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
	PORT_DIPNAME( 0x10, 0x10, DEF_STR( Unknown ) )
	PORT_DIPSETTING(    0x10, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
	PORT_DIPNAME( 0x20, 0x20, DEF_STR( Unknown ) )
	PORT_DIPSETTING(    0x20, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
	PORT_DIPNAME( 0x40, 0x40, DEF_STR( Unknown ) )
	PORT_DIPSETTING(    0x40, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
	PORT_DIPNAME( 0x80, 0x80, DEF_STR( Unknown ) )
	PORT_DIPSETTING(    0x80, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
	
	PORT_START("INPUTS")
	PORT_BIT( 0x01, IP_ACTIVE_LOW, IPT_COIN1 )
	PORT_BIT( 0x02, IP_ACTIVE_LOW, IPT_COIN2 )
	PORT_BIT( 0x04, IP_ACTIVE_LOW, IPT_SERVICE1 )
	PORT_BIT( 0x08, IP_ACTIVE_LOW, IPT_START1 )
	PORT_BIT( 0x10, IP_ACTIVE_LOW, IPT_START2 )
	PORT_BIT( 0x20, IP_ACTIVE_LOW, IPT_BUTTON1 )
	PORT_DIPNAME( 0x40, 0x40, DEF_STR( Unknown ) )
	PORT_DIPSETTING(    0x40, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
	PORT_DIPNAME( 0x80, 0x80, DEF_STR( Unknown ) )
	PORT_DIPSETTING(    0x80, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
INPUT_PORTS_END

static const gfx_layout charlayout =
{
	8,8,
	RGN_FRAC(1,1),
	4,
	{ STEP4(0,1) },
	{ STEP8(0,4) },
	{ STEP8(0,4*8) },
	8*8*4
};

static GFXDECODE_START( konblands )
	GFXDECODE_ENTRY( "gfx", 0, charlayout,     0, 1 )
GFXDECODE_END


void konblands_state::machine_start()
{
}

void konblands_state::machine_reset()
{
	m_nmi_enable = false;
	m_irq_enable = false;
	m_firq_enable = false;
}

INTERRUPT_GEN_MEMBER(konblands_state::vblank_irq)
{
	if (m_nmi_enable == true)
		m_maincpu->set_input_line(INPUT_LINE_NMI, PULSE_LINE);
}

INTERRUPT_GEN_MEMBER(konblands_state::timer_irq)
{
	if (m_firq_enable == true)
		m_maincpu->set_input_line(M6809_FIRQ_LINE, HOLD_LINE);
}

WRITE_LINE_MEMBER(konblands_state::ld_command_strobe_cb)
{
	if(m_irq_enable == true)
		m_maincpu->set_input_line(M6809_IRQ_LINE, state ? ASSERT_LINE : CLEAR_LINE);
}

MACHINE_CONFIG_START(konblands_state::konblands)

	/* basic machine hardware */
	MCFG_CPU_ADD("maincpu",MC6809E,MASTER_CLOCK/12)
	MCFG_CPU_PROGRAM_MAP(konblands_map)
	MCFG_CPU_VBLANK_INT_DRIVER("screen", konblands_state,  vblank_irq)
	MCFG_CPU_PERIODIC_INT_DRIVER(konblands_state, timer_irq,  8) // 8 times per frame

	/* video hardware */
	MCFG_LASERDISC_LDV1000_ADD("laserdisc")
	MCFG_LASERDISC_LDV1000_COMMAND_STROBE_CB(WRITELINE(konblands_state, ld_command_strobe_cb))
	// TODO: might be different
	MCFG_LASERDISC_OVERLAY_DRIVER(512, 256, konblands_state, screen_update)
	MCFG_LASERDISC_OVERLAY_PALETTE("palette")

	/* video hardware */
	MCFG_LASERDISC_SCREEN_ADD_NTSC("screen", "laserdisc")

	MCFG_GFXDECODE_ADD("gfxdecode", "palette", konblands)

	MCFG_PALETTE_ADD("palette", 32)
	MCFG_PALETTE_INIT_OWNER(konblands_state, konblands)

	/* sound hardware */
	MCFG_SPEAKER_STANDARD_MONO("mono")
	MCFG_SOUND_ADD("sn", SN76496, MASTER_CLOCK/12)
	MCFG_SOUND_ROUTE(ALL_OUTPUTS, "mono", 0.50)
MACHINE_CONFIG_END

MACHINE_CONFIG_START(konblands_state::konblandsh)
	konblands(config);
	MCFG_CPU_MODIFY("maincpu")
	MCFG_CPU_PROGRAM_MAP(konblandsh_map)
MACHINE_CONFIG_END

/***************************************************************************

  Machine driver(s)

***************************************************************************/

ROM_START( kbadlands )
	ROM_REGION( 0x4000, "ipl", ROMREGION_ERASE00 )
	ROM_LOAD( "badlands.a13",   0x0000, 0x2000, CRC(a44776d6) SHA1(eb3b5b57a99e9bd2559ced3b279fb3566c918f9a) )
	ROM_LOAD( "badlands.a14",   0x2000, 0x2000, CRC(82cb4614) SHA1(0cab824b4f3fb29e300f9c05911422d6047d073b) )

	ROM_REGION( 0x2000, "gfx", ROMREGION_ERASE00 )
	ROM_LOAD( "badlands.c8",   0x0000, 0x2000, CRC(590209fe) SHA1(8dfc836420e4c3fa417ec0aefb617a7abd0ccbc2) )	

	ROM_REGION( 0x20, "proms", 0 )
	ROM_LOAD( "badlands.c4",    0x000, 0x020, CRC(6757be8d) SHA1(1c9c24e29017f0a16b8a7dedd9776109e7e5734c) )
	
	DISK_REGION( "laserdisc" )
	DISK_IMAGE_READONLY( "badlands", 0, NO_DUMP )
ROM_END


ROM_START( kbadlandsh )
	ROM_REGION( 0x4000, "ipl", ROMREGION_ERASE00 )
    ROM_LOAD( "bl_hit.7a",    0x0000, 0x2000, CRC(a135e444) SHA1(7ef5394c698a5867aef200f577b8708df455b653) )
    ROM_LOAD( "bl_hit.6a",    0x2000, 0x2000, CRC(4c287f37) SHA1(b6b6b64174f1fd014b6c808015f1b0e65b56d24b) )

	ROM_REGION( 0x2000, "gfx", ROMREGION_ERASE00 )
    ROM_LOAD( "bl_hit.9c",    0x000000, 0x002000, CRC(44c3441e) SHA1(6b42961d31e5d025758cdfdc573648a83004577d) )

	ROM_REGION( 0x20, "proms", 0 )
    ROM_LOAD( "bl_hit.4f",    0x000000, 0x000020, CRC(0226f881) SHA1(b17c5681fca5ae65128793cf263725e2fe1314de) )
		
	DISK_REGION( "laserdisc" )
	DISK_IMAGE_READONLY( "badlands", 0, NO_DUMP )
ROM_END


GAME( 1984, kbadlands,  0,           konblands,  konblands, konblands_state,  0,       ROT0, "Konami",      "Badlands (Konami, set 1)", MACHINE_NO_SOUND | MACHINE_NOT_WORKING )
GAME( 1984, kbadlandsh, kbadlands,   konblandsh, konblands, konblands_state,  0,       ROT0, "Konami",      "Badlands (Konami, set 2)", MACHINE_NO_SOUND | MACHINE_NOT_WORKING )