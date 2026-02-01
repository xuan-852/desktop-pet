// 包含头文件

#define WIN32_LEAN_AND_MEAN
#include <windows.h>
#include <shellapi.h>
#include <WebView2.h>
#include <objidl.h>
#include <gdiplus.h>
#include <commdlg.h>
#include <wchar.h>
#include <math.h>
#include <stdlib.h>

//宏定义

// 定义暂停时间（单位：毫秒）
#define BEING_TIME 1500

// 聊天窗口初始位置与大小（相对宠物窗口）
#define CHAT_WINDOW_OFFSET_X 2
#define CHAT_WINDOW_OFFSET_Y -50
#define CHAT_WINDOW_INIT_WIDTH 420
#define CHAT_WINDOW_INIT_HEIGHT 180

// 地面任务权重（可在此调整五个任务的比重）
#define TASK_WEIGHT_MOVE_LEFT_EDGE   3 // 向左走到边缘
#define TASK_WEIGHT_MOVE_RIGHT_EDGE  3 // 向右走到边缘
#define TASK_WEIGHT_MOVE_LEFT_TIME   20 // 向左走固定时间
#define TASK_WEIGHT_MOVE_RIGHT_TIME  20 // 向右走固定时间
#define TASK_WEIGHT_STOP_TIME        10 // 停止固定时间
// 点击图片权重（可在此调整点击随机图片比重）
#define CLICK_WEIGHT_6  9// 点击图片 6.png 的权重
#define CLICK_WEIGHT_7  1// 点击图片 7.png 的权重
// 停止时触发图片权重（可在此调整停止触发比重）
#define STOP_WEIGHT_1  4// 停止时图片 1.png 的权重
#define STOP_WEIGHT_4  1// 停止时图片 4.png 的权重
#define STOP_TRIGGER_CHANCE 50 // 停止时触发图片的概率（百分比）
// 拖动释放 Y 轴阈值（绝对坐标，越大越靠近屏幕底部）
#define DRAG_HEIGHT_THRESHOLD 750 // 释放时 Y 值超过该值使用 5.png，否则使用 2.png
// 拖拽落地时图片权重（可在此调整落地随机图片比重）
#define LAND_WEIGHT_2  9// 落地图片 2.png 的权重
#define LAND_WEIGHT_8  1// 落地图片 8.png 的权重
// 地面任务持续时间（单位：毫秒）
#define TASK_MOVE_TIME_MS 5000
#define TASK_STOP_TIME_MS 1500

// 聊天窗口控件 ID
#define IDC_CHAT_OUTPUT 2001
#define IDC_CHAT_INPUT 2002
#define IDC_CHAT_SEND 2003
#define IDC_CHAT_CONFIG 2004
#define IDC_CHAT_COLOR_BG 2005
#define IDC_CHAT_COLOR_TEXT 2006

using namespace Gdiplus;

// 图片缩放比例（1.0 为原始大小，可改为 0.5、2.0 等）
static const float kPetScale = 0.15f;

// 宠物的运行状态
typedef struct {
    int x;
    int y;     // 宠物的当前坐标
    int vx;
    int vy;    // 宠物的当前速度
    int width; // 宠物宽度
    int height;// 宠物高度
} PetState;

// 窗口类名
static const wchar_t kClassName[] = L"DesktopPetWindow";
// 初始位置、速度与大小（大小会被图片尺寸覆盖）
static PetState g_pet = { 50, 25, 0, 2, 2, 2 };

// GDI+ 资源
static ULONG_PTR g_gdiplusToken = 0;
static Image* g_image = NULL;           // 当前显示的图片
static Image* g_imageDefault = NULL;    // 默认图片
static Image* g_imageDragLeft = NULL;   // 拖动时图片 3.png（left文件夹）
static Image* g_imageDragRight = NULL;  // 拖动时图片 3.png（right文件夹）
static Image* g_imageFallLeft = NULL;   // 下落时图片 5.png（left文件夹）
static Image* g_imageFallRight = NULL;  // 下落时图片 5.png（right文件夹）
static Image* g_imageWalkLeft = NULL;   // 向左走图片 0.png（left文件夹）
static Image* g_imageWalkRight = NULL;  // 向右走图片 0.png（right文件夹）
static Image* g_imageNearGroundLeft = NULL;  // 低高度下落图片 2.png（left文件夹）
static Image* g_imageNearGroundRight = NULL; // 低高度下落图片 2.png（right文件夹）
static Image* g_imageLandLeft = NULL;    // 落地图片 2.png（left文件夹）
static Image* g_imageLandRight = NULL;   // 落地图片 2.png（right文件夹）
static Image* g_imageLandSpecialLeft = NULL; // 落地特殊图片 8.png（left文件夹）
static Image* g_imageLandSpecialRight = NULL; // 落地特殊图片 8.png（right文件夹）
static BOOL g_movedSinceClick = TRUE;   // 是否在上次点击后发生过移动
static BOOL g_onGround = FALSE;         // 是否在地面上
static int g_landWeight2 = LAND_WEIGHT_2;
static int g_landWeight8 = LAND_WEIGHT_8;

//函数声明处

static Image* PickStopImage1ByDirection(void) ;
static Image* PickStopImage4ByDirection(void) ;
static void ShowChatWindow(HWND owner);
static LRESULT CALLBACK ChatWndProc(HWND hwnd, UINT msg, WPARAM wParam, LPARAM lParam);
static LRESULT CALLBACK WebViewHostWndProc(HWND hwnd, UINT msg, WPARAM wParam, LPARAM lParam);
static void EnsureWebView2Initialized(HWND owner);
static void PostChatToWebView(const wchar_t* text, int requestId);
static void SendChatMessage(void);
static void ShowWebViewConfigWindow(HWND owner);
static void SwitchToImage(Image* img);
static void ApplyChatColor(BOOL refresh);
static BOOL PickChatColor(HWND owner, BOOL isText);

// 点击图片资源（按方向区分 left/right）
typedef struct {
    const wchar_t* fileName;
    int weight;
    Image* left;
    Image* right;
} ClickImageItem;

static ClickImageItem g_clickItems[] = {
    { L"6.png", CLICK_WEIGHT_6, NULL, NULL },
    { L"7.png", CLICK_WEIGHT_7, NULL, NULL }
};
static const int kClickItemCount = sizeof(g_clickItems) / sizeof(g_clickItems[0]);

// 停止触发图片资源（按方向区分 left/right）
static ClickImageItem g_stopItems[] = {
    { L"1.png", STOP_WEIGHT_1, NULL, NULL },
    { L"4.png", STOP_WEIGHT_4, NULL, NULL }
};
static const int kStopItemCount = sizeof(g_stopItems) / sizeof(g_stopItems[0]);

// 状态控制变量
static BOOL g_isPaused = FALSE;         // 是否暂停移动
static DWORD g_pauseUntil = 0;          // 暂停结束的时间戳
static HWND g_hwnd = NULL;              // 主窗口句柄
static BOOL g_isDragging = FALSE;       // 是否正在拖动窗口（用于StepPet判断）
static BOOL g_hasLanded = FALSE;        // 是否已经落地（用于控制首次落地效果）
static BOOL g_ignoreClick = FALSE;      // 双击时忽略一次单击触发

// 聊天窗口与 WebView2
static HWND g_chatHwnd = NULL;
static HWND g_chatOutput = NULL;
static HWND g_chatInput = NULL;
static HWND g_chatSendBtn = NULL;
static HWND g_chatConfigBtn = NULL;
static COLORREF g_chatBgColor = RGB(255, 236, 255);
static COLORREF g_chatTextColor = RGB(0, 0, 0);
static HBRUSH g_chatBgBrush = NULL;
static COLORREF g_chatCustomColors[16] = { 0 };
static BOOL g_chatCustomColorsInit = FALSE;
static HWND g_webviewHostHwnd = NULL;
static BOOL g_webviewReady = FALSE;
static BOOL g_webviewInitPending = FALSE;
static int g_chatRequestId = 0;
static BOOL g_hasPendingSend = FALSE;
static wchar_t g_pendingSendText[2048] = { 0 };
static int g_pendingSendId = 0;

static ICoreWebView2Environment* g_webviewEnv = NULL;
static ICoreWebView2Controller* g_webviewController = NULL;
static ICoreWebView2* g_webview = NULL;

// 地面移动任务
typedef enum {
    TASK_NONE = 0,
    TASK_MOVE_LEFT_EDGE,
    TASK_MOVE_RIGHT_EDGE,
    TASK_MOVE_LEFT_TIME,
    TASK_MOVE_RIGHT_TIME,
    TASK_STOP_TIME
} GroundTask;

static GroundTask g_groundTask = TASK_NONE;
static DWORD g_groundTaskEnd = 0;
static GroundTask g_lastGroundTask = TASK_NONE;

// 区分点击和拖拽的变量
static BOOL g_beganDrag = FALSE;        // 标记是否已经开始系统拖拽
static POINT g_startPos = { 0 };        // 记录鼠标按下的起始位置
// 拖动速度记录（像素/帧）
static POINT g_dragPrevPos = { 0, 0 };
static DWORD g_dragPrevTick = 0;
static int g_dragVx = 0;
static int g_dragVy = 0;
static int g_dragPrevWndX = 0;
static int g_lastDragDir = 0; // -1 向左，1 向右，0 未知
static int g_dragStartWndY = 0; // 拖动开始时窗口 Y
static BOOL g_useLowFallImage = FALSE; // 是否使用 2.png 下落图
static BOOL g_useWeightedFallImage = FALSE; // 是否在高位下落时按权重选择 2.png/8.png
static BOOL g_weightedFallChosen = FALSE; // 是否已选定高位下落图片
static BOOL g_weightedFallUseSpecial = FALSE; // TRUE=8.png, FALSE=2.png

// 设置窗口相关
static HWND g_settingsHwnd = NULL;
static HWND g_editClick6 = NULL;
static HWND g_editClick7 = NULL;
static HWND g_editStop1 = NULL;
static HWND g_editStop4 = NULL;
static HWND g_editLand2 = NULL;
static HWND g_editLand8 = NULL;
static BOOL g_settingsClassRegistered = FALSE;
static BOOL g_isPausedForSettings = FALSE;
static BOOL g_prevPaused = FALSE;
static DWORD g_prevPauseUntil = 0;
static Image* g_prevImage = NULL;
static BOOL g_isPausedForChat = FALSE;
static BOOL g_prevPausedChat = FALSE;
static DWORD g_prevPauseUntilChat = 0;
static Image* g_prevImageChat = NULL;

// 判断文件是否存在
static BOOL FileExists(const wchar_t* path) {
    DWORD attrs = GetFileAttributesW(path);
    return (attrs != INVALID_FILE_ATTRIBUTES) && !(attrs & FILE_ATTRIBUTE_DIRECTORY);
}

