/*
 * Code will start running at this symbol which is places at the start of the
 * image.
 */

OUTPUT_FORMAT("elf64-littleaarch64", "elf64-littleaarch64", "elf64-littleaarch64")
OUTPUT_ARCH(aarch64)

ENTRY(entry)

SECTIONS
{
	/*
	 * Set the image origin to a platform specific address. The images are
	 * relocatable but some platforms, e.g. QEMU, load to the same address
	 * and it makes debugging easier if the addresses match the symbols.
	 */
	. = 0x40001000;
	_image_start = .;

	/*
	 * Collect together the code. This is page aligned so it can be mapped
	 * as executable-only.
	 */
	_text_start = .;

	.entry : {
        	*(.init.entry)
        	*(.entry)
	}

	.text : {
		*(.text*)
		*(.text.*)
	}
	. = ALIGN(4096);
	_text_end = .;
	_text_size = ABSOLUTE(. - _text_start);

	/*
	 * Collect together read-only data including relocations at the end
	 * which are applied by the entry code.  This is page aligned so it can
	 * be mapped as read-only and non-executable.
	 */
	. = ALIGN(4096);
	_rodata_start = .;
	.rodata : {
		*(.rodata.*)
	}
	. = ALIGN(4096);
	_rodata_end = .;
	_rodata_size = ABSOLUTE(. - _rodata_start);

	/*
	 * Collect together the read-write data including .bss at the end which
	 * will be zero'd by the entry code. This is page aligned so it can be
	 * mapped as non-executable.
	 */
	. = ALIGN(4096);
	_data_start = .;
	.data : {
		*(.data)
	}
	_bin_end = .;

	/* The entry point code assumes that .bss is 16-byte aligned. */
	. = ALIGN(16);
	_bss_start = .;
	.bss : {
		*(.bss)
		*(COMMON)
	}
	. = ALIGN(16);
	_bss_end = .;

	/* not zero here, to much */
	xlat_table (NOLOAD) : {
		*(xlat_table)
	}

	_mp_stack : {
		*(_mp_stack)
	}
	. = ALIGN(4096);
	_data_end = .;
	_data_size = ABSOLUTE(. - _data_start);

	/*
	 * Remove unused sections from the image.
	 */
	/DISCARD/ : {
		/* The image loads itself so doesn't need these sections. */
		/* ld.lld doesn't allow these to be discarded.
		*(.dynsym)
		*(.dynstr)
		*(.dynamic)
		*/
		*(.gnu.hash)
		*(.hash)
		*(.interp)
	}

	/*
	 * Make note of some useful values.
	 */

	/* Note the first page not used in the image. */
	. = ALIGN(4096);
	_image_end = .;

	/*
	 * Calculate sizes of the binary file and image loaded into memory as
	 * well as the text, read-only and read-write data sections.
	 */
	_bin_size = ABSOLUTE(_bin_end - _image_start);
	_image_size = ABSOLUTE(_image_end - _image_start);
}
