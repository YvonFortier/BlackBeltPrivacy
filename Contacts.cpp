#include "StdAfx.h"
#include "Contacts.h"
#include "microsip.h"
#include "settings.h"
#include <pjsua-lib/pjsua.h>
#include <pjsua-lib/pjsua_internal.h>
#include "microsipDlg.h"
#include "utf.h"

#ifndef _GLOBAL_NO_CONTACTS

static CmicrosipDlg *microsipDlg;

Contacts::Contacts(CWnd* pParent /*=NULL*/)
: CBaseDialog(Contacts::IDD, pParent)
{
	microsipDlg = (CmicrosipDlg* ) AfxGetMainWnd();	
	Create (IDD, pParent);
}

Contacts::~Contacts(void)
{
}

BOOL Contacts::OnInitDialog()
{
	CBaseDialog::OnInitDialog();

	AutoMove(IDC_CONTACTS,0,0,100,100);

	TranslateDialog(this->m_hWnd);

	addDlg = new AddDlg(this);
	imageList = new CImageList();
	imageList->Create(16,16,ILC_COLOR32,3,3);
	imageList->SetBkColor(RGB(255, 255, 255));
	imageList->Add(theApp.LoadIcon(IDI_UNKNOWN));
	imageList->Add(theApp.LoadIcon(IDI_OFFLINE));
	imageList->Add(theApp.LoadIcon(IDI_AWAY));
	imageList->Add(theApp.LoadIcon(IDI_ONLINE));
	imageList->Add(theApp.LoadIcon(IDI_ACTIVE));
	imageList->Add(theApp.LoadIcon(IDI_BLANK));
	imageList->Add(theApp.LoadIcon(IDI_BUSY));
	imageList->Add(theApp.LoadIcon(IDI_DEFAULT));


	CListCtrl *list= (CListCtrl*)GetDlgItem(IDC_CONTACTS);
	list->SetImageList(imageList,LVSIL_SMALL);

	ContactsLoad();

	presenceOperationIsActive = false;

	return TRUE;
}

void Contacts::PostNcDestroy()
{
	CBaseDialog::PostNcDestroy();
	if (pj_ready) {
		PresenceUnsubscribe();
	}
	microsipDlg->pageContacts=NULL;
	delete imageList;
	delete this;
}

BEGIN_MESSAGE_MAP(Contacts, CBaseDialog)
	ON_BN_CLICKED(IDOK, OnBnClickedOk)
	ON_BN_CLICKED(IDCANCEL, OnBnClickedCancel)
	ON_COMMAND(ID_CALL_PICKUP,OnMenuCallPickup)
	ON_COMMAND(ID_CALL,OnMenuCall)
	ON_COMMAND(ID_CHAT,OnMenuChat)
	ON_COMMAND(ID_ADD,OnMenuAdd)
	ON_COMMAND(ID_EDIT,OnMenuEdit)
	ON_COMMAND(ID_COPY,OnMenuCopy)
	ON_COMMAND(ID_DELETE,OnMenuDelete)
	ON_MESSAGE(WM_CONTEXTMENU,OnContextMenu)
	ON_NOTIFY(NM_DBLCLK, IDC_CONTACTS, &Contacts::OnNMDblclkContacts)
#ifdef _GLOBAL_VIDEO
	ON_COMMAND(ID_VIDEOCALL,OnMenuCallVideo)
#endif
END_MESSAGE_MAP()


void Contacts::OnBnClickedOk()
{
	CListCtrl *list= (CListCtrl*)GetDlgItem(IDC_CONTACTS);
	POSITION pos = list->GetFirstSelectedItemPosition();
	if (pos) {
		DefaultItemAction(list->GetNextSelectedItem(pos));
	}
}

void Contacts::DefaultItemAction(int i)
{
	CListCtrl *list= (CListCtrl*)GetDlgItem(IDC_CONTACTS);
	Contact *pContact = (Contact *) list->GetItemData(i);
	if (pContact->ringing) {
		OnMenuCallPickup();
	} else {
		MessageDlgOpen(accountSettings.singleMode);
	}
}

void Contacts::OnBnClickedCancel()
{
	microsipDlg->ShowWindow(SW_HIDE);
}

