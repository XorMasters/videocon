#define LOGGING 1

#include <cmath>
#include <iostream>

#include "talk/base/logging.h"
#include "talk/media/base/videoframe.h"

#include "AsciiVideoRenderer.h"

const uint8_t AsciiVideoRenderer::_characters[] = " .,:;i1tfLCG08@";

void AsciiVideoRenderer::SetSize(int width, int height) {
	LOG(LS_INFO) << "Video renderer size: w=" << width << ", h=" << height; 
	if( width < 0 || height < 0) {
		LOG(LS_WARNING) << "Invalid size passed to SetSize: w=" << width << ", h=" << height; 
		return;
	}

	_width  =  width;
	_height = height;
}

void AsciiVideoRenderer::RenderFrame(const cricket::VideoFrame* frame) {
	size_t width  = frame->GetWidth();
	size_t height = frame->GetHeight();
	if( width != _width && height != _height) {
		LOG(LS_WARNING) << "Frame height does not match height of renderer";
	}

	const uint8_t* yPlane = frame->GetYPlane();

	const uint8_t* buffer = _factor == 0 ? yPlane : _scaleFrame(yPlane);
	_renderBuffer(buffer, width, height);
}

const uint8_t* AsciiVideoRenderer::_scaleFrame(const uint8_t* frame) {
	uint8_t scale = (2 << (_factor << 1));
	uint8_t len = _width * _height / scale;
	if(!_scaleBuffer) {
		_scaleBuffer = new uint8_t[len];
	}

	for(size_t i = 0; i < len; i++) {
		uint16_t newPixel = 0;
		uint16_t offset = i * scale * scale;
		for(size_t y = 0; y < _factor; y++) {
			for( size_t x = 0; x < _factor; x++) {
				newPixel += frame[offset + x + y * _width];
			}
		}
		_scaleBuffer[i] = newPixel / scale;
	}

	return _scaleBuffer;
}

void AsciiVideoRenderer::_renderBuffer(const uint8_t* buffer, size_t width, size_t height) {

	std::string frameStr;
	uint8_t maxIdx = sizeof(_characters) - 1;
	for(size_t h = 0; h < 43; ++h) {
		for(size_t w = 0; w < width; ++w) {
			uint8_t pixel = buffer[h*width + w];
			if( w < 132 ) frameStr += _characters[maxIdx -  (uint8_t)::round(_contrast*pixel/255 * maxIdx)];
		}
		frameStr += "\n";
	}
	std::cout << frameStr;
	std::cout << "================================================================================" << std::endl;
}