// 获取聊天 HTML 路径
static BOOL GetChatHtmlPath(wchar_t* outPath, size_t outSize) {
    const wchar_t* kChatFileName = L"Midsummer's Bird.html";
    wchar_t modulePath[MAX_PATH] = { 0 };
    GetModuleFileNameW(NULL, modulePath, MAX_PATH);
    wchar_t* last = wcsrchr(modulePath, L'\\');
    if (last) {
        *last = L'\0';
    }

    // 优先使用 exe 路径的上级目录（与 photo_* 同级）
    swprintf(outPath, outSize, L"%ls\\..\\ai_chat\\%ls", modulePath, kChatFileName);
    if (FileExists(outPath)) {
        return TRUE;
    }

    // 再尝试当前工作目录
    wchar_t cwd[MAX_PATH] = { 0 };
    GetCurrentDirectoryW(MAX_PATH, cwd);
    swprintf(outPath, outSize, L"%ls\\ai_chat\\%ls", cwd, kChatFileName);
    if (FileExists(outPath)) {
        return TRUE;
    }
    return FALSE;
}

// 打开聊天窗口（默认浏览器）
static void AppendChatText(const wchar_t* prefix, const wchar_t* text) {
    if (!g_chatOutput) {
        return;
    }
    int len = GetWindowTextLengthW(g_chatOutput);
    SendMessageW(g_chatOutput, EM_SETSEL, (WPARAM)len, (LPARAM)len);
    if (prefix) {
        SendMessageW(g_chatOutput, EM_REPLACESEL, FALSE, (LPARAM)prefix);
    }
    if (text) {
        SendMessageW(g_chatOutput, EM_REPLACESEL, FALSE, (LPARAM)text);
    }
    SendMessageW(g_chatOutput, EM_REPLACESEL, FALSE, (LPARAM)L"\r\n");
}

static void PathToFileUrl(const wchar_t* path, wchar_t* out, size_t outSize) {
    if (!path || !out || outSize == 0) {
        return;
    }
    const wchar_t* prefix = L"file:///";
    size_t idx = 0;
    for (size_t i = 0; prefix[i] && idx + 1 < outSize; ++i) {
        out[idx++] = prefix[i];
    }
    for (size_t i = 0; path[i] && idx + 1 < outSize; ++i) {
        wchar_t ch = path[i];
        if (ch == L'\\') {
            out[idx++] = L'/';
        } else if (ch == L' ') {
            if (idx + 3 < outSize) {
                out[idx++] = L'%';
                out[idx++] = L'2';
                out[idx++] = L'0';
            }
        } else {
            out[idx++] = ch;
        }
    }
    out[idx] = 0;
}

static void JsonEscape(const wchar_t* input, wchar_t* out, size_t outSize) {
    if (!input || !out || outSize == 0) {
        return;
    }
    size_t idx = 0;
    for (size_t i = 0; input[i] && idx + 1 < outSize; ++i) {
        wchar_t ch = input[i];
        if (ch == L'"' || ch == L'\\') {
            if (idx + 2 >= outSize) break;
            out[idx++] = L'\\';
            out[idx++] = ch;
        } else if (ch == L'\n') {
            if (idx + 2 >= outSize) break;
            out[idx++] = L'\\';
            out[idx++] = L'n';
        } else if (ch == L'\r') {
            if (idx + 2 >= outSize) break;
            out[idx++] = L'\\';
            out[idx++] = L'r';
        } else if (ch == L'\t') {
            if (idx + 2 >= outSize) break;
            out[idx++] = L'\\';
            out[idx++] = L't';
        } else {
            out[idx++] = ch;
        }
    }
    out[idx] = 0;
}

static BOOL JsonExtractString(const wchar_t* json, const wchar_t* key, wchar_t* out, size_t outSize) {
    if (!json || !key || !out || outSize == 0) {
        return FALSE;
    }
    wchar_t pattern[64] = { 0 };
    swprintf(pattern, 64, L"\"%ls\":\"", key);
    const wchar_t* p = wcsstr(json, pattern);
    if (!p) {
        return FALSE;
    }
    p += wcslen(pattern);
    size_t idx = 0;
    while (*p && idx + 1 < outSize) {
        if (*p == L'\\') {
            ++p;
            if (!*p) break;
            switch (*p) {
                case L'n': out[idx++] = L'\n'; break;
                case L'r': out[idx++] = L'\r'; break;
                case L't': out[idx++] = L'\t'; break;
                case L'"': out[idx++] = L'"'; break;
                case L'\\': out[idx++] = L'\\'; break;
                default: out[idx++] = *p; break;
            }
            ++p;
            continue;
        }
        if (*p == L'"') {
            break;
        }
        out[idx++] = *p++;
    }
    out[idx] = 0;
    return TRUE;
}

static BOOL JsonExtractInt(const wchar_t* json, const wchar_t* key, int* outValue) {
    if (!json || !key || !outValue) {
        return FALSE;
    }
    wchar_t pattern[64] = { 0 };
    swprintf(pattern, 64, L"\"%ls\":", key);
    const wchar_t* p = wcsstr(json, pattern);
    if (!p) {
        return FALSE;
    }
    p += wcslen(pattern);
    *outValue = (int)wcstol(p, NULL, 10);
    return TRUE;
}

static void ApplyChatColor(BOOL refresh) {// 应用聊天窗口颜色
    if (g_chatBgBrush) {
        DeleteObject(g_chatBgBrush);
        g_chatBgBrush = NULL;
    }
    g_chatBgBrush = CreateSolidBrush(g_chatBgColor);

    if (refresh) {
        if (g_chatOutput) {
            InvalidateRect(g_chatOutput, NULL, TRUE);
        }
        if (g_chatInput) {
            InvalidateRect(g_chatInput, NULL, TRUE);
        }
        if (g_chatHwnd) {
            InvalidateRect(g_chatHwnd, NULL, TRUE);
        }
    }
}

static void HandleWebViewMessage(const wchar_t* json) {
    if (!json) {
        return;
    }
    wchar_t type[32] = { 0 };
    wchar_t content[2048] = { 0 };
    if (!JsonExtractString(json, L"type", type, 32)) {
        return;
    }
    if (wcscmp(type, L"reply") == 0) {
        if (JsonExtractString(json, L"content", content, 2048)) {
            AppendChatText(L"AI: ", content);
        }
    } else if (wcscmp(type, L"error") == 0) {
        if (JsonExtractString(json, L"message", content, 2048)) {
            AppendChatText(L"系统: ", content);
        }
    }
}

class WebView2RefCounted {
public:
    WebView2RefCounted() : refCount(1) {}
    virtual ~WebView2RefCounted() {}
    ULONG AddRef() { return InterlockedIncrement(&refCount); }
    ULONG Release() {
        ULONG res = InterlockedDecrement(&refCount);
        if (res == 0) {
            delete this;
        }
        return res;
    }
private:
    ULONG refCount;
};

class WebViewEnvCompletedHandler : public ICoreWebView2CreateCoreWebView2EnvironmentCompletedHandler, public WebView2RefCounted {
public:
    explicit WebViewEnvCompletedHandler(HWND owner) : m_owner(owner) {}

    HRESULT STDMETHODCALLTYPE QueryInterface(REFIID riid, void** ppvObject) override {
        if (!ppvObject) return E_POINTER;
        if (IsEqualIID(riid, IID_IUnknown) || IsEqualIID(riid, IID_ICoreWebView2CreateCoreWebView2EnvironmentCompletedHandler)) {
            *ppvObject = static_cast<ICoreWebView2CreateCoreWebView2EnvironmentCompletedHandler*>(this);
            AddRef();
            return S_OK;
        }
        *ppvObject = NULL;
        return E_NOINTERFACE;
    }
    ULONG STDMETHODCALLTYPE AddRef() override { return WebView2RefCounted::AddRef(); }
    ULONG STDMETHODCALLTYPE Release() override { return WebView2RefCounted::Release(); }

    HRESULT STDMETHODCALLTYPE Invoke(HRESULT result, ICoreWebView2Environment* env) override;

private:
    HWND m_owner;
};

class WebViewControllerCompletedHandler : public ICoreWebView2CreateCoreWebView2ControllerCompletedHandler, public WebView2RefCounted {
public:
    explicit WebViewControllerCompletedHandler(HWND owner) : m_owner(owner) {}

    HRESULT STDMETHODCALLTYPE QueryInterface(REFIID riid, void** ppvObject) override {
        if (!ppvObject) return E_POINTER;
        if (IsEqualIID(riid, IID_IUnknown) || IsEqualIID(riid, IID_ICoreWebView2CreateCoreWebView2ControllerCompletedHandler)) {
            *ppvObject = static_cast<ICoreWebView2CreateCoreWebView2ControllerCompletedHandler*>(this);
            AddRef();
            return S_OK;
        }
        *ppvObject = NULL;
        return E_NOINTERFACE;
    }
    ULONG STDMETHODCALLTYPE AddRef() override { return WebView2RefCounted::AddRef(); }
    ULONG STDMETHODCALLTYPE Release() override { return WebView2RefCounted::Release(); }

    HRESULT STDMETHODCALLTYPE Invoke(HRESULT result, ICoreWebView2Controller* controller) override;

private:
    HWND m_owner;
};

class WebViewMessageReceivedHandler : public ICoreWebView2WebMessageReceivedEventHandler, public WebView2RefCounted {
public:
    HRESULT STDMETHODCALLTYPE QueryInterface(REFIID riid, void** ppvObject) override {
        if (!ppvObject) return E_POINTER;
        if (IsEqualIID(riid, IID_IUnknown) || IsEqualIID(riid, IID_ICoreWebView2WebMessageReceivedEventHandler)) {
            *ppvObject = static_cast<ICoreWebView2WebMessageReceivedEventHandler*>(this);
            AddRef();
            return S_OK;
        }
        *ppvObject = NULL;
        return E_NOINTERFACE;
    }
    ULONG STDMETHODCALLTYPE AddRef() override { return WebView2RefCounted::AddRef(); }
    ULONG STDMETHODCALLTYPE Release() override { return WebView2RefCounted::Release(); }

    HRESULT STDMETHODCALLTYPE Invoke(ICoreWebView2* sender, ICoreWebView2WebMessageReceivedEventArgs* args) override {
        (void)sender;
        LPWSTR msg = NULL;
        if (SUCCEEDED(args->get_WebMessageAsJson(&msg)) && msg) {
            HandleWebViewMessage(msg);
            CoTaskMemFree(msg);
        }
        return S_OK;
    }
};

class WebViewNavigationCompletedHandler : public ICoreWebView2NavigationCompletedEventHandler, public WebView2RefCounted {
public:
    HRESULT STDMETHODCALLTYPE QueryInterface(REFIID riid, void** ppvObject) override {
        if (!ppvObject) return E_POINTER;
        if (IsEqualIID(riid, IID_IUnknown) || IsEqualIID(riid, IID_ICoreWebView2NavigationCompletedEventHandler)) {
            *ppvObject = static_cast<ICoreWebView2NavigationCompletedEventHandler*>(this);
            AddRef();
            return S_OK;
        }
        *ppvObject = NULL;
        return E_NOINTERFACE;
    }
    ULONG STDMETHODCALLTYPE AddRef() override { return WebView2RefCounted::AddRef(); }
    ULONG STDMETHODCALLTYPE Release() override { return WebView2RefCounted::Release(); }

    HRESULT STDMETHODCALLTYPE Invoke(ICoreWebView2* sender, ICoreWebView2NavigationCompletedEventArgs* args) override {
        (void)sender;
        (void)args;
        g_webviewReady = TRUE;
        g_webviewInitPending = FALSE;
        if (g_hasPendingSend) {
            g_hasPendingSend = FALSE;
            PostChatToWebView(g_pendingSendText, g_pendingSendId);
        }
        return S_OK;
    }
};

