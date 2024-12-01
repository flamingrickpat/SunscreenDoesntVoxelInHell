#include <windows.h>
#include <d3d11.h>
#include <d3dcompiler.h>
#include <DirectXMath.h>
#include <string>
#include <vector>
#include <stdexcept>

#pragma comment(lib, "d3d11.lib")
#pragma comment(lib, "d3dcompiler.lib")

using namespace DirectX;

#include <cstdlib>  // For rand()
#include <cmath>    // For sin, cos, etc.



// Vertex structure
struct Vertex {
    XMFLOAT3 position;
    XMFLOAT4 color;
};

std::vector<Vertex> vecVerts;
std::vector<UINT> vecIndices;
int indexCnt = 0;


void addCube(float x, float y, float z) {
    auto start = vecVerts.size();


    Vertex tmp[] = {
        // Front face
        {{-0.5f, -0.5f, -0.5f}, {0.5f, 0.0f, 0.0f, 0.5f}}, // Bottom-left
        {{0.5f, -0.5f, -0.5f}, {0.0f, 0.5f, 0.0f, 0.5f}},  // Bottom-right
        {{-0.5f, 0.5f, -0.5f}, {0.0f, 0.0f, 0.5f, 0.5f}},  // Top-left
        {{0.5f, 0.5f, -0.5f}, {0.5f, 0.5f, 0.0f, 0.5f}},   // Top-right

        // Back face
        {{-0.5f, -0.5f, 0.5f}, {0.5f, 0.0f, 0.5f, 0.5f}},  // Bottom-left
        {{0.5f, -0.5f, 0.5f}, {0.0f, 0.5f, 0.5f, 0.5f}},   // Bottom-right
        {{-0.5f, 0.5f, 0.5f}, {0.5f, 0.5f, 0.5f, 0.5f}},   // Top-left
        {{0.5f, 0.5f, 0.5f}, {0.0f, 0.0f, 0.0f, 0.5f}},    // Top-right
    };

    for (int i = 0; i < 8; i++) {
        tmp[i].position.x += x;
        tmp[i].position.y += y;
        tmp[i].position.z += z;

        vecVerts.push_back(tmp[i]);
    }


    vecIndices.push_back(start + 0);
    vecIndices.push_back(start + 2);
    vecIndices.push_back(start + 1);
    vecIndices.push_back(start + 2);
    vecIndices.push_back(start + 3);
    vecIndices.push_back(start + 1);
    vecIndices.push_back(start + 4);
    vecIndices.push_back(start + 5);
    vecIndices.push_back(start + 6);
    vecIndices.push_back(start + 5);
    vecIndices.push_back(start + 7);
    vecIndices.push_back(start + 6);
    vecIndices.push_back(start + 0);
    vecIndices.push_back(start + 4);
    vecIndices.push_back(start + 2);
    vecIndices.push_back(start + 4);
    vecIndices.push_back(start + 6);
    vecIndices.push_back(start + 2);
    vecIndices.push_back(start + 1);
    vecIndices.push_back(start + 3);
    vecIndices.push_back(start + 5);
    vecIndices.push_back(start + 3);
    vecIndices.push_back(start + 7);
    vecIndices.push_back(start + 5);
    vecIndices.push_back(start + 2);
    vecIndices.push_back(start + 6);
    vecIndices.push_back(start + 3);
    vecIndices.push_back(start + 6);
    vecIndices.push_back(start + 7);
    vecIndices.push_back(start + 3);
    vecIndices.push_back(start + 0);
    vecIndices.push_back(start + 1);
    vecIndices.push_back(start + 4);
    vecIndices.push_back(start + 1);
    vecIndices.push_back(start + 5);
    vecIndices.push_back(start + 4);

    indexCnt += 36;

}


float noise3D(float x, float y, float z) {
    // Simple 3D noise using random numbers
    srand(static_cast<unsigned>(x * 31 + y * 17 + z * 13)); // Seed using unique combination
    return static_cast<float>(rand()) / RAND_MAX;           // Random value in [0, 1]
}

