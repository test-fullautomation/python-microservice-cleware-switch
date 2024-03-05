
// MicroserviceManagerDlg.cpp : implementation file
//

#include "pch.h"
#include "framework.h"
#include "MicroserviceManager.h"
#include "MicroserviceManagerDlg.h"
#include "afxdialogex.h"
#include <json/json.h>

#ifdef _DEBUG
#define new DEBUG_NEW
#endif


// CAboutDlg dialog used for App About

class CAboutDlg : public CDialogEx
{
public:
	CAboutDlg();

// Dialog Data
#ifdef AFX_DESIGN_TIME
	enum { IDD = IDD_ABOUTBOX };
#endif

	protected:
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV support

// Implementation
protected:
	DECLARE_MESSAGE_MAP()
};

CAboutDlg::CAboutDlg() : CDialogEx(IDD_ABOUTBOX)
{
}

void CAboutDlg::DoDataExchange(CDataExchange* pDX)
{
	CDialogEx::DoDataExchange(pDX);
}

BEGIN_MESSAGE_MAP(CAboutDlg, CDialogEx)
END_MESSAGE_MAP()


// CMicroserviceManagerDlg dialog



CMicroserviceManagerDlg::CMicroserviceManagerDlg(CWnd* pParent /*=nullptr*/)
	: CDialogEx(IDD_MICROSERVICEMANAGER_DIALOG, pParent)
{
	m_hIcon = AfxGetApp()->LoadIcon(IDR_MAINFRAME);
}

void CMicroserviceManagerDlg::DoDataExchange(CDataExchange* pDX)
{
	CDialogEx::DoDataExchange(pDX);
	DDX_Control(pDX, IDC_COMBO_DEVICE_NUM, m_comboDeviceNumber);
	DDX_Control(pDX, IDC_COMBO_DEVICE_TYPE, m_comboDeviceType);
	DDX_Control(pDX, IDC_BUTTON_INITIALIZE, m_btInitialize);
	DDX_Control(pDX, IDC_BUTTON_APPLY, m_btApply);
	for (int i = IDC_OUT1; i <= IDC_OUT4; i++)
	{
		CButton* btn = new CButton();
		DDX_Control(pDX, i, *btn);
		m_vRadioMplxOut.push_back(btn);
	}

	for (int i = IDC_IN1; i <= IDC_IN2; i++)
	{
		CButton* btn = new CButton();
		DDX_Control(pDX, i, *btn);
		m_vRadioMplxIn.push_back(btn);
	}

	m_vMultiplexer.insert(m_vMultiplexer.end(), m_vRadioMplxIn.begin(), m_vRadioMplxIn.end());
	m_vMultiplexer.insert(m_vMultiplexer.end(), m_vRadioMplxOut.begin(), m_vRadioMplxOut.end());
	m_vMultiplexer.push_back((CButton*)GetDlgItem(IDC_IN_GROUP));
	m_vMultiplexer.push_back((CButton*)GetDlgItem(IDC_OUT_GROUP));

	for (int i = IDC_SW1; i <= IDC_SW8; i++)
	{
		CButton* btn = new CButton();
		DDX_Control(pDX, i, *btn);
		m_vRadioSwbox.push_back(btn);
	}

	m_vSwitchBox.insert(m_vSwitchBox.end(), m_vRadioSwbox.begin(), m_vRadioSwbox.end());
	m_vSwitchBox.push_back((CButton*)GetDlgItem(IDC_SWBOX_GROUP));
}

BEGIN_MESSAGE_MAP(CMicroserviceManagerDlg, CDialogEx)
	ON_WM_SYSCOMMAND()
	ON_WM_PAINT()
	ON_WM_QUERYDRAGICON()
	ON_BN_CLICKED(IDC_BUTTON_INITIALIZE, &CMicroserviceManagerDlg::OnBnClickedButtonInitialize)
	ON_BN_CLICKED(IDC_BUTTON_APPLY, &CMicroserviceManagerDlg::OnBnClickedButtonApply)
	ON_CONTROL_RANGE(BN_CLICKED, IDC_OUT1, IDC_IN2, &CMicroserviceManagerDlg::OnMultiplexerSwitchChanged)
	ON_CONTROL_RANGE(BN_CLICKED, IDC_SW1, IDC_SW8, &CMicroserviceManagerDlg::OnSwitchboxSwitchChanged)
	ON_CBN_SELCHANGE(IDC_COMBO_DEVICE_TYPE, &CMicroserviceManagerDlg::OnCbnSelchangeComboDeviceType)
