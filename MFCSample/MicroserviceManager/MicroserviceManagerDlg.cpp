
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
	DDX_Control(pDX, IDC_OUT1, m_radioOut[0]);
	DDX_Control(pDX, IDC_OUT2, m_radioOut[1]);
	DDX_Control(pDX, IDC_OUT3, m_radioOut[2]);
	DDX_Control(pDX, IDC_OUT4, m_radioOut[3]);
	DDX_Control(pDX, IDC_IN1, m_radioIn[0]);
	DDX_Control(pDX, IDC_IN2, m_radioIn[1]);

}

BEGIN_MESSAGE_MAP(CMicroserviceManagerDlg, CDialogEx)
	ON_WM_SYSCOMMAND()
	ON_WM_PAINT()
	ON_WM_QUERYDRAGICON()
	ON_BN_CLICKED(IDC_BUTTON_INITIALIZE, &CMicroserviceManagerDlg::OnBnClickedButtonInitialize)
	ON_BN_CLICKED(IDC_BUTTON_APPLY, &CMicroserviceManagerDlg::OnBnClickedButtonApply)
	ON_CONTROL_RANGE(BN_CLICKED, IDC_OUT1, IDC_IN2, &CMicroserviceManagerDlg::OnRadioButtonClicked)
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

void CMicroserviceManagerDlg::SetSwitchRequest(string selectedDevice, int switchNum, string state)
{
	ServiceClient* srv = new ServiceClient("ServiceCleware", "127.0.0.1", 5672, "ServiceClewareKey");
	Json::Value jsonObject;
	jsonObject[METHOD] = REQ_SET_SWITCH;

	Json::Value listArray;
	listArray.append(selectedDevice.c_str());
	listArray.append(switchNum);
	listArray.append(state.c_str());
	jsonObject[ARGS] = listArray;

	Json::StreamWriterBuilder writer;
	std::string jsonString = Json::writeString(writer, jsonObject);
	srv->RequestService(jsonString, [this](const std::string& result) {
		// Handle the result
		std::cout << "Result callback: " << result << std::endl;
		});
	delete srv;
}

void CMicroserviceManagerDlg::UpdateAllDevicesStateRequest()
{
	ServiceClient* srv = new ServiceClient("ServiceCleware", "127.0.0.1", 5672, "ServiceClewareKey");
	Json::Value jsonObject;
	jsonObject[METHOD] = REQ_GET_ALL_DEVICES_STATE;
	jsonObject[ARGS] = Json::nullValue;
	Json::StreamWriterBuilder writer;
	std::string jsonString = Json::writeString(writer, jsonObject);
	srv->RequestService(jsonString, [this](const std::string& result) {
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

	delete srv;
}

void CMicroserviceManagerDlg::OnBnClickedButtonInitialize()
{
	// TODO: Add your control notification handler code here
	UpdateAllDevicesStateRequest();
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
	// Handle a button click, for example, retrieve the selected item text
	int selectedIndex = m_comboDeviceNumber.GetCurSel();

	if (selectedIndex != CB_ERR) {
		CString selectedItemText;
		m_comboDeviceNumber.GetLBText(selectedIndex, selectedItemText);

		// Now, 'selectedItemText' contains the text of the selected item
		std::string selectedKey = CT2A(selectedItemText);
		if (jsonDeviceState.isMember(selectedKey)) {
			// Key exists, handle accordingly
			Json::Value switchState = jsonDeviceState[selectedKey];

			// Create a std::vector to store the values
			std::vector<int> valuesArray;

			// Iterate through the values in the JSON object and add them to the vector
			for (const auto& value : switchState) {
				valuesArray.push_back(value.asInt());
			}

			int onIndex = findOnIndex(valuesArray);
			if (onIndex != -1)
			{
				onIndex < 4 ? m_radioIn[0].SetCheck(BST_CHECKED) : m_radioIn[1].SetCheck(BST_CHECKED);
				m_radioOut[onIndex % 4].SetCheck(BST_CHECKED);
			}
			
		}
		else {
			// Key doesn't exist
			AfxMessageBox(selectedItemText + _T(" is not a valid device."));
		}

	}
	else {
		// No item selected or an error occurred
		AfxMessageBox(_T("No device selected."));
	}
}


void CMicroserviceManagerDlg::OnRadioButtonClicked(UINT nID) {
	// Determine which radio button was clicked
	int clickedButtonIndex = nID - IDC_OUT1;

	int outSwitchCheckIdx = -1;
	for (int i = 0; i < 4; ++i) {
		if (m_radioOut[i].GetCheck() == BST_CHECKED) {
			outSwitchCheckIdx = i;
			break;
		}
	}

	int inSwitchCheckIdx = -1;
	for (int i = 0; i < 2; ++i) {
		if (m_radioIn[i].GetCheck() == BST_CHECKED) {
			inSwitchCheckIdx = i;
			break;
		}
	}

	int setSwitchIdx = -1;
	if (outSwitchCheckIdx != -1 && inSwitchCheckIdx != -1)
	{
		setSwitchIdx = 16 + inSwitchCheckIdx * 4 + outSwitchCheckIdx;
	}

	int selectedIndex = m_comboDeviceNumber.GetCurSel();
	CString selectedItemText;
	m_comboDeviceNumber.GetLBText(selectedIndex, selectedItemText);
	std::string selectedKey = CT2A(selectedItemText);

	SetSwitchRequest(selectedKey, setSwitchIdx, "on");
	
	TRACE(_T("Radio button %d clicked!\n"), clickedButtonIndex);
}