HRESULT WebViewEnvCompletedHandler::Invoke(HRESULT result, ICoreWebView2Environment* env) {
    if (FAILED(result) || !env) {
        MessageBoxW(m_owner, L"创建 WebView2 环境失败。", L"WebView2", MB_ICONERROR | MB_OK);
        g_webviewInitPending = FALSE;
        return S_OK;
    }
    g_webviewEnv = env;
    g_webviewEnv->AddRef();
    env->CreateCoreWebView2Controller(g_webviewHostHwnd, new WebViewControllerCompletedHandler(m_owner));
    return S_OK;
}

HRESULT WebViewControllerCompletedHandler::Invoke(HRESULT result, ICoreWebView2Controller* controller) {
    if (FAILED(result) || !controller) {
        MessageBoxW(m_owner, L"创建 WebView2 控制器失败。", L"WebView2", MB_ICONERROR | MB_OK);
        g_webviewInitPending = FALSE;
        return S_OK;
    }
    g_webviewController = controller;
    g_webviewController->AddRef();
    g_webviewController->get_CoreWebView2(&g_webview);
    if (g_webview) {
        g_webview->AddRef();
    }
    if (g_webviewController) {
        RECT bounds = { 0, 0, 1, 1 };
        g_webviewController->put_Bounds(bounds);
        g_webviewController->put_IsVisible(FALSE);
    }

    if (g_webview) {
        EventRegistrationToken token = { 0 };
        g_webview->add_WebMessageReceived(new WebViewMessageReceivedHandler(), &token);
        EventRegistrationToken navToken = { 0 };
        g_webview->add_NavigationCompleted(new WebViewNavigationCompletedHandler(), &navToken);

        wchar_t htmlPath[MAX_PATH] = { 0 };
        if (GetChatHtmlPath(htmlPath, MAX_PATH)) {
            wchar_t url[2048] = { 0 };
            PathToFileUrl(htmlPath, url, 2048);
            g_webview->Navigate(url);
        } else {
            MessageBoxW(m_owner, L"未找到 ai_chat 的 HTML 文件。", L"WebView2", MB_ICONERROR | MB_OK);
            g_webviewInitPending = FALSE;
            g_webviewReady = FALSE;
        }
    }
    return S_OK;
}

static void PostChatToWebView(const wchar_t* text, int requestId) {
    if (!text || !g_webview || !g_webviewReady) {
        g_hasPendingSend = TRUE;
        g_pendingSendId = requestId;
        wcsncpy(g_pendingSendText, text ? text : L"", 2047);
        g_pendingSendText[2047] = 0;
        AppendChatText(L"系统: ", L"聊天引擎加载中...");
        return;
    }
    wchar_t escaped[4096] = { 0 };
    JsonEscape(text, escaped, 4096);
    wchar_t json[4600] = { 0 };
    swprintf(json, 4600, L"{\"type\":\"send\",\"id\":%d,\"text\":\"%ls\"}", requestId, escaped);
    g_webview->PostWebMessageAsJson(json);
}

static LRESULT CALLBACK WebViewHostWndProc(HWND hwnd, UINT msg, WPARAM wParam, LPARAM lParam) {
    switch (msg) {
        case WM_SIZE:
            if (g_webviewController) {
                RECT rc;
                GetClientRect(hwnd, &rc);
                g_webviewController->put_Bounds(rc);
            }
            return 0;
        case WM_CLOSE:
            if (g_webviewController) {
                g_webviewController->put_IsVisible(FALSE);
            }
            ShowWindow(hwnd, SW_HIDE);
            return 0;
        case WM_DESTROY:
            g_webviewHostHwnd = NULL;
            return 0;
    }
    return DefWindowProcW(hwnd, msg, wParam, lParam);
}

static void EnsureWebView2Initialized(HWND owner) {
    if (g_webviewReady || g_webviewInitPending) {
        return;
    }
    g_webviewInitPending = TRUE;

    if (!g_webviewHostHwnd) {
        const wchar_t* hostClass = L"WebView2HostWindow";
        WNDCLASSW wc = { 0 };
        wc.lpfnWndProc = WebViewHostWndProc;
        wc.hInstance = GetModuleHandleW(NULL);
        wc.lpszClassName = hostClass;
        RegisterClassW(&wc);

        g_webviewHostHwnd = CreateWindowExW(
            WS_EX_TOOLWINDOW, hostClass, L"AI 聊天配置",
            WS_OVERLAPPEDWINDOW,
            CW_USEDEFAULT, CW_USEDEFAULT, 1200, 800, owner, NULL, wc.hInstance, NULL);
        ShowWindow(g_webviewHostHwnd, SW_HIDE);
    }

    HMODULE loader = LoadLibraryW(L"WebView2Loader.dll");
    if (!loader) {
        MessageBoxW(owner, L"未找到 WebView2Loader.dll。请安装 WebView2 Runtime。", L"WebView2", MB_ICONERROR | MB_OK);
        g_webviewInitPending = FALSE;
        return;
    }

    typedef HRESULT (STDAPICALLTYPE *PFN_CreateWebView2Env)(PCWSTR, PCWSTR, ICoreWebView2EnvironmentOptions*, ICoreWebView2CreateCoreWebView2EnvironmentCompletedHandler*);
    PFN_CreateWebView2Env createEnv = (PFN_CreateWebView2Env)GetProcAddress(loader, "CreateCoreWebView2EnvironmentWithOptions");
    if (!createEnv) {
        MessageBoxW(owner, L"无法加载 CreateCoreWebView2EnvironmentWithOptions。", L"WebView2", MB_ICONERROR | MB_OK);
        g_webviewInitPending = FALSE;
        return;
    }
    createEnv(nullptr, nullptr, nullptr, new WebViewEnvCompletedHandler(owner));
}

static void ShowWebViewConfigWindow(HWND owner) {
    EnsureWebView2Initialized(owner);
    if (!g_webviewHostHwnd) {
        return;
    }
    ShowWindow(g_webviewHostHwnd, SW_SHOWNORMAL);
    SetForegroundWindow(g_webviewHostHwnd);
    if (g_webviewController) {
        RECT rc;
        GetClientRect(g_webviewHostHwnd, &rc);
        g_webviewController->put_Bounds(rc);
        g_webviewController->put_IsVisible(TRUE);
    }
}

static void SendChatMessage(void) {
    if (!g_chatInput) {
        return;
    }
    wchar_t text[1024] = { 0 };
    GetWindowTextW(g_chatInput, text, 1024);
    if (text[0] == L'\0') {
        return;
    }
    SetWindowTextW(g_chatInput, L"");
    AppendChatText(L"我: ", text);

    int requestId = ++g_chatRequestId;
    EnsureWebView2Initialized(g_chatHwnd);
    PostChatToWebView(text, requestId);
}

static LRESULT CALLBACK ChatWndProc(HWND hwnd, UINT msg, WPARAM wParam, LPARAM lParam) {
    switch (msg) {
        case WM_CREATE:
        {
            g_chatOutput = CreateWindowExW(WS_EX_CLIENTEDGE, L"EDIT", L"",
                WS_CHILD | WS_VISIBLE | WS_VSCROLL | ES_MULTILINE | ES_AUTOVSCROLL | ES_READONLY,
                8, 8, 360, 200, hwnd, (HMENU)IDC_CHAT_OUTPUT, GetModuleHandleW(NULL), NULL);

            g_chatInput = CreateWindowExW(WS_EX_CLIENTEDGE, L"EDIT", L"",
                WS_CHILD | WS_VISIBLE | ES_AUTOHSCROLL,
                8, 216, 270, 26, hwnd, (HMENU)IDC_CHAT_INPUT, GetModuleHandleW(NULL), NULL);

            g_chatSendBtn = CreateWindowExW(0, L"BUTTON", L"发送",
                WS_CHILD | WS_VISIBLE | BS_PUSHBUTTON,
                286, 216, 82, 26, hwnd, (HMENU)IDC_CHAT_SEND, GetModuleHandleW(NULL), NULL);

            g_chatConfigBtn = CreateWindowExW(0, L"BUTTON", L"配置",
                WS_CHILD | WS_VISIBLE | BS_PUSHBUTTON,
                372, 216, 82, 26, hwnd, (HMENU)IDC_CHAT_CONFIG, GetModuleHandleW(NULL), NULL);
            ApplyChatColor(TRUE);
            return 0;
        }

        case WM_SIZE:// 调整窗口大小时重新布局控件
        {
            int width = LOWORD(lParam);
            int height = HIWORD(lParam);
            int padding = 8;
            int inputHeight = 26;
            int buttonWidth = 82;
            int buttonGap = 8;
            int outputHeight = height - padding * 3 - inputHeight;
            int inputWidth = width - padding * 3 - buttonWidth * 2 - buttonGap;

            MoveWindow(g_chatOutput, padding, padding, width - padding * 2, outputHeight, TRUE);
            MoveWindow(g_chatInput, padding, padding + outputHeight + padding, inputWidth, inputHeight, TRUE);
            MoveWindow(g_chatSendBtn, padding + inputWidth + padding, padding + outputHeight + padding, buttonWidth, inputHeight, TRUE);
            MoveWindow(g_chatConfigBtn, padding + inputWidth + padding + buttonWidth + buttonGap, padding + outputHeight + padding, buttonWidth, inputHeight, TRUE);
            return 0;
        }

        case WM_COMMAND:
            if (LOWORD(wParam) == IDC_CHAT_SEND && HIWORD(wParam) == BN_CLICKED) {
                SendChatMessage();
                return 0;
            }
            if (LOWORD(wParam) == IDC_CHAT_CONFIG && HIWORD(wParam) == BN_CLICKED) {
                ShowWebViewConfigWindow(hwnd);
                return 0;
            }
            return 0;

        case WM_CTLCOLORSTATIC:
        case WM_CTLCOLOREDIT:
        {
            HDC hdc = (HDC)wParam;
            HWND hCtrl = (HWND)lParam;
            if (hCtrl == g_chatOutput || hCtrl == g_chatInput) {
                SetTextColor(hdc, g_chatTextColor);
                SetBkColor(hdc, g_chatBgColor);
                return (LRESULT)g_chatBgBrush;
            }
            break;
        }

        case WM_CLOSE:
            ShowWindow(hwnd, SW_HIDE);
            if (g_isPausedForChat) {
                g_isPausedForChat = FALSE;
                g_isPaused = g_prevPausedChat;
                g_pauseUntil = g_prevPauseUntilChat;
                if (g_prevImageChat) {
                    SwitchToImage(g_prevImageChat);
                }
            }
            return 0;

        case WM_DESTROY:
            g_chatHwnd = NULL;
            g_chatOutput = NULL;
            g_chatInput = NULL;
            g_chatSendBtn = NULL;
            g_chatConfigBtn = NULL;
            return 0;
    }
    return DefWindowProcW(hwnd, msg, wParam, lParam);
}