END_MESSAGE_MAP()


// CMicroserviceManagerDlg message handlers

BOOL CMicroserviceManagerDlg::OnInitDialog()
{
	CDialogEx::OnInitDialog();

	// Add "About..." menu item to system menu.

	// IDM_ABOUTBOX must be in the system command range.
	ASSERT((IDM_ABOUTBOX & 0xFFF0) == IDM_ABOUTBOX);
	ASSERT(IDM_ABOUTBOX < 0xF000);

	CMenu* pSysMenu = GetSystemMenu(FALSE);
	if (pSysMenu != nullptr)
	{
		BOOL bNameValid;
		CString strAboutMenu;
		bNameValid = strAboutMenu.LoadString(IDS_ABOUTBOX);
		ASSERT(bNameValid);
		if (!strAboutMenu.IsEmpty())
		{
			pSysMenu->AppendMenu(MF_SEPARATOR);
			pSysMenu->AppendMenu(MF_STRING, IDM_ABOUTBOX, strAboutMenu);
		}
	}

	// Set the icon for this dialog.  The framework does this automatically
	//  when the application's main window is not a dialog
	SetIcon(m_hIcon, TRUE);			// Set big icon
	SetIcon(m_hIcon, FALSE);		// Set small icon

	// TODO: Add extra initialization here
	m_comboDeviceType.AddString(_T("Multiplexer"));
	m_comboDeviceType.AddString(_T("Switchbox"));
	m_comboDeviceType.SetCurSel(0);
	m_comboDeviceType.EnableWindow(FALSE);
	m_comboDeviceNumber.EnableWindow(FALSE);
	m_btApply.EnableWindow(FALSE);

	mClewareService = new ServiceClient("ServiceCleware", "127.0.0.1", 5672, "ServiceClewareKey");

	m_bIsInitialized = false;
	m_bIsApplied = false;


	return TRUE;  // return TRUE  unless you set the focus to a control
}

void CMicroserviceManagerDlg::OnSysCommand(UINT nID, LPARAM lParam)
{
	if ((nID & 0xFFF0) == IDM_ABOUTBOX)
	{
		CAboutDlg dlgAbout;
		dlgAbout.DoModal();
	}
	else
	{
		CDialogEx::OnSysCommand(nID, lParam);
	}
}

// If you add a minimize button to your dialog, you will need the code below
//  to draw the icon.  For MFC applications using the document/view model,
//  this is automatically done for you by the framework.

void CMicroserviceManagerDlg::OnPaint()
{
	if (IsIconic())
	{
		CPaintDC dc(this); // device context for painting

		SendMessage(WM_ICONERASEBKGND, reinterpret_cast<WPARAM>(dc.GetSafeHdc()), 0);

		// Center icon in client rectangle
		int cxIcon = GetSystemMetrics(SM_CXICON);
		int cyIcon = GetSystemMetrics(SM_CYICON);
		CRect rect;
		GetClientRect(&rect);
		int x = (rect.Width() - cxIcon + 1) / 2;
		int y = (rect.Height() - cyIcon + 1) / 2;

		// Draw the icon
		dc.DrawIcon(x, y, m_hIcon);
	}
	else
	{
		CDialogEx::OnPaint();
	}
}

void CMicroserviceManagerDlg::OnDestroy()
{
	for (auto control : m_vRadioMplxIn)
	{
		delete control;
	}
	m_vRadioMplxIn.clear();

	for (auto control : m_vRadioMplxOut)
	{
		delete control;
	}
	m_vRadioMplxOut.clear();

	for (auto control : m_vRadioSwbox)
	{
		delete control;
	}
	m_vRadioSwbox.clear();

	mClewareService->disconnect();
	delete mClewareService;

	CDialogEx::OnDestroy();
}

