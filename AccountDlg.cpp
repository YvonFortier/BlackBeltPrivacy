#include "StdAfx.h"
#include "AccountDlg.h"
#include "microsipDlg.h"
#include <ws2tcpip.h>
#include "settings.h"
#include "miniupnpc\upnpapi.hpp"

#include "util.hpp"

// Spidy - needed for accountStatus update timer
bool accountDlgIsActive = false;

#ifndef _GLOBAL_NO_ACCOUNT

CmicrosipDlg *microsipDlg;

AccountDlg::AccountDlg(CWnd* pParent /*=NULL*/)
: CDialog(AccountDlg::IDD, pParent)
{
	microsipDlg = (CmicrosipDlg* ) AfxGetMainWnd();	
	Create (IDD, pParent);

	accountId = 0;
}

AccountDlg::~AccountDlg(void)
{
	// Spidy - used by PJAccountAndStatusUpdate
	accountDlgIsActive = false;
}


BOOL AccountDlg::OnInitDialog()
{
	CDialog::OnInitDialog();
	
	TranslateDialog(this->m_hWnd);

	GetDlgItem(IDC_SYSLINK_DELETE)->ShowWindow(SW_HIDE);

	CString str;

	str.Format(_T("<a>%s</a>"),Translate(_T("Remove account")));
	GetDlgItem(IDC_SYSLINK_DELETE)->SetWindowText(str);

#if !defined _GLOBAL_ACCOUNT_PASSWORD && !defined _GLOBAL_NO_DISPLAY_PASSWORD
#ifndef _GLOBAL_ACCOUNT_PIN
	str.Format(_T("<a>%s</a>"),Translate(_T("display password")));
	GetDlgItem(IDC_SYSLINK_DISPLAY_PASSWORD)->SetWindowText(str);
#else
	str.Format(_T("<a>%s</a>"),Translate(_T("display Pin No")));
	GetDlgItem(IDC_SYSLINK_DISPLAY_PASSWORD)->SetWindowText(str);
#endif
#endif

#ifdef _GLOBAL_ACCOUNT_REG
	str.Format(_T("<a>%s</a>"),Translate(_T("Create Account")));
	GetDlgItem(IDC_SYSLINK_REG)->SetWindowText(str);
#endif

#ifndef _GLOBAL_ACCOUNT_MINI

	CEdit* edit;
	CComboBox *combobox;

	combobox= (CComboBox*)GetDlgItem(IDC_SRTP);
	combobox->AddString(Translate(_T("Disabled")));
	combobox->AddString(Translate(_T("Optional")));
	combobox->AddString(Translate(_T("Mandatory")));
	combobox->SetCurSel(0);

	combobox= (CComboBox*)GetDlgItem(IDC_TRANSPORT);
	combobox->AddString(Translate(_T("Auto")));
	combobox->AddString(_T("UDP"));
	combobox->AddString(_T("TCP"));
	combobox->AddString(_T("TLS"));
	combobox->SetCurSel(0);


	combobox= (CComboBox*)GetDlgItem(IDC_PUBLIC_ADDR);
	// Spidy - let account load handle the setting of auto string into this combo box
	//combobox->AddString(Translate(_T("Auto")));
	combobox->AddString(CString(uPnP::instance()->getLanIPAddress()));
	combobox->SetCurSel(0);

	combobox= (CComboBox*)GetDlgItem(IDC_LISTEN_PORT);
	// Spidy - dont set listen port here - let account load set the dialog listen port
	//combobox->AddString(Translate(_T("Auto")));
	//combobox->SetCurSel(0);

#endif

	accountDlgIsActive = true;

	return TRUE;
}

void AccountDlg::PostNcDestroy()
{
	CDialog::PostNcDestroy();
	delete this;
}

