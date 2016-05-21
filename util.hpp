/*
WASTE - util.hpp (General utility code)
Copyright (C) 2003 Nullsoft, Inc.
Copyright (C) 2004 WASTE Development Team

WASTE is free software; you can redistribute it and/or modify
it under the terms of the GNU General Public License as published by
the Free Software Foundation; either version 2 of the License, or
(at your option) any later version.

WASTE  is distributed in the hope that it will be useful,
but WITHOUT ANY WARRANTY; without even the implied warranty of
MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
GNU General Public License for more details.

You should have received a copy of the GNU General Public License
along with WASTE; if not, write to the Free Software
Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA  02111-1307  USA
*/
#pragma once

// Spidy - autostart registry util
bool regAutoStartExists();
bool regAutoStartMinimizedExists();
bool regSetAutoStart(bool minimized);
void regDeleteAutoStart();

// Spidy - systray
BOOL systray_shutdown_notify(HWND hWnd, UINT uniqueID);
BOOL systray_del(HWND hWnd, UINT uniqueID);
BOOL systray_add(HWND hwnd, HICON icon);

void tostring(char str[], int num);
wchar_t *convertCharArrayToLPCWSTR(const char* charArray);