// The system calls this function to obtain the cursor to display while the user drags
//  the minimized window.
HCURSOR CMicroserviceManagerDlg::OnQueryDragIcon()
{
	return static_cast<HCURSOR>(m_hIcon);
}


Json::Value jsonDeviceState;

std::map<std::string, Json::Value> jsonToMap(const Json::Value& jsonValue) {
	std::map<std::string, Json::Value> resultMap;

	// Iterate through the JSON object
	for (const auto& key : jsonValue.getMemberNames()) {
		resultMap[key] = jsonValue[key];
	}

	return resultMap;
}

void CMicroserviceManagerDlg::SetSwitch(string selectedDevice, int switchNum, SetSwitchRequest::SWITCH_STATE state)
{
	ServiceRequest* req = new SetSwitchRequest(selectedDevice, switchNum, state);
	mClewareService->sendRequest(req, [this](const std::string& result) {
			// Handle the result
			std::cout << "Result callback: " << result << std::endl;
	});

	delete req;
}

void CMicroserviceManagerDlg::UpdateAllDevicesState()
{
	ServiceRequest* req = new GetAllDeviceStateRequest();
	mClewareService->sendRequest(req, [this](const std::string& result) {
		// Handle the result
		std::cout << "Result callback: " << result << std::endl;
		Json::CharReaderBuilder builder;
		Json::Value jsonRoot;
		JSONCPP_STRING err;
		std::istringstream iss(result);
		if (!Json::parseFromStream(builder, iss, &jsonRoot, &err)) {
			std::cerr << "Error parsing JSON: " << err << std::endl;
		}
		else if (!jsonRoot.isObject()) {
			std::cerr << "Invalid JSON format: Root should be an object." << std::endl;
		}
		else
		{
			Json::Value jsonDevicesInfor = jsonRoot["result_data"];
			jsonDeviceState = jsonDevicesInfor;
			std::map<std::string, Json::Value> resultMap = jsonToMap(jsonDevicesInfor);
			Json::StreamWriterBuilder writerBuilder;
			std::string jsonString = Json::writeString(writerBuilder, jsonDevicesInfor);
			for (Json::Value::const_iterator it = jsonDevicesInfor.begin(); it != jsonDevicesInfor.end(); ++it) {
				std::string key = it.key().asString();
				m_comboDeviceNumber.AddString(CString(key.c_str()));
			}

		}

	});

	delete req;
}

void CMicroserviceManagerDlg::OnBnClickedButtonInitialize()
{
	// TODO: Add your control notification handler code here
	if (!m_bIsInitialized)
	{
		int res = mClewareService->connect();
		if (res != 1)
		{
			CString message;
			message.Format(_T("Unable to connect Cleware service. Error code: %d"), res);
			AfxMessageBox(message);
		}
		else
		{
			m_comboDeviceNumber.ResetContent();
			UpdateAllDevicesState();			
			m_btInitialize.SetWindowTextW(_T("Uninitialize"));
			m_btApply.EnableWindow(TRUE);
			m_comboDeviceType.EnableWindow(TRUE);
			m_comboDeviceNumber.EnableWindow(TRUE);			
			m_bIsInitialized = true;
		}
	}
	else
	{
		mClewareService->disconnect();
		m_btInitialize.SetWindowTextW(_T("Initialize"));
		m_btApply.EnableWindow(FALSE);
		m_comboDeviceType.EnableWindow(FALSE);		
		m_comboDeviceNumber.EnableWindow(FALSE);
		int selectedTypeIdx = m_comboDeviceType.GetCurSel();
		if (selectedTypeIdx != CB_ERR) 
		{
			if (selectedTypeIdx == 0)
			{
				std::for_each(m_vMultiplexer.begin(), m_vMultiplexer.end(), CMicroserviceManagerDlg::DisableElement);

			}
			else
			{
				std::for_each(m_vSwitchBox.begin(), m_vSwitchBox.end(), CMicroserviceManagerDlg::DisableElement);
			}
		}
		m_comboDeviceType.SetCurSel(0);
		m_bIsInitialized = false;
	}	
}