LRESULT Contacts::OnContextMenu(WPARAM wParam,LPARAM lParam)
{
	int x = GET_X_LPARAM(lParam); 
	int y = GET_Y_LPARAM(lParam); 
	POINT pt = { x, y };
	RECT rc;
	CListCtrl *list= (CListCtrl*)GetDlgItem(IDC_CONTACTS);
	POSITION pos = list->GetFirstSelectedItemPosition();
	int selectedItem = -1;
	if (pos) {
		selectedItem = list->GetNextSelectedItem(pos);
	}
	if (x!=-1 || y!=-1) {
		ScreenToClient(&pt);
		GetClientRect(&rc); 
		if (!PtInRect(&rc, pt)) {
			x = y = -1;
		} 
	} else {
		if (selectedItem != -1) {
			list->GetItemPosition(selectedItem,&pt);
			list->ClientToScreen(&pt);
			x = 40+pt.x;
			y = 8+pt.y;
		} else {
			::ClientToScreen((HWND)wParam, &pt);
			x = 10+pt.x;
			y = 10+pt.y;
		}
	}
	if (x!=-1 || y!=-1) {
		CMenu menu;
		menu.LoadMenu(IDR_MENU_CONTACT);
		CMenu* tracker = menu.GetSubMenu(0);
		TranslateMenu(tracker->m_hMenu);
		if ( selectedItem != -1 ) {
			Contact *pContact = (Contact *) list->GetItemData(selectedItem);
			if (pContact->ringing) {
				tracker->InsertMenu(ID_CALL,0,ID_CALL_PICKUP,Translate(_T("Call pickup")));
			}
			tracker->EnableMenuItem(ID_CALL, FALSE);
#ifdef _GLOBAL_VIDEO
			tracker->EnableMenuItem(ID_VIDEOCALL, FALSE);
#endif
			tracker->EnableMenuItem(ID_CHAT, FALSE);
			tracker->EnableMenuItem(ID_EDIT, FALSE);
			tracker->EnableMenuItem(ID_COPY, FALSE);
			tracker->EnableMenuItem(ID_DELETE, FALSE);
		} else {
			tracker->EnableMenuItem(ID_CALL, TRUE);
#ifdef _GLOBAL_VIDEO
			tracker->EnableMenuItem(ID_VIDEOCALL, TRUE);
#endif
			tracker->EnableMenuItem(ID_CHAT, TRUE);
			tracker->EnableMenuItem(ID_EDIT, TRUE);
			tracker->EnableMenuItem(ID_COPY, TRUE);
			tracker->EnableMenuItem(ID_DELETE, TRUE);
		}
#ifdef _GLOBAL_NO_MESSAGING
		tracker->RemoveMenu(ID_CHAT,MF_BYCOMMAND);
#endif
		tracker->TrackPopupMenu( 0, x, y, this );
		return TRUE;
	}
	return DefWindowProc(WM_CONTEXTMENU,wParam,lParam);
}

void Contacts::MessageDlgOpen(BOOL isCall, BOOL hasVideo)
{
	CListCtrl *list= (CListCtrl*)GetDlgItem(IDC_CONTACTS);
	POSITION pos = list->GetFirstSelectedItemPosition();
	if (pos) {
		int i = list->GetNextSelectedItem(pos);
		Contact *pContact = (Contact *) list->GetItemData(i);
		microsipDlg->messagesDlg->AddTab(FormatNumber(pContact->number), pContact->name, TRUE, NULL, isCall && accountSettings.singleMode);
		if (isCall)
		{
			microsipDlg->messagesDlg->Call(hasVideo);
		}
	}
}

void Contacts::OnNMDblclkContacts(NMHDR *pNMHDR, LRESULT *pResult)
{
	LPNMITEMACTIVATE pNMItemActivate = reinterpret_cast<LPNMITEMACTIVATE>(pNMHDR);
	if (pNMItemActivate->iItem!=-1) {
		DefaultItemAction(pNMItemActivate->iItem);
	}
	*pResult = 0;
}

void Contacts::OnMenuCallPickup()
{
	CListCtrl *list= (CListCtrl*)GetDlgItem(IDC_CONTACTS);
	POSITION pos = list->GetFirstSelectedItemPosition();
	if (pos) {
		int i = list->GetNextSelectedItem(pos);
		Contact *pContact = (Contact *) list->GetItemData(i);
		microsipDlg->messagesDlg->CallMake(_T("**")+pContact->number);
	}
}

void Contacts::OnMenuCall()
{
	MessageDlgOpen(TRUE);
}