void generateAsteroidField(float centerX, float centerY, float centerZ,
    float radius, int numAsteroids) {
    for (int i = 0; i < numAsteroids; i++) {
        // Generate random position within a sphere
        float theta = static_cast<float>(rand()) / RAND_MAX * 2.0f * 3.141592653f; // Random angle theta
        float phi = static_cast<float>(rand()) / RAND_MAX * 3.141592653f;          // Random angle phi
        float r = radius * noise3D(theta, phi, i) * 0.9f;                          // Scale radius with noise

        float x = centerX + r * sinf(phi) * cosf(theta); // Spherical to Cartesian
        float y = centerY + r * sinf(phi) * sinf(theta);
        float z = centerZ + r * cosf(phi);

        // Add random cubes around the position to form an asteroid-like shape
        for (int j = 0; j < 10; j++) { // 10 cubes per asteroid
            float offsetX = round((static_cast<float>(rand()) / RAND_MAX - 0.5f) * 2.0f);
            float offsetY = round((static_cast<float>(rand()) / RAND_MAX - 0.5f) * 2.0f);
            float offsetZ = round((static_cast<float>(rand()) / RAND_MAX - 0.5f) * 2.0f);
            addCube(x + offsetX, y + offsetY, z + offsetZ);
        }
    }
}



Vertex* getVerts() {
    return &vecVerts[0];
}


UINT* getIndices() {
    return &vecIndices[0];
}


// Global variables
HWND g_hWnd = nullptr;
ID3D11Device* g_device = nullptr;
ID3D11DeviceContext* g_context = nullptr;
IDXGISwapChain* g_swapChain = nullptr;
ID3D11RenderTargetView* g_renderTargetView = nullptr;
ID3D11Buffer* g_vertexBuffer = nullptr;
ID3D11Buffer* g_indexBuffer = nullptr;
ID3D11Buffer* g_constantBuffer = nullptr;
ID3D11VertexShader* g_vertexShader = nullptr;
ID3D11PixelShader* g_pixelShader = nullptr;
ID3D11InputLayout* g_inputLayout = nullptr;
ID3D11DepthStencilView* g_depthStencilView;

// Movement
XMFLOAT3 g_cameraPosition = { 0.0f, 2.0f, -5.0f };
XMFLOAT3 g_cameraRotation = { 0.0f, 0.0f, 0.0f }; // Pitch, Yaw, Roll (we use only pitch and yaw)
float g_cameraSpeed = 0.1f;
bool g_mouseCaptured = false;
POINT g_lastMousePos;

// Shader source code
const char* vertexShaderSource = R"(
cbuffer ConstantBuffer : register(b0) {
    matrix worldViewProj;
};

struct VS_INPUT {
    float3 position : POSITION;
    float4 color : COLOR;
};

struct PS_INPUT {
    float4 position : SV_POSITION;
    float4 color : COLOR;
};

PS_INPUT main(VS_INPUT input) {
    PS_INPUT output;
    output.position = mul(float4(input.position, 1.0), worldViewProj);
    output.color = input.color;
    return output;
}
)";

const char* pixelShaderSource = R"(
struct PS_INPUT {
    float4 position : SV_POSITION;
    float4 color : COLOR;
};

float4 main(PS_INPUT input) : SV_TARGET {
    return input.color;
}
)";

// Constant buffer structure
struct ConstantBuffer {
    XMMATRIX worldViewProj;
};

void InitDirectX(HWND hWnd);
void Render();
void Cleanup();
LRESULT CALLBACK WindowProc(HWND hWnd, UINT message, WPARAM wParam, LPARAM lParam);
void InitPipeline();
void InitBuffers();

int APIENTRY WinMain(HINSTANCE hInstance, HINSTANCE hPrevInstance, LPSTR lpCmdLine, int nCmdShow) {
    WNDCLASS wc = {};
    wc.lpfnWndProc = WindowProc;
    wc.hInstance = hInstance;
    wc.lpszClassName = L"CubeApp";

    RegisterClass(&wc);

    g_hWnd = CreateWindowEx(0, L"CubeApp", L"DirectX 11 Cube", WS_OVERLAPPEDWINDOW,
        CW_USEDEFAULT, CW_USEDEFAULT, 800, 600,
        nullptr, nullptr, hInstance, nullptr);

    ShowWindow(g_hWnd, nCmdShow);

    InitDirectX(g_hWnd);

    MSG msg = {};
    while (msg.message != WM_QUIT) {
        if (PeekMessage(&msg, nullptr, 0, 0, PM_REMOVE)) {
            TranslateMessage(&msg);
            DispatchMessage(&msg);
        }
        else {
            Render();
        }
    }

    Cleanup();
    return 0;
}

