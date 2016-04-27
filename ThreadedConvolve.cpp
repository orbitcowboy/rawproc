
#include "ThreadedConvolve.h"
#include "FreeImage16.h"
#include "util.h"


ThreadedConvolve::ThreadedConvolve(FIBITMAP *psrc, FIBITMAP *pdst, unsigned pstartrow, unsigned pincrement, double pkernel[3][3])
: wxThread(wxTHREAD_JOINABLE)
{
	src = psrc;
	dst = pdst;			//calling routine needs to create this with a FreeImage_Clone(psrc)
	startrow = pstartrow;
	increment = pincrement;
	for (int x=0; x<3; x++)
		for (int y=0; y<3; y++)
			kernel[x][y] = pkernel[x][y];
}

ThreadedConvolve::~ThreadedConvolve() 
{ 

}

wxThread::ExitCode ThreadedConvolve::Entry()
{
	unsigned x, y;
	BYTE * bsrcpix, * bdstpix;
	FIRGB16 * wsrcpix, * wdstpix;
	BYTE *bits = NULL;
	
	int bpp = FreeImage_GetBPP(src);
	int bytespp = bpp/8;

	unsigned pitch = FreeImage_GetPitch(src);
	BYTE *dibbits = (BYTE*)FreeImage_GetBits(src);
	//bits = dibbits +(pitch*(y))+(x*(bytespp));

	unsigned spitch = FreeImage_GetPitch(src);
	unsigned dpitch = FreeImage_GetPitch(dst);
	void * srcbits = FreeImage_GetBits(src);
	void * dstbits = FreeImage_GetBits(dst);

	double R, G, B;
	FIRGB16 value;

	switch(bpp) {
		case 48:
			for(y = startrow+1; y < FreeImage_GetHeight(src)-1; y+=increment) {
				for(x = 1; x < FreeImage_GetWidth(src)-1; x++) {
					wdstpix = (FIRGB16 *) (dstbits + dpitch*y + 6*x);
					R=0.0; G=0.0; B=0.0;
					for (int kx=0; kx<3; kx++) {
						int ix=kx*3;
						for (int ky=0; ky<3; ky++) {
							int i = ix+ky;
							FIRGB16 *pixel = (FIRGB16 *) (srcbits + spitch*(y-1+ky) + 6*(x-1+kx));
							R += pixel->red   * kernel[kx][ky];
							G += pixel->green * kernel[kx][ky];
							B += pixel->blue  * kernel[kx][ky];
						}
						wdstpix->red   = MIN(MAX(int(R), 0), 65535);
						wdstpix->green = MIN(MAX(int(G), 0), 65535);
						wdstpix->blue  = MIN(MAX(int(B), 0), 65535);
					}

				}
			}
			break;	            
		case 24 :
			for(y = startrow+1; y < FreeImage_GetHeight(src)-1; y+=increment) {
				for(x = 1; x < FreeImage_GetWidth(src)-1; x++) {
					bdstpix = (BYTE *) dstbits + dpitch*y + 3*x;
					R=0.0; G=0.0; B=0.0;
					for (int kx=0; kx<3; kx++) {
						int ix=kx*3;
						for (int ky=0; ky<3; ky++) { 
							int i = ix+ky;
							BYTE *pixel = (BYTE *) srcbits + spitch*(y-1+ky) + 3*(x-1+kx);
							R += pixel[FI_RGBA_RED]   * kernel[kx][ky];
							G += pixel[FI_RGBA_GREEN] * kernel[kx][ky];
							B += pixel[FI_RGBA_BLUE]  * kernel[kx][ky];
						}
						bdstpix[FI_RGBA_RED]   = MIN(MAX(int(R), 0), 255);
						bdstpix[FI_RGBA_GREEN] = MIN(MAX(int(G), 0), 255);
						bdstpix[FI_RGBA_BLUE]  = MIN(MAX(int(B), 0), 255);
					}
				}
			}
			break;
	}

	return (wxThread::ExitCode)0;
}