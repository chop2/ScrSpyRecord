// CControlDlg.cpp : 实现文件
//

#include "stdafx.h"
#include "ScreenSpy.h"
#include "CControlDlg.h"
#include "afxdialogex.h"
#include "CvvImage.h"
#include "ScreenSpyDlg.h"
// CCControlDlg 对话框

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


// CCControlDlg 消息处理程序
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
	// 获得显示控件的 DC
	CDC* pDC = GetDlgItem(ID)->GetDC();
	// 获取 HDC(设备句柄) 来进行绘图操作		
	HDC hDC = pDC->GetSafeHdc();

	CRect rect;
	GetDlgItem(ID)->GetClientRect(&rect);
	// 求出图片控件的宽和高
	int rw = rect.right - rect.left;
	int rh = rect.bottom - rect.top;
	// 读取图片的宽和高
	int iw = img->width;
	int ih = img->height;
	// 使图片的显示位置正好在控件的正中
	int tx = (int)(rw - iw) / 2;
	int ty = (int)(rh - ih) / 2;
	SetRect(rect, tx, ty, tx + iw, ty + ih);
	// 复制图片
	CvvImage cimg;
	cimg.CopyOf(img);
	// 将图片绘制到显示控件的指定区域内	
	cimg.DrawToHDC(hDC, &rect);

	ReleaseDC(pDC);
}

void CCControlDlg::ResizeImage(const Mat& img_, Mat& dst, UINT uID)
{
	Mat img = img_.clone();
	if (!dst.empty()) dst.release();
	// 读取图片的宽和高
	int w = img.cols;
	int h = img.rows;

	CRect rect;
	GetDlgItem(uID)->GetClientRect(&rect);
	dst = Mat::zeros(cv::Size(rect.Width(), rect.Height()), img.type());

	// 找出宽和高中的较大值者
	int max = (w > h) ? w : h;
	int maxx = (rect.Width() > rect.Height()) ? rect.Width() : rect.Height();
	// 计算将图片缩放到TheImage区域所需的比例因子
	float scale = (float)((float)max / maxx);

	// 缩放后图片的宽和高
	int nw = (int)(w / scale);
	int nh = (int)(h / scale);

	// 为了将缩放后的图片存入 TheImage 的正中部位，需计算图片在 TheImage 左上角的期望坐标值
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
	// 设置 TheImage 的 ROI 区域，用来存入图片 img
	cvSetImageROI(dstIpl/*&dst.operator IplImage()*/, cvRect(tlx, tly, nw, nh));

	// 对图片 img 进行缩放，并存入到 TheImage 中
	cvResize(imgIpl/*&img.operator IplImage()*/, dstIpl/*&dst.operator IplImage()*/);

	// 重置 TheImage 的 ROI 准备读入下一幅图片
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
	// TODO:  在此添加控件通知处理程序代码
	this->ShowWindow(SW_HIDE);
	m_pScreenSpyDlg->ResetSelectROI();
	m_pScreenSpyDlg->ShowWindow(SW_SHOW);
	m_spyRecord.StopRecord();
	CDialogEx::OnOK();
}


void CCControlDlg::OnBnClickedCancel()
{
	// TODO:  在此添加控件通知处理程序代码
	m_pScreenSpyDlg->OnCancel();
	CDialogEx::OnCancel();
}