int findOnIndex(const std::vector<int>& vec) {
	for (size_t i = 0; i < vec.size(); ++i) {
		if (vec[i] == 1) {
			return static_cast<int>(i);  
		}
	}
	return -1;
}


void CMicroserviceManagerDlg::OnBnClickedButtonApply()
{
	int selectedNumIdx = m_comboDeviceNumber.GetCurSel();

	if (selectedNumIdx != CB_ERR) {
		CString selectedNumText;
		m_comboDeviceNumber.GetLBText(selectedNumIdx, selectedNumText);
		std::string selectedDeviceNum = CT2A(selectedNumText);
		int selectedTypeIdx = m_comboDeviceType.GetCurSel();
		UpdateAllDevicesState();
		if (jsonDeviceState.isMember(selectedDeviceNum)) {
			Json::Value switchState = jsonDeviceState[selectedDeviceNum];
			Json::Value::Members keys = switchState.getMemberNames();
			for (unsigned int i = 0; i < keys.size(); ++i) {
				const Json::Value& value = switchState[keys[i]];
				if (value.asInt() == 1)
				{
					if (selectedTypeIdx == 0)
					{
						i < 4 ? m_vRadioMplxIn[0]->SetCheck(BST_CHECKED) : m_vRadioMplxIn[1]->SetCheck(BST_CHECKED);
						m_vRadioMplxOut[i % 4]->SetCheck(BST_CHECKED);
						break;
					}
					else
					{
						m_vSwitchBox[i]->SetCheck(BST_CHECKED);
					}
				}
			}
			/*std::vector<int> valuesArray;
			for (const auto& value : switchState) {
				valuesArray.push_back(value.asInt());
			}

			int onIndex = findOnIndex(valuesArray);
			if (onIndex != -1)
			{
				onIndex < 4 ? m_vRadioMplxIn[0]->SetCheck(BST_CHECKED) : m_vRadioMplxIn[1]->SetCheck(BST_CHECKED);
				m_vRadioMplxOut[onIndex % 4]->SetCheck(BST_CHECKED);
			}			*/
		}
		else 
		{
			AfxMessageBox(selectedNumText + _T(" is not a valid device number."));
		}

		
		if (selectedTypeIdx != CB_ERR) 
		{
			if (selectedTypeIdx == 0)
			{
				std::for_each(m_vMultiplexer.begin(), m_vMultiplexer.end(), CMicroserviceManagerDlg::EnableElement);
				
			}
			else
			{
				std::for_each(m_vSwitchBox.begin(), m_vSwitchBox.end(), CMicroserviceManagerDlg::EnableElement);
			}
		}
		else 
		{
			CString selectedTypeText;
			m_comboDeviceNumber.GetLBText(selectedTypeIdx, selectedTypeText);
			AfxMessageBox(selectedTypeText + _T(" is not a valid device type."));
		}
	}
	else 
	{
		// No item selected or an error occurred
		AfxMessageBox(_T("No device selected."));
	}
}



