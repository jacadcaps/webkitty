#pragma once
#include <functional>

struct WebViewDelegate
{
	std::function<void()>           _fInvalidate;
	std::function<void(int, int)>   _fScroll;
	std::function<void(int, int)>   _setDocumentSize;
	std::function<void(int&, int&)> _getViewSize;
};
