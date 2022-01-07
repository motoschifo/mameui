// license:BSD-3-Clause
// copyright-holders:Juergen Buchmueller
/**********************************************************
 *   DIABLO drive image to hard disk interface
 **********************************************************/

#include "emu.h"
#include "diablo.h"

#include "emuopts.h"
#include "harddisk.h"
#include "romload.h"

#include "opresolv.h"


OPTION_GUIDE_START(dsk_option_guide)
	OPTION_INT('C', "cylinders",        "Cylinders")
	OPTION_INT('H', "heads",            "Heads")
	OPTION_INT('S', "sectors",          "Sectors")
	OPTION_INT('L', "sectorlength",     "Sector Words")
	OPTION_INT('K', "hunksize",         "Hunk Bytes")
OPTION_GUIDE_END

static char const *const dsk_option_spec =
	"C1-[203]-1024;H1/[2]/4/8;S1-[12]-64;L267;K6408";


// device type definition
DEFINE_DEVICE_TYPE(DIABLO, diablo_image_device, "diablo_image", "Diablo")

//-------------------------------------------------
//  diablo_image_device - constructor
//-------------------------------------------------

diablo_image_device::diablo_image_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock)
	: device_t(mconfig, DIABLO, tag, owner, clock),
		device_image_interface(mconfig, *this),
		m_chd(nullptr),
		m_hard_disk_handle(nullptr),
		m_device_image_load(*this),
		m_device_image_unload(*this),
		m_interface(nullptr)
{
}

//-------------------------------------------------
//  diablo_image_device - destructor
//-------------------------------------------------

diablo_image_device::~diablo_image_device()
{
}

//-------------------------------------------------
//  device_config_complete - perform any
//  operations now that the configuration is
//  complete
//-------------------------------------------------

void diablo_image_device::device_config_complete()
{
	add_format("chd", "CHD Hard drive", "chd,dsk", dsk_option_spec);
}

const util::option_guide &diablo_image_device::create_option_guide() const
{
	return dsk_option_guide;
}

//-------------------------------------------------
//  device_start - device-specific startup
//-------------------------------------------------

void diablo_image_device::device_start()
{
	m_device_image_load.resolve();
	m_device_image_unload.resolve();

	m_chd = nullptr;

	// try to locate the CHD from a DISK_REGION
	chd_file *handle = machine().rom_load().get_disk_handle(tag());
	if (handle != nullptr)
	{
		m_hard_disk_handle = hard_disk_open(handle);
	}
	else
	{
		m_hard_disk_handle = nullptr;
	}
}

void diablo_image_device::device_stop()
{
	if (m_hard_disk_handle)
		hard_disk_close(m_hard_disk_handle);
}

image_init_result diablo_image_device::call_load()
{
	image_init_result our_result;

	our_result = internal_load_dsk();
	/* Check if there is an image_load callback defined */
	if (!m_device_image_load.isnull())
	{
		/* Let the override do some additional work/checks */
		our_result = m_device_image_load(*this);
	}
	return our_result;

}

image_init_result diablo_image_device::call_create(int create_format, util::option_resolution *create_args)
{
	if (!create_args)
		throw emu_fatalerror("diablo_image_device::call_create: Expected create_args to not be nullptr");

	const uint32_t cylinders   = create_args->lookup_int('C');
	const uint32_t heads       = create_args->lookup_int('H');
	const uint32_t sectors     = create_args->lookup_int('S');
	const uint32_t sectorsize  = create_args->lookup_int('L') * sizeof(uint16_t);
	const uint32_t hunksize    = create_args->lookup_int('K');

	const uint32_t totalsectors = cylinders * heads * sectors;

	/* create the CHD file */
	chd_codec_type compression[4] = { CHD_CODEC_NONE };
	util::core_file::ptr proxy;
	std::error_condition err = util::core_file::open_proxy(image_core_file(), proxy);
	if (!err)
		m_origchd.create(std::move(proxy), uint64_t(totalsectors) * uint64_t(sectorsize), hunksize, sectorsize, compression);
	if (err)
		return image_init_result::FAIL;

	/* if we created the image and hence, have metadata to set, set the metadata */
	err = m_origchd.write_metadata(HARD_DISK_METADATA_TAG, 0, string_format(HARD_DISK_METADATA_FORMAT, cylinders, heads, sectors, sectorsize));
	m_origchd.close();

	if (err)
		return image_init_result::FAIL;

	return internal_load_dsk();
}

