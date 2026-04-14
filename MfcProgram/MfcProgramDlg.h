#pragma once

#include "ImageRepository.h"
#include "ImageApiService.h"
#include "DbConfig.h"

// CMfcProgramDlg 대화 상자
class CMfcProgramDlg : public CDialogEx
{
public:
	CMfcProgramDlg(CWnd* pParent = nullptr);

#ifdef AFX_DESIGN_TIME
	enum { IDD = IDD_MFCPROGRAM_DIALOG };
#endif

protected:
	virtual void DoDataExchange(CDataExchange* pDX);

protected:
	HICON _hIcon;

	virtual BOOL OnInitDialog();
	afx_msg void OnPaint();
	afx_msg HCURSOR OnQueryDragIcon();

	afx_msg void OnBnClickedButtonProcess();
	afx_msg void OnBnClickedButtonSave();
	afx_msg void OnBnClickedButtonDelete();
	afx_msg void OnBnClickedButtonRefresh();
	afx_msg void OnLvnItemchangedListResults(NMHDR* pNMHDR, LRESULT* pResult);

	DECLARE_MESSAGE_MAP()

private:
	// UI 컨트롤
	CListCtrl _listResults;

	CEdit _editTitle;
	CEdit _editImageUrl;
	CEdit _editBlurKsize;
	CEdit _editWidth;
	CEdit _editHeight;

	CStatic _staticStatus;
	CStatic _staticResultImage;

	// Picture Control 비트맵 핸들
	HBITMAP _hResultBitmap = nullptr;

	// DB / SDK 서비스
	ImageRepository _repository;
	ImageApiService _apiService;
	DbConfig _dbConfig;

	// 최근 처리 결과 임시 보관
	std::vector<unsigned char> _pendingImageBytes;
	CString _pendingOperationSummary;
	CString _pendingOutputFormat;
	CString _pendingTitle;
	CString _pendingImageUrl;
	bool _hasPendingResult = false;

	// API 서버 주소
	CString _apiBaseUrl = L"http://3.106.215.74:8080";

private:
	void InitializeListControl();
	void InitializeDefaultValues();
	void SetStatusText(const CString& text);
	CString BuildOperationSummaryFromUi() const;
	CString GetEditText(const CEdit& edit) const;
	bool IsChecked(int controlId) const;

	void ReloadProcessedImageList();
	int GetSelectedRecordId() const;

	void ShowImageInControl(const CString& imagePath, CStatic& targetControl, HBITMAP& bitmapHandle);
	void ClearResultImage();
};