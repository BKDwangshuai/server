
vector<BYTE> intMesDis;//��������ʾʱ����Ѵ�ӡ����Ϣ���ֽ���������Pixel��ֵÿ��1-4���ֽڣ�����˳��ʱ��ÿ�д������ϣ��ٴ�������


void DrawAllDynamic(CDC* pDC);
void OnPaint();
bool MesDisIsB(int MesDis, int isNumber)

void OnPaint()///�����ƺ���
{
	CWnd::Default();   //�ÿؼ������Լ��Ķ���
	if (m_pDC == NULL)
	{
		m_pDC = this->GetDC();
		GetClientRect(&m_rectClient);

		m_bitmapTemp.CreateCompatibleBitmap(m_pDC, 640, 129);//�����ڴ�λͼ
		m_dcMem.CreateCompatibleDC(m_pDC); //��������DC�������ݵ�DC
		m_dcMem.SelectObject(&m_bitmapTemp);
		m_cbrush.CreateSolidBrush(RGB(0, 0, 0));
		m_pBrush = m_dcMem.SelectObject(&m_cbrush);
	}

	//�����ɫ		
	CRect bkRect = m_rectClient;
	m_drawRect = m_rectClient;
	m_drawRect.top = m_rectClient.top + (32 - theApp.mainPicPixel)*m_pixSize;
	bkRect.bottom = (32 - theApp.mainPicPixel)*m_pixSize;
	m_dcMem.FillSolidRect(bkRect, theApp.m_BKcolor);
	m_dcMem.FillSolidRect(m_drawRect, RGB(255, 255, 255));   //�����ɫ

	//������
	CPen cPen;
	cPen.CreatePen(PS_SOLID, 1, RGB(220, 220, 220));
	CPen* pOldPen;
	pOldPen = m_dcMem.SelectObject(&cPen); //�����ˢ
	for (int i = 0; i <= m_rectClient.Width(); i += m_pixSize)//��
	{
		m_dcMem.MoveTo(i, m_rectClient.Height() - m_pixSize * theApp.mainPicPixel - 1);
		m_dcMem.LineTo(i, m_rectClient.Height());
	}
	for (int j = m_rectClient.Height() - m_pixSize * theApp.mainPicPixel - 1; j <= m_rectClient.Height(); j += m_pixSize)//��
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
	m_pDC->BitBlt(0, 0, m_rectClient.Width(), m_rectClient.Height(), &m_dcMem, 0, 0, SRCCOPY);//����ͼƬ����dc

	theApp.boDrawMainPic = true;
}

void DrawAllDynamic(CDC* pDC)//������ĵ�
{
	if (intMesDis.size() == 0) return;
	//vector<BYTE> intMesDis1 = intMesDis;//intMesDis��Ϊʵʱ���ĵģ�ֱ����������ɻ��Ұ�

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

bool MesDisIsB(int MesDis, int isNumber)//�ж�ĳһλ�Ƿ�Ϊ1
{
	return MesDis & (1 << isNumber);
}