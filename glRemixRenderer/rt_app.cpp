#include "rt_app.h"

#include "constants.h"
#include "imgui.h"

void glRemix::glRemixRenderer::create()
{
	for (UINT i = 0; i < m_frames_in_flight; i++)
	{
		THROW_IF_FALSE(m_context.create_command_allocator(&m_cmd_pools[i], &m_gfx_queue, "frame command allocator"));
	}

	// Create raster root signature
	{
		// TODO: Make a singular very large root signature that is used for all raster pipelines
        // TODO: Probably want a texture and sampler as this will be for UI probably
		D3D12_ROOT_SIGNATURE_DESC root_sig_desc;
		root_sig_desc.NumParameters = 0;
		root_sig_desc.pParameters = nullptr;
		root_sig_desc.NumStaticSamplers = 0;
		root_sig_desc.pStaticSamplers = nullptr;
		root_sig_desc.Flags = D3D12_ROOT_SIGNATURE_FLAG_ALLOW_INPUT_ASSEMBLER_INPUT_LAYOUT;

		THROW_IF_FALSE(m_context.create_root_signature(root_sig_desc, m_root_signature.ReleaseAndGetAddressOf(), "triangle root signature"));
	}

	// Compile dummy raster pipeline for testing
	ComPtr<IDxcBlobEncoding> vertex_shader;
	THROW_IF_FALSE(m_context.load_blob_from_file(L"./shaders/triangle_vs_6_6_VSMain.dxil", vertex_shader.ReleaseAndGetAddressOf()));
	ComPtr<IDxcBlobEncoding> pixel_shader;
	THROW_IF_FALSE(m_context.load_blob_from_file(L"./shaders/triangle_ps_6_6_PSMain.dxil", pixel_shader.ReleaseAndGetAddressOf()));

	dx::GraphicsPipelineDesc pipeline_desc
	{
		.render_targets
		{
			.num_render_targets = 1,
			.rtv_formats = { DXGI_FORMAT_R8G8B8A8_UNORM },
			.dsv_format = DXGI_FORMAT_UNKNOWN,
		},
	};
	
	pipeline_desc.root_signature = m_root_signature.Get();

	ComPtr<ID3D12ShaderReflection> shader_reflection_interface; // Needs to stay in scope until pipeline is created
	THROW_IF_FALSE(m_context.reflect_input_layout(vertex_shader.Get(), &pipeline_desc.input_layout, false, shader_reflection_interface.ReleaseAndGetAddressOf()));

	THROW_IF_FALSE(m_context.create_graphics_pipeline(pipeline_desc, 
		vertex_shader.Get(), pixel_shader.Get(), 
		m_raster_pipeline.ReleaseAndGetAddressOf(), "raster pipeline"));
	

	// Create raytracing global root signature
	// TODO: Make a singular very large root signature that is used for all ray tracing pipelines
	{
		std::array<D3D12_DESCRIPTOR_RANGE, 3> descriptor_ranges{};
		
		// Acceleration structure (SRV) at t0
		descriptor_ranges[0].RangeType = D3D12_DESCRIPTOR_RANGE_TYPE_SRV;
		descriptor_ranges[0].NumDescriptors = 1;
		descriptor_ranges[0].BaseShaderRegister = 0;
		descriptor_ranges[0].RegisterSpace = 0;
		descriptor_ranges[0].OffsetInDescriptorsFromTableStart = D3D12_DESCRIPTOR_RANGE_OFFSET_APPEND;

		// Output UAV at u0
		descriptor_ranges[1].RangeType = D3D12_DESCRIPTOR_RANGE_TYPE_UAV;
		descriptor_ranges[1].NumDescriptors = 1;
		descriptor_ranges[1].BaseShaderRegister = 0;
		descriptor_ranges[1].RegisterSpace = 0;
		descriptor_ranges[1].OffsetInDescriptorsFromTableStart = D3D12_DESCRIPTOR_RANGE_OFFSET_APPEND;

		// Constant buffer at b0
		descriptor_ranges[2].RangeType = D3D12_DESCRIPTOR_RANGE_TYPE_CBV;
		descriptor_ranges[2].NumDescriptors = 1;
		descriptor_ranges[2].BaseShaderRegister = 0;
		descriptor_ranges[2].RegisterSpace = 0;
		descriptor_ranges[2].OffsetInDescriptorsFromTableStart = D3D12_DESCRIPTOR_RANGE_OFFSET_APPEND;

		std::array<D3D12_ROOT_PARAMETER, 1> root_parameters{};

		// Single table for everything
		root_parameters[0].ParameterType = D3D12_ROOT_PARAMETER_TYPE_DESCRIPTOR_TABLE;
		root_parameters[0].ShaderVisibility = D3D12_SHADER_VISIBILITY_ALL;
		root_parameters[0].DescriptorTable.NumDescriptorRanges = 3;
		root_parameters[0].DescriptorTable.pDescriptorRanges = descriptor_ranges.data();

		D3D12_ROOT_SIGNATURE_DESC root_sig_desc{};
		root_sig_desc.NumParameters = static_cast<UINT>(root_parameters.size());
		root_sig_desc.pParameters = root_parameters.data();
		root_sig_desc.NumStaticSamplers = 0;
		root_sig_desc.pStaticSamplers = nullptr;
		root_sig_desc.Flags = D3D12_ROOT_SIGNATURE_FLAG_NONE;

		THROW_IF_FALSE(m_context.create_root_signature(root_sig_desc, m_rt_global_root_signature.ReleaseAndGetAddressOf(), "rt global root signature"));
	}

	// Compile ray tracing pipeline
	ComPtr<IDxcBlobEncoding> raytracing_shaders;
	THROW_IF_FALSE(m_context.load_blob_from_file(L"./shaders/raytracing_lib_6_6.dxil", raytracing_shaders.ReleaseAndGetAddressOf()));
	dx::RayTracingPipelineDesc rt_desc = dx::make_ray_tracing_pipeline_desc(
		L"RayGenMain",
		L"MissMain",
		L"ClosestHitMain"
		// TODO: Add Any Hit shader
		// TODO: Add Intersection shader if doing non-triangle geometry
	);
	rt_desc.global_root_signature = m_rt_global_root_signature.Get();
	rt_desc.max_recursion_depth = 1;
	// Make sure these match in the shader
	rt_desc.payload_size = sizeof(float) * 4;
	rt_desc.attribute_size = sizeof(float) * 2;
	THROW_IF_FALSE(m_context.create_raytracing_pipeline(rt_desc, raytracing_shaders.Get(), m_rt_pipeline.ReleaseAndGetAddressOf(), "rt pipeline"));

	// Simple geometry for triangle. Use GPU upload heaps to make life easier
	// TODO: Eventually replace with classic CPU -> GPU copy and only use GPU upload heaps for dynamic data
	struct Vertex
	{
		std::array<float, 3> position;
		std::array<float, 3> color;
	};
	std::array<Vertex, 3> triangle_vertices
	{
		{
			{ { 0.0f, 0.5f, 0.0f }, { 1.0f, 0.0f, 0.0f } },
			{ { 0.5f, -0.5f, 0.0f }, { 0.0f, 0.0f, 1.0f } },
			{ { -0.5f, -0.5f, 0.0f }, { 0.0f, 1.0f, 0.0f } }
		}
	};
	std::array<UINT, 3> triangle_indices
	{
		0, 1, 2
	};
	dx::BufferDesc vertex_buffer_desc
	{
		.size = sizeof(Vertex) * triangle_vertices.size(),
		.stride = sizeof(Vertex),
		.visibility = static_cast<dx::BufferVisibility>(dx::CPU | dx::GPU),
	};
	void* cpu_ptr;
	THROW_IF_FALSE(m_context.create_buffer(vertex_buffer_desc, &m_vertex_buffer, "triangle vertex buffer"));

	THROW_IF_FALSE(m_context.map_buffer(&m_vertex_buffer, &cpu_ptr));
	memcpy(cpu_ptr, triangle_vertices.data(), vertex_buffer_desc.size);
	m_context.unmap_buffer(&m_vertex_buffer);

	dx::BufferDesc index_buffer_desc
	{
		.size = sizeof(UINT) * triangle_indices.size(),
		.stride = sizeof(UINT),
		.visibility = static_cast<dx::BufferVisibility>(dx::CPU | dx::GPU),
	};
	THROW_IF_FALSE(m_context.create_buffer(index_buffer_desc, &m_index_buffer, "triangle index buffer"));

	THROW_IF_FALSE(m_context.map_buffer(&m_index_buffer, &cpu_ptr));
	memcpy(cpu_ptr, triangle_indices.data(), index_buffer_desc.size);
	m_context.unmap_buffer(&m_index_buffer);

	dx::BufferDesc raygen_cb_desc
	{
		.size = sizeof(RayGenConstantBuffer),
		.stride = sizeof(RayGenConstantBuffer),
		.visibility = static_cast<dx::BufferVisibility>(dx::CPU | dx::GPU),
	};
	THROW_IF_FALSE(m_context.create_buffer(raygen_cb_desc, &m_raygen_constant_buffer, "raygen constant buffer"));

	// Build BLAS here for now, but renderer will construct them dynamically for new geometry in render loop
	D3D12_RAYTRACING_GEOMETRY_DESC tri_desc
	{
		.Type = D3D12_RAYTRACING_GEOMETRY_TYPE_TRIANGLES,
		.Flags = D3D12_RAYTRACING_GEOMETRY_FLAG_OPAQUE, // Try to use this liberally as its faster
		.Triangles = m_context.get_buffer_rt_description(&m_vertex_buffer, &m_index_buffer),
	};
	D3D12_BUILD_RAYTRACING_ACCELERATION_STRUCTURE_INPUTS blas_desc
	{
		.Type = D3D12_RAYTRACING_ACCELERATION_STRUCTURE_TYPE_BOTTOM_LEVEL,
		// Lots of options here, probably want to use different ones, especially the update one
		.Flags = D3D12_RAYTRACING_ACCELERATION_STRUCTURE_BUILD_FLAG_NONE,
		.NumDescs = 1,
		.DescsLayout = D3D12_ELEMENTS_LAYOUT_ARRAY,
		.pGeometryDescs = &tri_desc,
	};

	const auto prebuild_info = m_context.get_acceleration_structure_prebuild_info(blas_desc);
	// TODO: Compute max size for scratch space and use for all BLAS
	// The same could be done for TLAS(s) as well
	dx::BufferDesc scratch_buffer_desc
	{
		.size = prebuild_info.ScratchDataSizeInBytes,
		.stride = 0,
		.visibility = dx::GPU,
		.uav = true, // Scratch space must be in UAV layout
	};
	THROW_IF_FALSE(m_context.create_buffer(scratch_buffer_desc, &m_scratch_space, "BLAS scratch space"));

	dx::BufferDesc blas_buffer_desc
	{
		.size = prebuild_info.ResultDataMaxSizeInBytes,
		.stride = 0,
		.visibility = dx::GPU,
		.acceleration_structure = true,
	};
	THROW_IF_FALSE(m_context.create_buffer(blas_buffer_desc, &m_blas_buffer, "BLAS buffer"));
	
	const auto build_desc = m_context.get_raytracing_acceleration_structure(blas_desc, &m_blas_buffer, nullptr, &m_scratch_space);

	// Record transitions, copies
	THROW_IF_FALSE(SUCCEEDED(m_cmd_pools[get_frame_index()].cmd_allocator->Reset()));
	ComPtr<ID3D12GraphicsCommandList7> cmd_list;
	THROW_IF_FALSE(m_context.create_command_list(cmd_list.ReleaseAndGetAddressOf(), m_cmd_pools[get_frame_index()]));

	// Transition scratch space to UAV format
	D3D12_BUFFER_BARRIER scratch_space_uav
	{
		.SyncBefore = D3D12_BARRIER_SYNC_NONE,
		.SyncAfter = D3D12_BARRIER_SYNC_BUILD_RAYTRACING_ACCELERATION_STRUCTURE,
		.AccessBefore = D3D12_BARRIER_ACCESS_NO_ACCESS,
		.AccessAfter = D3D12_BARRIER_ACCESS_UNORDERED_ACCESS,
		.pResource = m_scratch_space.allocation->GetResource(),
		.Offset = 0,
		.Size = UINT64_MAX,
	};

	D3D12_BUFFER_BARRIER blas_buffer_barrier
	{
		.SyncBefore = D3D12_BARRIER_SYNC_NONE,
		.SyncAfter = D3D12_BARRIER_SYNC_BUILD_RAYTRACING_ACCELERATION_STRUCTURE,
		.AccessBefore = D3D12_BARRIER_ACCESS_NO_ACCESS,
		.AccessAfter = D3D12_BARRIER_ACCESS_RAYTRACING_ACCELERATION_STRUCTURE_WRITE,
		.pResource = m_blas_buffer.allocation->GetResource(),
		.Offset = 0,
		.Size = UINT64_MAX,
	};

	std::array buffer_barriers{ scratch_space_uav, blas_buffer_barrier };

	D3D12_BARRIER_GROUP barrier_group
	{
		.Type = D3D12_BARRIER_TYPE_BUFFER,
		.NumBarriers = static_cast<UINT>(buffer_barriers.size()),
		.pBufferBarriers = buffer_barriers.data(),
	};

	cmd_list->Barrier(1, &barrier_group);

	cmd_list->BuildRaytracingAccelerationStructure(&build_desc, 0, nullptr);

	// TODO: Whenever buffers are moved to CPU -> GPU copy method instead of GPU upload heaps record the copy here as well

	THROW_IF_FALSE(SUCCEEDED(cmd_list->Close()));
	const std::array<ID3D12CommandList*, 1> lists = { cmd_list.Get() };
	m_gfx_queue.queue->ExecuteCommandLists(1, lists.data());

	// Signal fence and wait for GPU to finish
	auto current_fence_value = ++m_fence_frame_ready_val[get_frame_index()];
	m_gfx_queue.queue->Signal(m_fence_frame_ready.fence.Get(), current_fence_value);

	dx::WaitInfo wait_info
	{
		.wait_all = true,
		.count = 1,
		.fences = &m_fence_frame_ready,
		.values = &current_fence_value,
		.timeout = INFINITE,
	};

	// Init ImGui
	THROW_IF_FALSE(m_context.init_imgui());

	THROW_IF_FALSE(m_context.wait_fences(wait_info)); // Block CPU until done
}

