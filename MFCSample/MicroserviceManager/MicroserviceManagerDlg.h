
// MicroserviceManagerDlg.h : header file
//

#pragma once
#include "ServiceClient.h"
#include "ServiceRequest.h"


// CMicroserviceManagerDlg dialog
class CMicroserviceManagerDlg : public CDialogEx
{
// Construction
public:
	CMicroserviceManagerDlg(CWnd* pParent = nullptr);	// standard constructor

// Dialog Data
#ifdef AFX_DESIGN_TIME
	enum { IDD = IDD_MICROSERVICEMANAGER_DIALOG };	

#endif

	protected:
	virtual void DoDataExchange(CDataExchange* pDX);	// DDX/DDV support

	CComboBox m_comboDeviceNumber;
	CComboBox m_comboDeviceType;

	std::vector<CButton*> m_vMultiplexer;
	std::vector<CButton*> m_vSwitchBox;

	std::vector<CButton*> m_vRadioMplxIn;
	std::vector<CButton*> m_vRadioMplxOut;
	std::vector<CButton*> m_vRadioSwbox;

	ServiceClient*	mClewareService;

	const string METHOD = "method";
	const string ARGS = "args";

	const string REQ_GET_ALL_DEVICES_STATE = "svc_api_get_all_devices_state";
	const string REQ_SET_SWITCH = "svc_api_set_switch";


// Implementation
protected:
	HICON m_hIcon;


	// Generated message map functions
	virtual BOOL OnInitDialog();
	afx_msg void OnSysCommand(UINT nID, LPARAM lParam);
	afx_msg void OnPaint();
	afx_msg void OnDestroy();
	afx_msg HCURSOR OnQueryDragIcon();
	DECLARE_MESSAGE_MAP()
public:
	afx_msg void OnBnClickedButtonInitialize();
	afx_msg void OnBnClickedButtonApply();
	afx_msg void OnMultiplexerSwitchChanged(UINT nID);
	afx_msg void OnSwitchboxSwitchChanged(UINT nID);

	void UpdateAllDevicesState();
	void SetSwitch(string selectedDevice, int switchNum, SetSwitchRequest::SWITCH_STATE state);
	afx_msg void OnCbnSelchangeComboDeviceType();

	static void ShowElement(CButton*& element);
	static void HideElement(CButton*& element);
	static void EnableElement(CButton*& element);
	static void DisableElement(CButton*& element);
	static bool IsChecked(CButton*& element);
};
