/*
This is - util.cpp (General utility code)
Copyright (C) 2003 Nullsoft, Inc.
Copyright (C) 2004 WASTE Development Team

This is free software; you can redistribute it and/or modify
it under the terms of the GNU General Public License as published by
the Free Software Foundation; either version 2 of the License, or
(at your option) any later version.

This is distributed in the hope that it will be useful,
but WITHOUT ANY WARRANTY; without even the implied warranty of
MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
GNU General Public License for more details.

You should have received a copy of the GNU General Public License
along with WASTE; if not, write to the Free Software
Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA  02111-1307  USA
*/
#include "stdafx.h"
#include "util.hpp"
#include <string>
#include <windows.h>
#include <mmsystem.h>
#include <WinUser.h>

#define WM_USER_SYSTRAY (WM_USER+0x12)

// convert a number to a string
void tostring(char str[], int num)
{
	int i, rem, len = 0, n;
	n = num;
	while (n != 0)
	{
		len++;
		n /= 10;
	}

	for (i = 0; i < len; i++)
	{
		rem = num % 10;
		num = num / 10;
		str[len - (i + 1)] = rem + '0';
	}
	str[len] = '\0';
}


// Spidy - auto start option
void getExeNameStr(std::string * strResult) {
	CString CStrcaExePath;
	CStrcaExePath.Preallocate(MAX_PATH);
	GetModuleFileName(NULL, CStrcaExePath.GetBuffer(), MAX_PATH);
	CT2CA pszConvertedAnsiString (CStrcaExePath);
	std::string strAppName(pszConvertedAnsiString);

	strResult->assign(strAppName.substr(strAppName.rfind("\\") + 1));

	CStrcaExePath.ReleaseBuffer();
}

bool regAutoStartExists() {

	HKEY hKey;
	bool valueExists, valueAutostartExists, valueAutostartMinimizedExists = false;
	CString CStrRegPath;
	LPCSTR lpRegPath = ("SOFTWARE\\Microsoft\\Windows\\CurrentVersion\\Run");
	if (RegOpenKeyExA(HKEY_CURRENT_USER, lpRegPath,0,KEY_ALL_ACCESS,&hKey) == ERROR_SUCCESS) {
		std::string exeName;
		getExeNameStr(&exeName);
		if(RegQueryValueExA(hKey, (LPCSTR)(exeName.c_str()), NULL, NULL, NULL, NULL) == ERROR_FILE_NOT_FOUND) {
			valueAutostartExists = false;
		}
		else {
			valueAutostartExists = true;
		}

		if (RegQueryValueExA(hKey, (LPCSTR)(exeName.append(" /minimized")).c_str(), NULL, NULL, NULL, NULL) == ERROR_FILE_NOT_FOUND) {
			valueAutostartMinimizedExists = false;
		} else {
			valueAutostartMinimizedExists = true;
		}
	}
	CStrRegPath.ReleaseBuffer();
	RegCloseKey(hKey);

	if (valueAutostartExists || valueAutostartMinimizedExists) {
		return true;
	}
	else {
		return false;
	}
}

bool regAutoStartMinimizedExists() {

	HKEY hKey;
	bool valueExists = false;
	LPCSTR lpRegPath = ("SOFTWARE\\Microsoft\\Windows\\CurrentVersion\\Run");
	if (RegOpenKeyExA(HKEY_CURRENT_USER, lpRegPath,0,KEY_ALL_ACCESS,&hKey) == ERROR_SUCCESS) {
		std::string exeName;
		getExeNameStr(&exeName);
		if (RegQueryValueExA(hKey, (LPCSTR)(exeName.append(" /minimized")).c_str(), NULL, NULL, NULL, NULL) == ERROR_FILE_NOT_FOUND) {
			valueExists = false;
		}
		else {
			valueExists = true;
		}
	}
	RegCloseKey(hKey);
	return valueExists;
}

bool regSetAutoStart(bool minimized) {
	HKEY hKey;

	TCHAR szPath[MAX_PATH];
	GetModuleFileName(NULL, szPath, MAX_PATH );	

	bool lRet, valueExists = false;
	if (RegOpenKeyExA(HKEY_CURRENT_USER, (LPCSTR)"SOFTWARE\\Microsoft\\Windows\\CurrentVersion\\Run",0,KEY_ALL_ACCESS,&hKey) == ERROR_SUCCESS) {
		if (minimized) {
			CString fullPathWithMinimized;
			fullPathWithMinimized += "\"";
			fullPathWithMinimized += szPath;
			fullPathWithMinimized += "\"";
			fullPathWithMinimized += " /minimized";

			CT2CA pszConvertedAnsiString (fullPathWithMinimized);
			std::string strFullPathWithMinimized(pszConvertedAnsiString);

			std::string exeName;
			getExeNameStr(&exeName);
			if(RegSetValueExA(hKey, (LPCSTR)((exeName.append(" /minimized")).c_str()), 0, REG_SZ, (BYTE *)strFullPathWithMinimized.c_str(), MAX_PATH) == ERROR_SUCCESS) {
				lRet = true;
			}
		}
		else {
			CString fullPath;
			fullPath += "\"";
			fullPath += szPath;
			fullPath += "\"";

			CT2CA pszConvertedAnsiString (fullPath);
			std::string strFullPath(pszConvertedAnsiString);

			std::string exeName;
			getExeNameStr(&exeName);
			if(RegSetValueExA(hKey, (LPCSTR)(exeName.c_str()), 0, REG_SZ, (BYTE *)strFullPath.c_str(), MAX_PATH) == ERROR_SUCCESS) {
				lRet = true;
			}
		}
		RegCloseKey(hKey);
	}
	return lRet;
}

