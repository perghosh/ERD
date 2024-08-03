#pragma once

#include <d3d12.h>
#include <dxgi1_4.h>

#include "Frame.h"


/**
 * \brief
 *
 *
 *
 \code
 \endcode
 */
class CFrameD3D : public window::CFrame
{
public:
   struct FrameContext
   {
      ID3D12CommandAllocator* CommandAllocator;
      uint64_t                FenceValue;
   };


   /**
    * \brief
    *
    *
    */
   struct D3D
   {
      // ## construction -------------------------------------------------------------

      D3D() {}
      ~D3D() {}


      void SetFrameIndex(unsigned int uFrameIndex) { m_uFrameIndex = uFrameIndex; }
      unsigned int GetFrameIndex() const { return m_uFrameIndex; }

      void SetFenceLastSignaledValue(uint64_t uFenceLastSignaledValue) { m_uFenceLastSignaledValue = uFenceLastSignaledValue; }
      uint64_t GetFenceLastSignaledValue() const { return m_uFenceLastSignaledValue; }

      void SetSwapChainOccluded(bool bSwapChainOccluded) { m_bSwapChainOccluded = bSwapChainOccluded; }
      bool GetSwapChainOccluded() const { return m_bSwapChainOccluded; }

      // handle ---------------------------------------------------------------
      HANDLE GetFenceEvent() { return m_hFenceEvent; }
      void SetFenceEvent( HANDLE hFenceEvent ) { m_hFenceEvent = hFenceEvent; }
      HANDLE GetSwapChainWaitableObject() { return m_hSwapChainWaitableObject; }
      void SetSwapChainWaitableObjects( HANDLE hSwapChainWaitableObject ) { m_hSwapChainWaitableObject = hSwapChainWaitableObject; }

      // pointers -------------------------------------------------------------

      void SetDevice(ID3D12Device* pid3d12device) { m_pid3d12device = pid3d12device; }
      ID3D12Device* GetDevice() const { return m_pid3d12device; }
      ID3D12Device** GetDeviceAddress() { return &m_pid3d12device; }

      void SetSwapChain(IDXGISwapChain3* pSwapChain) { m_pSwapChain = pSwapChain; }
      IDXGISwapChain3* GetSwapChain() const { return m_pSwapChain; }
      IDXGISwapChain3** GetSwapChainAddress() { return &m_pSwapChain; }

      void SetRtvDescriptorHeap(ID3D12DescriptorHeap* pid3d12Rtvdescriptorheap) { assert( m_pid3d12Rtvdescriptorheap == nullptr ); m_pid3d12Rtvdescriptorheap = pid3d12Rtvdescriptorheap; }
      ID3D12DescriptorHeap* GetRtvDescriptorHeap() const { return m_pid3d12Rtvdescriptorheap; }
      ID3D12DescriptorHeap** GetRtvDescriptorHeapAddress() { return &m_pid3d12Rtvdescriptorheap; }

      void SetSrvDescriptorHeap(ID3D12DescriptorHeap* pid3d12Srvdescriptorheap) { assert( m_pid3d12Srvdescriptorheap == nullptr ); m_pid3d12Srvdescriptorheap = pid3d12Srvdescriptorheap; }
      ID3D12DescriptorHeap* GetSrvDescriptorHeap() const { return m_pid3d12Srvdescriptorheap; }
      ID3D12DescriptorHeap** GetSrvDescriptorHeapAddress() { return &m_pid3d12Srvdescriptorheap; }

      void SetCommandQueue(ID3D12CommandQueue* pid3d12commandqueue) { assert( m_pid3d12commandqueue == nullptr ); m_pid3d12commandqueue = pid3d12commandqueue; }
      ID3D12CommandQueue* GetCommandQueue() const { return m_pid3d12commandqueue; }
      ID3D12CommandQueue** GetCommandQueueAddress() { return &m_pid3d12commandqueue; }

      void SetCommandList(ID3D12GraphicsCommandList* pid3d12graphicscommandlist) { assert( m_pid3d12graphicscommandlist == nullptr ); m_pid3d12graphicscommandlist = pid3d12graphicscommandlist; }
      ID3D12GraphicsCommandList* GetCommandList() const { return m_pid3d12graphicscommandlist; }
      ID3D12GraphicsCommandList** GetCommandListAddress() { return &m_pid3d12graphicscommandlist; }

      void SetFence(ID3D12Fence* pid3d12fence) { assert( m_pid3d12fence == nullptr ); m_pid3d12fence = pid3d12fence; }
      ID3D12Fence* GetFence() const { return m_pid3d12fence; }
      ID3D12Fence** GetFenceAddress() { return &m_pid3d12fence; }