static void ShowChatWindow(HWND owner) {
    if (!g_chatHwnd) {
        const wchar_t* chatClass = L"DesktopPetChatWindow";
        WNDCLASSW wc = { 0 };
        wc.lpfnWndProc = ChatWndProc;
        wc.hInstance = GetModuleHandleW(NULL);
        wc.lpszClassName = chatClass;
        wc.hCursor = LoadCursor(NULL, IDC_ARROW);
        RegisterClassW(&wc);

        int x = g_pet.x + g_pet.width + CHAT_WINDOW_OFFSET_X;
        int y = g_pet.y + CHAT_WINDOW_OFFSET_Y;
        g_chatHwnd = CreateWindowExW(
            WS_EX_TOOLWINDOW,
            chatClass,
            L"宠物聊天",
            WS_OVERLAPPED | WS_CAPTION | WS_SYSMENU | WS_MINIMIZEBOX | WS_SIZEBOX,
            x, y, CHAT_WINDOW_INIT_WIDTH, CHAT_WINDOW_INIT_HEIGHT,
            owner, NULL, wc.hInstance, NULL);
    }

    ShowWindow(g_chatHwnd, SW_SHOWNORMAL);
    SetForegroundWindow(g_chatHwnd);
    EnsureWebView2Initialized(owner);
}

// 获取默认图片路径
static void GetImagePath(wchar_t* outPath, size_t outSize) {
    wchar_t modulePath[MAX_PATH] = { 0 };
    GetModuleFileNameW(NULL, modulePath, MAX_PATH);
    wchar_t* last = wcsrchr(modulePath, L'\\');
    if (last) {
        *last = L'\0';
    }

    // 默认先尝试 right 目录的 0.png（基于 exe 路径）
    swprintf(outPath, outSize, L"%ls\\..\\photo_right\\0.png", modulePath);
    if (FileExists(outPath)) {
        return;
    }

    // 再尝试 left 目录的 0.png（基于 exe 路径）
    swprintf(outPath, outSize, L"%ls\\..\\photo_left\\0.png", modulePath);
    if (FileExists(outPath)) {
        return;
    }

    // 再尝试在 right 目录里找第一个 png（基于 exe 路径）
    wchar_t searchPath[MAX_PATH] = { 0 };
    swprintf(searchPath, MAX_PATH, L"%ls\\..\\photo_right\\*.png", modulePath);

    WIN32_FIND_DATAW fd = { 0 };
    HANDLE hFind = FindFirstFileW(searchPath, &fd);
    if (hFind != INVALID_HANDLE_VALUE) {
        FindClose(hFind);
        swprintf(outPath, outSize, L"%ls\\..\\photo_right\\%ls", modulePath, fd.cFileName);
        return;
    }

    // 再尝试在 left 目录里找第一个 png（基于 exe 路径）
    swprintf(searchPath, MAX_PATH, L"%ls\\..\\photo_left\\*.png", modulePath);

    hFind = FindFirstFileW(searchPath, &fd);
    if (hFind != INVALID_HANDLE_VALUE) {
        FindClose(hFind);
        swprintf(outPath, outSize, L"%ls\\..\\photo_left\\%ls", modulePath, fd.cFileName);
        return;
    }

    // 如果 exe 路径找不到，尝试当前工作目录
    wchar_t cwd[MAX_PATH] = { 0 };
    GetCurrentDirectoryW(MAX_PATH, cwd);

    swprintf(outPath, outSize, L"%ls\\photo_right\\0.png", cwd);
    if (FileExists(outPath)) {
        return;
    }

    swprintf(outPath, outSize, L"%ls\\photo_left\\0.png", cwd);
    if (FileExists(outPath)) {
        return;
    }

    swprintf(searchPath, MAX_PATH, L"%ls\\photo_right\\*.png", cwd);
    hFind = FindFirstFileW(searchPath, &fd);
    if (hFind != INVALID_HANDLE_VALUE) {
        FindClose(hFind);
        swprintf(outPath, outSize, L"%ls\\photo_right\\%ls", cwd, fd.cFileName);
        return;
    }

    swprintf(searchPath, MAX_PATH, L"%ls\\photo_left\\*.png", cwd);
    hFind = FindFirstFileW(searchPath, &fd);
    if (hFind != INVALID_HANDLE_VALUE) {
        FindClose(hFind);
        swprintf(outPath, outSize, L"%ls\\photo_left\\%ls", cwd, fd.cFileName);
        return;
    }

    // 都找不到，返回 exe 路径的固定路径（方便提示）
    swprintf(outPath, outSize, L"%ls\\..\\photo_right\\0.png", modulePath);
}

// 获取指定文件名的图片路径（指定目录）
static BOOL GetImagePathByNameInFolder(const wchar_t* folderName, const wchar_t* fileName, wchar_t* outPath, size_t outSize) {
    wchar_t modulePath[MAX_PATH] = { 0 };
    GetModuleFileNameW(NULL, modulePath, MAX_PATH);
    wchar_t* last = wcsrchr(modulePath, L'\\');
    if (last) {
        *last = L'\0';
    }

    swprintf(outPath, outSize, L"%ls\\..\\%ls\\%ls", modulePath, folderName, fileName);
    if (FileExists(outPath)) {
        return TRUE;
    }

    wchar_t cwd[MAX_PATH] = { 0 };
    GetCurrentDirectoryW(MAX_PATH, cwd);
    swprintf(outPath, outSize, L"%ls\\%ls\\%ls", cwd, folderName, fileName);
    if (FileExists(outPath)) {
        return TRUE;
    }
    return FALSE;
}

// 更新宠物窗口大小以适应图片
static void UpdatePetSizeFromImage(Image* img) {
    if (!img) {
        return;
    }
    int w = (int)(img->GetWidth() * kPetScale);
    int h = (int)(img->GetHeight() * kPetScale);
    if (w < 1) w = 1;
    if (h < 1) h = 1;
    g_pet.width = w;
    g_pet.height = h;
    if (g_hwnd) {
        SetWindowPos(g_hwnd, NULL, g_pet.x, g_pet.y, g_pet.width, g_pet.height,
                     SWP_NOZORDER | SWP_NOACTIVATE);
    }
}

// 从路径加载图片
static Image* LoadImageFromPath(const wchar_t* path, BOOL showError) {// 从路径加载图片
    if (!FileExists(path)) {
        if (showError) {
            wchar_t msg[800] = { 0 };
            swprintf(msg, 800, L"图片文件不存在。\n路径:%ls", path);
            MessageBoxW(NULL, msg, L"LoadImage", MB_ICONERROR | MB_OK);
        }
        return NULL;
    }
    Image* img = new Image(path);
    Status status = img->GetLastStatus();
    if (status != Ok) {
        if (showError) {
            wchar_t msg[800] = { 0 };
            swprintf(msg, 800, L"图片加载失败。状态码=%d\n路径:%ls", (int)status, path);
            MessageBoxW(NULL, msg, L"LoadImage", MB_ICONERROR | MB_OK);
        }
        delete img;
        return NULL;
    }
    return img;
}

// 前置声明（用于 NextClickImage 调用）
static void SwitchToImage(Image* img);
static void ShowSettingsWindow(HWND owner);

// 加载默认图片
static BOOL LoadPetImage(void) {
    wchar_t path[MAX_PATH] = { 0 };
    GetImagePath(path, MAX_PATH);

    if (!FileExists(path)) {
        wchar_t modulePath[MAX_PATH] = { 0 };
        wchar_t cwd[MAX_PATH] = { 0 };
        GetModuleFileNameW(NULL, modulePath, MAX_PATH);
        GetCurrentDirectoryW(MAX_PATH, cwd);

        wchar_t msg[1200] = { 0 };
        swprintf(msg, 1200,
                 L"图片文件不存在。\n\n候选路径:\n%ls\n\nexe 路径:\n%ls\n\n当前工作目录:\n%ls",
                 path, modulePath, cwd);
        MessageBoxW(NULL, msg, L"LoadPetImage", MB_ICONERROR | MB_OK);
        return FALSE;
    }

    Image* img = LoadImageFromPath(path, TRUE);
    if (!img) {
        return FALSE;
    }

    g_imageDefault = img;
    g_image = g_imageDefault;
    UpdatePetSizeFromImage(g_image);
    return TRUE;
}

// 加载点击后的图片
static void LoadClickedImage(void) {
    for (int i = 0; i < kClickItemCount; ++i) {
        wchar_t path[MAX_PATH] = { 0 };
        if (GetImagePathByNameInFolder(L"photo_left", g_clickItems[i].fileName, path, MAX_PATH)) {
            g_clickItems[i].left = LoadImageFromPath(path, FALSE);
        }
        if (GetImagePathByNameInFolder(L"photo_right", g_clickItems[i].fileName, path, MAX_PATH)) {
            g_clickItems[i].right = LoadImageFromPath(path, FALSE);
        }
    }
    for (int i = 0; i < kStopItemCount; ++i) {
        wchar_t path[MAX_PATH] = { 0 };
        if (GetImagePathByNameInFolder(L"photo_left", g_stopItems[i].fileName, path, MAX_PATH)) {
            g_stopItems[i].left = LoadImageFromPath(path, FALSE);
        }
        if (GetImagePathByNameInFolder(L"photo_right", g_stopItems[i].fileName, path, MAX_PATH)) {
            g_stopItems[i].right = LoadImageFromPath(path, FALSE);
        }
    }
}

// 加载拖动/下落图片（3.png/5.png）
static void LoadDragAndFallImages(void) {
    wchar_t path[MAX_PATH] = { 0 };
    if (GetImagePathByNameInFolder(L"photo_left", L"3.png", path, MAX_PATH)) {
        g_imageDragLeft = LoadImageFromPath(path, FALSE);
    }
    if (GetImagePathByNameInFolder(L"photo_right", L"3.png", path, MAX_PATH)) {
        g_imageDragRight = LoadImageFromPath(path, FALSE);
    }
    if (GetImagePathByNameInFolder(L"photo_left", L"5.png", path, MAX_PATH)) {
        g_imageFallLeft = LoadImageFromPath(path, FALSE);
    }
    if (GetImagePathByNameInFolder(L"photo_right", L"5.png", path, MAX_PATH)) {
        g_imageFallRight = LoadImageFromPath(path, FALSE);
    }
    if (GetImagePathByNameInFolder(L"photo_left", L"2.png", path, MAX_PATH)) {
        g_imageNearGroundLeft = LoadImageFromPath(path, FALSE);
    }
    if (GetImagePathByNameInFolder(L"photo_right", L"2.png", path, MAX_PATH)) {
        g_imageNearGroundRight = LoadImageFromPath(path, FALSE);
    }
    // 加载落地图片 2.png
    if (GetImagePathByNameInFolder(L"photo_left", L"2.png", path, MAX_PATH)) {
        g_imageLandLeft = LoadImageFromPath(path, FALSE);
    }
    if (GetImagePathByNameInFolder(L"photo_right", L"2.png", path, MAX_PATH)) {
        g_imageLandRight = LoadImageFromPath(path, FALSE);
    }
    // 加载落地特殊图片 8.png
    if (GetImagePathByNameInFolder(L"photo_left", L"8.png", path, MAX_PATH)) {
        g_imageLandSpecialLeft = LoadImageFromPath(path, FALSE);
    }
    if (GetImagePathByNameInFolder(L"photo_right", L"8.png", path, MAX_PATH)) {
        g_imageLandSpecialRight = LoadImageFromPath(path, FALSE);
    }
}

