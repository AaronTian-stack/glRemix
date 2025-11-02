#include "d3d12_context.h"

#include <cassert>
#include <stdexcept>

#include "application.h"

using namespace glremix::dx;

void D3D12Context::set_debug_name(ID3D12Object* obj, const char* name)
{
	if (name)
	{
		obj->SetPrivateData(WKPDID_D3DDebugObjectName, strlen(name), name);
	}
}

bool D3D12Context::create(bool enable_debug_layer)
{
	UINT dxgi_factory_flags = 0;
	if (enable_debug_layer)
	{
		if (SUCCEEDED(D3D12GetDebugInterface(IID_PPV_ARGS(&m_debug))))
		{
			m_debug->EnableDebugLayer();
			dxgi_factory_flags |= DXGI_CREATE_FACTORY_DEBUG;
		}
	}

	// Create the DXGI factory
	if (FAILED(CreateDXGIFactory2(dxgi_factory_flags, IID_PPV_ARGS(m_dxgi_factory.ReleaseAndGetAddressOf()))))
	{
		OutputDebugStringA("D3D12 ERROR: Failed to create DXGI factory\n");
		return false;
	}
	if (enable_debug_layer)
	{
		m_dxgi_factory->SetPrivateData(WKPDID_D3DDebugObjectName, sizeof("DXGI Factory") - 1, "DXGI Factory");
	}

	// Pick discrete GPU
	ComPtr<IDXGIAdapter1> adapter;
	if (FAILED(m_dxgi_factory->EnumAdapterByGpuPreference(
		0, // Adapter index
		DXGI_GPU_PREFERENCE_HIGH_PERFORMANCE,
		__uuidof(IDXGIAdapter1),
		reinterpret_cast<void**>(adapter.GetAddressOf())
	)))
	{
		OutputDebugStringA("D3D12 ERROR: Failed to find discrete GPU. Defaulting to 0th adapter\n");
		if (FAILED(m_dxgi_factory->EnumAdapters1(0, &adapter)))
		{
			return false;
		}
	}

	DXGI_ADAPTER_DESC1 desc;
	HRESULT hr = adapter->GetDesc1(&desc);
	if (FAILED(hr))
	{
		OutputDebugStringA("D3D12 ERROR: Failed to get adapter description");
	}
	else
	{
        WCHAR buffer[256];
        swprintf_s(buffer, L"D3D12: Selected adapter: %s\n", desc.Description);
        OutputDebugStringW(buffer);
	}

	// DX12 Ultimate!
	if (FAILED(D3D12CreateDevice(adapter.Get(), D3D_FEATURE_LEVEL_12_2, IID_PPV_ARGS(m_device.ReleaseAndGetAddressOf()))))
	{
		OutputDebugStringA("D3D12 ERROR: Failed to create device");
		return false;
	}

	D3D12_FEATURE_DATA_D3D12_OPTIONS12 m_options12{}; // For enhanced barriers
	if (FAILED(m_device->CheckFeatureSupport(D3D12_FEATURE_D3D12_OPTIONS12, &m_options12, sizeof(m_options12))))
	{
		OutputDebugStringA("D3D12 ERROR: Failed to query D3D12 options 12\n");
	}
	if (!m_options12.EnhancedBarriersSupported)
	{
		OutputDebugStringA("D3D12 ERROR: Enhanced barriers are not supported\n");
		return false;
	}

	D3D12_FEATURE_DATA_D3D12_OPTIONS5 m_options5{}; // For raytracing
	if (FAILED(m_device->CheckFeatureSupport(D3D12_FEATURE_D3D12_OPTIONS5, &m_options5, sizeof(m_options5))))
	{
		OutputDebugStringA("D3D12 ERROR: Failed to query D3D12 options 5");
	}
	if (m_options5.RaytracingTier < D3D12_RAYTRACING_TIER_1_1)
	{
		OutputDebugStringA("D3D12 ERROR: Raytracing tier 1.1 is not supported\n");
		return false;
	}

    D3D12_FEATURE_DATA_D3D12_OPTIONS16 options16 = {}; // For GPU upload heaps
    if(FAILED(m_device->CheckFeatureSupport(D3D12_FEATURE_D3D12_OPTIONS16, &options16, sizeof(options16))))
    {
        OutputDebugStringA("D3D12 ERROR: Failed to query D3D12 options 16");
        return false;
    }

	// Find the highest supported shader model
	constexpr std::array shader_models
	{
		D3D_SHADER_MODEL_6_6,
		D3D_SHADER_MODEL_6_5,
		D3D_SHADER_MODEL_6_4,
		D3D_SHADER_MODEL_6_2,
		D3D_SHADER_MODEL_6_1,
		D3D_SHADER_MODEL_6_0,
		D3D_SHADER_MODEL_5_1,
	};
	D3D12_FEATURE_DATA_SHADER_MODEL m_shader_model{};
	for (const auto& model : shader_models)
	{
		m_shader_model.HighestShaderModel = model;
		hr = m_device->CheckFeatureSupport(D3D12_FEATURE_SHADER_MODEL, &m_shader_model, sizeof(m_shader_model));
		if (SUCCEEDED(hr))
		{
			break;
		}
	}

	const D3D12MA::ALLOCATOR_DESC allocator_desc =
	{
		.Flags = 
			D3D12MA::ALLOCATOR_FLAG_MSAA_TEXTURES_ALWAYS_COMMITTED |
			D3D12MA::ALLOCATOR_FLAG_DEFAULT_POOLS_NOT_ZEROED, // Optional but recommended
		.pDevice = m_device.Get(),
		.PreferredBlockSize{},
		.pAllocationCallbacks{},
		.pAdapter = adapter.Get(),
	};

	if (FAILED(CreateAllocator(&allocator_desc, &m_allocator)))
	{
		OutputDebugStringA("D3D12 ERROR: Failed to create memory allocator");
		return false;
	}

	if (enable_debug_layer && SUCCEEDED(DXGIGetDebugInterface1(0, IID_PPV_ARGS(&m_dxgi_debug))))
	{
		m_dxgi_debug->EnableLeakTrackingForThread();
	}

	return create_fence(&m_fence_wait_all, 0, "wait all fence");
}

