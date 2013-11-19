#pragma once

#include<windows.h>

class Bitmap
{
public:
	Bitmap();
	Bitmap(int width, int height);
	Bitmap(const char* filename);
	Bitmap(const Bitmap& obj);
	~Bitmap();
	
	Bitmap& operator=(const Bitmap& obj);
	
	void Create(int width, int height);
	void Create(const char* filename);
	
	void WriteToBitmapFile(const char* filename);
	
	void copy(Bitmap* obj) const;
	
	int width() const { return mInfo.bmiHeader.biWidth; }
	int height() const { return mInfo.bmiHeader.biHeight; }
	
	void Composition(Bitmap &src, int x, int y);
	void Composition(Bitmap &src, int x, int y, unsigned long transColor);
	
	unsigned long& operator()(int x, int y);
	
	void Draw(HDC hdc, int x, int y, int w, int h);
private:
	bool mCreated;
	
	BITMAPINFO mInfo;
	unsigned long *mData;
};
