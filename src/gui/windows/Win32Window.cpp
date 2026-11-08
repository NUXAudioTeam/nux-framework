// Windows (Win32) window backend for nxf_gui.
//
// Verification status (honest): compiled/linked by Windows CI. A
// window appearing on a real display is NOT verified - CI has no
// interactive desktop to assert against. Painting is a placeholder
// fill plus the onPaint seam; the real renderer is Phase 8.
#ifndef NOMINMAX
#define NOMINMAX
#endif
#ifndef WIN32_LEAN_AND_MEAN
#define WIN32_LEAN_AND_MEAN
#endif
#include <windows.h>

#include <nxf/gui/WindowFactory.h>

namespace nxf::gui {
namespace {

std::wstring toWide(const std::string& text)
{
    if (text.empty()) return {};
    const int length = MultiByteToWideChar(CP_UTF8, 0, text.c_str(), -1, nullptr, 0);
    if (length <= 1) return {};
    std::wstring out(static_cast<std::size_t>(length - 1), L'\0');
    MultiByteToWideChar(CP_UTF8, 0, text.c_str(), -1, out.data(), length);
    return out;
}

MouseButton buttonOf(UINT message)
{
    switch (message) {
        case WM_LBUTTONDOWN: case WM_LBUTTONUP: return MouseButton::Left;
        case WM_MBUTTONDOWN: case WM_MBUTTONUP: return MouseButton::Middle;
        case WM_RBUTTONDOWN: case WM_RBUTTONUP: return MouseButton::Right;
        default: return MouseButton::None;
    }
}

class Win32Window final : public IWindow {
public:
    Win32Window() = default;
    ~Win32Window() override {
        if (handle_ != nullptr) DestroyWindow(handle_);
    }

    bool create(const WindowConfig& config)
    {
        static bool classRegistered = false;
        if (!classRegistered) {
            WNDCLASSEXW wc{};
            wc.cbSize = sizeof(wc);
            wc.style = CS_HREDRAW | CS_VREDRAW;
            wc.lpfnWndProc = &Win32Window::windowProc;
            wc.hInstance = GetModuleHandleW(nullptr);
            wc.hCursor = LoadCursorW(nullptr, MAKEINTRESOURCEW(32512));   // IDC_ARROW
            wc.lpszClassName = L"NXFWindowClass";
            if (RegisterClassExW(&wc) == 0 && GetLastError() != ERROR_CLASS_ALREADY_EXISTS) return false;
            classRegistered = true;
        }
        handle_ = CreateWindowExW(0, L"NXFWindowClass", toWide(config.title).c_str(),
                                  WS_OVERLAPPEDWINDOW, CW_USEDEFAULT, CW_USEDEFAULT,
                                  config.width, config.height, nullptr, nullptr,
                                  GetModuleHandleW(nullptr), this);
        return handle_ != nullptr;
    }

    void show() override { if (handle_) { ShowWindow(handle_, SW_SHOW); UpdateWindow(handle_); } }
    void hide() override { if (handle_) ShowWindow(handle_, SW_HIDE); }
    [[nodiscard]] bool isVisible() const override { return handle_ != nullptr && IsWindowVisible(handle_) != 0; }
    void setTitle(const std::string& title) override { if (handle_) SetWindowTextW(handle_, toWide(title).c_str()); }
    void setRootComponent(Component* root) override {
        root_ = root;
        if (root_ != nullptr && handle_ != nullptr) {
            RECT client{};
            GetClientRect(handle_, &client);
            root_->setBounds({0, 0, static_cast<int>(client.right), static_cast<int>(client.bottom)});
        }
    }
    void runEventLoop() override {
        MSG message{};
        while (GetMessageW(&message, nullptr, 0, 0) > 0) {
            TranslateMessage(&message);
            DispatchMessageW(&message);
        }
    }

private:
    static LRESULT CALLBACK windowProc(HWND hwnd, UINT message, WPARAM wParam, LPARAM lParam)
    {
        Win32Window* self = nullptr;
        if (message == WM_NCCREATE) {
            auto* create = reinterpret_cast<CREATESTRUCTW*>(lParam);
            self = static_cast<Win32Window*>(create->lpCreateParams);
            SetWindowLongPtrW(hwnd, GWLP_USERDATA, reinterpret_cast<LONG_PTR>(self));
            self->handle_ = hwnd;
        } else {
            self = reinterpret_cast<Win32Window*>(GetWindowLongPtrW(hwnd, GWLP_USERDATA));
        }
        if (self != nullptr) return self->handleMessage(message, wParam, lParam);
        return DefWindowProcW(hwnd, message, wParam, lParam);
    }