bool D3D12Context::create_swapchain(const HWND window, D3D12Queue* const queue, UINT* const frame_index)
{
	assert(frame_index);

	// I hope the host app doesn't immediately resize the window after creation!
	RECT client_rect;
	if (!GetClientRect(window, &client_rect)) 
	{
		OutputDebugStringA("WinUser ERROR: Failed to get client rect\n");
		return false;
	}
	UINT width = client_rect.right - client_rect.left;
	UINT height = client_rect.bottom - client_rect.top;

    DXGI_SWAP_CHAIN_DESC1 swapchain_descriptor
    {
        .Width = width,
        .Height = height,
        .Format = DXGI_FORMAT_R8G8B8A8_UNORM,
        .SampleDesc = 
		{
	        .Count = 1,
        	.Quality = 0
        },
        .BufferUsage = DXGI_USAGE_RENDER_TARGET_OUTPUT,
        .BufferCount = Application::m_frames_in_flight,
        .Scaling = DXGI_SCALING_STRETCH,
        .SwapEffect = DXGI_SWAP_EFFECT_FLIP_DISCARD,
        .AlphaMode = DXGI_ALPHA_MODE_UNSPECIFIED,
        .Flags = 0
    };

	ComPtr<IDXGISwapChain1> swapchain1;
	if (FAILED(m_dxgi_factory->CreateSwapChainForHwnd(
		queue->queue.Get(), // Force flush
		window,
		&swapchain_descriptor,
		nullptr,
		nullptr,
		swapchain1.ReleaseAndGetAddressOf()
	)))
	{
		OutputDebugStringA("D3D12 ERROR: Failed to create swapchain\n");
		return false;
	}

	if (FAILED(swapchain1.As(&this->m_swapchain)))
	{
		OutputDebugStringA("D3D12 ERROR: Failed to cast swapchain\n");
		return false;
	}

	for (UINT i = 0; i < swapchain_descriptor.BufferCount; ++i) 
	{
		if (FAILED(m_swapchain->GetBuffer(i, IID_PPV_ARGS(&m_swapchain_buffers[i])))) {
			OutputDebugStringA("D3D12 ERROR: Failed to get swapchain buffer\n");
			return false;
		}
	}

	m_swapchain_queue = queue;
	*frame_index = m_swapchain->GetCurrentBackBufferIndex();

	return true;
}

