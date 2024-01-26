//----------------------------------------------------------------------------
//  EPI String Utilities
//----------------------------------------------------------------------------
//
//  Copyright (c) 2007-2024 The EDGE Team.
//
//  This program is free software; you can redistribute it and/or
//  modify it under the terms of the GNU General Public License
//  as published by the Free Software Foundation; either version 3
//  of the License, or (at your option) any later version.
//
//  This program is distributed in the hope that it will be useful,
//  but WITHOUT ANY WARRANTY; without even the implied warranty of
//  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
//  GNU General Public License for more details.
//
//----------------------------------------------------------------------------

#ifndef __EPI_STR_UTIL_H__
#define __EPI_STR_UTIL_H__

#include <string>
#include <vector>

namespace epi
{

void STR_Lower(std::string &s);
void STR_Upper(std::string &s);

void STR_TextureNameFromFilename(std::string &buf, const std::string &stem);

std::string STR_Format(const char *fmt, ...) GCCATTR((format(printf, 1, 2)));

std::vector<std::string> STR_SepStringVector(std::string str, char separator);

uint32_t STR_Hash32(std::string str_to_hash);

#ifdef _WIN32
// Technically these are to and from UTF-16, but since these are only for 
// Windows "wide" APIs I think we'll be ok - Dasho
std::string wstring_to_utf8(const wchar_t *instring);
std::wstring utf8_to_wstring(std::string_view instring);
#endif

} // namespace epi

#endif /* __EPI_STR_UTIL_H__ */

//--- editor settings ---
// vi:ts=4:sw=4:noexpandtab