BEGIN_MESSAGE_MAP(AccountDlg, CDialog)
	ON_WM_CLOSE()
	ON_BN_CLICKED(IDCANCEL, &AccountDlg::OnBnClickedCancel)
	ON_BN_CLICKED(IDOK, &AccountDlg::OnBnClickedOk)
#ifndef _GLOBAL_CUSTOM
	ON_NOTIFY(NM_CLICK, IDC_SYSLINK_SIP_SERVER, &AccountDlg::OnNMClickSyslinkSipServer)
	ON_NOTIFY(NM_CLICK, IDC_SYSLINK_SIP_PROXY, &AccountDlg::OnNMClickSyslinkSipProxy)
	ON_NOTIFY(NM_CLICK, IDC_SYSLINK_USERNAME, &AccountDlg::OnNMClickSyslinkUsername)
	ON_NOTIFY(NM_CLICK, IDC_SYSLINK_DOMAIN, &AccountDlg::OnNMClickSyslinkDomain)
	ON_NOTIFY(NM_CLICK, IDC_SYSLINK_AUTHID, &AccountDlg::OnNMClickSyslinkAuthID)
	ON_NOTIFY(NM_CLICK, IDC_SYSLINK_PASSWORD, &AccountDlg::OnNMClickSyslinkPassword)
	ON_NOTIFY(NM_CLICK, IDC_SYSLINK_NAME, &AccountDlg::OnNMClickSyslinkName)
	ON_NOTIFY(NM_CLICK, IDC_SYSLINK_ENCRYPTION, &AccountDlg::OnNMClickSyslinkEncryption)
	ON_NOTIFY(NM_CLICK, IDC_SYSLINK_TRANSPORT, &AccountDlg::OnNMClickSyslinkTransport)
	ON_NOTIFY(NM_CLICK, IDC_SYSLINK_PUBLIC_ADDRESS, &AccountDlg::OnNMClickSyslinkPublicAddress)
	ON_NOTIFY(NM_CLICK, IDC_SYSLINK_LOCAL_PORT, &AccountDlg::OnNMClickSyslinkLocalPort)
	ON_NOTIFY(NM_CLICK, IDC_SYSLINK_PUBLISH_PRESENCE, &AccountDlg::OnNMClickSyslinkPublishPresence)
	ON_NOTIFY(NM_CLICK, IDC_SYSLINK_STUN_SERVER, &AccountDlg::OnNMClickSyslinkStunServer)
	ON_NOTIFY(NM_CLICK, IDC_SYSLINK_ICE, &AccountDlg::OnNMClickSyslinkIce)
	ON_NOTIFY(NM_CLICK, IDC_SYSLINK_REWRITE, &AccountDlg::OnNMClickSyslinkRewrite)
#endif
#if !defined _GLOBAL_ACCOUNT_PASSWORD && !defined _GLOBAL_NO_DISPLAY_PASSWORD
	ON_NOTIFY(NM_CLICK, IDC_SYSLINK_DISPLAY_PASSWORD, &AccountDlg::OnNMClickSyslinkDisplayPasswod)
	ON_NOTIFY(NM_RETURN, IDC_SYSLINK_DISPLAY_PASSWORD, &AccountDlg::OnNMClickSyslinkDisplayPasswod)
#endif
	ON_NOTIFY(NM_CLICK, IDC_SYSLINK_DELETE, &AccountDlg::OnNMClickSyslinkDelete)
	ON_NOTIFY(NM_RETURN, IDC_SYSLINK_DELETE, &AccountDlg::OnNMClickSyslinkDelete)
#ifdef _GLOBAL_ACCOUNT_REG
	ON_NOTIFY(NM_CLICK, IDC_SYSLINK_REG, &AccountDlg::OnNMClickSyslinkReg)
	ON_NOTIFY(NM_RETURN, IDC_SYSLINK_REG, &AccountDlg::OnNMClickSyslinkReg)
#endif
	
END_MESSAGE_MAP()


void AccountDlg::OnClose() 
{
	DestroyWindow();
}