      ID3D12Resource* GetResource( unsigned uIndex ) { assert( uIndex < NUM_BACK_BUFFERS ); return m_ppid3d12resource[uIndex]; }
      void SetResource( unsigned uIndex, ID3D12Resource* p_ ) { m_ppid3d12resource[uIndex] = p_; }
      FrameContext GetFrameContext( unsigned uIndex ) { assert( uIndex < NUM_FRAMES_IN_FLIGHT ); return m_pframecontext[uIndex]; }
      FrameContext* GetFrameContextPointer( unsigned uIndex ) { assert( uIndex < NUM_FRAMES_IN_FLIGHT ); return &m_pframecontext[uIndex]; }
      void SetFrameContextCommandAllocator( unsigned uIndex, ID3D12CommandAllocator* p_ ) { assert( uIndex < NUM_FRAMES_IN_FLIGHT ); m_pframecontext[uIndex].CommandAllocator = p_; }

      D3D12_CPU_DESCRIPTOR_HANDLE GetDescriptorHandle( unsigned uIndex ) const { assert( uIndex < NUM_BACK_BUFFERS ); return m_phDescriptorHandle[uIndex]; }
      void SetDescriptorHandle( unsigned uIndex, D3D12_CPU_DESCRIPTOR_HANDLE hHandle ) { assert( uIndex < NUM_BACK_BUFFERS ); m_phDescriptorHandle[uIndex] = hHandle; }


      static const int NUM_FRAMES_IN_FLIGHT = 3;
      static const int NUM_BACK_BUFFERS = 3;

      // ## attributes
      unsigned       int m_uFrameIndex = 0;
      uint64_t       m_uFenceLastSignaledValue = 0;
      bool           m_bSwapChainOccluded = false;
      HANDLE         m_hFenceEvent = nullptr;
      HANDLE         m_hSwapChainWaitableObject = nullptr;
      ID3D12Device*  m_pid3d12device = nullptr;
      IDXGISwapChain3* m_pSwapChain = nullptr;
      ID3D12DescriptorHeap* m_pid3d12Rtvdescriptorheap = nullptr;
      ID3D12DescriptorHeap* m_pid3d12Srvdescriptorheap = nullptr;
      ID3D12CommandQueue* m_pid3d12commandqueue = nullptr;
      ID3D12GraphicsCommandList* m_pid3d12graphicscommandlist = nullptr;
      ID3D12Fence*   m_pid3d12fence = nullptr;
      ID3D12Resource* m_ppid3d12resource[NUM_BACK_BUFFERS] = {};
      FrameContext   m_pframecontext[NUM_FRAMES_IN_FLIGHT] = {};
      D3D12_CPU_DESCRIPTOR_HANDLE m_phDescriptorHandle[NUM_BACK_BUFFERS] = {};
   };


// ## construction -------------------------------------------------------------
public:
   CFrameD3D() {}
   // copy
   CFrameD3D(const CFrameD3D& o) { common_construct(o); }
   CFrameD3D(CFrameD3D&& o) noexcept { common_construct(std::move(o)); }
   // assign
   CFrameD3D& operator=(const CFrameD3D& o) { common_construct(o); return *this; }
   CFrameD3D& operator=(CFrameD3D&& o) noexcept { common_construct(std::move(o)); return *this; }

   ~CFrameD3D() {}
private:
   // common copy
   void common_construct(const CFrameD3D& o) {}
   void common_construct(CFrameD3D&& o) noexcept {}

   // ## operator -----------------------------------------------------------------
public:


   // ## methods ------------------------------------------------------------------
public:
   /** \name GET/SET
   *///@{

   //@}

   /** \name OPERATION
   *///@{

   //@}

protected:
   /** \name INTERNAL
   *///@{

   //@}

public:
   /** \name DEBUG
   *///@{

   //@}


   // ## attributes ----------------------------------------------------------------
public:
   D3D m_D3D;

   // ## free functions ------------------------------------------------------------
public:

   std::pair<bool, std::string> CreateDeviceD3D(HWND hWnd, D3D& d3d);
   void CleanupDeviceD3D( D3D& d3d );
   void CreateRenderTarget( unsigned uBufferCount, D3D& d3d );
   void CleanupRenderTarget( unsigned uBufferCount, D3D& d3d );
   void WaitForLastSubmittedFrame( D3D& d3d );
   FrameContext* WaitForNextFrameResources( D3D& d3d );


};