void regDeleteAutoStart () {
	HKEY hKey;
	if (RegOpenKeyExA(HKEY_CURRENT_USER, (LPCSTR)"SOFTWARE\\Microsoft\\Windows\\CurrentVersion\\Run",0,KEY_ALL_ACCESS,&hKey) == ERROR_SUCCESS) {
		std::string exeName;
		getExeNameStr(&exeName);
		RegDeleteValueA(hKey, (LPCSTR)(exeName.c_str()));
		RegDeleteValueA(hKey, (LPCSTR) (exeName.append(" /minimized")).c_str());
		RegCloseKey(hKey);
	}
}

BOOL systray_shutdown_notify(HWND hWnd, UINT uniqueID)
{
	// http://www.codeproject.com/kb/shell/stealthdialog.aspx?display=printall
	HICON hicon = LoadIcon(NULL,MAKEINTRESOURCE(IDI_INFORMATION));

	// Tray icon
	NOTIFYICONDATA tnid = {};
	tnid.cbSize = sizeof(NOTIFYICONDATA);
	memset( &tnid, 0, sizeof(tnid) );

	tnid.hWnd = hWnd;
	tnid.uID = uniqueID;

	tnid.uVersion = NOTIFYICON_VERSION;
	tnid.uFlags = NIF_ICON;
	tnid.uCallbackMessage = WM_USER_SYSTRAY;
	tnid.hIcon = hicon;
	Shell_NotifyIcon(NIM_DELETE, &tnid);
	Shell_NotifyIcon(NIM_SETVERSION, &tnid);
	Shell_NotifyIcon(NIM_ADD, &tnid);

	// Notification balloon
	// https://stackoverflow.com/questions/1414947/display-balloon-tooltip-in-systemtray
	tnid.hWnd = GetActiveWindow();
	tnid.cbSize =sizeof(NOTIFYICONDATA);
	tnid.uCallbackMessage = WM_USER_SYSTRAY;
	tnid.hIcon = hicon;
	tnid.uTimeout = 300000;
	tnid.uVersion = NOTIFYICON_VERSION;
	tnid.uFlags = NIF_INFO | NIF_MESSAGE | NIF_ICON | NIF_TIP;
	tnid.uID = uniqueID;
	tnid.dwInfoFlags = NIIF_INFO;
	StrCpyW(tnid.szInfoTitle,_T("MicroSIP: Shutting Down..."));
	char cInfo[256];
	sprintf(cInfo, "%s", "Unbinding uPnP");
	StrCpyW(tnid.szInfo, convertCharArrayToLPCWSTR(cInfo));
	StrCpyW(tnid.szTip, _T("MicroSIP: Shutting Down...\nUnbinding uPnP"));
	Shell_NotifyIcon(NIM_MODIFY,&tnid);

	if (hicon) 
        DestroyIcon(hicon); 

	// Play windows notification sound as we have put a notification on the box
	PlaySound(TEXT("SystemExit"), NULL, SND_ALIAS);
	return true;
}

BOOL systray_add(HWND hwnd, HICON icon)
{
	NOTIFYICONDATA tnid;
	tnid.cbSize = sizeof(NOTIFYICONDATA);
	tnid.hWnd = hwnd;
	tnid.uID = 232;
	tnid.uFlags = NIF_ICON | NIF_TIP | NIF_MESSAGE;
	tnid.uCallbackMessage = WM_USER_SYSTRAY;
	tnid.hIcon = icon;
	GetWindowText(hwnd,tnid.szTip,sizeof(tnid.szTip));
	return (Shell_NotifyIcon(NIM_ADD, &tnid));
}

BOOL systray_del(HWND hWnd, UINT uniqueID)
{
	NOTIFYICONDATA tnid;
	tnid.cbSize = sizeof(NOTIFYICONDATA);
	tnid.hWnd = hWnd;
	tnid.uID = uniqueID;
	return(Shell_NotifyIcon(NIM_DELETE, &tnid));
}

wchar_t *convertCharArrayToLPCWSTR(const char* charArray)
{
	// https://stackoverflow.com/questions/19715144/how-to-convert-char-to-lpcwstr
    wchar_t* wString=new wchar_t[4096];
    MultiByteToWideChar(CP_ACP, 0, charArray, -1, wString, 4096);
    return wString;
}