void AccountDlg::OnBnClickedCancel()
{
	OnClose();
}

void AccountDlg::Load(int id)
{
	CEdit* edit;
	CComboBox *combobox;

	accountId = id;
	accountSettings.AccountLoad(accountId,account);

#ifndef _GLOBAL_ACCOUNT_SIP_SERVER
	edit = (CEdit*)GetDlgItem(IDC_EDIT_SERVER);
	edit->SetWindowText(account.server);
#endif

#ifndef _GLOBAL_ACCOUNT_SIP_PROXY
	edit = (CEdit*)GetDlgItem(IDC_EDIT_PROXY);
	edit->SetWindowText(account.proxy);
#endif

#ifndef _GLOBAL_ACCOUNT_DOMAIN
	edit = (CEdit*)GetDlgItem(IDC_EDIT_DOMAIN);
	//edit->SetWindowText(account.domain);
	// Spidy - set the domain to the hostname if empty (including when first creating an account)
	if (account.domain.IsEmpty()) {
		hostent * host = gethostbyname(NULL); 
		edit->SetWindowText(CString(host->h_name));
	}
	else {
		edit->SetWindowText(account.domain);
	}
#endif

#ifdef _GLOBAL_PROFILE
	combobox= (CComboBox*)GetDlgItem(IDC_EDIT_DOMAIN);
	combobox->AddString(_T("A"));
	combobox->AddString(_T("B"));
#ifdef _GLOBAL_PROFILE_C
	combobox->AddString(_T("C"));
#endif
	if (account.domain==_T(_GLOBAL_PROFILE_B)) {
		combobox->SetCurSel(1);
#ifdef _GLOBAL_PROFILE_C
	} else if (account.domain==_T(_GLOBAL_PROFILE_C)) {
		combobox->SetCurSel(2);
#endif
	} else {
		combobox->SetCurSel(0);
	}
#endif

#ifndef _GLOBAL_ACCOUNT_LOGIN
	edit = (CEdit*)GetDlgItem(IDC_EDIT_AUTHID);
	edit->SetWindowText(account.authID);
#endif


#ifndef _GLOBAL_ACCOUNT_PIN
	edit = (CEdit*)GetDlgItem(IDC_EDIT_USERNAME);
#ifndef _GLOBAL_API

	//edit->SetWindowText(account.username);
	// Spidy - set the username to the hostname if empty (including when first creating an account)
	if (account.username.IsEmpty()) {
		hostent * host = gethostbyname(NULL); 
		edit->SetWindowText(CString(host->h_name));
	}
	else {
		edit->SetWindowText(account.username);
	}

#else
	edit->SetWindowText(account.apiLogin);
#endif
#endif

#ifndef _GLOBAL_ACCOUNT_PASSWORD
	edit = (CEdit*)GetDlgItem(IDC_EDIT_PASSWORD);
#ifndef _GLOBAL_API
	edit->SetWindowText(account.password);
#else
	edit->SetWindowText(account.apiPassword);
#endif
#endif

#ifdef _GLOBAL_ACCOUNT_REMEMBER_PASSWORD
	((CButton*)GetDlgItem(IDC_REMEMBER_PASSWORD))->SetCheck(account.rememberPassword);
#endif

#ifdef _GLOBAL_API_ID
	edit = (CEdit*)GetDlgItem(IDC_EDIT_API_ID);
	edit->SetWindowText(account.apiId);
#endif

#ifndef _GLOBAL_ACCOUNT_NAME
	edit = (CEdit*)GetDlgItem(IDC_EDIT_DISPLAYNAME);
	//edit->SetWindowText(account.displayName);
	// Spidy - set the display to the hostname if empty (including when first creating an account)
	if (account.displayName.IsEmpty()) {
		hostent * host = gethostbyname(NULL); 
		edit->SetWindowText(CString(host->h_name));
	}
	else {
		edit->SetWindowText(account.displayName);
	}
#endif

#ifndef _GLOBAL_ACCOUNT_MINI
	int i;
	combobox= (CComboBox*)GetDlgItem(IDC_SRTP);
	if (account.srtp==_T("optional")) {
		i=1;
	} else if (account.srtp==_T("mandatory")) {
		i=2;
	} else {
		i=0;
	}
	if (i>0) {
		combobox->SetCurSel(i);
	}

	combobox= (CComboBox*)GetDlgItem(IDC_TRANSPORT);
	if (account.transport==_T("udp")) {
		i=1;
	} else if (account.transport==_T("tcp")) {
		i=2;
	} else if (account.transport==_T("tls")) {
		i=3;
	} else {
		i=0;
	}
	if (i>0) {
		combobox->SetCurSel(i);
	}

	combobox= (CComboBox*)GetDlgItem(IDC_PUBLIC_ADDR);
	//combobox->AddString(Translate(_T("Auto")));
	//if (account.publicAddr.GetLength()) {
	//	combobox->SetWindowText(account.publicAddr);
	//}

	// Spidy - use the external ip if its available as the default ip.
	if (strlen(uPnP::instance()->getExternalIPAddress()) > 0) {
		CString CStringExternalIPAddress("");
		CStringExternalIPAddress = uPnP::instance()->getExternalIPAddress();
		combobox->AddString(CStringExternalIPAddress);
		combobox->SetWindowText(CStringExternalIPAddress);
	} else if (strlen(uPnP::instance()->getLanIPAddress()) > 0) {
			CString CStringLanIPAddress("");
			CStringLanIPAddress = uPnP::instance()->getLanIPAddress();
			combobox->SetWindowText(CStringLanIPAddress);
	}

	combobox= (CComboBox*)GetDlgItem(IDC_LISTEN_PORT);
	combobox->AddString(Translate(_T("Auto")));
	if (account.listenPort.GetLength()) {
		combobox->SetWindowText(account.listenPort);
	} else {
		combobox->SetCurSel(0);
	}

	((CButton*)GetDlgItem(IDC_PUBLISH))->SetCheck(account.publish);

	edit = (CEdit*)GetDlgItem(IDC_STUN);
	edit->SetWindowText(account.stun);

	((CButton*)GetDlgItem(IDC_ICE))->SetCheck(account.ice);
	((CButton*)GetDlgItem(IDC_REWRITE))->SetCheck(account.allowRewrite);
#endif
	if (id>0) {
		GetDlgItem(IDC_SYSLINK_DELETE)->ShowWindow(SW_SHOW);
	}
}