void InitDirectX(HWND hWnd) {
    DXGI_SWAP_CHAIN_DESC scDesc = {};
    scDesc.BufferDesc.Width = 800;
    scDesc.BufferDesc.Height = 600;
    scDesc.BufferDesc.Format = DXGI_FORMAT_R8G8B8A8_UNORM;
    scDesc.SampleDesc.Count = 1;
    scDesc.BufferUsage = DXGI_USAGE_RENDER_TARGET_OUTPUT;
    scDesc.BufferCount = 1;
    scDesc.OutputWindow = hWnd;
    scDesc.Windowed = TRUE;

    HRESULT hr = D3D11CreateDeviceAndSwapChain(
        nullptr, D3D_DRIVER_TYPE_HARDWARE, nullptr, 0, nullptr, 0,
        D3D11_SDK_VERSION, &scDesc, &g_swapChain, &g_device, nullptr, &g_context);

    if (FAILED(hr)) {
        MessageBox(nullptr, L"Failed to create device and swap chain.", L"Error", MB_OK);
        return;
    }

    ID3D11Texture2D* backBuffer = nullptr;
    hr = g_swapChain->GetBuffer(0, IID_PPV_ARGS(&backBuffer));
    if (FAILED(hr)) {
        MessageBox(nullptr, L"Failed to get back buffer.", L"Error", MB_OK);
        return;
    }

    hr = g_device->CreateRenderTargetView(backBuffer, nullptr, &g_renderTargetView);
    backBuffer->Release();
    if (FAILED(hr)) {
        MessageBox(nullptr, L"Failed to create render target view.", L"Error", MB_OK);
        return;
    }

    // Depth-Stencil Buffer
    D3D11_TEXTURE2D_DESC depthBufferDesc = {};
    depthBufferDesc.Width = 800;
    depthBufferDesc.Height = 600;
    depthBufferDesc.MipLevels = 1;
    depthBufferDesc.ArraySize = 1;
    depthBufferDesc.Format = DXGI_FORMAT_D24_UNORM_S8_UINT;
    depthBufferDesc.SampleDesc.Count = 1;
    depthBufferDesc.Usage = D3D11_USAGE_DEFAULT;
    depthBufferDesc.BindFlags = D3D11_BIND_DEPTH_STENCIL;

    ID3D11Texture2D* depthStencilBuffer = nullptr;
    hr = g_device->CreateTexture2D(&depthBufferDesc, nullptr, &depthStencilBuffer);
    if (FAILED(hr)) {
        MessageBox(nullptr, L"Failed to create depth stencil buffer.", L"Error", MB_OK);
        return;
    }

    D3D11_DEPTH_STENCIL_VIEW_DESC depthStencilViewDesc = {};
    depthStencilViewDesc.Format = depthBufferDesc.Format;
    depthStencilViewDesc.ViewDimension = D3D11_DSV_DIMENSION_TEXTURE2D;
    depthStencilViewDesc.Texture2D.MipSlice = 0;

    hr = g_device->CreateDepthStencilView(depthStencilBuffer, &depthStencilViewDesc, &g_depthStencilView);
    depthStencilBuffer->Release();
    if (FAILED(hr)) {
        MessageBox(nullptr, L"Failed to create depth stencil view.", L"Error", MB_OK);
        return;
    }

    g_context->OMSetRenderTargets(1, &g_renderTargetView, g_depthStencilView);

    // Set Viewport
    D3D11_VIEWPORT viewport = {};
    viewport.Width = 800.0f;
    viewport.Height = 600.0f;
    viewport.MinDepth = 0.0f;
    viewport.MaxDepth = 1.0f;
    g_context->RSSetViewports(1, &viewport);

    InitPipeline();
    InitBuffers();
}

