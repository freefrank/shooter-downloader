// ShooterContextMenuExt.cpp : Implementation of CShooterContextMenuExt

#include "stdafx.h"
#include "ShooterContextMenuExt.h"


// CShooterContextMenuExt

STDMETHODIMP CShooterContextMenuExt::Initialize ( 
  LPCITEMIDLIST pidlFolder,
  LPDATAOBJECT pDataObj,
  HKEY hProgID )
{
	TCHAR     szFile[MAX_PATH];
	FORMATETC fmt = { CF_HDROP, NULL, DVASPECT_CONTENT, -1, TYMED_HGLOBAL };
	STGMEDIUM stg = { TYMED_HGLOBAL };
	HDROP     hDrop;

	// Look for CF_HDROP data in the data object.
	if ( FAILED( pDataObj->GetData ( &fmt, &stg ) ))
	{
		// Nope! Return an "invalid argument" error back to Explorer.
		return E_INVALIDARG;
	}

	// Get a pointer to the actual data.
	hDrop = (HDROP) GlobalLock ( stg.hGlobal );

	// Make sure it worked.
	if ( NULL == hDrop )
	{
		ReleaseStgMedium ( &stg );
		return E_INVALIDARG;
	}

	// Sanity check - make sure there is at least one filename.
	UINT uNumFiles = DragQueryFile ( hDrop, 0xFFFFFFFF, NULL, 0 );
	HRESULT hr = S_OK;

	if ( 0 == uNumFiles )
	{
		GlobalUnlock ( stg.hGlobal );
		ReleaseStgMedium ( &stg );
		return E_INVALIDARG;
	}

	// Get the name of the first file and store it in our member variable m_szFile.
	//if ( 0 == DragQueryFile ( hDrop, 0, m_szFile, MAX_PATH ) )
	//	hr = E_INVALIDARG;
	for(UINT uFile = 0 ; uFile < uNumFiles ; uFile++)
	{
		if(0 == DragQueryFile(hDrop, uFile, szFile, MAX_PATH))
			continue;

		ATLTRACE("Checking file <%s>\n", szFile);

		m_fileList.push_back(szFile);

	}


	GlobalUnlock ( stg.hGlobal );
	ReleaseStgMedium ( &stg );

	return hr;
}

STDMETHODIMP CShooterContextMenuExt::QueryContextMenu (
    HMENU hmenu, UINT uMenuIndex, UINT uidFirstCmd,
    UINT uidLastCmd, UINT uFlags )
{
	// If the flags include CMF_DEFAULTONLY then we shouldn't do anything.
    if ( uFlags & CMF_DEFAULTONLY )
        return MAKE_HRESULT ( SEVERITY_SUCCESS, FACILITY_NULL, 0 );

    InsertMenu ( hmenu, uMenuIndex, MF_BYPOSITION, uidFirstCmd, _T("Download Subtitles") );

    return MAKE_HRESULT ( SEVERITY_SUCCESS, FACILITY_NULL, 1 );
}

STDMETHODIMP CShooterContextMenuExt::GetCommandString (
    UINT_PTR idCmd, UINT uFlags, UINT* pwReserved, LPSTR pszName, UINT cchMax )
{
	USES_CONVERSION;

	// Check idCmd, it must be 0 since we have only one menu item.
	if ( 0 != idCmd )
		return E_INVALIDARG;

	// If Explorer is asking for a help string, copy our string into the
	// supplied buffer.
	if ( uFlags & GCS_HELPTEXT )
	{
		LPCTSTR szText = _T("Download subtitles.");

		if ( uFlags & GCS_UNICODE )
		{
			// We need to cast pszName to a Unicode string, and then use the
			// Unicode string copy API.
			lstrcpynW ( (LPWSTR) pszName, T2CW(szText), cchMax );
		}
		else
		{
			// Use the ANSI string copy API to return the help string.
			lstrcpynA ( pszName, T2CA(szText), cchMax );
		}

		return S_OK;
	}

	return E_INVALIDARG;
}

STDMETHODIMP CShooterContextMenuExt::InvokeCommand ( LPCMINVOKECOMMANDINFO pCmdInfo )
{
	// If lpVerb really points to a string, ignore this function call and bail out.
	if ( 0 != HIWORD( pCmdInfo->lpVerb ) )
		return E_INVALIDARG;

	// Get the command index - the only valid one is 0.
	switch ( LOWORD( pCmdInfo->lpVerb) )
	{
	case 0:
		{
			TCHAR szShooterDir[MAX_PATH];
			TCHAR szShooterDldrPath[MAX_PATH];

			//_tcscpy_s(szMsg, _T("The selected file was:\n"));

			//StringList::const_iterator itor;
			//for(itor = m_fileList.begin(); itor != m_fileList.end(); itor++)
			//{
			//	_tcscat_s(szMsg, _T("\n"));
			//	_tcscat_s(szMsg, itor->c_str());
			//}
			HINSTANCE hModule = _AtlBaseModule.GetModuleInstance();
			GetModuleFileName((HMODULE) hModule, szShooterDir, sizeof(szShooterDir));
			TCHAR* pLastSlash = _tcsrchr(szShooterDir, _T('\\'));
			*(pLastSlash + 1) = _T('\0');


			_tcscpy_s(szShooterDldrPath, szShooterDir);
			_tcscat_s(szShooterDldrPath, SHOOTER_DLDR_FILE_NAME);

			TCHAR szTempDir[MAX_PATH], szTempFilePath[MAX_PATH];
			// Get the temp path.
			DWORD dwRetVal = GetTempPath(MAX_PATH,     // length of the buffer
				szTempDir); // buffer for path 
			if (dwRetVal > MAX_PATH || (dwRetVal == 0))
			{
				return E_FAIL;
			}

			// Create a temporary file. 
			UINT uRetVal = GetTempFileName(szTempDir, // directory for tmp files
				TEXT("SDL"),  // temp file name prefix 
				0,            // create unique name 
				szTempFilePath);  // buffer for name 
			if (uRetVal == 0)
			{
				return E_FAIL;
			}
			MessageBox ( pCmdInfo->hwnd, szTempFilePath, _T("ShooterDownloader"),
				MB_ICONINFORMATION );

			FILE* fp; 
			errno_t ret = _tfopen_s(&fp, szTempFilePath, _T("w, ccs=UTF-8"));
			if(ret != 0)
			{
				return E_FAIL;
			}
			
			//_ftprintf_s(fp, "%s\n", 
			StringList::const_iterator itor;
			for(itor = m_fileList.begin(); itor != m_fileList.end(); itor++)
			{
				_ftprintf_s(fp, _T("%s\n"), itor->c_str());
			}

			fflush(fp);
			fclose(fp);

			return S_OK;
		}
		break;

	default:
		return E_INVALIDARG;
		break;
	}
}
