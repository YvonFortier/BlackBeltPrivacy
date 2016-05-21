#pragma once

#include "resource.h"
#include "global.h"
#include "AddDlg.h"
#include "BaseDialog.h"

#ifndef _GLOBAL_NO_CONTACTS

class Contacts :
	public CBaseDialog
{
public:
	Contacts(CWnd* pParent = NULL);	// standard constructor
	~Contacts();
	enum { IDD = IDD_CONTACTS };
	AddDlg* addDlg;
	BOOL isSubscribed;
	bool presenceOperationIsActive;

	bool ContactAdd(CString number, CString name, char presence, char directory, BOOL save = FALSE, BOOL fromDirectory = FALSE);

	void ContactDelete(int i, bool notSave = false);
	void ContactsSave();

	void SetCanditates();
	int DeleteCanditates();

	void UpdateCallButton();
	CString GetNameByNumber(CString number);
	void PresenceUpdateOne(Contact *pContact);
	void PresenceSubscribeOne(Contact *pContact);
	void PresenceUnsubscribeOne(Contact *pContact);
	void PresenceUpdate();
	void PresenceSubscribe();
	void PresenceUnsubscribe();

private:
	CImageList* imageList;
	void ContactsLoad();
	void ContactDecode(CString str, CString &number, CString &name, BOOL &presence, BOOL &fromDirectory);
	void MessageDlgOpen(BOOL isCall = FALSE, BOOL hasVideo = FALSE);
	void DefaultItemAction(int i);

protected:
	virtual BOOL OnInitDialog();
	virtual void PostNcDestroy();
	DECLARE_MESSAGE_MAP()
public:
	afx_msg void OnBnClickedOk();
	afx_msg void OnBnClickedCancel();
	afx_msg void OnMenuCallPickup();
	afx_msg void OnMenuCall(); 
	afx_msg void OnMenuChat();
	afx_msg void OnMenuAdd(); 
	afx_msg void OnMenuEdit(); 
	afx_msg void OnMenuCopy(); 
	afx_msg void OnMenuDelete(); 
	afx_msg LRESULT OnContextMenu(WPARAM wParam,LPARAM lParam);
	afx_msg void OnNMDblclkContacts(NMHDR *pNMHDR, LRESULT *pResult);
#ifdef _GLOBAL_VIDEO
	afx_msg void OnMenuCallVideo(); 
#endif

};

#endif