void glRemix::glRemixRenderer::render()
{
	// Start ImGui frame
	m_context.start_imgui_frame();
	ImGui::ShowDemoWindow();

	// Be careful not to call the ID3D12Interface reset instead
	THROW_IF_FALSE(SUCCEEDED(m_cmd_pools[get_frame_index()].cmd_allocator->Reset()));

	// Create a command list in the open state
	ComPtr<ID3D12GraphicsCommandList7> cmd_list;
	THROW_IF_FALSE(m_context.create_command_list(cmd_list.ReleaseAndGetAddressOf(), m_cmd_pools[get_frame_index()]));

	const auto swapchain_idx = m_context.get_swapchain_index();

	// Transition swapchain image
	{
		D3D12_TEXTURE_BARRIER swapchain_render
		{
			.SyncBefore = D3D12_BARRIER_SYNC_DRAW, // Ensure we are not drawing anything to swapchain
			.SyncAfter = D3D12_BARRIER_SYNC_RENDER_TARGET, // Setting swapchain as render target requires transition to finish first

			.AccessBefore = D3D12_BARRIER_ACCESS_COMMON,
			.AccessAfter = D3D12_BARRIER_ACCESS_RENDER_TARGET,

			.LayoutBefore = D3D12_BARRIER_LAYOUT_PRESENT,
			.LayoutAfter = D3D12_BARRIER_LAYOUT_RENDER_TARGET,

			.Flags = D3D12_TEXTURE_BARRIER_FLAG_NONE // Or discard
		};
		m_context.set_barrier_swapchain(&swapchain_render);
		D3D12_BARRIER_GROUP barrier_group =
		{
			.Type = D3D12_BARRIER_TYPE_TEXTURE,
			.NumBarriers = 1,
			.pTextureBarriers = &swapchain_render,
		};
		cmd_list->Barrier(1, &barrier_group);
	}

	XMUINT2 win_dims{};
	m_context.get_window_dimensions(&win_dims);

	// Set viewport, scissor
	const D3D12_VIEWPORT viewport
	{
		.TopLeftX = 0,
		.TopLeftY = 0,
		.Width = static_cast<float>(win_dims.x),
		.Height = static_cast<float>(win_dims.y),
		.MinDepth = 0.0f,
		.MaxDepth = 1.0f,
	};
	const D3D12_RECT scissor_rect
	{
		.left = 0,
		.top = 0,
		.right = static_cast<LONG>(win_dims.x),
		.bottom = static_cast<LONG>(win_dims.y),
	};
	cmd_list->RSSetViewports(1, &viewport);
	cmd_list->RSSetScissorRects(1, &scissor_rect);

	// Build TLAS

	// Draw everything
	D3D12_CPU_DESCRIPTOR_HANDLE swapchain_rtv{};
	m_swapchain_descriptors.heap->get_cpu_descriptor(&swapchain_rtv, m_swapchain_descriptors.offset + swapchain_idx);
	const std::array clear_color = { 1.0f, 0.0f, 0.0f, 1.0f };
	cmd_list->OMSetRenderTargets(1, &swapchain_rtv, FALSE, nullptr);
	cmd_list->ClearRenderTargetView(swapchain_rtv, clear_color.data(), 0, nullptr);

	// This is where rasterization will go

	// Render ImGui
	m_context.render_imgui_draw_data(cmd_list.Get());

	// Transition swapchain image to present
	{
		D3D12_TEXTURE_BARRIER swapchain_present
		{
			.SyncBefore = D3D12_BARRIER_SYNC_DRAW, // Ensure we are not drawing anything to swapchain
			.SyncAfter = D3D12_BARRIER_SYNC_NONE, // Setting swapchain as render target requires transition to finish first

			.AccessBefore = D3D12_BARRIER_ACCESS_RENDER_TARGET,
			.AccessAfter = D3D12_BARRIER_ACCESS_NO_ACCESS,

			.LayoutBefore = D3D12_BARRIER_LAYOUT_RENDER_TARGET,
			.LayoutAfter = D3D12_BARRIER_LAYOUT_PRESENT,

			.Flags = D3D12_TEXTURE_BARRIER_FLAG_NONE // Or discard
		};
		m_context.set_barrier_swapchain(&swapchain_present);
		D3D12_BARRIER_GROUP barrier_group =
		{
			.Type = D3D12_BARRIER_TYPE_TEXTURE,
			.NumBarriers = 1,
			.pTextureBarriers = &swapchain_present,
		};
		cmd_list->Barrier(1, &barrier_group);
	}
		
	// Submit the command list
	auto current_fence_value = ++m_fence_frame_ready_val[get_frame_index()]; // Increment wait value
	THROW_IF_FALSE(SUCCEEDED(cmd_list->Close()));
	const std::array<ID3D12CommandList*, 1> lists = { cmd_list.Get() };
	m_gfx_queue.queue->ExecuteCommandLists(1, lists.data());

	// End of all work for queue, signal fence
	m_gfx_queue.queue->Signal(m_fence_frame_ready.fence.Get(), current_fence_value);

	// Present
	m_context.present();

	increment_frame_index();

	// If next frame is ready to be used, otherwise wait
	if (m_fence_frame_ready.fence->GetCompletedValue() < m_fence_frame_ready_val[get_frame_index()])
	{
		dx::WaitInfo wait_info
		{
			.wait_all = true,
			.count = 1,
			.fences = &m_fence_frame_ready,
			.values = &current_fence_value,
			.timeout = INFINITE
		};
		THROW_IF_FALSE(m_context.wait_fences(wait_info));
	}
	m_fence_frame_ready_val[get_frame_index()] = current_fence_value + 1;
}

void glRemix::glRemixRenderer::destroy()
{
	m_context.destroy_imgui();
}

glRemix::glRemixRenderer::~glRemixRenderer()
{

}