void InitPipeline() {
    ID3DBlob* vsBlob;
    ID3DBlob* psBlob;
    D3DCompile(vertexShaderSource, strlen(vertexShaderSource), nullptr, nullptr, nullptr, "main", "vs_5_0", 0, 0, &vsBlob, nullptr);
    D3DCompile(pixelShaderSource, strlen(pixelShaderSource), nullptr, nullptr, nullptr, "main", "ps_5_0", 0, 0, &psBlob, nullptr);

    g_device->CreateVertexShader(vsBlob->GetBufferPointer(), vsBlob->GetBufferSize(), nullptr, &g_vertexShader);
    g_device->CreatePixelShader(psBlob->GetBufferPointer(), psBlob->GetBufferSize(), nullptr, &g_pixelShader);

    g_context->VSSetShader(g_vertexShader, nullptr, 0);
    g_context->PSSetShader(g_pixelShader, nullptr, 0);

    D3D11_INPUT_ELEMENT_DESC layout[] = {
        {"POSITION", 0, DXGI_FORMAT_R32G32B32_FLOAT, 0, 0, D3D11_INPUT_PER_VERTEX_DATA, 0},
        {"COLOR", 0, DXGI_FORMAT_R32G32B32A32_FLOAT, 0, 12, D3D11_INPUT_PER_VERTEX_DATA, 0},
    };

    g_device->CreateInputLayout(layout, 2, vsBlob->GetBufferPointer(), vsBlob->GetBufferSize(), &g_inputLayout);
    g_context->IASetInputLayout(g_inputLayout);

    vsBlob->Release();
    psBlob->Release();
}

void HandleInput() {
    if (GetAsyncKeyState('W') & 0x8000) { // Forward
        g_cameraPosition.x += sinf(g_cameraRotation.y) * g_cameraSpeed;
        g_cameraPosition.z += cosf(g_cameraRotation.y) * g_cameraSpeed;
    }
    if (GetAsyncKeyState('S') & 0x8000) { // Backward
        g_cameraPosition.x -= sinf(g_cameraRotation.y) * g_cameraSpeed;
        g_cameraPosition.z -= cosf(g_cameraRotation.y) * g_cameraSpeed;
    }
    if (GetAsyncKeyState('A') & 0x8000) { // Left
        g_cameraPosition.x -= cosf(g_cameraRotation.y) * g_cameraSpeed;
        g_cameraPosition.z += sinf(g_cameraRotation.y) * g_cameraSpeed;
    }
    if (GetAsyncKeyState('D') & 0x8000) { // Right
        g_cameraPosition.x += cosf(g_cameraRotation.y) * g_cameraSpeed;
        g_cameraPosition.z -= sinf(g_cameraRotation.y) * g_cameraSpeed;
    }
}

void HandleMouse() {
    POINT currentMousePos;
    GetCursorPos(&currentMousePos);

    if (g_mouseCaptured) {
        // Calculate mouse delta
        float deltaX = (float)(currentMousePos.x - g_lastMousePos.x) * 0.002f; // Sensitivity
        float deltaY = -(float)(currentMousePos.y - g_lastMousePos.y) * 0.002f;

        // Update camera rotation
        g_cameraRotation.x += deltaY; // Pitch
        g_cameraRotation.y += deltaX; // Yaw

        // Clamp pitch to avoid flipping
        g_cameraRotation.x = max(-XM_PIDIV2, min(XM_PIDIV2, g_cameraRotation.x));

        // Reset mouse to the center of the screen
        RECT rect;
        GetClientRect(g_hWnd, &rect);
        POINT center = { rect.right / 2, rect.bottom / 2 };
        ClientToScreen(g_hWnd, &center);
        SetCursorPos(center.x, center.y);
        g_lastMousePos = center;
    }
    else {
        g_lastMousePos = currentMousePos;
    }
}


void InitBuffers() {


    generateAsteroidField(0.0f, 0.0f, 0.0f, 500.0f, 100); // 50 asteroids in a 100-unit radius field

    // Vertex buffer
    D3D11_BUFFER_DESC vertexBufferDesc = {};
    vertexBufferDesc.Usage = D3D11_USAGE_DEFAULT;
    vertexBufferDesc.ByteWidth = sizeof(Vertex) * vecVerts.size();
    vertexBufferDesc.BindFlags = D3D11_BIND_VERTEX_BUFFER;

    D3D11_SUBRESOURCE_DATA vertexData = {};
    vertexData.pSysMem = getVerts();

    g_device->CreateBuffer(&vertexBufferDesc, &vertexData, &g_vertexBuffer);

    // Index buffer
    D3D11_BUFFER_DESC indexBufferDesc = {};
    indexBufferDesc.Usage = D3D11_USAGE_DEFAULT;
    indexBufferDesc.ByteWidth = sizeof(UINT) * vecIndices.size();
    indexBufferDesc.BindFlags = D3D11_BIND_INDEX_BUFFER;

    D3D11_SUBRESOURCE_DATA indexData = {};
    indexData.pSysMem = getIndices();

    g_device->CreateBuffer(&indexBufferDesc, &indexData, &g_indexBuffer);

    // Constant buffer
    D3D11_BUFFER_DESC constantBufferDesc = {};
    constantBufferDesc.Usage = D3D11_USAGE_DEFAULT;
    constantBufferDesc.ByteWidth = sizeof(ConstantBuffer);
    constantBufferDesc.BindFlags = D3D11_BIND_CONSTANT_BUFFER;

    g_device->CreateBuffer(&constantBufferDesc, nullptr, &g_constantBuffer);
}