void CMicroserviceManagerDlg::OnMultiplexerSwitchChanged(UINT nID) {
	// Determine which radio button was clicked
	int clickedButtonIndex = nID - IDC_OUT1;
	int outSwitchCheckIdx = -1;
	int inSwitchCheckIdx = -1;
	int activeSwitchIdx = -1;
		
	auto btnOutChecked = std::find_if(m_vRadioMplxOut.begin(), m_vRadioMplxOut.end(), CMicroserviceManagerDlg::IsChecked);
	auto btnInChecked = std::find_if(m_vRadioMplxIn.begin(), m_vRadioMplxIn.end(), CMicroserviceManagerDlg::IsChecked);

	if (btnOutChecked != m_vRadioMplxOut.end()) {
		int idx = ((CButton*)*btnOutChecked)->GetDlgCtrlID();
		outSwitchCheckIdx = idx - IDC_OUT1;
	}

	if (btnInChecked != m_vRadioMplxIn.end()) {
		inSwitchCheckIdx = ((CButton*)*btnInChecked)->GetDlgCtrlID() - IDC_IN1;
	}

	if (outSwitchCheckIdx != -1 && inSwitchCheckIdx != -1)
	{
		activeSwitchIdx = 16 + inSwitchCheckIdx * 4 + outSwitchCheckIdx;
	}

	int selectedIndex = m_comboDeviceNumber.GetCurSel();
	CString selectedItemText;
	m_comboDeviceNumber.GetLBText(selectedIndex, selectedItemText);
	std::string selectedKey = CT2A(selectedItemText);

	SetSwitch(selectedKey, activeSwitchIdx, SetSwitchRequest::SWITCH_STATE::ON);
	
	TRACE(_T("Radio button %d clicked!\n"), clickedButtonIndex);
}

void CMicroserviceManagerDlg::OnSwitchboxSwitchChanged(UINT nID) {
	// Determine which radio button was clicked
	int clickedButtonIndex = nID - IDC_SW1;
	int currentState = m_vRadioSwbox[clickedButtonIndex]->GetCheck();
	SetSwitchRequest::SWITCH_STATE state = currentState ? SetSwitchRequest::SWITCH_STATE::ON : SetSwitchRequest::SWITCH_STATE::OFF;
	int switchCheckIdx = clickedButtonIndex + 16;
	int selectedIndex = m_comboDeviceNumber.GetCurSel();
	if (selectedIndex != -1)
	{
		CString selectedItemText;
		m_comboDeviceNumber.GetLBText(selectedIndex, selectedItemText);
		std::string selectedKey = CT2A(selectedItemText);
		SetSwitch(selectedKey, switchCheckIdx, state);
	}
	TRACE(_T("Radio button %d clicked!\n"), clickedButtonIndex);
}

void CMicroserviceManagerDlg::ShowElement(CButton* &element)
{
	element->ShowWindow(SW_SHOWNORMAL);
}

void CMicroserviceManagerDlg::HideElement(CButton* &element)
{
	element->ShowWindow(SW_HIDE);
}

void CMicroserviceManagerDlg::EnableElement(CButton*& element)
{
	element->EnableWindow(TRUE);
}

void CMicroserviceManagerDlg::DisableElement(CButton*& element)
{
	element->EnableWindow(FALSE);
}

bool CMicroserviceManagerDlg::IsChecked(CButton*& element)
{
	return element->GetCheck() == BST_CHECKED;
}


void CMicroserviceManagerDlg::OnCbnSelchangeComboDeviceType()
{
	// TODO: Add your control notification handler code here
	int selectedIndex = m_comboDeviceType.GetCurSel();

	if (selectedIndex != CB_ERR) {
		CString selectedItemText;
		m_comboDeviceType.GetLBText(selectedIndex, selectedItemText);
		std::string selectedKey = CT2A(selectedItemText);
		if (selectedKey == "Multiplexer")
		{
			std::for_each(m_vMultiplexer.begin(), m_vMultiplexer.end(), CMicroserviceManagerDlg::ShowElement);
			std::for_each(m_vMultiplexer.begin(), m_vMultiplexer.end(), CMicroserviceManagerDlg::DisableElement);
			std::for_each(m_vSwitchBox.begin(), m_vSwitchBox.end(), CMicroserviceManagerDlg::HideElement);

		}
		else if (selectedKey == "Switchbox")
		{
			std::for_each(m_vMultiplexer.begin(), m_vMultiplexer.end(), CMicroserviceManagerDlg::HideElement);
			std::for_each(m_vSwitchBox.begin(), m_vSwitchBox.end(), CMicroserviceManagerDlg::ShowElement);
			std::for_each(m_vSwitchBox.begin(), m_vSwitchBox.end(), CMicroserviceManagerDlg::DisableElement);
		}

	}
}
