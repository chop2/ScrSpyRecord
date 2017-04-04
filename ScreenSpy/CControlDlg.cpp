// CControlDlg.cpp : ʵ���ļ�
//

#include "stdafx.h"
#include "ScreenSpy.h"
#include "CControlDlg.h"
#include "afxdialogex.h"
#include "CvvImage.h"
#include "ScreenSpyDlg.h"
// CCControlDlg �Ի���

IMPLEMENT_DYNAMIC(CCControlDlg, CDialogEx)

CCControlDlg::CCControlDlg(CWnd* pParent /*=NULL*/)
	: CDialogEx(CCControlDlg::IDD, pParent)
{

}

CCControlDlg::~CCControlDlg()
{
}

void CCControlDlg::DoDataExchange(CDataExchange* pDX)
{
	CDialogEx::DoDataExchange(pDX);
}


BEGIN_MESSAGE_MAP(CCControlDlg, CDialogEx)
	ON_BN_CLICKED(IDOK, &CCControlDlg::OnBtnRecapture)
	ON_BN_CLICKED(IDCANCEL, &CCControlDlg::OnBnClickedCancel)
END_MESSAGE_MAP()


// CCControlDlg ��Ϣ�������
void CCControlDlg::ShowImage(const cv::Mat& src, UINT uID)
{
	//cvZero(theImage);
	Mat dst;
	ResizeImage(src, dst, uID);
#if CV_MAJOR_VERSION < 3
	ShowImage(&dst.operator IplImage(), uID);
#else
	ShowImage(&IplImage(dst),uID);
#endif
}

void CCControlDlg::ShowImage(IplImage* img, UINT ID)
{
	// �����ʾ�ؼ��� DC
	CDC* pDC = GetDlgItem(ID)->GetDC();
	// ��ȡ HDC(�豸���) �����л�ͼ����		
	HDC hDC = pDC->GetSafeHdc();

	CRect rect;
	GetDlgItem(ID)->GetClientRect(&rect);
	// ���ͼƬ�ؼ��Ŀ�͸�
	int rw = rect.right - rect.left;
	int rh = rect.bottom - rect.top;
	// ��ȡͼƬ�Ŀ�͸�
	int iw = img->width;
	int ih = img->height;
	// ʹͼƬ����ʾλ�������ڿؼ�������
	int tx = (int)(rw - iw) / 2;
	int ty = (int)(rh - ih) / 2;
	SetRect(rect, tx, ty, tx + iw, ty + ih);
	// ����ͼƬ
	CvvImage cimg;
	cimg.CopyOf(img);
	// ��ͼƬ���Ƶ���ʾ�ؼ���ָ��������	
	cimg.DrawToHDC(hDC, &rect);

	ReleaseDC(pDC);
}

void CCControlDlg::ResizeImage(const Mat& img_, Mat& dst, UINT uID)
{
	Mat img = img_.clone();
	if (!dst.empty()) dst.release();
	// ��ȡͼƬ�Ŀ�͸�
	int w = img.cols;
	int h = img.rows;

	CRect rect;
	GetDlgItem(uID)->GetClientRect(&rect);
	dst = Mat::zeros(cv::Size(rect.Width(), rect.Height()), img.type());

	// �ҳ���͸��еĽϴ�ֵ��
	int max = (w > h) ? w : h;
	int maxx = (rect.Width() > rect.Height()) ? rect.Width() : rect.Height();
	// ���㽫ͼƬ���ŵ�TheImage��������ı�������
	float scale = (float)((float)max / maxx);

	// ���ź�ͼƬ�Ŀ�͸�
	int nw = (int)(w / scale);
	int nh = (int)(h / scale);

	// Ϊ�˽����ź��ͼƬ���� TheImage �����в�λ�������ͼƬ�� TheImage ���Ͻǵ���������ֵ
	int tlx = (nw > nh) ? 0 : (int)(rect.Width() - nw) / 2;
	int tly = (nw > nh) ? (int)(rect.Height() - nh) / 2 : 0;

	//Mat sub = dst(Rect(tlx, tly, nw, nh));
	//resize(img, sub, sub.size());

	IplImage* dstIpl,*imgIpl;
#if CV_MAJOR_VERSION < 3
	dstIpl = &dst.operator IplImage();
	imgIpl = &img.operator IplImage();
#else
	imgIpl = &IplImage(img);
	dstIpl = &IplImage(dst);
#endif
	// ���� TheImage �� ROI ������������ͼƬ img
	cvSetImageROI(dstIpl/*&dst.operator IplImage()*/, cvRect(tlx, tly, nw, nh));

	// ��ͼƬ img �������ţ������뵽 TheImage ��
	cvResize(imgIpl/*&img.operator IplImage()*/, dstIpl/*&dst.operator IplImage()*/);

	// ���� TheImage �� ROI ׼��������һ��ͼƬ
	cvResetImageROI(dstIpl/*&dst.operator IplImage()*/);

	//dst = img(cv::Rect(tlx, tly, nw, nh));
}

void CCControlDlg::streamCallback(int height, int width, const unsigned char* data,void* param)
{
	CCControlDlg* pDlg = (CCControlDlg*)param;
	if (pDlg == NULL) return;
	cv::Mat src = cv::Mat(height, width, CV_8UC3, (uchar*)data);
	Mat gray;
	cvtColor(src, gray, COLOR_BGR2GRAY);
	int means = (int)mean(gray).val[0];
	threshold(gray, gray, means, 255, THRESH_BINARY);
	//Mat elem = getStructuringElement(MORPH_RECT, cv::Size(4, 4));
	//morphologyEx(gray, gray, MORPH_CLOSE, elem);

	vector<vector<cv::Point> > contours;
	findContours(gray.clone(), contours, RETR_EXTERNAL, CHAIN_APPROX_SIMPLE);
	for (size_t i = 0; i < contours.size(); i++)
	{
		cv::Rect rec = boundingRect(contours[i]);
		int r = std::max(rec.width, rec.height) /
			std::min(rec.width, rec.height);
		if (rec.area() < 40 || r > 5){
			//drawContours(gray, contours, i, Scalar::all(0), -1);
			continue;
		}
		rectangle(src, rec, Scalar(0, 255, 0), 2);
	}

	pDlg->ShowImage(src, IDC_PIC_SHOW);
}

void CCControlDlg::StartCaptureStream(const CRect& roi)
{
	m_spyRecord.RegisterStreamCallback(&streamCallback, this);
	string strFilter = m_spyRecord.MakeCropFilterStr(roi.left,
		roi.top, roi.Width(), roi.Height());
	if (!m_spyRecord.IsBusy())
		m_spyRecord.Run(strFilter);
}

void CCControlDlg::OnBtnRecapture()
{
	// TODO:  �ڴ���ӿؼ�֪ͨ����������
	this->ShowWindow(SW_HIDE);
	m_pScreenSpyDlg->ResetSelectROI();
	m_pScreenSpyDlg->ShowWindow(SW_SHOW);
	m_spyRecord.StopRecord();
	CDialogEx::OnOK();
}


void CCControlDlg::OnBnClickedCancel()
{
	// TODO:  �ڴ���ӿؼ�֪ͨ����������
	m_pScreenSpyDlg->OnCancel();
	CDialogEx::OnCancel();
}