void AccountDlg::OnBnClickedOk()
{
	CEdit* edit;
	CString str;
	CComboBox *combobox;
	int i;

#ifndef _GLOBAL_ACCOUNT_SIP_SERVER
	edit = (CEdit*)GetDlgItem(IDC_EDIT_SERVER);
	edit->GetWindowText(str);
	account.server=str.Trim();
#endif

#ifndef _GLOBAL_ACCOUNT_SIP_PROXY
	edit = (CEdit*)GetDlgItem(IDC_EDIT_PROXY);
	edit->GetWindowText(str);
	account.proxy=str.Trim();
#endif

#ifndef _GLOBAL_ACCOUNT_DOMAIN
	edit = (CEdit*)GetDlgItem(IDC_EDIT_DOMAIN);
	edit->GetWindowText(str);
	account.domain=str.Trim();
#endif

#ifdef _GLOBAL_PROFILE
	combobox= (CComboBox*)GetDlgItem(IDC_EDIT_DOMAIN);
	switch (combobox->GetCurSel()) {
		case 1:
			account.domain=_T(_GLOBAL_PROFILE_B);
			break;
#ifdef _GLOBAL_PROFILE_C
		case 2:
			account.domain=_T(_GLOBAL_PROFILE_C);
			break;
#endif
		default:
			account.domain=_T(_GLOBAL_PROFILE_A);
	}
	account.server=account.domain;
	account.proxy=account.domain;
#endif

#ifndef _GLOBAL_ACCOUNT_LOGIN
	edit = (CEdit*)GetDlgItem(IDC_EDIT_AUTHID);
	edit->GetWindowText(str);
	account.authID=str.Trim();
#endif

#ifndef _GLOBAL_ACCOUNT_PIN
	edit = (CEdit*)GetDlgItem(IDC_EDIT_USERNAME);
#else
	edit = (CEdit*)GetDlgItem(IDC_EDIT_PASSWORD);
#endif
	edit->GetWindowText(str);
#ifndef _GLOBAL_API
	account.username=str.Trim();
#else
	account.apiLogin=str.Trim();
#endif

#ifndef _GLOBAL_ACCOUNT_PASSWORD
	edit = (CEdit*)GetDlgItem(IDC_EDIT_PASSWORD);
	edit->GetWindowText(str);
#ifndef _GLOBAL_API
	account.password=str.Trim();
#else
	account.apiPassword=str.Trim();
#endif
#endif

#ifdef _GLOBAL_API_ID
	edit = (CEdit*)GetDlgItem(IDC_EDIT_API_ID);
	edit->GetWindowText(str);
	account.apiId=str.Trim();
#endif

#ifndef _GLOBAL_ACCOUNT_NAME
	edit = (CEdit*)GetDlgItem(IDC_EDIT_DISPLAYNAME);
	edit->GetWindowText(str);
	account.displayName=str.Trim();
#endif

#ifdef _GLOBAL_ACCOUNT_REMEMBER_PASSWORD
	account.rememberPassword = ((CButton*)GetDlgItem(IDC_REMEMBER_PASSWORD))->GetCheck();
#else
	account.rememberPassword = 1;
#endif

#ifndef _GLOBAL_ACCOUNT_MINI
	combobox= (CComboBox*)GetDlgItem(IDC_SRTP);
	i = combobox->GetCurSel();
	switch (i) {
		case 1:
			account.srtp=_T("optional");
			break;
		case 2:
			account.srtp=_T("mandatory");
			break;
		default:
			account.srtp=_T("");
	}

	combobox= (CComboBox*)GetDlgItem(IDC_TRANSPORT);
	i = combobox->GetCurSel();
	switch (i) {
		case 1:
			account.transport=_T("udp");
			break;
		case 2:
			account.transport=_T("tcp");
			break;
		case 3:
			account.transport=_T("tls");
			break;
		default:
			account.transport=_T("");
	}

	account.publish = ((CButton*)GetDlgItem(IDC_PUBLISH))->GetCheck();

	edit = (CEdit*)GetDlgItem(IDC_STUN);
	edit->GetWindowText(str);
	account.stun=str.Trim();

	account.ice = ((CButton*)GetDlgItem(IDC_ICE))->GetCheck();

	account.allowRewrite = ((CButton*)GetDlgItem(IDC_REWRITE))->GetCheck();

	combobox= (CComboBox*)GetDlgItem(IDC_PUBLIC_ADDR);
	i = combobox->GetCurSel();
	combobox->GetWindowText(account.publicAddr);
	if (account.publicAddr==Translate(_T("Auto")))
	{
		account.publicAddr = _T("");
	}

	// Spidy - capture port differences for upnp update
	OldNewUpnpSpec OldNewUpnpSpec;
	OldNewUpnpSpec.oldPort = getRealPort(account);
	combobox= (CComboBox*)GetDlgItem(IDC_LISTEN_PORT);
	i = combobox->GetCurSel();
	combobox->GetWindowText(account.listenPort);
	int port = atoi(CStringA(account.listenPort));
	if (port<=0 || port>65535 || account.listenPort==Translate(_T("Auto")))
	{
		account.listenPort = _T("");
	}
#endif

	if (account.domain.IsEmpty() || account.username.IsEmpty()) {
		// Spidy - allow users to specify empty details as some just wanna call - dont wanna register an account
		//return;
	}

	if (account.domain.IsEmpty()) {
		// Spidy - allow users to specify empty details as some just wanna call - dont wanna register an account
		//return;
		hostent * host = gethostbyname(NULL); 
		account.domain.SetString(CString(host->h_name));
	}

	this->ShowWindow(SW_HIDE);

	if (!accountId) {
		Account account;
		int i = 1;
		while (true) {
			if (!accountSettings.AccountLoad(i,account)) {
				break;
			}
			i++;
		}
		accountId = i;
	}

	accountSettings.AccountSave(accountId,&account);
	
	if (accountSettings.accountId) {
		microsipDlg->PJAccountDelete();
	}
	accountSettings.accountId = accountId;
	accountSettings.account = account;
	accountSettings.SettingsSave();
	microsipDlg->PJAccountAdd();

	// Spidy - grab listen port for update.
	//OldNewUpnpSpec.newPort = atoi((CStringA)(accountSettings.account.listenPort)) == 0 ? 5060: atoi((CStringA)(accountSettings.account.listenPort));
	OldNewUpnpSpec.newPort = getRealPort(accountSettings.account);

	uPnP::instance()->updateAllPortsMultiThreaded(OldNewUpnpSpec.oldPort, OldNewUpnpSpec.newPort, 6);

	OnClose();
}