// 加载行走图片（0.png）
static void LoadWalkImages(void) {
    wchar_t path[MAX_PATH] = { 0 };
    if (GetImagePathByNameInFolder(L"photo_left", L"0.png", path, MAX_PATH)) {
        g_imageWalkLeft = LoadImageFromPath(path, FALSE);
    }
    if (GetImagePathByNameInFolder(L"photo_right", L"0.png", path, MAX_PATH)) {
        g_imageWalkRight = LoadImageFromPath(path, FALSE);
    }
}

static void SwitchToImage(Image* img) {// 切换当前显示的图片
    if (!img) {
        return;
    }
    g_image = img;
    UpdatePetSizeFromImage(g_image);
}

static void ApplyWalkImageByVx(int vx) {// 根据水平速度切换到对应的行走图片
    if (vx < 0 && g_imageWalkLeft) {
        SwitchToImage(g_imageWalkLeft);
    } else if (vx > 0 && g_imageWalkRight) {
        SwitchToImage(g_imageWalkRight);
    }
}

static void ApplyDragImageByVx(int vx) {// 根据拖拽水平速度切换到对应的拖拽图片
    if (vx < 0 && g_imageDragLeft) {
        SwitchToImage(g_imageDragLeft);
    } else if (vx > 0 && g_imageDragRight) {
        SwitchToImage(g_imageDragRight);
    }
}

static void ApplyFallImageByDir(int dir) {// 根据方向切换到对应的下落图片
    if (dir < 0 && g_imageFallLeft) {
        SwitchToImage(g_imageFallLeft);
    } else if (dir > 0 && g_imageFallRight) {
        SwitchToImage(g_imageFallRight);
    } else if (g_imageFallRight) {
        SwitchToImage(g_imageFallRight);
    } else if (g_imageFallLeft) {
        SwitchToImage(g_imageFallLeft);
    }
}

static Image* PickClickImageByDirection(BOOL toLeft) {
    int total = 0;
    for (int i = 0; i < kClickItemCount; ++i) {
        Image* img = toLeft ? g_clickItems[i].left : g_clickItems[i].right;
        if (img && g_clickItems[i].weight > 0) {
            total += g_clickItems[i].weight;
        }
    }
    if (total <= 0) {
        return NULL;
    }
    int r = rand() % total;
    for (int i = 0; i < kClickItemCount; ++i) {
        Image* img = toLeft ? g_clickItems[i].left : g_clickItems[i].right;
        if (img && g_clickItems[i].weight > 0) {
            if (r < g_clickItems[i].weight) {
                return img;
            }
            r -= g_clickItems[i].weight;
        }
    }
    return NULL;
}

static Image* PickStopImageByDirection(BOOL toLeft) {
    int total = 0;
    for (int i = 0; i < kStopItemCount; ++i) {
        Image* img = toLeft ? g_stopItems[i].left : g_stopItems[i].right;
        if (img && g_stopItems[i].weight > 0) {
            total += g_stopItems[i].weight;
        }
    }
    if (total <= 0) {
        return NULL;
    }
    int r = rand() % total;
    for (int i = 0; i < kStopItemCount; ++i) {
        Image* img = toLeft ? g_stopItems[i].left : g_stopItems[i].right;
        if (img && g_stopItems[i].weight > 0) {
            if (r < g_stopItems[i].weight) {
                return img;
            }
            r -= g_stopItems[i].weight;
        }
    }
    return NULL;
}

// 选择并启动新的地面任务
static GroundTask PickNextGroundTask(void) {
    int w1 = TASK_WEIGHT_MOVE_LEFT_EDGE;
    int w2 = TASK_WEIGHT_MOVE_RIGHT_EDGE;
    int w3 = TASK_WEIGHT_MOVE_LEFT_TIME;
    int w4 = TASK_WEIGHT_MOVE_RIGHT_TIME;
    int w5 = TASK_WEIGHT_STOP_TIME;
    BOOL allowStop = (g_lastGroundTask == TASK_MOVE_LEFT_TIME || g_lastGroundTask == TASK_MOVE_RIGHT_TIME);
    if (!allowStop) {
        w5 = 0;
    }
    int total = w1 + w2 + w3 + w4 + w5;
    if (total <= 0) {
        return TASK_MOVE_LEFT_EDGE;
    }
    int r = rand() % total;
    if (r < w1) return TASK_MOVE_LEFT_EDGE;
    r -= w1;
    if (r < w2) return TASK_MOVE_RIGHT_EDGE;
    r -= w2;
    if (r < w3) return TASK_MOVE_LEFT_TIME;
    r -= w3;
    if (r < w4) return TASK_MOVE_RIGHT_TIME;
    return TASK_STOP_TIME;
}

static GroundTask PickNextGroundTaskFromLeftEdge(void) {
    int wEdge = TASK_WEIGHT_MOVE_RIGHT_EDGE;
    int wTime = TASK_WEIGHT_MOVE_RIGHT_TIME;
    int total = wEdge + wTime;
    if (total <= 0) {
        return TASK_MOVE_RIGHT_EDGE;
    }
    int r = rand() % total;
    if (r < wEdge) return TASK_MOVE_RIGHT_EDGE;
    return TASK_MOVE_RIGHT_TIME;
}

static GroundTask PickNextGroundTaskFromRightEdge(void) {
    int wEdge = TASK_WEIGHT_MOVE_LEFT_EDGE;
    int wTime = TASK_WEIGHT_MOVE_LEFT_TIME;
    int total = wEdge + wTime;
    if (total <= 0) {
        return TASK_MOVE_LEFT_EDGE;
    }
    int r = rand() % total;
    if (r < wEdge) return TASK_MOVE_LEFT_EDGE;
    return TASK_MOVE_LEFT_TIME;
}

static void StartGroundTask(GroundTask task) {
    g_groundTask = task;
    g_lastGroundTask = task;
    g_groundTaskEnd = 0;
    DWORD now = GetTickCount();
    int speed = (rand() % 3) + 1; // 1~3
    int prevVx = g_pet.vx;

    switch (task) {
        case TASK_MOVE_LEFT_EDGE:
            g_pet.vx = -speed;
            ApplyWalkImageByVx(g_pet.vx);
            break;
        case TASK_MOVE_RIGHT_EDGE:
            g_pet.vx = speed;
            ApplyWalkImageByVx(g_pet.vx);
            break;
        case TASK_MOVE_LEFT_TIME:
            g_pet.vx = -speed;
            g_groundTaskEnd = now + TASK_MOVE_TIME_MS;
            ApplyWalkImageByVx(g_pet.vx);
            break;
        case TASK_MOVE_RIGHT_TIME:
            g_pet.vx = speed;
            g_groundTaskEnd = now + TASK_MOVE_TIME_MS;
            ApplyWalkImageByVx(g_pet.vx);
            break;
        case TASK_STOP_TIME:
            g_pet.vx = 0;
            g_groundTaskEnd = now + TASK_STOP_TIME_MS;
            if ((rand() % 100) < STOP_TRIGGER_CHANCE) {
                Image* stopImage = NULL;
                if (prevVx < 0) {
                    stopImage = PickStopImageByDirection(TRUE);
                } else if (prevVx > 0) {
                    stopImage = PickStopImageByDirection(FALSE);
                } else {
                    stopImage = PickStopImageByDirection(FALSE);
                    if (!stopImage) {
                        stopImage = PickStopImageByDirection(TRUE);
                    }
                }
                if (stopImage) {
                    SwitchToImage(stopImage);
                }
            }
            break;
        default:
            break;
    }
}

static void StartNextGroundTask(void) {
    StartGroundTask(PickNextGroundTask());
}

static void StartNextGroundTaskFromLeftEdge(void) {
    StartGroundTask(PickNextGroundTaskFromLeftEdge());
}

static void StartNextGroundTaskFromRightEdge(void) {
    StartGroundTask(PickNextGroundTaskFromRightEdge());
}

// 更新暂停状态（判断暂停时间是否结束）
static void UpdatePauseState(void) {// 更新暂停状态（判断暂停时间是否结束）如果暂停时间结束，恢复正常移动
    if (!g_isPaused) {
        return;
    }
    if (GetTickCount() >= g_pauseUntil) {
        g_isPaused = FALSE;

        // 暂停结束后在地面上继续执行地面任务
        if (g_onGround) {
            StartNextGroundTask();
        } else {
            ApplyWalkImageByVx(g_pet.vx);
        }
    }
}

