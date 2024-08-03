#include "FrameD3D.h"

std::pair<bool, std::string> CFrameD3D::CreateDeviceD3D( HWND hWnd, D3D& d3d )
{
   // Setup swap chain
   DXGI_SWAP_CHAIN_DESC1 dxscd_;
   {
      ZeroMemory(&dxscd_, sizeof(dxscd_));
      dxscd_.BufferCount = D3D::NUM_BACK_BUFFERS;
      dxscd_.Width = 0;
      dxscd_.Height = 0;
      dxscd_.Format = DXGI_FORMAT_R8G8B8A8_UNORM;
      dxscd_.Flags = DXGI_SWAP_CHAIN_FLAG_FRAME_LATENCY_WAITABLE_OBJECT;
      dxscd_.BufferUsage = DXGI_USAGE_RENDER_TARGET_OUTPUT;
      dxscd_.SampleDesc.Count = 1;
      dxscd_.SampleDesc.Quality = 0;
      dxscd_.SwapEffect = DXGI_SWAP_EFFECT_FLIP_DISCARD;
      dxscd_.AlphaMode = DXGI_ALPHA_MODE_UNSPECIFIED;
      dxscd_.Scaling = DXGI_SCALING_STRETCH;
      dxscd_.Stereo = FALSE;
   }

   // [DEBUG] Enable debug interface
#ifdef DX12_ENABLE_DEBUG_LAYER
   ID3D12Debug* pdx12Debug = nullptr;
   if(SUCCEEDED(D3D12GetDebugInterface(IID_PPV_ARGS(&pdx12Debug))))
      pdx12Debug->EnableDebugLayer();
#endif

   // Create device
   D3D_FEATURE_LEVEL featureLevel = D3D_FEATURE_LEVEL_11_0;
   if(D3D12CreateDevice(nullptr, featureLevel, IID_PPV_ARGS(d3d.GetDeviceAddress())) != S_OK) return { false, "Failed to run D3D12CreateDevice" };

   // [DEBUG] Setup debug interface to break on any warnings/errors
#ifdef DX12_ENABLE_DEBUG_LAYER
   if(pdx12Debug != nullptr)
   {
      ID3D12InfoQueue* pInfoQueue = nullptr;
      d3d.GetDevice()->QueryInterface(IID_PPV_ARGS(&pInfoQueue));
      pInfoQueue->SetBreakOnSeverity(D3D12_MESSAGE_SEVERITY_ERROR, true);
      pInfoQueue->SetBreakOnSeverity(D3D12_MESSAGE_SEVERITY_CORRUPTION, true);
      pInfoQueue->SetBreakOnSeverity(D3D12_MESSAGE_SEVERITY_WARNING, true);
      pInfoQueue->Release();
      pdx12Debug->Release();
   }
#endif

   {
      D3D12_DESCRIPTOR_HEAP_DESC desc = {};
      desc.Type = D3D12_DESCRIPTOR_HEAP_TYPE_RTV;
      desc.NumDescriptors = D3D::NUM_BACK_BUFFERS;
      desc.Flags = D3D12_DESCRIPTOR_HEAP_FLAG_NONE;
      desc.NodeMask = 1;
      if(d3d.GetDevice()->CreateDescriptorHeap(&desc, IID_PPV_ARGS(d3d.GetRtvDescriptorHeapAddress())) != S_OK) return { false, "Failed to run CreateDescriptorHeap" };

      SIZE_T rtvDescriptorSize = d3d.GetDevice()->GetDescriptorHandleIncrementSize(D3D12_DESCRIPTOR_HEAP_TYPE_RTV);
      D3D12_CPU_DESCRIPTOR_HANDLE rtvHandle = d3d.GetRtvDescriptorHeap()->GetCPUDescriptorHandleForHeapStart();
      for(unsigned u = 0; u < D3D::NUM_BACK_BUFFERS; u++)
      {
         d3d.SetDescriptorHandle( u, rtvHandle );
         rtvHandle.ptr += rtvDescriptorSize;
      }
   }

   {
      D3D12_DESCRIPTOR_HEAP_DESC desc = {};
      desc.Type = D3D12_DESCRIPTOR_HEAP_TYPE_CBV_SRV_UAV;
      desc.NumDescriptors = 1;
      desc.Flags = D3D12_DESCRIPTOR_HEAP_FLAG_SHADER_VISIBLE;
      if(d3d.GetDevice()->CreateDescriptorHeap(&desc, IID_PPV_ARGS(d3d.GetSrvDescriptorHeapAddress())) != S_OK) return { false, "Failed to run CreateDescriptorHeap" };
   }

   {
      D3D12_COMMAND_QUEUE_DESC desc = {};
      desc.Type = D3D12_COMMAND_LIST_TYPE_DIRECT;
      desc.Flags = D3D12_COMMAND_QUEUE_FLAG_NONE;
      desc.NodeMask = 1;
      if(d3d.GetDevice()->CreateCommandQueue(&desc, IID_PPV_ARGS(d3d.GetCommandQueueAddress())) != S_OK) return { false, "Failed to run CreateCommandQueue" };
   }

   for(unsigned u = 0; u < D3D::NUM_FRAMES_IN_FLIGHT; u++)
   {
      FrameContext* pframecontext = d3d.GetFrameContextPointer( u );
      if(d3d.GetDevice()->CreateCommandAllocator(D3D12_COMMAND_LIST_TYPE_DIRECT, IID_PPV_ARGS(&pframecontext->CommandAllocator)) != S_OK) return {false, "Failed to run CreateCommandAllocator"};
   }

   if(d3d.GetDevice()->CreateCommandList(0, D3D12_COMMAND_LIST_TYPE_DIRECT, d3d.GetFrameContext(0).CommandAllocator, nullptr, IID_PPV_ARGS(d3d.GetCommandListAddress())) != S_OK || d3d.GetCommandList()->Close() != S_OK) return { false, "Failed to run CreateCommandList" };

   if(d3d.GetDevice()->CreateFence(0, D3D12_FENCE_FLAG_NONE, IID_PPV_ARGS(d3d.GetFenceAddress())) != S_OK) return { false, "Failed to run CreateFence" };

   auto fenceevent_ = CreateEvent(nullptr, FALSE, FALSE, nullptr);
   if(fenceevent_ == nullptr) return { false, "Failed to run CreateEvent" };
   d3d.SetFenceEvent( fenceevent_ );

   {
      IDXGIFactory4* dxgiFactory = nullptr;
      IDXGISwapChain1* pidxgiswapchain1 = nullptr;
      if(CreateDXGIFactory1(IID_PPV_ARGS(&dxgiFactory)) != S_OK) return { false, "Failed to run CreateDXGIFactory1" };
      if(dxgiFactory->CreateSwapChainForHwnd(d3d.GetCommandQueue(), hWnd, &dxscd_, nullptr, nullptr, &pidxgiswapchain1) != S_OK) return { false, "Failed to run CreateSwapChainForHwnd" };
      if(pidxgiswapchain1->QueryInterface(IID_PPV_ARGS(d3d.GetSwapChainAddress())) != S_OK) return { false, "Failed to run QueryInterface" };
      pidxgiswapchain1->Release();
      dxgiFactory->Release();
      d3d.GetSwapChain()->SetMaximumFrameLatency(D3D::NUM_BACK_BUFFERS);
      d3d.SetSwapChainWaitableObjects( d3d.GetSwapChain()->GetFrameLatencyWaitableObject() );
   }

   CreateRenderTarget( D3D::NUM_BACK_BUFFERS, d3d );

   return { true, "" };
}