#ifdef _GLOBAL_VIDEO
void Contacts::OnMenuCallVideo()
{
	MessageDlgOpen(TRUE, TRUE);
}
#endif

void Contacts::OnMenuChat()
{
	MessageDlgOpen();
}

void Contacts::OnMenuAdd()
{
	if (!addDlg->IsWindowVisible()) {
		addDlg->ShowWindow(SW_SHOW);
	} else {
		addDlg->SetForegroundWindow();
	}
	addDlg->listIndex = -1;
	addDlg->GetDlgItem(IDC_EDIT_NUMBER)->SetWindowText(NULL);
	addDlg->GetDlgItem(IDC_EDIT_NAME)->SetWindowText(NULL);
#ifndef _GLOBAL_ACCOUNT_MINI
	((CButton *)addDlg->GetDlgItem(IDC_PRESENCE))->SetCheck(0);
#endif
	addDlg->GetDlgItem(IDC_EDIT_NUMBER)->SetFocus();
}

void Contacts::OnMenuEdit()
{
	OnMenuAdd();
	CListCtrl *list= (CListCtrl*)GetDlgItem(IDC_CONTACTS);
	POSITION pos = list->GetFirstSelectedItemPosition();
	int i = list->GetNextSelectedItem(pos);
	addDlg->listIndex = i;

	Contact *pContact = (Contact *) list->GetItemData(i);
	addDlg->GetDlgItem(IDC_EDIT_NUMBER)->SetWindowText(pContact->number);
	addDlg->GetDlgItem(IDC_EDIT_NAME)->SetWindowText(pContact->name);
#ifndef _GLOBAL_ACCOUNT_MINI
	((CButton *)addDlg->GetDlgItem(IDC_PRESENCE))->SetCheck(pContact->presence);
#endif
}

void Contacts::OnMenuCopy()
{
	CListCtrl *list= (CListCtrl*)GetDlgItem(IDC_CONTACTS);
	POSITION pos = list->GetFirstSelectedItemPosition();
	if (pos) {
		int i = list->GetNextSelectedItem(pos);
		Contact *pContact = (Contact *) list->GetItemData(i);
		microsipDlg->CopyStringToClipboard(pContact->number);
	}
}

void Contacts::OnMenuDelete()
{
	CListCtrl *pList= (CListCtrl*)GetDlgItem(IDC_CONTACTS);
	POSITION pos = pList->GetFirstSelectedItemPosition();
	while (pos)	{
		ContactDelete(pList->GetNextSelectedItem(pos));
		pos = pList->GetFirstSelectedItemPosition();
	}
}

void Contacts::ContactDelete(int i, bool notSave)
{
	CListCtrl *list= (CListCtrl*)GetDlgItem(IDC_CONTACTS);
	Contact *pContact = (Contact *) list->GetItemData(i);
	PresenceUnsubscribeOne(pContact);
	delete pContact;
	list->DeleteItem(i);
	if (!notSave) {
		ContactsSave();
	}
}

bool Contacts::ContactAdd(CString number, CString name, char presence, char directory, BOOL save, BOOL fromDirectory)
{
	CListCtrl *list= (CListCtrl*)GetDlgItem(IDC_CONTACTS);
	if (fromDirectory || save) {
		int count = list->GetItemCount();
		for (int i=0;i<count;i++) {
			Contact *pContact = (Contact *) list->GetItemData(i);
			CString contactData;
			if (pContact->number == number) {
				pContact->candidate = FALSE;
				bool changed = false;
				if (!name.IsEmpty() && name!=pContact->name) {
					list->SetItemText(i,0,name);
					pContact->name=name;
					changed = true;
				}
				if (presence!=-1 && presence!=pContact->presence) {
					pContact->presence=presence;
					changed = true;
				}
				if (!fromDirectory && directory!=-1 && directory!=pContact->directory) {
					pContact->directory=directory;
					changed = true;
				}
				if (save && changed) {
					ContactsSave();
				}
				return changed;
			}
		}
	}
	if (name.IsEmpty()) {
		name = number;
	}
	Contact *pContact =  new Contact();
	pContact->number = number;
	pContact->name = name;
	pContact->presence = presence>0;
	pContact->directory = directory>0;
	int i = list->InsertItem(LVIF_TEXT|LVIF_PARAM|LVIF_IMAGE,0,name,0,0,7,(LPARAM)pContact);
	if (save) {
		ContactsSave();
		PresenceSubscribeOne(pContact);
	}
	return true;
}

