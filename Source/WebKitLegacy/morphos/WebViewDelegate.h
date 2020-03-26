#pragma once
#include <functional>

struct WebViewDelegate
{
	std::function<void()> _fInvalidate;
	std::function<void(int, int, int, int)> _fInvalidateRect;
};