bool D3D12Context::create_swapchain_descriptors(D3D12DescriptorTable* descriptors, D3D12DescriptorHeap* rtv_heap)
{
	assert(descriptors);
	if (rtv_heap->desc.Flags & D3D12_DESCRIPTOR_HEAP_FLAG_SHADER_VISIBLE)
	{
		OutputDebugStringA("D3D12 ERROR: Not RTV heap\n");
		return false;
	}
	if (rtv_heap->desc.Type != D3D12_DESCRIPTOR_HEAP_TYPE_RTV)
	{
		OutputDebugStringA("D3D12 ERROR: Not RTV heap\n");
		return false;
	}

	if (!rtv_heap->allocate(Application::m_frames_in_flight, descriptors))
	{
		OutputDebugStringA("D3D12 ERROR: Failed to allocate descriptors\n");
		return false;
	}

	for (UINT i = 0; i < Application::m_frames_in_flight; i++)
    {
        D3D12_CPU_DESCRIPTOR_HANDLE rtv_handle{};
		rtv_heap->get_cpu_descriptor(&rtv_handle, descriptors->offset + i);
		assert(m_swapchain_buffers[i]);
        m_device->CreateRenderTargetView(m_swapchain_buffers[i].Get(), nullptr, rtv_handle);
    }

	return true;
}

UINT D3D12Context::get_swapchain_index() const
{
	return m_swapchain->GetCurrentBackBufferIndex();
}

ID3D12Resource* D3D12Context::get_swapchain_buffer(UINT index) const
{
	assert(index < Application::m_frames_in_flight);
	return m_swapchain_buffers[index].Get();
}

bool D3D12Context::create_descriptor_heap(const D3D12_DESCRIPTOR_HEAP_DESC& desc, D3D12DescriptorHeap* heap,
                                          const char* debug_name) const
{
	assert(heap);
	const auto result = heap->create(m_device.Get(), desc);
	set_debug_name(heap->m_heap.Get(), debug_name);
	return result;
}

bool D3D12Context::create_texture(const D3D12_RESOURCE_DESC1& desc, D3D12_BARRIER_LAYOUT init_layout,
                                  D3D12MA::Allocation** allocation, const char* debug_name)
{
	assert(allocation);
	D3D12MA::ALLOCATION_DESC allocation_desc
	{
		.HeapType = D3D12_HEAP_TYPE_GPU_UPLOAD, // GPU Upload only?
	};
	D3D12_CLEAR_VALUE* clear_ptr = nullptr;
	if (FAILED(m_allocator->CreateResource3(
		&allocation_desc,
		&desc,
		init_layout,
		clear_ptr,
		0, nullptr,
		allocation,
		IID_NULL, NULL)))
	{
		OutputDebugStringA("D3D12 ERROR: Failed to create texture\n");
		return false;
	}
	set_debug_name((*allocation)->GetResource(), debug_name);
	return true;
}