// 物理步进：更新位置、边界反弹
static void StepPet(HWND hwnd) {
    RECT rc;
    GetWindowRect(hwnd, &rc);

    int screenW = GetSystemMetrics(SM_CXSCREEN);
    int screenH = GetSystemMetrics(SM_CYSCREEN);

    if (g_isDragging || g_isPaused) {
        return;
    }
    g_pet.x += g_pet.vx;
    g_pet.y += g_pet.vy;
    g_movedSinceClick = TRUE;

    // 左右边界处理
    if (g_pet.x <= 0) {
        g_pet.x = 0;
        if (g_pet.vx < 0) {
            if (g_onGround && g_groundTask == TASK_MOVE_LEFT_EDGE) {
                g_pet.vx = 0;
            } else {
                g_pet.vx = -g_pet.vx;
                ApplyWalkImageByVx(g_pet.vx);
            }
        }
    } else if (g_pet.x + g_pet.width >= screenW) {
        g_pet.x = screenW - g_pet.width;
        if (g_pet.vx > 0) {
            if (g_onGround && g_groundTask == TASK_MOVE_RIGHT_EDGE) {
                g_pet.vx = 0;
            } else {
                g_pet.vx = -g_pet.vx;
                ApplyWalkImageByVx(g_pet.vx);
            }
        }
    }

    // 顶部边界
    if (g_pet.y <= 0) {
        g_pet.y = 0;
        if (g_pet.vy < 0) {
            g_pet.vy = -g_pet.vy;
        }
    }

    // 底部落地
    if (g_pet.y + g_pet.height >= screenH) {
        g_pet.y = screenH - g_pet.height;
        if (g_pet.vy > 0) {
            g_pet.vy = 0;
            g_onGround = TRUE;
            g_useLowFallImage = FALSE;
            g_useWeightedFallImage = FALSE;
            g_weightedFallChosen = FALSE;
            g_weightedFallUseSpecial = FALSE;
            
            // 添加短暂延迟后再开始水平移动，让用户能看到向下移动的效果
            if (!g_hasLanded) {
                g_hasLanded = TRUE;
                // 短暂停留（约1秒）
                g_isPaused = TRUE;
                g_pauseUntil = GetTickCount() + 1000;
            } else {
                // 非首次落地：直接向左或向右移动，避免触发停止任务
                if ((rand() % 2) == 0) {
                    StartGroundTask(TASK_MOVE_LEFT_TIME);
                } else {
                    StartGroundTask(TASK_MOVE_RIGHT_TIME);
                }
            }
        }
    } else {
        g_onGround = FALSE;
        // 在下落过程中保持显示下落图片
        if (g_pet.vy > 0) {
            int dir = g_lastDragDir;
            if (dir == 0) {
                dir = (g_pet.vx < 0) ? -1 : (g_pet.vx > 0 ? 1 : 0);
            }
            if (g_useWeightedFallImage && g_pet.y > DRAG_HEIGHT_THRESHOLD) {
                if (!g_weightedFallChosen) {
                    int totalWeight = g_landWeight2 + g_landWeight8;
                    if (totalWeight > 0) {
                        int random = rand() % totalWeight;
                        g_weightedFallUseSpecial = (random >= g_landWeight2);
                    } else {
                        g_weightedFallUseSpecial = FALSE;
                    }
                    g_weightedFallChosen = TRUE;
                }
                if (g_weightedFallUseSpecial) {
                    if (dir < 0 && g_imageLandSpecialLeft) {
                        SwitchToImage(g_imageLandSpecialLeft);
                    } else if (dir > 0 && g_imageLandSpecialRight) {
                        SwitchToImage(g_imageLandSpecialRight);
                    } else if (g_imageLandSpecialRight) {
                        SwitchToImage(g_imageLandSpecialRight);
                    } else if (g_imageLandSpecialLeft) {
                        SwitchToImage(g_imageLandSpecialLeft);
                    } else {
                        ApplyFallImageByDir(dir);
                    }
                } else {
                    if (dir < 0 && g_imageLandLeft) {
                        SwitchToImage(g_imageLandLeft);
                    } else if (dir > 0 && g_imageLandRight) {
                        SwitchToImage(g_imageLandRight);
                    } else if (g_imageLandRight) {
                        SwitchToImage(g_imageLandRight);
                    } else if (g_imageLandLeft) {
                        SwitchToImage(g_imageLandLeft);
                    } else {
                        ApplyFallImageByDir(dir);
                    }
                }
            } else if (g_useLowFallImage) {
                if (dir < 0 && g_imageNearGroundLeft) {
                    SwitchToImage(g_imageNearGroundLeft);
                } else if (dir > 0 && g_imageNearGroundRight) {
                    SwitchToImage(g_imageNearGroundRight);
                } else if (g_imageNearGroundRight) {
                    SwitchToImage(g_imageNearGroundRight);
                } else if (g_imageNearGroundLeft) {
                    SwitchToImage(g_imageNearGroundLeft);
                } else {
                    ApplyFallImageByDir(dir);
                }
            } else {
                ApplyFallImageByDir(dir);
            }
        }
    }

    // 地面任务执行与切换
    if (g_onGround && !g_isPaused && !g_isDragging) {
        if (g_groundTask == TASK_NONE) {
            StartNextGroundTask();
        }

        if (g_groundTask == TASK_MOVE_LEFT_EDGE) {
            if (g_pet.x <= 0) {
                StartNextGroundTaskFromLeftEdge();
            }
        } else if (g_groundTask == TASK_MOVE_RIGHT_EDGE) {
            if (g_pet.x + g_pet.width >= screenW) {
                StartNextGroundTaskFromRightEdge();
            }
        } else if (g_groundTask == TASK_MOVE_LEFT_TIME || g_groundTask == TASK_MOVE_RIGHT_TIME || g_groundTask == TASK_STOP_TIME) {
            if (g_groundTaskEnd != 0 && GetTickCount() >= g_groundTaskEnd) {
                StartNextGroundTask();
            }
        }
    }
}

// 渲染函数：使用分层窗口绘制 PNG
static void RenderPet(HWND hwnd) {
    HDC screenDC = GetDC(NULL);
    HDC memDC = CreateCompatibleDC(screenDC);

    BITMAPINFO bmi = { 0 };
    bmi.bmiHeader.biSize = sizeof(BITMAPINFOHEADER);
    bmi.bmiHeader.biWidth = g_pet.width;
    bmi.bmiHeader.biHeight = -g_pet.height; // 负值表示自上而下
    bmi.bmiHeader.biPlanes = 1;
    bmi.bmiHeader.biBitCount = 32;
    bmi.bmiHeader.biCompression = BI_RGB;

    void* bits = NULL;
    HBITMAP dib = CreateDIBSection(screenDC, &bmi, DIB_RGB_COLORS, &bits, NULL, 0);
    HBITMAP oldBmp = (HBITMAP)SelectObject(memDC, dib);

    Graphics graphics(memDC);
    graphics.SetCompositingMode(CompositingModeSourceCopy);
    graphics.Clear(Color(0, 0, 0, 0));

    if (g_image) {
        graphics.SetCompositingMode(CompositingModeSourceOver);
        graphics.DrawImage(g_image, 0, 0, g_pet.width, g_pet.height);
    } else {
        // 兜底：没加载图片时画一个圆
        HBRUSH brush = CreateSolidBrush(RGB(255, 200, 0));
        HBRUSH oldBrush = (HBRUSH)SelectObject(memDC, brush);
        HPEN pen = CreatePen(PS_SOLID, 2, RGB(120, 80, 0));
        HPEN oldPen = (HPEN)SelectObject(memDC, pen);
        Ellipse(memDC, 5, 5, g_pet.width - 5, g_pet.height - 5);
        SelectObject(memDC, oldBrush);
        SelectObject(memDC, oldPen);
        DeleteObject(brush);
        DeleteObject(pen);
    }
/*     
    // 绘制实时坐标信息
    {
        wchar_t coordText[50] = { 0 };
        swprintf(coordText, 50, L"(%d, %d)", g_pet.x, g_pet.y);
        
        // 创建字体和画刷
        Font font(L"Arial", 10, FontStyleRegular, UnitPoint);
        SolidBrush brush(Color(255, 255, 255)); // 白色文字
        
        // 设置文字绘制格式
        StringFormat format;
        format.SetAlignment(StringAlignmentNear);
        format.SetLineAlignment(StringAlignmentNear);
        
        // 绘制文字（在图片左上角）
        graphics.DrawString(coordText, -1, &font, PointF(5.0f, 5.0f), &format, &brush);
    } */

    POINT ptPos = { g_pet.x, g_pet.y };
    SIZE sizeWnd = { g_pet.width, g_pet.height };
    POINT ptSrc = { 0, 0 };
    BLENDFUNCTION bf = { AC_SRC_OVER, 0, 255, AC_SRC_ALPHA };

    UpdateLayeredWindow(hwnd, screenDC, &ptPos, &sizeWnd, memDC, &ptSrc, 0, &bf, ULW_ALPHA);

    SelectObject(memDC, oldBmp);
    DeleteObject(dib);
    DeleteDC(memDC);
    ReleaseDC(NULL, screenDC);
}

// 窗口过程：处理消息
static LRESULT CALLBACK WndProc(HWND hwnd, UINT msg, WPARAM wParam, LPARAM lParam) {
    switch (msg) {
        case WM_CREATE:
            SetTimer(hwnd, 1, 10, NULL);//改帧率，第三位越小，帧率越高
            return 0;

        case WM_TIMER:
            UpdatePauseState();
            StepPet(hwnd);
            RenderPet(hwnd);
            return 0;

        case WM_LBUTTONDOWN:
        {
            if (g_isPausedForChat) {
                return 0;
            }
            // 记录按下的初始位置，暂不触发拖拽
            g_beganDrag = FALSE;
            GetCursorPos(&g_startPos);
            SetCapture(hwnd); // 捕获鼠标，防止移出窗口丢失消息
            return 0;
        }

        case WM_MOUSEMOVE:
        {
            if (g_isPausedForChat) {
                return 0;
            }
            // 如果鼠标左键按住，且尚未判定为拖拽
            if ((wParam & MK_LBUTTON) && !g_beganDrag) {
                POINT curPos;
                GetCursorPos(&curPos);
                
                int dx = abs(curPos.x - g_startPos.x);
                int dy = abs(curPos.y - g_startPos.y);

                // 阈值设为 5 像素，超过则视为拖拽
                if (dx > 5 || dy > 5) {
                    g_beganDrag = TRUE;
                    ReleaseCapture();
                    
                    // 启动系统标准的窗口拖拽
                    SendMessage(hwnd, WM_NCLBUTTONDOWN, HTCAPTION, 0);
                }
            }
            return 0;
        }

        case WM_LBUTTONUP:
        {
            if (g_isPausedForChat) {
                ReleaseCapture();
                g_beganDrag = FALSE;
                return 0;
            }
            ReleaseCapture(); // 释放鼠标捕获

            if (g_ignoreClick) {
                g_ignoreClick = FALSE;
                g_beganDrag = FALSE;
                return 0;
            }

            if (!g_beganDrag) {
                // 判定为点击：根据方向选择文件夹，并按权重随机图片
                Image* targetImage = NULL;
                if (g_pet.vx < 0) {
                    targetImage = PickClickImageByDirection(TRUE);
                } else if (g_pet.vx > 0) {
                    targetImage = PickClickImageByDirection(FALSE);
                } else {
                    // 静止时优先用右侧文件夹，右侧无可用再用左侧
                    targetImage = PickClickImageByDirection(FALSE);
                    if (!targetImage) {
                        targetImage = PickClickImageByDirection(TRUE);
                    }
                }
                if (!targetImage) {
                    // 兜底：使用当前默认图片
                    targetImage = g_imageDefault ? g_imageDefault : g_image;
                }
                
                if (targetImage) {
                    SwitchToImage(targetImage);
                    g_isPaused = TRUE;
                    g_pauseUntil = GetTickCount() + BEING_TIME;
                }
            } else {
                // 拖拽结束，逻辑由 WM_EXITSIZEMOVE 处理
                // 这里仅重置标志位防止残留
                g_beganDrag = FALSE;
            }
            return 0;
        }

        case WM_RBUTTONUP:
        {
            ShowSettingsWindow(hwnd);
            return 0;
        }

        case WM_LBUTTONDBLCLK:
        {
            g_ignoreClick = TRUE;
            if (!g_isPausedForChat) {
                g_prevPausedChat = g_isPaused;
                g_prevPauseUntilChat = g_pauseUntil;
                g_prevImageChat = g_image;
            }
            g_isPaused = TRUE;
            g_pauseUntil = 0xFFFFFFFF;
            g_isPausedForChat = TRUE;
            g_pet.vx = 0;
            g_pet.vy = 0;
            {
                Image* img1 = PickStopImage1ByDirection();
                if (!img1) {
                    img1 = g_imageDefault ? g_imageDefault : g_image;
                }
                if (img1) {
                    SwitchToImage(img1);
                }
            }
            ShowChatWindow(hwnd);
            return 0;
        }

        case WM_ENTERSIZEMOVE:// 窗口开始拖拽
            if (g_isPausedForChat) {
                return 0;
            }
            g_isDragging = TRUE;
            g_isPaused = FALSE;
            g_pauseUntil = 0;
            GetCursorPos(&g_dragPrevPos);
            g_dragPrevTick = GetTickCount();
            g_dragVx = 0;
            g_dragVy = 0;
            {
                RECT rc;
                GetWindowRect(hwnd, &rc);// 记录拖拽开始时窗口位置
                g_dragPrevWndX = rc.left;
                g_dragStartWndY = rc.top;
            }
            g_lastDragDir = 0;
            ApplyDragImageByVx(g_pet.vx);// 切换到拖拽图片
            return 0;

        case WM_EXITSIZEMOVE:
        {
            RECT rc;
            GetWindowRect(hwnd, &rc);
            g_pet.x = rc.left;
            g_pet.y = rc.top;
            g_movedSinceClick = TRUE;
            g_isPaused = FALSE;
            g_pauseUntil = 0;
            g_onGround = FALSE;
            // 释放后不再按鼠标方向推出；未到左右边界则垂直下落
            int screenW = GetSystemMetrics(SM_CXSCREEN);
            if (g_pet.x > 0 && g_pet.x + g_pet.width < screenW) {
                g_pet.vx = 0;
                if (g_pet.vy <= 0) {
                    g_pet.vy = 2; // 保证有向下速度且不太快
                }
            }
            {
                int releaseY = rc.top;
                g_useWeightedFallImage = (releaseY >= DRAG_HEIGHT_THRESHOLD);
                g_weightedFallChosen = FALSE;
                g_weightedFallUseSpecial = FALSE;
                if (releaseY < DRAG_HEIGHT_THRESHOLD) {
                    g_useLowFallImage = FALSE;
                    ApplyFallImageByDir(g_lastDragDir);// 低于阈值：使用普通下落图片
                } else {
                    g_useLowFallImage = TRUE;
                }
            }
            g_isDragging = FALSE;
            g_beganDrag = FALSE; // 确保拖拽结束后重置
            return 0;
        }

        case WM_MOVE:
        {
            RECT rc;
            GetWindowRect(hwnd, &rc);
            g_pet.x = rc.left;
            g_pet.y = rc.top;
            int wndDx = g_pet.x - g_dragPrevWndX;
            g_dragPrevWndX = g_pet.x;
            
            // 拖动过程中立即重绘窗口，解决闪屏问题
            RenderPet(hwnd);
            
            if (g_isDragging) {
                POINT curPos;
                GetCursorPos(&curPos);
                DWORD now = GetTickCount();
                DWORD dt = now - g_dragPrevTick;
                if (dt == 0) {
                    dt = 1;
                }
                int dx = curPos.x - g_dragPrevPos.x;
                int dy = curPos.y - g_dragPrevPos.y;
                // 把鼠标速度换算为每帧速度（16ms）
                double vx = (double)dx * 16.0 / (double)dt;
                double vy = (double)dy * 16.0 / (double)dt;
                // 限制速度范围
                if (vx > 12.0) vx = 12.0;
                if (vx < -12.0) vx = -12.0;
                if (vy > 12.0) vy = 12.0;
                if (vy < -12.0) vy = -12.0;
                g_dragVx = (int)vx;
                g_dragVy = (int)vy;
                g_dragPrevPos = curPos;
                g_dragPrevTick = now;
                // 确保正确设置拖动方向，即使是垂直拖动也能识别水平分量
                if (wndDx != 0) {
                    g_lastDragDir = (wndDx < 0) ? -1 : 1;
                    ApplyDragImageByVx(wndDx);
                } else if (dx != 0) {
                    g_lastDragDir = (dx < 0) ? -1 : 1;
                    ApplyDragImageByVx(dx);
                } else {
                    ApplyDragImageByVx(0);
                }
            }
            return 0;
        }

        case WM_PAINT: {
            PAINTSTRUCT ps;
            BeginPaint(hwnd, &ps);
            EndPaint(hwnd, &ps);
            return 0;
        }

        case WM_DESTROY:
            KillTimer(hwnd, 1);
            PostQuitMessage(0);
            return 0;

        default:
            return DefWindowProc(hwnd, msg, wParam, lParam);
    }
    return 0;
}

