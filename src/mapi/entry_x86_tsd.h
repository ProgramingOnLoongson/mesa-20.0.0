/*
 * Mesa 3-D graphics library
 *
 * Copyright (C) 2010 LunarG Inc.
 *
 * Permission is hereby granted, free of charge, to any person obtaining a
 * copy of this software and associated documentation files (the "Software"),
 * to deal in the Software without restriction, including without limitation
 * the rights to use, copy, modify, merge, publish, distribute, sublicense,
 * and/or sell copies of the Software, and to permit persons to whom the
 * Software is furnished to do so, subject to the following conditions:
 *
 * The above copyright notice and this permission notice shall be included
 * in all copies or substantial portions of the Software.
 *
 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
 * IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
 * FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT.  IN NO EVENT SHALL
 * THE AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
 * LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING
 * FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER
 * DEALINGS IN THE SOFTWARE.
 *
 * Authors:
 *    Chia-I Wu <olv@lunarg.com>
 */

#ifdef HAVE_FUNC_ATTRIBUTE_VISIBILITY
#define HIDDEN __attribute__((visibility("hidden")))
#else
#define HIDDEN
#endif

#define X86_ENTRY_SIZE 64

__asm__(".text\n"
        ".balign 32\n"
        "x86_entry_start:");

#define STUB_ASM_ENTRY(func)        \
   ".globl " func "\n"              \
   ".type " func ", @function\n"    \
   ".balign 32\n"                   \
   func ":"

#define STUB_ASM_CODE(slot)         \
   "call 1f\n\t"                    \
   "1:\n\t"                         \
   "popl %ecx\n\t"                  \
   "addl $_GLOBAL_OFFSET_TABLE_+[.-1b], %ecx\n\t" \
   "movl " ENTRY_CURRENT_TABLE "@GOT(%ecx), %eax\n\t" \
   "mov (%eax), %eax\n\t"           \
   "testl %eax, %eax\n\t"           \
   "jne 1f\n\t"                     \
   "push %ebx\n\t"                  \
   "movl %ecx, %ebx\n\t"            \
   "call " ENTRY_CURRENT_TABLE_GET "@PLT\n\t" \
   "popl %ebx\n\t"                  \
   "1:\n\t"                         \
   "jmp *(4 * " slot ")(%eax)"

#define MAPI_TMP_STUB_ASM_GCC
#include "mapi_tmp.h"

#ifndef MAPI_MODE_BRIDGE

__asm__(".balign 32\n"
        "x86_entry_end:");

#include <string.h>
#include "u_execmem.h"

extern const char x86_entry_start[] HIDDEN;
extern const char x86_entry_end[] HIDDEN;

void
entry_patch_public(void)
{
}

mapi_func
entry_get_public(int slot)
{
   return (mapi_func) (x86_entry_start + slot * X86_ENTRY_SIZE);
}

void
entry_patch(mapi_func entry, int slot)
{
   char *code = (char *) entry;

   *((unsigned long *) (code + 11)) = slot * sizeof(mapi_func);
   *((unsigned long *) (code + 22)) = slot * sizeof(mapi_func);
}

mapi_func
entry_generate(int slot)
{
   const char *code_templ = x86_entry_end - X86_ENTRY_SIZE;
   void *code;
   mapi_func entry;

   code = u_execmem_alloc(X86_ENTRY_SIZE);
   if (!code)
      return NULL;

   memcpy(code, code_templ, X86_ENTRY_SIZE);
   entry = (mapi_func) code;
   entry_patch(entry, slot);

   return entry;
}

#endif /* MAPI_MODE_BRIDGE */