void CFrameD3D::CleanupDeviceD3D( D3D& d3d )
{
   CleanupRenderTarget( D3D::NUM_BACK_BUFFERS, d3d );
   if(d3d.GetSwapChain() != nullptr) { d3d.GetSwapChain()->SetFullscreenState(false, nullptr); d3d.GetSwapChain()->Release(); d3d.SetSwapChain( nullptr ); }

   if(d3d.GetSwapChainWaitableObject() != nullptr) { CloseHandle(d3d.GetSwapChainWaitableObject()); d3d.SetSwapChainWaitableObjects( nullptr ); }

   for(unsigned u = 0; u < D3D::NUM_FRAMES_IN_FLIGHT; u++)
   {
      if( d3d.GetFrameContext( u ).CommandAllocator) { d3d.GetFrameContext( u ).CommandAllocator->Release(); d3d.SetFrameContextCommandAllocator( u, nullptr ); }
   }

   if(d3d.GetCommandQueue() != nullptr) { d3d.GetCommandQueue()->Release(); d3d.SetCommandQueue(nullptr); }
   if(d3d.GetCommandList() != nullptr) { d3d.GetCommandList()->Release(); d3d.SetCommandList(nullptr); }
   if(d3d.GetRtvDescriptorHeap() != nullptr) { d3d.GetRtvDescriptorHeap()->Release(); d3d.SetRtvDescriptorHeap(nullptr); }
   if(d3d.GetSrvDescriptorHeap() != nullptr) { d3d.GetSrvDescriptorHeap()->Release(); d3d.SetSrvDescriptorHeap(nullptr); }
   if(d3d.GetFence() != nullptr) { d3d.GetFence()->Release(); d3d.SetFence(nullptr); }
   if(d3d.GetFenceEvent() != nullptr) { CloseHandle(d3d.GetFenceEvent()); d3d.SetFenceEvent(nullptr); }
   if(d3d.GetDevice() != nullptr) { d3d.GetDevice()->Release(); d3d.SetDevice(nullptr); }

#ifdef DX12_ENABLE_DEBUG_LAYER
   IDXGIDebug1* pDebug = nullptr;
   if(SUCCEEDED(DXGIGetDebugInterface1(0, IID_PPV_ARGS(&pDebug))))
   {
      pDebug->ReportLiveObjects(DXGI_DEBUG_ALL, DXGI_DEBUG_RLO_SUMMARY);
      pDebug->Release();
   }
#endif
}

