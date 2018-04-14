// oleddisp.h

#ifndef SRC_OLEDDISP_H_
#define SRC_OLEDDISP_H_

#include "platform.h"

typedef enum
{
	OLED_CTRL_UNKNOWN = 0,
	OLED_CTRL_SSD1306,
//
} TOledCtrlType;

class TOledDisp
{
public:
	bool            initialized = false;

	TOledCtrlType   ctrltype = OLED_CTRL_UNKNOWN;
	bool            mirrorx = false;

	uint32_t        updatecnt = 0; // for screen update control
	uint32_t        lastupdate = 0x12345678;

	int             updatestate = 0;

	uint8_t    			rotation = 0;
	uint8_t         contrast = 0x8F;
	bool            externalvcc = false;

	uint16_t			 	hwwidth =  0;
	uint16_t				hwheight = 0;

	uint16_t				width =  0;
	uint16_t				height = 0;

	uint8_t *       pdispbuf;

	virtual ~TOledDisp() {} // to avoid warning

public:
	// interface dependent

	virtual bool InitInterface();

	//virtual void ResetPanel();

	virtual void WriteCmd(uint8_t adata);

	//virtual void SetAddrWindow(uint16_t x0, uint16_t y0, uint16_t w,  uint16_t h);
	//virtual void FillColor(uint16_t acolor, unsigned acount);

public:
	// interface independent functions

	bool Init(TOledCtrlType atype, uint16_t awidth, uint16_t aheight, uint8_t * adispbuf);

	void InitPanel();

	void SetRotation(uint8_t m);

	void FillRect(int16_t x, int16_t y, int16_t w, int16_t h, uint16_t color);
	void FillScreen(uint16_t color);
	void DrawPixel(int16_t x, int16_t y, uint16_t color);

	void DrawChar(int16_t x, int16_t y, char ch);

	bool UpdateFinished();
	virtual void Run();
};

#endif /* SRC_OLEDDISP_H_ */