    LRESULT handleMessage(UINT message, WPARAM wParam, LPARAM lParam)
    {
        switch (message) {
            case WM_SIZE:
                if (root_ != nullptr)
                    root_->setBounds({0, 0, LOWORD(lParam), HIWORD(lParam)});
                return 0;
            case WM_LBUTTONDOWN: case WM_MBUTTONDOWN: case WM_RBUTTONDOWN:
            case WM_LBUTTONUP: case WM_MBUTTONUP: case WM_RBUTTONUP:
            case WM_MOUSEMOVE: case WM_MOUSEWHEEL: {
                if (root_ == nullptr) break;
                MouseEvent event;
                event.position = {static_cast<int>(static_cast<short>(LOWORD(lParam))),
                                  static_cast<int>(static_cast<short>(HIWORD(lParam)))};
                event.button = buttonOf(message);
                event.shiftDown = (wParam & MK_SHIFT) != 0;
                event.controlDown = (wParam & MK_CONTROL) != 0;
                switch (message) {
                    case WM_LBUTTONDOWN: case WM_MBUTTONDOWN: case WM_RBUTTONDOWN:
                        event.type = MouseEventType::Down; SetCapture(handle_); break;
                    case WM_LBUTTONUP: case WM_MBUTTONUP: case WM_RBUTTONUP:
                        event.type = MouseEventType::Up; ReleaseCapture(); break;
                    case WM_MOUSEMOVE: event.type = MouseEventType::Move; break;
                    case WM_MOUSEWHEEL:
                        event.type = MouseEventType::Wheel;
                        event.wheelDelta = GET_WHEEL_DELTA_WPARAM(wParam);
                        POINT point{event.position.x, event.position.y};
                        ScreenToClient(handle_, &point);
                        event.position = {point.x, point.y};
                        break;
                    default: break;
                }
                root_->dispatchMouseEvent(event);
                return 0;
            }
            case WM_CHAR: {
                if (root_ == nullptr) break;
                KeyEvent event;
                event.character = static_cast<char32_t>(wParam);
                root_->dispatchKeyEvent(event);
                return 0;
            }
            case WM_KEYDOWN: {
                if (root_ == nullptr) break;
                KeyEvent event;
                event.code = static_cast<std::uint32_t>(wParam);
                root_->dispatchKeyEvent(event);
                return 0;
            }
            case WM_PAINT: {
                PAINTSTRUCT paint{};
                HDC dc = BeginPaint(handle_, &paint);
                HBRUSH brush = CreateSolidBrush(RGB(24, 26, 30));
                FillRect(dc, &paint.rcPaint, brush);
                DeleteObject(brush);
                if (onPaint) onPaint();
                EndPaint(handle_, &paint);
                return 0;
            }
            case WM_CLOSE:
                DestroyWindow(handle_);
                return 0;
            case WM_DESTROY:
                handle_ = nullptr;
                PostQuitMessage(0);
                return 0;
            default: break;
        }
        return DefWindowProcW(handle_, message, wParam, lParam);
    }

    HWND handle_ = nullptr;
    Component* root_ = nullptr;
};

} // namespace

std::string_view windowBackendName() noexcept { return "win32"; }

std::unique_ptr<IWindow> createWindow(const WindowConfig& config)
{
    auto window = std::make_unique<Win32Window>();
    if (!window->create(config)) return nullptr;
    return window;
}

} // namespace nxf::gui