void Contacts::ContactsSave()
{
	CXMLFile xmlFile; 
	CXMLElement* xmlRoot = new CXMLElement();
	xmlRoot->Create(_T("XML:ROOT"), XET_TAG);
	CXMLElement* xmlContacts = new CXMLElement();
	xmlContacts->Create(_T("contacts"), XET_TAG);
	xmlRoot->AppendChild(xmlContacts);
	CListCtrl *list= (CListCtrl*)GetDlgItem(IDC_CONTACTS);
	int count = list->GetItemCount();
	CXMLElement* xmlContact;
	CXMLElement* xmlAttr;
	for (int i=0;i<count;i++) {
		Contact *pContact = (Contact *) list->GetItemData(i);
		xmlContact = new CXMLElement();
		xmlContact->Create(_T("contact"), XET_TAG);
		xmlAttr = new CXMLElement();
		xmlAttr->Create(_T("number"), XET_ATTRIBUTE);
		xmlAttr->SetValue(AnsiToUnicode(Utf8EncodeUcs2(XMLEntityEncode(pContact->number))).GetBuffer());
		xmlContact->AppendChild(xmlAttr);
		xmlAttr = new CXMLElement();
		xmlAttr->Create(_T("name"), XET_ATTRIBUTE);
		xmlAttr->SetValue(AnsiToUnicode(Utf8EncodeUcs2(XMLEntityEncode(pContact->name))).GetBuffer());
		xmlContact->AppendChild(xmlAttr);
		xmlAttr = new CXMLElement();
		xmlAttr->Create(_T("presence"), XET_ATTRIBUTE);
		xmlAttr->SetValue(pContact->presence?_T("1"):_T("0"));
		xmlContact->AppendChild(xmlAttr);
		xmlAttr = new CXMLElement();
		xmlAttr->Create(_T("directory"), XET_ATTRIBUTE);
		xmlAttr->SetValue(pContact->directory?_T("1"):_T("0"));
		xmlContact->AppendChild(xmlAttr);
		xmlContacts->AppendChild(xmlContact);
	}
	xmlFile.SetRoot(xmlRoot);
	// Spidy - use the current directory sip resides in
	//CString filename = accountSettings.pathRoaming;
	CString filename = getCurrentWorkingDirectory();
	filename.Append(_T("Contacts.xml"));
	xmlFile.SaveToFile(filename.GetBuffer());
}

void Contacts::ContactsLoad()
{
	CXMLFile xmlFile;
	// Spidy - use the current directory sip resides in
	//CString filename = accountSettings.pathRoaming;
	CString filename = getCurrentWorkingDirectory();
	filename.Append(_T("Contacts.xml"));
	if (xmlFile.LoadFromFile(filename.GetBuffer())) {
		CXMLElement *xmlRoot = xmlFile.GetRoot();
		CXMLElement *xmlContacts = xmlRoot->GetFirstChild();
		while (xmlContacts) {
			if (xmlContacts->GetElementType() == XET_TAG) {
				CXMLElement *xmlContact = xmlContacts->GetFirstChild();
				while (xmlContact) {
					if (xmlContact->GetElementType() == XET_TAG) {
						CXMLElement *xmlAttr = xmlContact->GetFirstChild();
						CString number;
						CString name;
						BOOL presence = FALSE;
						BOOL directory = FALSE;
						CString rab;
						while (xmlAttr) {
							if (xmlAttr->GetElementType() == XET_ATTRIBUTE) {
								CString attrName = xmlAttr->GetElementName();
								if (attrName == _T("number")) {
									number = XMLEntityDecode(Utf8DecodeUni(UnicodeToAnsi(xmlAttr->GetValue())));
								} else if (attrName == _T("name")) {
									name = XMLEntityDecode(Utf8DecodeUni(UnicodeToAnsi(xmlAttr->GetValue())));
								} else if (attrName == _T("presence")) {
									rab = xmlAttr->GetValue();
									presence = rab==_T("1");
								} else if (attrName == _T("directory")) {
									rab = xmlAttr->GetValue();
									directory = rab==_T("1");
								}
							}
							xmlAttr = xmlContact->GetNextChild();
						}
						if (!number.IsEmpty()) {
							ContactAdd(number, name, presence, directory, FALSE);
						}
					}
					xmlContact = xmlContacts->GetNextChild();
				}
			}
			xmlContacts = xmlRoot->GetNextChild();
		}
	} else {
		CString key;
		CString val;
		LPTSTR ptr = val.GetBuffer(255);
		int i=0;
		while (TRUE) {
			key.Format(_T("%d"),i);
			if (GetPrivateProfileString(_T("Contacts"), key, NULL, ptr, 256, accountSettings.iniFile)) {
				CString number;
				CString name;
				BOOL presence;
				BOOL directory;
				ContactDecode(ptr, number, name, presence, directory);
				ContactAdd(number, name, presence, directory, FALSE);
			} else {
				break;
			}
			i++;
		}
		WritePrivateProfileSection(_T("Contacts"),  NULL, accountSettings.iniFile);
		ContactsSave();
	}
}

