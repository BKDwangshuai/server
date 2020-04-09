
vector<BYTE> intMesDis;//主界面显示时表达已打印的信息（字节流，根据Pixel的值每列1-4个字节，排列顺序时先每列从下往上，再从左向右


void DrawAllDynamic(CDC* pDC);
void OnPaint();
bool MesDisIsB(int MesDis, int isNumber)

void OnPaint()///主绘制函数
{
	CWnd::Default();   //让控件画它自己的东西
	if (m_pDC == NULL)
	{
		m_pDC = this->GetDC();
		GetClientRect(&m_rectClient);

		m_bitmapTemp.CreateCompatibleBitmap(m_pDC, 640, 129);//创建内存位图
		m_dcMem.CreateCompatibleDC(m_pDC); //依附窗口DC创建兼容的DC
		m_dcMem.SelectObject(&m_bitmapTemp);
		m_cbrush.CreateSolidBrush(RGB(0, 0, 0));
		m_pBrush = m_dcMem.SelectObject(&m_cbrush);
	}

	//填充颜色		
	CRect bkRect = m_rectClient;
	m_drawRect = m_rectClient;
	m_drawRect.top = m_rectClient.top + (32 - theApp.mainPicPixel)*m_pixSize;
	bkRect.bottom = (32 - theApp.mainPicPixel)*m_pixSize;
	m_dcMem.FillSolidRect(bkRect, theApp.m_BKcolor);
	m_dcMem.FillSolidRect(m_drawRect, RGB(255, 255, 255));   //填充颜色

	//画网格
	CPen cPen;
	cPen.CreatePen(PS_SOLID, 1, RGB(220, 220, 220));
	CPen* pOldPen;
	pOldPen = m_dcMem.SelectObject(&cPen); //载入笔刷
	for (int i = 0; i <= m_rectClient.Width(); i += m_pixSize)//竖
	{
		m_dcMem.MoveTo(i, m_rectClient.Height() - m_pixSize * theApp.mainPicPixel - 1);
		m_dcMem.LineTo(i, m_rectClient.Height());
	}
	for (int j = m_rectClient.Height() - m_pixSize * theApp.mainPicPixel - 1; j <= m_rectClient.Height(); j += m_pixSize)//横
	{
		m_dcMem.MoveTo(0, j);
		m_dcMem.LineTo(m_rectClient.Width(), j);
	}
	m_dcMem.SelectObject(pOldPen);
	cPen.DeleteObject();
	if (theApp.mainPicMatrx == 14)
	{
		CRect bkRect = m_rectClient;
		bkRect.bottom = (32 - 7)*m_pixSize;
		bkRect.top = (32 - 8)*m_pixSize;
		m_dcMem.FillSolidRect(bkRect, theApp.m_BKcolor);
	}
	theApp.m_MessagePrint.DrawAllDynamic(&m_dcMem);
	m_pDC->BitBlt(0, 0, m_rectClient.Width(), m_rectClient.Height(), &m_dcMem, 0, 0, SRCCOPY);//绘制图片到主dc

	theApp.boDrawMainPic = true;
}

void DrawAllDynamic(CDC* pDC)//画字体的点
{
	if (intMesDis.size() == 0) return;
	//vector<BYTE> intMesDis1 = intMesDis;//intMesDis他为实时更改的，直接用它会造成混乱吧

	int pixSize = 4;
	ModuleMain myModuleMain;
	pDC->SelectStockObject(NULL_PEN);
	if (Matrix != 14)
	{
		for (int k = 0; k < intRowMax; k++)
			for (int i = 0; i <= Pixel; i++)
			{
				bool bDraw = false;
				if (Pixel < 8)
					bDraw = myModuleMain.MesDisIsB(intMesDis[11 + k], i);
				else if (Pixel > 7 && Pixel < 16)
					bDraw = myModuleMain.MesDisIsB(intMesDis[11 + 2 * k] + (intMesDis[11 + 2 * k + 1] * pow(2, 8)), i);
				else if (Pixel > 15 && Pixel < 24)
					bDraw = myModuleMain.MesDisIsB(intMesDis[11 + 3 * k] + (intMesDis[11 + 3 * k + 1] * pow(2, 8)) + (intMesDis[11 + 3 * k + 2] * pow(2, 16)), i);
				else if (Pixel > 23)
					bDraw = myModuleMain.MesDisIsB(intMesDis[11 + 4 * k] + (intMesDis[11 + 4 * k + 1] * pow(2, 8)) + (intMesDis[11 + 4 * k + 2] * pow(2, 16)) + (intMesDis[11 + 4 * k + 3] * pow(2, 24)), i);

				if (bDraw)
				{
					pDC->SelectStockObject(BLACK_BRUSH);
					pDC->Rectangle(k*pixSize + 1, (31 - i)*pixSize + 1, (k + 1)*pixSize, (32 - i)*pixSize);
				}
				else {
					pDC->SelectStockObject(WHITE_BRUSH);
					pDC->Rectangle(k*pixSize + 1, (31 - i)*pixSize + 1, (k + 1)*pixSize, (32 - i)*pixSize);
				}
				//Sleep(1);
			}
	}
	else
	{
		for (int k = 0; k < intRowMax; k++)
		{
			BYTE tempByte1 = intMesDis[k * 2 + 11];
			BYTE tempByte2 = intMesDis[k * 2 + 11 + 1];

			for (int i = 0; i < 7; i = i + 2)
			{
				bool bDraw = false;
				if (myModuleMain.MesDisIsB(tempByte1, i))
					pDC->Rectangle(k*pixSize + 1, (31 - i / 2)*pixSize + 1, (k + 1)*pixSize, (32 - i / 2)*pixSize);
				//Sleep(1);
				if (i < 6)
				{
					if (myModuleMain.MesDisIsB(tempByte2, i))
						pDC->Rectangle(k*pixSize + 1, (31 - 4 - i / 2)*pixSize + 1, (k + 1)*pixSize, (32 - 4 - i / 2)*pixSize);
					//Sleep(1);
				}
			}
		}
	}
}

bool MesDisIsB(int MesDis, int isNumber)//判断某一位是否为1
{
	return MesDis & (1 << isNumber);
}