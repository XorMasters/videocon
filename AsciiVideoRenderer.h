#ifndef __ASCII_VIDEO_RENDERER_H__
#define __ASCII_VIDEO_RENDERER_H__

#include <iostream>
#include "talk/app/webrtc/mediastreaminterface.h"

class AsciiVideoRenderer : public webrtc::VideoRendererInterface {
public:
  virtual void SetSize(int width, int height);
  virtual void RenderFrame(const cricket::VideoFrame* frame);

public:
	AsciiVideoRenderer(std::ostream& out, uint8_t factor = 0, double contrast = 1.0) : 
		_factor(factor), _contrast(contrast), _out(out) {}
	virtual ~AsciiVideoRenderer() { if(_scaleBuffer) delete[] _scaleBuffer; }

private:
	const uint8_t* _scaleFrame(const uint8_t* frame);
	void _renderBuffer(const uint8_t* buffer, size_t width, size_t height);

private:
	int _width;
	int _height;
	uint8_t _factor;
	double _contrast;
	std::ostream& _out;
	uint8_t* _scaleBuffer = NULL;
	static const uint8_t _characters[];
};

#endif // __ASCII_VIDEO_RENDERER_H__
