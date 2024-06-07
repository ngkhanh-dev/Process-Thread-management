
// MFCApplication2Dlg.h : header file
//

#pragma once


// CMFCApplication2Dlg dialog
class CMFCApplication2Dlg : public CDialog
{
// Construction
public:
	CMFCApplication2Dlg(CWnd* pParent = nullptr);	// standard constructor

// Dialog Data
#ifdef AFX_DESIGN_TIME
	enum { IDD = IDD_MFCAPPLICATION2_DIALOG };
#endif

	protected:
	virtual void DoDataExchange(CDataExchange* pDX);	// DDX/DDV support


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
	afx_msg void OnLvnItemchangedList1(NMHDR* pNMHDR, LRESULT* pResult);
protected:
	
	CButton btn_new;
	CEdit text_edit;
	CButton btn_table;
	CListCtrl table;
	CButton btn_end;
	
public:
	afx_msg void OnBnClickedBtnTable();
	afx_msg void OnBnClickedBtnNew();
	afx_msg void OnBnClickedBtnEnd();

};
