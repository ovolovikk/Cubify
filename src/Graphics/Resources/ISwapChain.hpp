#pragma once

struct SwapChainDesc
{
    void* windowHandle = nullptr;
    int width = 0;
    int height = 0;
};

// The window's back buffers, what a finished frame is presented to
class ISwapChain
{
public:
    virtual ~ISwapChain() = default;

    virtual void resize(int width, int height) = 0;
    virtual void present() = 0;

    virtual int width() const = 0;
    virtual int height() const = 0;
};
