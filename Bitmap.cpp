#include"Bitmap.h"
#include<fstream>
#include<assert.h>
using namespace std;

Bitmap::Bitmap() : 
	mCreated(false), mData(0)
{
}
Bitmap::Bitmap(int width, int height) : 
	mCreated(false), mData(0)
{
	Create(width, height);
}
Bitmap::Bitmap(const char* filename) : 
	mCreated(false), mData(0)
{
	Create(filename);
}
Bitmap::Bitmap(const Bitmap& obj) : 
	mCreated(false), mData(0)
{
	obj.copy(this);
}

Bitmap::~Bitmap()
{
	if(mData) delete [] mData;
}

void Bitmap::WriteToBitmapFile(const char* filename)
{
	if(!mCreated) assert(!"not created!");
	
	ofstream ofs(filename, ios::out | ios::binary | ios::trunc);
	
	BITMAPFILEHEADER bmpFileHeader = {0};
	BITMAPINFOHEADER bmpInfoHeader = {0};
	
	//パティング
	int len;
	if((width() * 3) % 4 == 0)
	{
		len = width() * 3;
	}
	else
	{
		len = width() * 3 + (4 - (width() * 3) % 4);
	}
	
	bmpFileHeader.bfType = 'M' << 8 | 'B';
	bmpFileHeader.bfSize = sizeof(BITMAPFILEHEADER) + sizeof(BITMAPINFOHEADER) + len * height();
	bmpFileHeader.bfOffBits = sizeof(BITMAPFILEHEADER) + sizeof(BITMAPINFOHEADER);
	ofs.write((char*)&bmpFileHeader, sizeof(bmpFileHeader));
	
	bmpInfoHeader.biSize = sizeof(BITMAPINFOHEADER);
	bmpInfoHeader.biWidth = width();
	bmpInfoHeader.biHeight = height();
	bmpInfoHeader.biPlanes = 1;
	bmpInfoHeader.biBitCount = 24;
	bmpInfoHeader.biCompression = BI_RGB;
	ofs.write((char*)&bmpInfoHeader, sizeof(bmpInfoHeader));
	
	for(int y = 0;y < height();y++)
	{
		for(int x = 0;x < width();x++)
		{
			ofs.write((char*)&(*this)(x, height() - y - 1), sizeof(char) * 3);
		}
		//パティング
		for(int i = 0;i < len - width() * 3;i++)
		{
			char p = 0;
			ofs.write(&p, sizeof(char));
		}
	}
}

void Bitmap::copy(Bitmap* obj) const
{
	obj->mCreated = false;
	obj->Create(width(), height());
	for(int i = 0;i < width() * height();i++)
	{
		obj->mData[i] = mData[i];
	}
}

Bitmap& Bitmap::operator=(const Bitmap& obj)
{
	obj.copy(this);
	
	return *this;
}

void Bitmap::Create(int width, int height)
{
	if(mCreated) return;
	
	if(mData) delete [] mData;
	mData = new unsigned long[width * height];
	for(int i = 0;i < width * height;i++)
	{
		mData[i] = RGB(255, 255, 255);
	}
	
	ZeroMemory(&mInfo, sizeof(mInfo));
	mInfo.bmiHeader.biSize = sizeof(BITMAPINFOHEADER);
	mInfo.bmiHeader.biWidth = width;
	mInfo.bmiHeader.biHeight = height;
	mInfo.bmiHeader.biPlanes = 1;
	mInfo.bmiHeader.biBitCount = 32;
	mInfo.bmiHeader.biCompression = BI_RGB;
	
	mCreated = true;
}

void Bitmap::Create(const char* filename)
{
	ifstream ifs(filename, ios::in | ios::binary);
	if(!ifs) return;
	
	BITMAPFILEHEADER bmpFileHeader;
	BITMAPINFOHEADER bmpInfoHeader;
	
	ifs.read((char*)&bmpFileHeader, sizeof(bmpFileHeader));
	if(bmpFileHeader.bfType != 0x4d42) return;
	
	ifs.read((char*)&bmpInfoHeader, sizeof(bmpInfoHeader));
	Create(bmpInfoHeader.biWidth, bmpInfoHeader.biHeight);
	
	for(int y = 0;y < height();y++)
	{
		for(int x = 0;x < width();x++)
		{
			unsigned char r, g, b;
			ifs.read((char*)&r, sizeof(unsigned char));
			ifs.read((char*)&g, sizeof(unsigned char));
			ifs.read((char*)&b, sizeof(unsigned char));
			(*this)(x, height() - y - 1) = RGB(r, g, b);
		}
		//パティング
		if((width() * 3) % 4 != 0)
		{
			ifs.seekg(sizeof(unsigned char) * (4 - (width() * 3) % 4), ios_base::cur);
		}
	}
}

void Bitmap::Composition(Bitmap &src, int ax, int ay)
{
	for(int cy = 0;cy < src.height();cy++)
	{
		int y = ay + cy;
		if(y < 0 || height() <= y) continue;
		for(int cx = 0;cx < src.width();cx++)
		{
			int x = ax + cx;
			if(x < 0 || width() <= x) continue;
			(*this)(x, y) = src(cx, cy);
		}
	}
}

void Bitmap::Composition(Bitmap &src, int ax, int ay, unsigned long transColor)
{
	for(int cy = 0;cy < src.height();cy++)
	{
		int y = ay + cy;
		if(y < 0 || height() <= y) continue;
		for(int cx = 0;cx < src.width();cx++)
		{
			int x = ax + cx;
			if(x < 0 || width() <= x) continue;
			if(src(cx, cy) != transColor) (*this)(x, y) = src(cx, cy);
		}
	}
}

unsigned long& Bitmap::operator()(int x, int y)
{
	if(x < 0 || width() <= x || y < 0 || height() <= y) assert(!"out of index!");
	if(!mCreated) assert(!"not created!");
	return mData[(height() - y - 1) * width() + x];
}

void Bitmap::Draw(HDC hdc, int x, int y, int w, int h)
{
	if(!mCreated) return;
	StretchDIBits(hdc, x, y, w, h, 0, 0, width(), height(), mData, &mInfo, DIB_PAL_COLORS, SRCCOPY);
}