void Contacts::ContactDecode(CString str, CString &number, CString &name, BOOL &presence, BOOL &fromDirectory)
{
	CString rab;
	int begin;
	int end;
	begin = 0;
	end = str.Find(';', begin);
	if (end != -1)
	{
		number=str.Mid(begin, end-begin);
		begin = end + 1;
		end = str.Find(';', begin);
		if (end != -1)
		{
			name=str.Mid(begin, end-begin);
			begin = end + 1;
			end = str.Find(';', begin);
			if (end != -1)
			{
				rab=str.Mid(begin, end-begin);
				presence = rab == _T("1");
				begin = end + 1;
				end = str.Find(';', begin);
				if (end != -1)
				{
					rab=str.Mid(begin, end-begin);
					fromDirectory = rab == _T("1");
				} else 
				{
					rab=str.Mid(begin);
					fromDirectory = rab == _T("1");
				}
			} else 
			{
				rab=str.Mid(begin);
				presence = rab == _T("1");
				fromDirectory = FALSE;
			}
		} else 
		{
			name = str.Mid(begin);
			presence = FALSE;
			fromDirectory = FALSE;
		}
	} else 
	{
		number=str;
		name = number;
		presence = FALSE;
		fromDirectory = FALSE;
	}
}

CString Contacts::GetNameByNumber(CString number)
{
	CString name;
	CListCtrl *list= (CListCtrl*)GetDlgItem(IDC_CONTACTS);

	CString sipURI = GetSIPURI(number);
	int n = list->GetItemCount();
	for (int i=0; i<n; i++) {
		Contact* pContact = (Contact *) list->GetItemData(i);
		if (GetSIPURI(pContact->number) == sipURI)
		{
			name = pContact->name;
			break;
		}
	}
	return name;
}

void Contacts::PresenceSubscribeOne(Contact *pContact)
{
	if (isSubscribed && pContact->presence)
	{
		pjsua_buddy_id p_buddy_id;
		pjsua_buddy_config buddy_cfg;
		pjsua_buddy_config_default(&buddy_cfg);
		buddy_cfg.subscribe=PJ_TRUE;
		pjsua_acc_id acc_id;
		SelectSIPAccount(pContact->number,account,buddy_cfg.uri);
		buddy_cfg.user_data = (void *)pContact;
		buddy_cfg.uri = StrToPjStr(GetSIPURI(pContact->number, NULL, NULL));
		pjsua_buddy_add(&buddy_cfg, &p_buddy_id);
	}
}

void Contacts::PresenceUpdateOne(Contact *pContact)
{
	if (isSubscribed && pContact->presence)
	{
		pjsua_buddy_id p_buddy_id;
		pjsua_buddy_config buddy_cfg;
		pjsua_buddy_config_default(&buddy_cfg);
		buddy_cfg.subscribe=PJ_TRUE;
		pjsua_acc_id acc_id;
		SelectSIPAccount(pContact->number,acc_id,buddy_cfg.uri);
		buddy_cfg.user_data = (void *)pContact;
		buddy_cfg.uri = StrToPjStr(GetSIPURI(pContact->number, NULL, NULL));
		pjsua_buddy_update_pres(p_buddy_id);
	}
}

void Contacts::PresenceUnsubscribeOne(Contact *pContact)
{
	if (isSubscribed)
	{
		pjsua_buddy_id ids[PJSUA_MAX_BUDDIES];
		unsigned count = PJSUA_MAX_BUDDIES;
		pjsua_enum_buddies(ids,&count);
		for (unsigned i=0;i<count;i++)
		{
			if ((Contact *)pjsua_buddy_get_user_data(ids[i])==pContact)
			{
				pjsua_buddy_del(ids[i]);
				break;
			}
		}
	}
}