void CFrameD3D::CreateRenderTarget( unsigned uBufferCount, D3D& d3d )
{
   for(unsigned u = 0; u < uBufferCount; u++)
   {
      ID3D12Resource* pBackBuffer = nullptr;
      d3d.GetSwapChain()->GetBuffer(u, IID_PPV_ARGS(&pBackBuffer));
      d3d.GetDevice()->CreateRenderTargetView(pBackBuffer, nullptr, d3d.GetDescriptorHandle( u ));
      d3d.SetResource( u,  pBackBuffer );
   }
}

void CFrameD3D::CleanupRenderTarget( unsigned uBufferCount, D3D& d3d )
{
   CFrameD3D::WaitForLastSubmittedFrame( d3d );

   for(unsigned u = 0; u < uBufferCount; u++)
   {
      if(d3d.GetResource(u)) { d3d.GetResource(u)->Release(); d3d.SetResource( u, nullptr ); }
   }
}

void CFrameD3D::WaitForLastSubmittedFrame( D3D& d3d )
{
   FrameContext* pframecontext = d3d.GetFrameContextPointer( d3d.GetFrameIndex() % D3D::NUM_FRAMES_IN_FLIGHT );

   //FrameContext* frameCtx = &g_frameContext[g_frameIndex % NUM_FRAMES_IN_FLIGHT];

   //FrameContext* pframecontext = &d3d.GetFrameContext[d3d.GetFrameIndex() % D3D::NUM_FRAMES_IN_FLIGHT];

   uint64_t uFenceValue = pframecontext->FenceValue;
   if(uFenceValue == 0) return; // No fence was signaled

   pframecontext->FenceValue = 0;
   if(d3d.GetFence()->GetCompletedValue() >= uFenceValue)  return;

   d3d.GetFence()->SetEventOnCompletion(uFenceValue, d3d.GetFenceEvent());
   WaitForSingleObject(d3d.GetFenceEvent(), INFINITE);
}

CFrameD3D::FrameContext* CFrameD3D::WaitForNextFrameResources( D3D& d3d )
{
   unsigned uNextFrame = d3d.GetFrameIndex() + 1;
   d3d.SetFrameIndex( uNextFrame );

   HANDLE phWaitableObjects[] = { d3d.GetSwapChainWaitableObject(), nullptr};
   uint32_t uWaitableObjects = 1;



   FrameContext* pframecontext = d3d.GetFrameContextPointer( uNextFrame % D3D::NUM_FRAMES_IN_FLIGHT );
   //FrameContext* frameCtx = &g_frameContext[nextFrameIndex % NUM_FRAMES_IN_FLIGHT];
   uint64_t uFenceValue = pframecontext->FenceValue;
   if(uFenceValue != 0) // means no fence was signaled
   {
      pframecontext->FenceValue = 0;
      d3d.GetFence()->SetEventOnCompletion(uFenceValue, d3d.GetFenceEvent());
      phWaitableObjects[1] = d3d.GetFenceEvent();
      uWaitableObjects = 2;
   }

   WaitForMultipleObjects(uWaitableObjects, phWaitableObjects, TRUE, INFINITE);

   return pframecontext;
}