#ifndef _GLOBAL_CUSTOM

void AccountDlg::OnNMClickSyslinkSipServer(NMHDR *pNMHDR, LRESULT *pResult)
{
	OpenHelp(_T("sipServer"));
	*pResult = 0;
}

void AccountDlg::OnNMClickSyslinkSipProxy(NMHDR *pNMHDR, LRESULT *pResult)
{
	OpenHelp(_T("sipProxy"));
	*pResult = 0;
}

void AccountDlg::OnNMClickSyslinkUsername(NMHDR *pNMHDR, LRESULT *pResult)
{
	OpenHelp(_T("username"));
	*pResult = 0;
}

void AccountDlg::OnNMClickSyslinkDomain(NMHDR *pNMHDR, LRESULT *pResult)
{
	OpenHelp(_T("domain"));
	*pResult = 0;
}

void AccountDlg::OnNMClickSyslinkAuthID(NMHDR *pNMHDR, LRESULT *pResult)
{
	OpenHelp(_T("login"));
	*pResult = 0;
}

void AccountDlg::OnNMClickSyslinkPassword(NMHDR *pNMHDR, LRESULT *pResult)
{
	OpenHelp(_T("password"));
	*pResult = 0;
}

void AccountDlg::OnNMClickSyslinkName(NMHDR *pNMHDR, LRESULT *pResult)
{
	OpenHelp(_T("name"));
	*pResult = 0;
}