void diablo_image_device::call_unload()
{
	/* Check if there is an image_unload callback defined */
	if ( !m_device_image_unload.isnull() )
	{
		m_device_image_unload(*this);
	}

	if (m_hard_disk_handle != nullptr)
	{
		hard_disk_close(m_hard_disk_handle);
		m_hard_disk_handle = nullptr;
	}

	m_origchd.close();
	m_diffchd.close();
	m_chd = nullptr;
}

/*-------------------------------------------------
    open_disk_diff - open a DISK diff file
-------------------------------------------------*/

static std::error_condition open_disk_diff(emu_options &options, const char *name, chd_file &source, chd_file &diff_chd)
{
	std::string fname = std::string(name).append(".dif");

	/* try to open the diff */
	//printf("Opening differencing image file: %s\n", fname.c_str());
	emu_file diff_file(options.diff_directory(), OPEN_FLAG_READ | OPEN_FLAG_WRITE);
	std::error_condition filerr = diff_file.open(fname);
	if (!filerr)
	{
		std::string fullpath(diff_file.fullpath());
		diff_file.close();

		//printf("Opening differencing image file: %s\n", fullpath.c_str());
		return diff_chd.open(fullpath, true, &source);
	}

	/* didn't work; try creating it instead */
	//printf("Creating differencing image: %s\n", fname.c_str());
	diff_file.set_openflags(OPEN_FLAG_READ | OPEN_FLAG_WRITE | OPEN_FLAG_CREATE | OPEN_FLAG_CREATE_PATHS);
	filerr = diff_file.open(fname);
	if (!filerr)
	{
		std::string fullpath(diff_file.fullpath());
		diff_file.close();

		/* create the CHD */
		//printf("Creating differencing image file: %s\n", fullpath.c_str());
		chd_codec_type compression[4] = { CHD_CODEC_NONE };
		std::error_condition err = diff_chd.create(fullpath, source.logical_bytes(), source.hunk_bytes(), compression, source);
		if (err)
			return err;

		return diff_chd.clone_all_metadata(source);
	}

	return std::errc::no_such_file_or_directory;
}

image_init_result diablo_image_device::internal_load_dsk()
{
	std::error_condition err;

	m_chd = nullptr;

	if (m_hard_disk_handle)
		hard_disk_close(m_hard_disk_handle);

	/* open the CHD file */
	if (loaded_through_softlist())
	{
		m_chd = device().machine().rom_load().get_disk_handle(device().subtag("harddriv").c_str());
	}
	else
	{
		util::core_file::ptr proxy;
		err = util::core_file::open_proxy(image_core_file(), proxy);
		if (!err)
			err = m_origchd.open(std::move(proxy), true);
		if (!err)
		{
			m_chd = &m_origchd;
		}
		else if (err == chd_file::error::FILE_NOT_WRITEABLE)
		{
			err = util::core_file::open_proxy(image_core_file(), proxy);
			if (!err)
				err = m_origchd.open(std::move(proxy), false);
			if (!err)
			{
				err = open_disk_diff(device().machine().options(), basename_noext(), m_origchd, m_diffchd);
				if (!err)
				{
					m_chd = &m_diffchd;
				}
			}
		}
	}

	if (m_chd != nullptr)
	{
		/* open the hard disk file */
		m_hard_disk_handle = hard_disk_open(m_chd);
		if (m_hard_disk_handle != nullptr)
			return image_init_result::PASS;
	}

	/* if we had an error, close out the CHD */
	m_origchd.close();
	m_diffchd.close();
	m_chd = nullptr;
	seterror(err, nullptr);

	return image_init_result::FAIL;
}

/*************************************
 *
 *  Get the CHD file (from the src/chd.c core)
 *  after an image has been opened with the hd core
 *
 *************************************/

chd_file *diablo_image_device::get_chd_file()
{
	chd_file *result = nullptr;
	hard_disk_file *hd_file = get_hard_disk_file();
	if (hd_file)
		result = hard_disk_get_chd(hd_file);
	return result;
}