int WINAPI WinMain(HINSTANCE hInstance, HINSTANCE hPrevInstance, LPSTR lpCmdLine, int nCmdShow) {
    (void)hPrevInstance;
    (void)lpCmdLine;

    srand((unsigned int)GetTickCount());

    HRESULT hrCoInit = CoInitializeEx(NULL, COINIT_APARTMENTTHREADED);
    BOOL coInited = SUCCEEDED(hrCoInit);

    // 初始化 GDI+
    GdiplusStartupInput gdiplusStartupInput = { 0 };
    gdiplusStartupInput.GdiplusVersion = 1;
    if (GdiplusStartup(&g_gdiplusToken, &gdiplusStartupInput, NULL) != Ok) {
        return 0;
    }

    ApplyChatColor(FALSE);

    // 加载图片
    LoadPetImage();
    LoadClickedImage();
    LoadDragAndFallImages();
    LoadWalkImages();

    // 注册窗口类
    WNDCLASSW wc = { 0 };
    wc.lpfnWndProc = WndProc;
    wc.style = CS_DBLCLKS;
    wc.hInstance = hInstance;
    wc.lpszClassName = kClassName;
    wc.hCursor = LoadCursor(NULL, IDC_ARROW);
    wc.hbrBackground = (HBRUSH)GetStockObject(BLACK_BRUSH);
    RegisterClassW(&wc);

    // 创建窗口
    HWND hwnd = CreateWindowExW(
        WS_EX_LAYERED | WS_EX_TOPMOST | WS_EX_TOOLWINDOW,
        kClassName,
        L"Desktop Pet",
        WS_POPUP,
        g_pet.x, g_pet.y, g_pet.width, g_pet.height,
        NULL, NULL, hInstance, NULL
    );

    if (!hwnd) {
        return 0;
    }

    g_hwnd = hwnd;
    ShowWindow(hwnd, nCmdShow);
    UpdateWindow(hwnd);
    RenderPet(hwnd);

    // 消息循环
    MSG msg;
    while (GetMessage(&msg, NULL, 0, 0)) {
        TranslateMessage(&msg);
        DispatchMessage(&msg);
    }

    // 清理资源
    for (int i = 0; i < kClickItemCount; ++i) {
        if (g_clickItems[i].left) {
            delete g_clickItems[i].left;
            g_clickItems[i].left = NULL;
        }
        if (g_clickItems[i].right) {
            delete g_clickItems[i].right;
            g_clickItems[i].right = NULL;
        }
    }
    if (g_imageDragLeft) {
        delete g_imageDragLeft;
        g_imageDragLeft = NULL;
    }
    if (g_imageDragRight) {
        delete g_imageDragRight;
        g_imageDragRight = NULL;
    }
    if (g_imageFallLeft) {
        delete g_imageFallLeft;
        g_imageFallLeft = NULL;
    }
    if (g_imageFallRight) {
        delete g_imageFallRight;
        g_imageFallRight = NULL;
    }
    if (g_imageLandLeft) {
        delete g_imageLandLeft;
        g_imageLandLeft = NULL;
    }
    if (g_imageLandRight) {
        delete g_imageLandRight;
        g_imageLandRight = NULL;
    }
    if (g_imageLandSpecialLeft) {
        delete g_imageLandSpecialLeft;
        g_imageLandSpecialLeft = NULL;
    }
    if (g_imageLandSpecialRight) {
        delete g_imageLandSpecialRight;
        g_imageLandSpecialRight = NULL;
    }
    if (g_imageLandLeft) {
        delete g_imageLandLeft;
        g_imageLandLeft = NULL;
    }
    if (g_imageLandRight) {
        delete g_imageLandRight;
        g_imageLandRight = NULL;
    }
    if (g_imageLandSpecialLeft) {
        delete g_imageLandSpecialLeft;
        g_imageLandSpecialLeft = NULL;
    }
    if (g_imageLandSpecialRight) {
        delete g_imageLandSpecialRight;
        g_imageLandSpecialRight = NULL;
    }
    if (g_imageLandLeft) {
        delete g_imageLandLeft;
        g_imageLandLeft = NULL;
    }
    if (g_imageLandRight) {
        delete g_imageLandRight;
        g_imageLandRight = NULL;
    }
    if (g_imageLandSpecialLeft) {
        delete g_imageLandSpecialLeft;
        g_imageLandSpecialLeft = NULL;
    }
    if (g_imageLandSpecialRight) {
        delete g_imageLandSpecialRight;
        g_imageLandSpecialRight = NULL;
    }
    if (g_imageWalkLeft) {
        delete g_imageWalkLeft;
        g_imageWalkLeft = NULL;
    }
    if (g_imageWalkRight) {
        delete g_imageWalkRight;
        g_imageWalkRight = NULL;
    }
    if (g_imageDefault) {
        delete g_imageDefault;
        g_imageDefault = NULL;
    }
    if (g_chatBgBrush) {
        DeleteObject(g_chatBgBrush);
        g_chatBgBrush = NULL;
    }
    if (g_webview) {
        g_webview->Release();
        g_webview = NULL;
    }
    if (g_webviewController) {
        g_webviewController->Release();
        g_webviewController = NULL;
    }
    if (g_webviewEnv) {
        g_webviewEnv->Release();
        g_webviewEnv = NULL;
    }
    g_image = NULL;
    if (g_gdiplusToken) {
        GdiplusShutdown(g_gdiplusToken);
        g_gdiplusToken = 0;
    }
    if (coInited) {
        CoUninitialize();
    }
    return 0;
}

static ClickImageItem* FindClickItemByName(const wchar_t* name, ClickImageItem* items, int count) {
    for (int i = 0; i < count; ++i) {
        if (items[i].fileName && wcscmp(items[i].fileName, name) == 0) {
            return &items[i];
        }
    }
    return NULL;
}

static void SetEditInt(HWND edit, int value) {
    wchar_t buf[32];
    swprintf(buf, 32, L"%d", value);
    SetWindowTextW(edit, buf);
}

static int GetEditInt(HWND edit, int fallback) {
    wchar_t buf[32] = { 0 };
    GetWindowTextW(edit, buf, 31);
    int v = _wtoi(buf);
    if (v < 0) v = 0;
    return (buf[0] == L'\0') ? fallback : v;
}

static BOOL PickChatColor(HWND owner, BOOL isText) {
    if (!g_chatCustomColorsInit) {
        for (int i = 0; i < 16; ++i) {
            g_chatCustomColors[i] = RGB(255, 255, 255);
        }
        g_chatCustomColors[0] = g_chatBgColor;
        g_chatCustomColors[1] = g_chatTextColor;
        g_chatCustomColorsInit = TRUE;
    }

    HMODULE comdlg = LoadLibraryW(L"comdlg32.dll");
    if (!comdlg) {
        return FALSE;
    }
    typedef BOOL (WINAPI *PFN_ChooseColorW)(LPCHOOSECOLORW);
    PFN_ChooseColorW pChooseColorW = (PFN_ChooseColorW)GetProcAddress(comdlg, "ChooseColorW");
    if (!pChooseColorW) {
        FreeLibrary(comdlg);
        return FALSE;
    }

    CHOOSECOLORW cc = { 0 };
    cc.lStructSize = sizeof(cc);
    cc.hwndOwner = owner;
    cc.rgbResult = isText ? g_chatTextColor : g_chatBgColor;
    cc.lpCustColors = g_chatCustomColors;
    cc.Flags = CC_FULLOPEN | CC_RGBINIT;

    BOOL ok = pChooseColorW(&cc);
    FreeLibrary(comdlg);
    if (ok) {
        if (isText) {
            g_chatTextColor = cc.rgbResult;
        } else {
            g_chatBgColor = cc.rgbResult;
        }
        ApplyChatColor(TRUE);
        return TRUE;
    }
    return FALSE;
}