void AccountDlg::OnNMClickSyslinkEncryption(NMHDR *pNMHDR, LRESULT *pResult)
{
	OpenHelp(_T("encryption"));
	*pResult = 0;
}

void AccountDlg::OnNMClickSyslinkTransport(NMHDR *pNMHDR, LRESULT *pResult)
{
	OpenHelp(_T("transport"));
	*pResult = 0;
}

void AccountDlg::OnNMClickSyslinkPublicAddress(NMHDR *pNMHDR, LRESULT *pResult)
{
	OpenHelp(_T("publicAddress"));
	*pResult = 0;
}

void AccountDlg::OnNMClickSyslinkLocalPort(NMHDR *pNMHDR, LRESULT *pResult)
{
	OpenHelp(_T("localPort"));
	*pResult = 0;
}

void AccountDlg::OnNMClickSyslinkPublishPresence(NMHDR *pNMHDR, LRESULT *pResult)
{
	OpenHelp(_T("publishPresence"));
	*pResult = 0;
}

void AccountDlg::OnNMClickSyslinkStunServer(NMHDR *pNMHDR, LRESULT *pResult)
{
	OpenHelp(_T("stunServer"));
	*pResult = 0;
}

void AccountDlg::OnNMClickSyslinkIce(NMHDR *pNMHDR, LRESULT *pResult)
{
	OpenHelp(_T("ice"));
	*pResult = 0;
}

