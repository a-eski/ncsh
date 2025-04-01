/* ncsh arena - a way to allocate memory used by ncsh */

/* Special thanks to skeeto, Chris Wellons, for his amazing documentation on arena allocators.
   For more information, please reference https://nullprogram.com/blog/2023/09/27/
   or https://gist.github.com/skeeto/42d8a23871642696b6b8de30d9222328 */

/* Copyright (C) ncsh by Alex Eski 2025 */

/* This program is free software: you can redistribute it and/or modify
   it under the terms of the GNU General Public License as published by
   the Free Software Foundation, either version 3 of the License, or
   (at your option) any later version.

   This program is distributed in the hope that it will be useful,
   but WITHOUT ANY WARRANTY; without even the implied warranty of
   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
   GNU General Public License for more details.

   You should have received a copy of the GNU General Public License
   along with this program.  If not, see <http://www.gnu.org/licenses/>.  */

#include <assert.h>
#include <stddef.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/cdefs.h>

#include <readline/ncsh_arena.h>

__attribute_malloc__
void* ncsh_arena_malloc_internal(struct ncsh_Arena* arena,
                     uintptr_t count,
                     uintptr_t size,
                     uintptr_t alignment)
{
    uintptr_t padding = -(uintptr_t)arena->start & (alignment - 1);
    uintptr_t available = (uintptr_t)arena->end - (uintptr_t)arena->start - padding;
    assert(count < available / size);
    if (available == 0 || count > available / size) {
        puts("ncsh: ran out of allocated memory.");
        abort();
    }
    void* val = arena->start + padding;
    arena->start += padding + count * size;
    return memset(val, 0, count * size);
}


__attribute_malloc__
void* ncsh_arena_realloc_internal(struct ncsh_Arena* arena,
                     uintptr_t count,
                     uintptr_t size,
                     uintptr_t alignment,
                     void* old_ptr,
                     uintptr_t old_count)
{
    uintptr_t padding = -(uintptr_t)arena->start & (alignment - 1);
    uintptr_t available = (uintptr_t)arena->end - (uintptr_t)arena->start - padding;
    assert(count < available / size);
    if (available == 0 || count > available / size) {
        puts("ncsh: ran out of allocated memory.");
        abort();
    }
    void* val = arena->start + padding;
    arena->start += padding + count * size;
    memset(val, 0, count * size);
    assert(old_ptr);
    return memcpy(val, old_ptr, old_count * size);
}
