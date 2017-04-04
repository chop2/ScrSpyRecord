#pragma once
#include "ffmpeg_screen_record.h"
// CCControlDlg �Ի���

class CCControlDlg : public CDialogEx
{
	DECLARE_DYNAMIC(CCControlDlg)

public:
	CCControlDlg(CWnd* pParent = NULL);   // ��׼���캯��
	virtual ~CCControlDlg();

// �Ի�������
	enum { IDD = IDD_CONTROL_DIALOG };

	static void streamCallback(int height, int width, 
		const unsigned char* data, void* param);

	void ShowImage(const cv::Mat& src, UINT uID);
	void ResizeImage(const Mat& img_, Mat& dst, UINT uID);
	void ShowImage(IplImage* img, UINT ID);
	
	void StartCaptureStream(const CRect& roi);

	class CScreenSpyDlg* m_pScreenSpyDlg;
protected:
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV ֧��

protected:
	FFmpeg_screen_record m_spyRecord;
	
	DECLARE_MESSAGE_MAP()
public:
	afx_msg void OnBtnRecapture();
	afx_msg void OnBnClickedCancel();
};