void Contacts::PresenceSubscribe()
{
	// Spidy
	if (/*PJAccountAndStatusUpdateRunning || */ presenceOperationIsActive || !pj_ready) {
		return;
	}

	presenceOperationIsActive = true;

	// Spidy - if there are accounts from which we can derive presence
	if (account > PJSUA_INVALID_ID) {
		if (pjsua_acc_is_valid(account)) {
			if (pj_ready) {
				if (accountSettings.account.publicAddr != "") {
					if (!isSubscribed) {
						isSubscribed=TRUE;
						CListCtrl *list= (CListCtrl*)GetDlgItem(IDC_CONTACTS);
						int n = list->GetItemCount();
						for (int i=0; i<n; i++) {
							if (account > PJSUA_INVALID_ID) {
								if (pjsua_acc_is_valid(account)) {
									if (pj_ready) {
										Contact *pContact = (Contact *) list->GetItemData(i);
										PresenceSubscribeOne(pContact);
									}
								} else {
									break;
								}
							} else {
								break;
							}
						}
					}
				}
			}
		}
	}

	presenceOperationIsActive = false;
}

void Contacts::PresenceUpdate()
{
	// Spidy
	if (/*PJAccountAndStatusUpdateRunning || */presenceOperationIsActive || !pj_ready) {
		return;
	}

	presenceOperationIsActive = true;
	if (isSubscribed) {
		// Spidy - if there are accounts from which we can derive presence
		if ((account > PJSUA_INVALID_ID)) {
			if (pjsua_acc_is_valid(account)) {
				if (pj_ready) {
					pjsua_buddy_id ids[PJSUA_MAX_BUDDIES];
					unsigned count = PJSUA_MAX_BUDDIES;
					if (pjsua_enum_buddies(ids,&count) != PJ_SUCCESS) {
						presenceOperationIsActive = false;
						return;
					}
					for (unsigned i=0;i<count;i++)
					{
						if ((account > PJSUA_INVALID_ID)) {
							if (pjsua_acc_is_valid(account)) {
									if (pj_ready) {
										pjsua_buddy_update_pres(ids[i]);
									}
							} else {
								break;
							}
						} else {
							break;
						}
					}
				}
			}
		}
	}
	presenceOperationIsActive = false;
}

void Contacts::PresenceUnsubscribe()
{
	if (/*PJAccountAndStatusUpdateRunning || */presenceOperationIsActive || !pj_ready) {
		return;
	}

	presenceOperationIsActive = true;

	if (account > PJSUA_INVALID_ID) {
		if (pjsua_acc_is_valid(account)) {
			if (pj_ready) {
				pjsua_buddy_id ids[PJSUA_MAX_BUDDIES];
				unsigned count = PJSUA_MAX_BUDDIES;
				pjsua_enum_buddies(ids,&count);
				for (unsigned i=0;i<count;i++)
				{
					if (account > PJSUA_INVALID_ID) {
						if (pjsua_acc_is_valid(account)) {
							pjsua_buddy_del(ids[i]);
						} else {
							break;
						}
					} else {
						break;
					}
				}
				if (::IsWindow(this->m_hWnd)) {
					CListCtrl *list= (CListCtrl *)GetDlgItem(IDC_CONTACTS);
					int n = list->GetItemCount();
					for (int i=0; i<n; i++)
					{
						list->SetItem(i, 0, LVIF_IMAGE, 0, 7, 0, 0, 0);
					}
				}
				isSubscribed=FALSE;
			}
		}
	}

	presenceOperationIsActive = false;
}

void Contacts::SetCanditates()
{
	CListCtrl *list= (CListCtrl*)GetDlgItem(IDC_CONTACTS);
	int count = list->GetItemCount();
	for (int i=0;i<count;i++)
	{
		Contact *pContact = (Contact *) list->GetItemData(i);
		if (pContact->directory) {
			pContact->candidate = TRUE;
		}
	}
}
int Contacts::DeleteCanditates()
{
	CListCtrl *list= (CListCtrl*)GetDlgItem(IDC_CONTACTS);
	int count = list->GetItemCount();
	int deleted = 0;
	for (int i=0;i<count;i++)
	{
		Contact *pContact = (Contact *) list->GetItemData(i);
		if (pContact->candidate) {
			ContactDelete(i, true);
			count--;
			i--;
			deleted++;
		}
	}
	return deleted;
}


#endif