void AccountDlg::OnNMClickSyslinkRewrite(NMHDR *pNMHDR, LRESULT *pResult)
{
	OpenHelp(_T("allowRewrite"));
	*pResult = 0;
}

#endif

#if !defined _GLOBAL_ACCOUNT_PASSWORD && !defined _GLOBAL_NO_DISPLAY_PASSWORD
void AccountDlg::OnNMClickSyslinkDisplayPasswod(NMHDR *pNMHDR, LRESULT *pResult)
{
	GetDlgItem(IDC_SYSLINK_DISPLAY_PASSWORD)->ShowWindow(SW_HIDE);
	CEdit* edit = (CEdit*)GetDlgItem(IDC_EDIT_PASSWORD);
	edit->SetPasswordChar(0);
	edit->Invalidate();
	edit->SetFocus();
	int nLength = edit->GetWindowTextLength();
	edit->SetSel(nLength,nLength);
	*pResult = 0;
}
#endif

void AccountDlg::OnNMClickSyslinkDelete(NMHDR *pNMHDR, LRESULT *pResult)
{
	if (accountId>0 && AfxMessageBox(Translate(_T("Are you sure you want to remove?")), MB_YESNO)==IDYES) {
		this->ShowWindow(SW_HIDE);

		Account account;
		int i = accountId;
		while (true) {
			if (!accountSettings.AccountLoad(i+1,account)) {
				break;
			}
			accountSettings.AccountSave(i,&account);
			if (accountSettings.accountId == i+1) {
				accountSettings.accountId = i;
				accountSettings.SettingsSave();
				accountId = 0;
			}
			i++;
		}

		// Spidy - grab listen port for update.
		OldNewUpnpSpec oldNewUpnpSpec;
		oldNewUpnpSpec.oldPort = atoi((CStringA)(accountSettings.account.listenPort));
		oldNewUpnpSpec.oldPort = oldNewUpnpSpec.oldPort == 0 ? 5060: oldNewUpnpSpec.oldPort;

		accountSettings.AccountDelete(i);
		if (accountId && accountId == accountSettings.accountId) {
			microsipDlg->PJAccountDelete();
			if (i>1) {
				accountSettings.accountId = 1;
				accountSettings.AccountLoad(accountSettings.accountId,accountSettings.account);
				microsipDlg->PJAccountAdd();
			} else {
				accountSettings.accountId = 0;
			}
			accountSettings.SettingsSave();
		}
		// Spidy - grab listen port for update.
		oldNewUpnpSpec.newPort = atoi((CStringA)(accountSettings.account.listenPort));
		oldNewUpnpSpec.newPort = oldNewUpnpSpec.newPort == 0 ? 5060: oldNewUpnpSpec.newPort;

		uPnP::instance()->updateAllPortsMultiThreaded(oldNewUpnpSpec.oldPort, oldNewUpnpSpec.newPort, 6);

		OnClose();
	}
	*pResult = 0;
}

#ifdef _GLOBAL_ACCOUNT_REG
void AccountDlg::OnNMClickSyslinkReg(NMHDR *pNMHDR, LRESULT *pResult)
{
		if (!microsipDlg->reg1Dlg)
		{
			microsipDlg->reg1Dlg = new Reg1(microsipDlg);
		} else {
			microsipDlg->reg1Dlg->SetForegroundWindow();
		}
	OnClose();
	*pResult = 0;
}
#endif

#endif