bool D3D12Context::create_queue(D3D12_COMMAND_LIST_TYPE type, D3D12Queue* queue, const char* debug_name) const
{
    D3D12_COMMAND_QUEUE_DESC desc
    {
        .Type = type,
        .Priority = D3D12_COMMAND_QUEUE_PRIORITY_NORMAL,
        .Flags = D3D12_COMMAND_QUEUE_FLAG_NONE,
        .NodeMask = 0
    };
	if (FAILED(m_device->CreateCommandQueue(&desc, IID_PPV_ARGS(queue->queue.ReleaseAndGetAddressOf()))))
	{
		OutputDebugStringA("D3D12 ERROR: Failed to create command queue\n");
		return false;
	}
	queue->type = type;
	set_debug_name(queue->queue.Get(), debug_name);
	return true;
}

bool D3D12Context::create_command_allocator(D3D12CommandAllocator* cmd_allocator, D3D12Queue* const queue, const char* debug_name) const
{
	if (FAILED(m_device->CreateCommandAllocator(
		queue->type,
		IID_PPV_ARGS(&cmd_allocator->cmd_allocator))))
	{
		OutputDebugStringA("D3D12 ERROR: Failed to create command allocator\n");
		return false;
	}
	cmd_allocator->queue = queue;
	set_debug_name(cmd_allocator->cmd_allocator.Get(), debug_name);
	return true;
}

bool D3D12Context::create_command_list(ID3D12GraphicsCommandList7** cmd_list, const D3D12CommandAllocator& cmd_allocator,
	const char* debug_name) const
{
	if (FAILED(m_device->CreateCommandList(
		0, // Single GPU
		cmd_allocator.queue->type,
		cmd_allocator.cmd_allocator.Get(),
		nullptr, // Initial pipeline state
		IID_PPV_ARGS(cmd_list))))
	{
		OutputDebugStringA("D3D12 ERROR: Failed to create command list\n");
		return false;
	}
	set_debug_name(*cmd_list, debug_name);
	return true;
}

bool D3D12Context::create_fence(D3D12Fence* fence, uint64_t initial_value, const char* debug_name) const
{
	assert(fence);
	if (FAILED(m_device->CreateFence(initial_value, D3D12_FENCE_FLAG_NONE, IID_PPV_ARGS(fence->fence.ReleaseAndGetAddressOf()))))
	{
		OutputDebugStringA("D3D12 ERROR: Failed to create fence\n");
		return false;
	}
	fence->event = CreateEvent(nullptr, FALSE, FALSE, nullptr);
	set_debug_name(fence->fence.Get(), debug_name);
	return true;
}

bool D3D12Context::wait_fences(const WaitInfo& info) const
{
	if (info.count > 16)
	{
		return false;
	}	
	std::array<HANDLE, 16> wait_handles{};
	for (UINT i = 0; i < info.count; i++)
	{
		const auto& d3d12_fence = info.fences[i];
		if (FAILED(d3d12_fence.fence->SetEventOnCompletion(info.values[i], d3d12_fence.event)))
		{
			OutputDebugStringA("D3D12 ERROR: Failed to set event on fence\n");
			return false;
		}
		wait_handles[i] = d3d12_fence.event;
	}
	WaitForMultipleObjectsEx(info.count, wait_handles.data(), info.wait_all, info.timeout, FALSE);
	return true;
}

bool D3D12Context::wait_idle(D3D12Queue* queue)
{
	auto value = m_fence_wait_all.fence->GetCompletedValue() + 1;

	queue->queue->Signal(m_fence_wait_all.fence.Get(), value);

	const WaitInfo wait_info
	{
		.wait_all = true,
		.count = 1,
		.fences = &m_fence_wait_all,
		.values = &value,
		.timeout = INFINITE
	};
	return wait_fences(wait_info);
}

D3D12Context::~D3D12Context()
{
	m_allocator.Reset();
	m_swapchain.Reset();
	m_dxgi_factory.Reset();
	if (m_debug)
	{
		// As long as the only thing left is factory or device it's fine
		m_dxgi_debug->ReportLiveObjects(DXGI_DEBUG_ALL, DXGI_DEBUG_RLO_IGNORE_INTERNAL);
		m_dxgi_debug.Reset();
		m_debug.Reset();
	}
	m_allocator.Reset();
	m_device.Reset();
}
