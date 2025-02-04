/* Disassemble support for GDB.
   Copyright (C) 2002-2021 Free Software Foundation, Inc.

   This file is part of GDB.

   This program is free software; you can redistribute it and/or modify
   it under the terms of the GNU General Public License as published by
   the Free Software Foundation; either version 3 of the License, or
   (at your option) any later version.

   This program is distributed in the hope that it will be useful,
   but WITHOUT ANY WARRANTY; without even the implied warranty of
   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
   GNU General Public License for more details.

   You should have received a copy of the GNU General Public License
   along with this program.  If not, see <http://www.gnu.org/licenses/>.  */

#ifndef DISASM_H
#define DISASM_H

#include "dis-asm.h"
#include "gdbsupport/enum-flags.h"

enum gdb_disassembly_flag
  {
    DISASSEMBLY_SOURCE_DEPRECATED = (0x1 << 0),
    DISASSEMBLY_RAW_INSN = (0x1 << 1),
    DISASSEMBLY_OMIT_FNAME = (0x1 << 2),
    DISASSEMBLY_FILENAME = (0x1 << 3),
    DISASSEMBLY_OMIT_PC = (0x1 << 4),
    DISASSEMBLY_SOURCE = (0x1 << 5),
    DISASSEMBLY_SPECULATIVE = (0x1 << 6),
  };
DEF_ENUM_FLAGS_TYPE (enum gdb_disassembly_flag, gdb_disassembly_flags);

struct gdbarch;
struct ui_out;
struct ui_file;

class gdb_disassembler
{
  using di_read_memory_ftype = decltype (disassemble_info::read_memory_func);

public:
  gdb_disassembler (struct gdbarch *gdbarch, struct ui_file *file)
    : gdb_disassembler (gdbarch, file, dis_asm_read_memory)
  {}

  ~gdb_disassembler ();

  DISABLE_COPY_AND_ASSIGN (gdb_disassembler);

  int print_insn (CORE_ADDR memaddr, int *branch_delay_insns = NULL);

  /* Return the gdbarch of gdb_disassembler.  */
  struct gdbarch *arch ()
  { return m_gdbarch; }

protected:
  gdb_disassembler (struct gdbarch *gdbarch, struct ui_file *file,
		    di_read_memory_ftype func);

  struct ui_file *stream ()
  { return (struct ui_file *) m_di.stream; }

private:
  struct gdbarch *m_gdbarch;

  /* Stores data required for disassembling instructions in
     opcodes.  */
  struct disassemble_info m_di;

  /* If we own the string in `m_di.disassembler_options', we do so
     using this field.  */
  std::string m_disassembler_options_holder;

  /* This member variable is given a value by calling dis_asm_memory_error.
     If after calling into the libopcodes disassembler we get back a
     negative value (which indicates an error), then, if this variable has
     a value, we report a memory error to the user, otherwise, we report a
     non-memory error.  */
  gdb::optional<CORE_ADDR> m_err_memaddr;

  static int dis_asm_read_memory (bfd_vma memaddr, gdb_byte *myaddr,
				  unsigned int len,
				  struct disassemble_info *info);
  static void dis_asm_memory_error (int err, bfd_vma memaddr,
				    struct disassemble_info *info);
  static void dis_asm_print_address (bfd_vma addr,
				     struct disassemble_info *info);
};

/* An instruction to be disassembled.  */

struct disasm_insn
{
  /* The address of the memory containing the instruction.  */
  CORE_ADDR addr;

  /* An optional instruction number.  If non-zero, it is printed first.  */
  unsigned int number;

  /* True if the instruction was executed speculatively.  */
  unsigned int is_speculative:1;
};

extern void gdb_disassembly (struct gdbarch *gdbarch, struct ui_out *uiout,
			     gdb_disassembly_flags flags, int how_many,
			     CORE_ADDR low, CORE_ADDR high);

/* Print the instruction at address MEMADDR in debugged memory,
   on STREAM.  Returns the length of the instruction, in bytes,
   and, if requested, the number of branch delay slot instructions.  */

extern int gdb_print_insn (struct gdbarch *gdbarch, CORE_ADDR memaddr,
			   struct ui_file *stream, int *branch_delay_insns);

/* Class used to pretty-print instructions.  */

class gdb_pretty_print_disassembler
{
public:
  explicit gdb_pretty_print_disassembler (struct gdbarch *gdbarch,
					  struct ui_out *uiout)
    : m_uiout (uiout),
      m_insn_stb (uiout->can_emit_style_escape ()),
      m_di (gdbarch, &m_insn_stb)
  {}

  /* Prints the instruction INSN into the saved ui_out and returns the
     length of the printed instruction in bytes.  */
  int pretty_print_insn (const struct disasm_insn *insn,
			 gdb_disassembly_flags flags);

private:
  /* Returns the architecture used for disassembling.  */
  struct gdbarch *arch () { return m_di.arch (); }

  /* The ui_out that is used by pretty_print_insn.  */
  struct ui_out *m_uiout;

  /* The buffer used to build the instruction string.  The
     disassembler is initialized with this stream.  */
  string_file m_insn_stb;

  /* The disassembler used for instruction printing.  */
  gdb_disassembler m_di;

  /* The buffer used to build the raw opcodes string.  */
  string_file m_opcode_stb;
};

/* Return the length in bytes of the instruction at address MEMADDR in
   debugged memory.  */

extern int gdb_insn_length (struct gdbarch *gdbarch, CORE_ADDR memaddr);

/* Return the length in bytes of INSN, originally at MEMADDR.  MAX_LEN
   is the size of the buffer containing INSN.  */

extern int gdb_buffered_insn_length (struct gdbarch *gdbarch,
				     const gdb_byte *insn, int max_len,
				     CORE_ADDR memaddr);

/* Returns GDBARCH's disassembler options.  */

extern char *get_disassembler_options (struct gdbarch *gdbarch);

/* Sets the active gdbarch's disassembler options to OPTIONS.  */

extern void set_disassembler_options (const char *options);

#endif