static void ApplySettingsFromUI(void) {
    ClickImageItem* item = NULL;

    item = FindClickItemByName(L"6.png", g_clickItems, kClickItemCount);
    if (item) item->weight = GetEditInt(g_editClick6, item->weight);
    item = FindClickItemByName(L"7.png", g_clickItems, kClickItemCount);
    if (item) item->weight = GetEditInt(g_editClick7, item->weight);

    item = FindClickItemByName(L"1.png", g_stopItems, kStopItemCount);
    if (item) item->weight = GetEditInt(g_editStop1, item->weight);
    item = FindClickItemByName(L"4.png", g_stopItems, kStopItemCount);
    if (item) item->weight = GetEditInt(g_editStop4, item->weight);

    g_landWeight2 = GetEditInt(g_editLand2, g_landWeight2);
    g_landWeight8 = GetEditInt(g_editLand8, g_landWeight8);

    if (g_landWeight2 < 0) g_landWeight2 = 0;
    if (g_landWeight8 < 0) g_landWeight8 = 0;

    item = FindClickItemByName(L"6.png", g_clickItems, kClickItemCount);
    SetEditInt(g_editClick6, item ? item->weight : 0);
    item = FindClickItemByName(L"7.png", g_clickItems, kClickItemCount);
    SetEditInt(g_editClick7, item ? item->weight : 0);

    item = FindClickItemByName(L"1.png", g_stopItems, kStopItemCount);
    SetEditInt(g_editStop1, item ? item->weight : 0);
    item = FindClickItemByName(L"4.png", g_stopItems, kStopItemCount);
    SetEditInt(g_editStop4, item ? item->weight : 0);

    SetEditInt(g_editLand2, g_landWeight2);
    SetEditInt(g_editLand8, g_landWeight8);
}

static LRESULT CALLBACK SettingsWndProc(HWND hwnd, UINT msg, WPARAM wParam, LPARAM lParam) {
  // 处理窗口消息
    switch (msg) {
        case WM_CREATE:
        {
            int xLabel = 10;
            int xEdit = 140;
            int y = 10;
            int rowH = 24;
            int editW = 120;
            CreateWindowW(L"STATIC", L"setting", WS_CHILD | WS_VISIBLE,
                          xLabel, y, 120, rowH, hwnd, NULL, NULL, NULL);
            y += rowH;
            CreateWindowW(L"STATIC", L"点击权重", WS_CHILD | WS_VISIBLE,
                          xLabel, y, 120, rowH, hwnd, NULL, NULL, NULL);
            y += rowH;
            CreateWindowW(L"STATIC", L"点击 6.png", WS_CHILD | WS_VISIBLE,
                          xLabel, y, 120, rowH, hwnd, NULL, NULL, NULL);
            g_editClick6 = CreateWindowW(L"EDIT", L"", WS_CHILD | WS_VISIBLE | WS_BORDER | ES_NUMBER,
                                          xEdit, y, editW, rowH, hwnd, NULL, NULL, NULL);
            y += rowH;
            CreateWindowW(L"STATIC", L"点击 7.png", WS_CHILD | WS_VISIBLE,
                          xLabel, y, 120, rowH, hwnd, NULL, NULL, NULL);
            g_editClick7 = CreateWindowW(L"EDIT", L"", WS_CHILD | WS_VISIBLE | WS_BORDER | ES_NUMBER,
                                          xEdit, y, editW, rowH, hwnd, NULL, NULL, NULL);
            y += rowH + 6;

            CreateWindowW(L"STATIC", L"停止权重", WS_CHILD | WS_VISIBLE,
                          xLabel, y, 120, rowH, hwnd, NULL, NULL, NULL);
            y += rowH;
            CreateWindowW(L"STATIC", L"停止 1.png", WS_CHILD | WS_VISIBLE,
                          xLabel, y, 120, rowH, hwnd, NULL, NULL, NULL);
            g_editStop1 = CreateWindowW(L"EDIT", L"", WS_CHILD | WS_VISIBLE | WS_BORDER | ES_NUMBER,
                                         xEdit, y, editW, rowH, hwnd, NULL, NULL, NULL);
            y += rowH;
            CreateWindowW(L"STATIC", L"停止 4.png", WS_CHILD | WS_VISIBLE,
                          xLabel, y, 120, rowH, hwnd, NULL, NULL, NULL);
            g_editStop4 = CreateWindowW(L"EDIT", L"", WS_CHILD | WS_VISIBLE | WS_BORDER | ES_NUMBER,
                                         xEdit, y, editW, rowH, hwnd, NULL, NULL, NULL);
            y += rowH + 6;

            CreateWindowW(L"STATIC", L"下落权重", WS_CHILD | WS_VISIBLE,
                          xLabel, y, 120, rowH, hwnd, NULL, NULL, NULL);
            y += rowH;
            CreateWindowW(L"STATIC", L"下落 2.png", WS_CHILD | WS_VISIBLE,
                          xLabel, y, 120, rowH, hwnd, NULL, NULL, NULL);
            g_editLand2 = CreateWindowW(L"EDIT", L"", WS_CHILD | WS_VISIBLE | WS_BORDER | ES_NUMBER,
                                         xEdit, y, editW, rowH, hwnd, NULL, NULL, NULL);
            y += rowH;
            CreateWindowW(L"STATIC", L"下落 8.png", WS_CHILD | WS_VISIBLE,
                          xLabel, y, 120, rowH, hwnd, NULL, NULL, NULL);
            g_editLand8 = CreateWindowW(L"EDIT", L"", WS_CHILD | WS_VISIBLE | WS_BORDER | ES_NUMBER,
                                         xEdit, y, editW, rowH, hwnd, NULL, NULL, NULL);
            y += rowH + 8;

            CreateWindowW(L"STATIC", L"聊天颜色", WS_CHILD | WS_VISIBLE,
                          xLabel, y, 120, rowH, hwnd, NULL, NULL, NULL);
            CreateWindowW(L"BUTTON", L"选择背景色", WS_CHILD | WS_VISIBLE,
                          xEdit, y, editW, rowH, hwnd, (HMENU)IDC_CHAT_COLOR_BG, NULL, NULL);
            y += rowH;
            CreateWindowW(L"BUTTON", L"选择文字色", WS_CHILD | WS_VISIBLE,
                          xEdit, y, editW, rowH, hwnd, (HMENU)IDC_CHAT_COLOR_TEXT, NULL, NULL);
            y += rowH + 8;

            CreateWindowW(L"BUTTON", L"应用", WS_CHILD | WS_VISIBLE,
                          xLabel, y, 80, 26, hwnd, (HMENU)1001, NULL, NULL);
            CreateWindowW(L"BUTTON", L"关闭", WS_CHILD | WS_VISIBLE,
                          xLabel + 100, y, 80, 26, hwnd, (HMENU)1002, NULL, NULL);

            // 初始化数值
            SetEditInt(g_editClick6, g_clickItems[0].weight);
            SetEditInt(g_editClick7, g_clickItems[1].weight);
            SetEditInt(g_editStop1, g_stopItems[0].weight);
            SetEditInt(g_editStop4, g_stopItems[1].weight);
            SetEditInt(g_editLand2, g_landWeight2);
            SetEditInt(g_editLand8, g_landWeight8);
            return 0;
        }
        case WM_COMMAND:
        {
            int id = LOWORD(wParam);
            if (id == 1001) {
                ApplySettingsFromUI();
                return 0;
            } else if (id == 1002) {
                DestroyWindow(hwnd);
                return 0;
            } else if (id == IDC_CHAT_COLOR_BG) {
                PickChatColor(hwnd, FALSE);
                return 0;
            } else if (id == IDC_CHAT_COLOR_TEXT) {
                PickChatColor(hwnd, TRUE);
                return 0;
            }
            break;
        }
        case WM_CLOSE:
            DestroyWindow(hwnd);
            return 0;
        case WM_DESTROY:
            g_settingsHwnd = NULL;
            if (g_isPausedForSettings) {
                g_isPausedForSettings = FALSE;
                g_isPaused = g_prevPaused;
                g_pauseUntil = g_prevPauseUntil;
                if (g_prevImage) {
                    SwitchToImage(g_prevImage);
                } else {
                    SwitchToImage(g_imageDefault ? g_imageDefault : g_image);
                }
            }
            return 0;
        default:
            break;
    }
    return DefWindowProc(hwnd, msg, wParam, lParam);
}

static void ShowSettingsWindow(HWND owner) {
    if (g_settingsHwnd) {
        if (!g_isPausedForSettings) {
            g_prevPaused = g_isPaused;
            g_prevPauseUntil = g_pauseUntil;
            g_prevImage = g_image;
            g_isPaused = TRUE;
            g_pauseUntil = 0xFFFFFFFF;
            g_isPausedForSettings = TRUE;
        }
        {
            Image* img1 = PickStopImage1ByDirection();
            if (!img1) {
                img1 = g_imageDefault ? g_imageDefault : g_image;
            }
            if (img1) {
                SwitchToImage(img1);
            }
        }
        ShowWindow(g_settingsHwnd, SW_SHOWNORMAL);
        SetForegroundWindow(g_settingsHwnd);
        return;
    }
    if (!g_settingsClassRegistered) {
        WNDCLASSW wc = { 0 };
        wc.lpfnWndProc = SettingsWndProc;
        wc.hInstance = GetModuleHandleW(NULL);
        wc.lpszClassName = L"PetSettingsWindow";
        wc.hCursor = LoadCursor(NULL, IDC_ARROW);
        wc.hbrBackground = (HBRUSH)(COLOR_WINDOW + 1);
        RegisterClassW(&wc);
        g_settingsClassRegistered = TRUE;
    }
    g_settingsHwnd = CreateWindowExW(
        WS_EX_TOOLWINDOW,
        L"PetSettingsWindow",
        L" ",
        WS_OVERLAPPED | WS_CAPTION | WS_SYSMENU,
        CW_USEDEFAULT, CW_USEDEFAULT, 300, 380,
        owner, NULL, GetModuleHandleW(NULL), NULL
    );
    if (g_settingsHwnd) {
        g_prevPaused = g_isPaused;
        g_prevPauseUntil = g_pauseUntil;
        g_prevImage = g_image;
        g_isPaused = TRUE;
        g_pauseUntil = 0xFFFFFFFF;
        g_isPausedForSettings = TRUE;
        {
            Image* img1 = PickStopImage1ByDirection();
            if (!img1) {
                img1 = g_imageDefault ? g_imageDefault : g_image;
            }
            if (img1) {
                SwitchToImage(img1);
            }
        }
        ShowWindow(g_settingsHwnd, SW_SHOWNORMAL);
        UpdateWindow(g_settingsHwnd);
    }
}

static Image* PickStopImage1ByDirection(void) {
    ClickImageItem* item = FindClickItemByName(L"1.png", g_stopItems, kStopItemCount);
    if (!item) {
        return NULL;
    }
    int dir = (g_pet.vx < 0) ? -1 : (g_pet.vx > 0 ? 1 : 0);
    if (dir < 0 && item->left) return item->left;
    if (dir > 0 && item->right) return item->right;
    if (item->right) return item->right;
    return item->left;
}