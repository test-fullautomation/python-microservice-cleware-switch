
// MicroserviceManagerDlg.h : header file
//

#pragma once
#include "ServiceClient.h"


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
	CButton m_radioOut[4];
	CButton m_radioIn[4];
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
	afx_msg HCURSOR OnQueryDragIcon();
	DECLARE_MESSAGE_MAP()
public:
	afx_msg void OnBnClickedButtonInitialize();
	afx_msg void OnBnClickedButtonApply();
	afx_msg void OnRadioButtonClicked(UINT nID);

	void UpdateAllDevicesStateRequest();
	void SetSwitchRequest(string selectedDevice, int switchNum, string state);
};