void Render() {
    HandleInput();
    HandleMouse();

    static float angle = 0.0f;
    angle += 0.01f;

    XMMATRIX world = XMMatrixRotationY(angle);
    XMMATRIX view = XMMatrixLookAtLH(
        XMVectorSet(g_cameraPosition.x, g_cameraPosition.y, g_cameraPosition.z, 0.0f),
        XMVectorSet(
            g_cameraPosition.x + cosf(g_cameraRotation.x) * sinf(g_cameraRotation.y),
            g_cameraPosition.y + sinf(g_cameraRotation.x),
            g_cameraPosition.z + cosf(g_cameraRotation.x) * cosf(g_cameraRotation.y),
            0.0f),
        XMVectorSet(0.0f, 1.0f, 0.0f, 0.0f));
    XMMATRIX proj = XMMatrixPerspectiveFovLH(XM_PIDIV4, 800.0f / 600.0f, 0.1f, 100.0f);
    XMMATRIX worldViewProj = world * view * proj;

    ConstantBuffer cb = { XMMatrixTranspose(worldViewProj) };
    g_context->UpdateSubresource(g_constantBuffer, 0, nullptr, &cb, 0, 0);
    g_context->VSSetConstantBuffers(0, 1, &g_constantBuffer);

    UINT stride = sizeof(Vertex);
    UINT offset = 0;
    g_context->IASetVertexBuffers(0, 1, &g_vertexBuffer, &stride, &offset);
    g_context->IASetIndexBuffer(g_indexBuffer, DXGI_FORMAT_R32_UINT, 0);
    g_context->IASetPrimitiveTopology(D3D11_PRIMITIVE_TOPOLOGY_TRIANGLELIST);

    float clearColor[] = { 0.1f, 0.1f, 0.2f, 1.0f };
    g_context->ClearRenderTargetView(g_renderTargetView, clearColor);
    g_context->ClearDepthStencilView(g_depthStencilView, D3D11_CLEAR_DEPTH, 1.0f, 0);

    // Draw your scene here
    g_context->DrawIndexed(vecIndices.size(), 0, 0);

    g_swapChain->Present(1, 0);
}

void Cleanup() {
    if (g_constantBuffer) g_constantBuffer->Release();
    if (g_indexBuffer) g_indexBuffer->Release();
    if (g_vertexBuffer) g_vertexBuffer->Release();
    if (g_inputLayout) g_inputLayout->Release();
    if (g_pixelShader) g_pixelShader->Release();
    if (g_vertexShader) g_vertexShader->Release();
    if (g_renderTargetView) g_renderTargetView->Release();
    if (g_swapChain) g_swapChain->Release();
    if (g_context) g_context->Release();
    if (g_device) g_device->Release();
}

LRESULT CALLBACK WindowProc(HWND hWnd, UINT message, WPARAM wParam, LPARAM lParam) {
    switch (message) {
    case WM_DESTROY:
        PostQuitMessage(0);
        return 0;

    case WM_RBUTTONDOWN:
        g_mouseCaptured = !g_mouseCaptured;
        ShowCursor(!g_mouseCaptured);
        if (g_mouseCaptured) {
            RECT rect;
            GetClientRect(hWnd, &rect);
            POINT center = { rect.right / 2, rect.bottom / 2 };
            ClientToScreen(hWnd, &center);
            SetCursorPos(center.x, center.y);
            g_lastMousePos = center;
        }
        return 0;
    }
    return DefWindowProc(hWnd, message, wParam, lParam);
}