#include "rt_app.h"

#include "constants.h"
#include "helper.h"
#include "imgui.h"

void glRemix::glRemixRenderer::create()
{
	for (UINT i = 0; i < m_frames_in_flight; i++)
	{
		THROW_IF_FALSE(m_context.create_command_allocator(&m_cmd_pools[i], &m_gfx_queue, "frame command allocator"));
	}

	// Create raster root signature
	{
        D3D12_ROOT_PARAMETER root_params[1]{};
        root_params[0].ParameterType = D3D12_ROOT_PARAMETER_TYPE_CBV;
        root_params[0].ShaderVisibility = D3D12_SHADER_VISIBILITY_VERTEX;
        root_params[0].Descriptor.ShaderRegister = 0;
        root_params[0].Descriptor.RegisterSpace = 0;

		// TODO: Make a singular very large root signature that is used for all raster pipelines
        // TODO: Probably want a texture and sampler as this will be for UI probably
		D3D12_ROOT_SIGNATURE_DESC root_sig_desc;
		root_sig_desc.NumParameters = 1;
		root_sig_desc.pParameters = root_params;
		root_sig_desc.NumStaticSamplers = 0;
		root_sig_desc.pStaticSamplers = nullptr;
		root_sig_desc.Flags = D3D12_ROOT_SIGNATURE_FLAG_ALLOW_INPUT_ASSEMBLER_INPUT_LAYOUT;

		THROW_IF_FALSE(m_context.create_root_signature(root_sig_desc, m_root_signature.ReleaseAndGetAddressOf(), "triangle root signature"));
	}
	// create mvp buffer
	dx::BufferDesc mvpDesc
	{
		.size = sizeof(MVP),
		.stride = 0,
		.visibility = static_cast<dx::BufferVisibility>(dx::CPU | dx::GPU),
	};
	THROW_IF_FALSE(m_context.create_buffer(mvpDesc, &m_mvp, "MVP buffer"));

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

	// Create ray tracing descriptor heap for TLAS SRV, Output UAV, and constant buffer
	{
		D3D12_DESCRIPTOR_HEAP_DESC descriptor_heap_desc
		{
			.Type = D3D12_DESCRIPTOR_HEAP_TYPE_CBV_SRV_UAV,
			.NumDescriptors = 3, // TLAS SRV, Output UAV, Raygen CB
			.Flags = D3D12_DESCRIPTOR_HEAP_FLAG_SHADER_VISIBLE,
		};
		THROW_IF_FALSE(m_context.create_descriptor_heap(descriptor_heap_desc, &m_rt_descriptor_heap, "ray tracing descriptor heap"));

		// Allocate descriptor table for all ray tracing descriptors (views will be created after resources are ready)
		THROW_IF_FALSE(m_rt_descriptor_heap.allocate(3, &m_rt_descriptors));
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

	// Create shader table buffer for ray tracing pipeline
	{
		ComPtr<ID3D12StateObjectProperties> rt_pipeline_properties;
		THROW_IF_FALSE(SUCCEEDED(m_rt_pipeline->QueryInterface(IID_PPV_ARGS(rt_pipeline_properties.ReleaseAndGetAddressOf()))));

		constexpr UINT64 shader_identifier_size = D3D12_SHADER_IDENTIFIER_SIZE_IN_BYTES;
		constexpr UINT64 shader_table_alignment = D3D12_RAYTRACING_SHADER_TABLE_BYTE_ALIGNMENT;
		
		// Calculate aligned offsets for each table
		m_raygen_shader_table_offset = 0;
		m_miss_shader_table_offset = align_u64(shader_identifier_size, shader_table_alignment);
		m_hit_group_shader_table_offset = align_u64(m_miss_shader_table_offset + shader_identifier_size, shader_table_alignment);
		
		UINT64 total_shader_table_size = align_u64(m_hit_group_shader_table_offset + shader_identifier_size, shader_table_alignment);

		// Create single buffer for all shader tables
		dx::BufferDesc shader_table_desc
		{
			.size = total_shader_table_size,
			.stride = 0,
			.visibility = static_cast<dx::BufferVisibility>(dx::CPU | dx::GPU),
		};
		THROW_IF_FALSE(m_context.create_buffer(shader_table_desc, &m_shader_table, "shader tables"));

		// Fill out shader table
		void* cpu_ptr;
		THROW_IF_FALSE(m_context.map_buffer(&m_shader_table, &cpu_ptr));

		void* raygen_identifier = rt_pipeline_properties->GetShaderIdentifier(L"RayGenMain");
		assert(raygen_identifier);
		memcpy(static_cast<UINT8*>(cpu_ptr) + m_raygen_shader_table_offset, raygen_identifier, shader_identifier_size);

		void* miss_identifier = rt_pipeline_properties->GetShaderIdentifier(L"MissMain");
		assert(miss_identifier);
		memcpy(static_cast<UINT8*>(cpu_ptr) + m_miss_shader_table_offset, miss_identifier, shader_identifier_size);

		void* hit_group_identifier = rt_pipeline_properties->GetShaderIdentifier(L"HG_Default");
		assert(hit_group_identifier);
		memcpy(static_cast<UINT8*>(cpu_ptr) + m_hit_group_shader_table_offset, hit_group_identifier, shader_identifier_size);
		
		m_context.unmap_buffer(&m_shader_table);
	}
  
	dx::D3D12Buffer t_vertex_buffer{};
	dx::D3D12Buffer t_index_buffer{};
    std::array<Vertex, 3> triangle_vertices{{{{0.0f, 0.5f, 0.0f}, {1.0f, 0.0f, 0.0f}},
                                             {{0.5f, -0.5f, 0.0f}, {0.0f, 0.0f, 1.0f}},
                                             {{-0.5f, -0.5f, 0.0f}, {0.0f, 1.0f, 0.0f}}}};
    std::array<UINT, 3> triangle_indices{0, 1, 2};
    dx::BufferDesc vertex_buffer_desc{
        .size = sizeof(Vertex) * triangle_vertices.size(),
        .stride = sizeof(Vertex),
        .visibility = static_cast<dx::BufferVisibility>(dx::CPU | dx::GPU),
    };
    void* cpu_ptr;
    THROW_IF_FALSE(
        m_context.create_buffer(vertex_buffer_desc, &t_vertex_buffer, "triangle vertex buffer"));

    THROW_IF_FALSE(m_context.map_buffer(&t_vertex_buffer, &cpu_ptr));
    memcpy(cpu_ptr, triangle_vertices.data(), vertex_buffer_desc.size);
    m_context.unmap_buffer(&t_vertex_buffer);

    dx::BufferDesc index_buffer_desc{
        .size = sizeof(UINT) * triangle_indices.size(),
        .stride = sizeof(UINT),
        .visibility = static_cast<dx::BufferVisibility>(dx::CPU | dx::GPU),
    };
    THROW_IF_FALSE(
        m_context.create_buffer(index_buffer_desc, &t_index_buffer, "triangle index buffer"));

    THROW_IF_FALSE(m_context.map_buffer(&t_index_buffer, &cpu_ptr));
    memcpy(cpu_ptr, triangle_indices.data(), index_buffer_desc.size);
    m_context.unmap_buffer(&t_index_buffer);

    dx::BufferDesc raygen_cb_desc
	{
		.size = align_u32(sizeof(RayGenConstantBuffer), 256),
		.stride = align_u32(sizeof(RayGenConstantBuffer), 256),
		.visibility = static_cast<dx::BufferVisibility>(dx::CPU | dx::GPU),
	};
	for (UINT i = 0; i < m_frames_in_flight; i++)
	{
		THROW_IF_FALSE(m_context.create_buffer(raygen_cb_desc, &m_raygen_constant_buffers[i], "raygen constant buffer"));
	}

	// Create UAV render target
	XMUINT2 win_dims{};
	THROW_IF_FALSE(m_context.get_window_dimensions(&win_dims));
	
	D3D12_RESOURCE_DESC1 uav_rt_desc
	{
		.Dimension = D3D12_RESOURCE_DIMENSION_TEXTURE2D,
		.Width = win_dims.x,
		.Height = win_dims.y,
		.DepthOrArraySize = 1,
		.MipLevels = 1,
		.Format = DXGI_FORMAT_R8G8B8A8_UNORM,
		.SampleDesc
		{
			.Count = 1,
			.Quality = 0,
		},
		.Layout = D3D12_TEXTURE_LAYOUT_UNKNOWN,
		.Flags = D3D12_RESOURCE_FLAG_ALLOW_UNORDERED_ACCESS,
	};

	dx::TextureCreateDesc uav_rt_create_desc
	{
		uav_rt_create_desc.init_layout = D3D12_BARRIER_LAYOUT_UNORDERED_ACCESS,
	};

	THROW_IF_FALSE(m_context.create_texture(uav_rt_desc, m_uav_render_target.ReleaseAndGetAddressOf(), uav_rt_create_desc, "UAV and RT texture"));

	// Build BLAS here for now, but renderer will construct them dynamically for new geometry in render loop
	D3D12_RAYTRACING_GEOMETRY_DESC tri_desc
	{
		.Type = D3D12_RAYTRACING_GEOMETRY_TYPE_TRIANGLES,
		.Flags = D3D12_RAYTRACING_GEOMETRY_FLAG_OPAQUE, // Try to use this liberally as its faster
		.Triangles = m_context.get_buffer_rt_description(&t_vertex_buffer, &t_index_buffer),
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

	const auto blas_prebuild_info = m_context.get_acceleration_structure_prebuild_info(blas_desc);
	// TODO: Reuse scratch space for all BLAS
	// The same could be done for TLAS(s) as well

	constexpr UINT64 scratch_size = 16 * 1024 * 1024;
	assert(blas_prebuild_info.ScratchDataSizeInBytes < scratch_size);
	dx::BufferDesc scratch_buffer_desc
	{
		.size = scratch_size,
		.stride = 0,
		.visibility = dx::GPU,
		.uav = true, // Scratch space must be in UAV layout
	};
	THROW_IF_FALSE(m_context.create_buffer(scratch_buffer_desc, &m_scratch_space, "BLAS scratch space"));

	dx::BufferDesc blas_buffer_desc
	{
		.size = blas_prebuild_info.ResultDataMaxSizeInBytes,
		.stride = 0,
		.visibility = dx::GPU,
		.acceleration_structure = true,
	};
	THROW_IF_FALSE(m_context.create_buffer(blas_buffer_desc, &m_blas_buffer, "BLAS buffer"));
	
	const auto blas_build_desc = m_context.get_raytracing_acceleration_structure(blas_desc, &m_blas_buffer, nullptr, &m_scratch_space);

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

	D3D12_BARRIER_GROUP barrier_group0
	{
		.Type = D3D12_BARRIER_TYPE_BUFFER,
		.NumBarriers = static_cast<UINT>(buffer_barriers.size()),
		.pBufferBarriers = buffer_barriers.data(),
	};

	cmd_list->Barrier(1, &barrier_group0);

	cmd_list->BuildRaytracingAccelerationStructure(&blas_build_desc, 0, nullptr);

	// TODO: It would be convenient if resources tracked their current state for barriers
	// Transition BLAS to read state
	blas_buffer_barrier =
	{
		.SyncBefore = D3D12_BARRIER_SYNC_BUILD_RAYTRACING_ACCELERATION_STRUCTURE,
		.SyncAfter = D3D12_BARRIER_SYNC_RAYTRACING,
		.AccessBefore = D3D12_BARRIER_ACCESS_RAYTRACING_ACCELERATION_STRUCTURE_WRITE,
		.AccessAfter = D3D12_BARRIER_ACCESS_RAYTRACING_ACCELERATION_STRUCTURE_READ,
		.pResource = m_blas_buffer.allocation->GetResource(),
		.Offset = 0,
		.Size = UINT64_MAX,
	};
	D3D12_BARRIER_GROUP barrier_group_read
	{
		.Type = D3D12_BARRIER_TYPE_BUFFER,
		.NumBarriers = 1,
		.pBufferBarriers = &blas_buffer_barrier,
	};
	cmd_list->Barrier(1, &barrier_group_read);

	// Instance of the BLAS for TLAS
	D3D12_RAYTRACING_INSTANCE_DESC instance_desc
	{
		.Transform
		{
			// Identity matrix
			1.0f, 0.0f, 0.0f, 0.0f,
			0.0f, 1.0f, 0.0f, 0.0f,
			0.0f, 0.0f, 1.0f, 0.0f,
		},
		.InstanceID = 0,
		.InstanceMask = 0xFF,
		.InstanceContributionToHitGroupIndex = 0,
		.Flags = D3D12_RAYTRACING_INSTANCE_FLAG_NONE,
		.AccelerationStructure = m_blas_buffer.allocation->GetResource()->GetGPUVirtualAddress(),
	};

	dx::BufferDesc instance_buffer_desc
	{
		.size = sizeof(D3D12_RAYTRACING_INSTANCE_DESC),
		.stride = sizeof(D3D12_RAYTRACING_INSTANCE_DESC),
		.visibility = static_cast<dx::BufferVisibility>(dx::CPU | dx::GPU),
	};
	THROW_IF_FALSE(m_context.create_buffer(instance_buffer_desc, &m_tlas.instance, "TLAS instance buffer"));

	THROW_IF_FALSE(m_context.map_buffer(&m_tlas.instance, &cpu_ptr));
	memcpy(cpu_ptr, &instance_desc, instance_buffer_desc.size);
	m_context.unmap_buffer(&m_tlas.instance);

	D3D12_BUILD_RAYTRACING_ACCELERATION_STRUCTURE_INPUTS tlas_desc
	{
		.Type = D3D12_RAYTRACING_ACCELERATION_STRUCTURE_TYPE_TOP_LEVEL,
		// Lots of options here, probably want to use different ones, especially the update one
		.Flags = D3D12_RAYTRACING_ACCELERATION_STRUCTURE_BUILD_FLAG_NONE,
		.NumDescs = 1,
		.InstanceDescs = m_tlas.instance.allocation.Get()->GetResource()->GetGPUVirtualAddress(),
	};

	const auto tlas_prebuild_info = m_context.get_acceleration_structure_prebuild_info(tlas_desc);
	assert(tlas_prebuild_info.ScratchDataSizeInBytes < scratch_size);
	dx::BufferDesc tlas_buffer_desc
	{
		.size = tlas_prebuild_info.ResultDataMaxSizeInBytes,
		.stride = 0,
		.visibility = dx::GPU,
		.acceleration_structure = true,
	};
	THROW_IF_FALSE(m_context.create_buffer(tlas_buffer_desc, &m_tlas.buffer, "TLAS buffer"));

	D3D12_BUFFER_BARRIER tlas_buffer_barrier
	{
		.SyncBefore = D3D12_BARRIER_SYNC_NONE,
		.SyncAfter = D3D12_BARRIER_SYNC_BUILD_RAYTRACING_ACCELERATION_STRUCTURE,
		.AccessBefore = D3D12_BARRIER_ACCESS_NO_ACCESS,
		.AccessAfter = D3D12_BARRIER_ACCESS_RAYTRACING_ACCELERATION_STRUCTURE_WRITE,
		.pResource = m_tlas.buffer.allocation->GetResource(),
		.Offset = 0,
		.Size = UINT64_MAX,
	};
	D3D12_BARRIER_GROUP barrier_group_tlas
	{
		.Type = D3D12_BARRIER_TYPE_BUFFER,
		.NumBarriers = 1,
		.pBufferBarriers = &tlas_buffer_barrier,
	};
	cmd_list->Barrier(1, &barrier_group_tlas);

	const auto tlas_build_desc = m_context.get_raytracing_acceleration_structure(tlas_desc, &m_tlas.buffer, nullptr, &m_scratch_space);

	cmd_list->BuildRaytracingAccelerationStructure(&tlas_build_desc, 0, nullptr);

	// Transition TLAS to read state after build
	D3D12_BUFFER_BARRIER tlas_buffer_barrier_read
	{
		.SyncBefore = D3D12_BARRIER_SYNC_BUILD_RAYTRACING_ACCELERATION_STRUCTURE,
		.SyncAfter = D3D12_BARRIER_SYNC_RAYTRACING,
		.AccessBefore = D3D12_BARRIER_ACCESS_RAYTRACING_ACCELERATION_STRUCTURE_WRITE,
		.AccessAfter = D3D12_BARRIER_ACCESS_RAYTRACING_ACCELERATION_STRUCTURE_READ,
		.pResource = m_tlas.buffer.allocation->GetResource(),
		.Offset = 0,
		.Size = UINT64_MAX,
	};

	D3D12_BARRIER_GROUP tlas_barrier_group_read
	{
		.Type = D3D12_BARRIER_TYPE_BUFFER,
		.NumBarriers = 1,
		.pBufferBarriers = &tlas_buffer_barrier_read,
	};
	cmd_list->Barrier(1, &tlas_barrier_group_read);

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

	// Create descriptor views after all resources are ready
	{
		XMUINT2 win_dims_for_desc{};
		m_context.get_window_dimensions(&win_dims_for_desc);

		// TODO: Use CPU descriptors and copy to GPU visible heap

		m_context.create_shader_resource_view_acceleration_structure(m_tlas.buffer.allocation->GetResource(), &m_rt_descriptors, 0);

		m_context.create_unordered_access_view_texture(m_uav_render_target.Get(), uav_rt_desc.Format, &m_rt_descriptors, 1);

		// Will be updated each frame
		m_context.create_constant_buffer_view(&m_raygen_constant_buffers[0], &m_rt_descriptors, 2);
	}

	// Init ImGui
	THROW_IF_FALSE(m_context.init_imgui());

	THROW_IF_FALSE(m_context.wait_fences(wait_info)); // Block CPU until done
}

// Reads an incoming stream of OpenGL commands and launches corresponding renderer functions
void glRemix::glRemixRenderer::read_gl_command_stream()
{
    if (!m_ipc.InitReader())
    {
        std::cout << "IPC Manager could not init a reader instance. Have you launched an "
                     "executable that utilizes glRemix?"
                  << std::endl;
        return;
    }

    const uint32_t bufCapacity = m_ipc.GetCapacity();  // ask shared mem for capacity
    std::vector<uint8_t> ipcBuf(bufCapacity);          // decoupled local buffer here

    uint32_t bytesRead = 0;
    if (!m_ipc.TryConsumeFrame(ipcBuf.data(), static_cast<uint32_t>(ipcBuf.size()), &bytesRead))
    {
        std::cout << "No frame data available." << std::endl;
        return;
    }

    const auto* frameHeader = reinterpret_cast<const glRemix::GLFrameUnifs*>(ipcBuf.data());

    if (frameHeader->payloadSize == 0)
    {
        //std::cout << "Frame " << frameHeader->frameIndex << ": no new commands.\n";
		return;
    }

	// for acceleration structure building
	THROW_IF_FALSE(SUCCEEDED(m_cmd_pools[get_frame_index()].cmd_allocator->Reset()));
	ComPtr<ID3D12GraphicsCommandList7> cmd_list;
	THROW_IF_FALSE(m_context.create_command_list(cmd_list.ReleaseAndGetAddressOf(), m_cmd_pools[get_frame_index()]));

    // loop through data from frame
    // can comment out the logs here too
	std::vector<Vertex> vertices;
	std::vector<uint32_t> indices;
    size_t offset = sizeof(glRemix::GLFrameUnifs); // offset skips header of ipc data payload
    while (offset + sizeof(glRemix::GLCommandUnifs) <= bytesRead) // while header + additional gl header is not at byte limit
    {
        const auto* header = reinterpret_cast<const glRemix::GLCommandUnifs*>(ipcBuf.data() // get most recent header
                                                                              + offset);
        offset += sizeof(glRemix::GLCommandUnifs); // move into data payload

        std::cout << "  Command: " << static_cast<int>(header->type)
                  << " (size: " << header->dataSize << ")" << std::endl;

        switch (header->type)
        {
			// if we encounter GL_BEGIN we know that a new geometry is to be created
			case glRemix::GLCommandType::GL_BEGIN: 
			{
				const auto* type = reinterpret_cast<const glRemix::GLBeginCommand*>(ipcBuf.data() + offset); // reach into data payload
				offset += header->dataSize; // move past data to next command
				read_geometry(ipcBuf, offset, vertices, indices, static_cast<glRemix::GLTopology>(type->mode), bytesRead, cmd_list); // store geometry data in vertex buffers depending on topology type
				break;
			}
            default:
			{
				 std::cout << "    (Unhandled base command)" << std::endl; 
				 return;
				 break;
			}
        }
    }

	// set vertex and index buffers
	dx::BufferDesc vertex_buffer_desc{
        .size = sizeof(Vertex) * vertices.size(),
        .stride = sizeof(Vertex),
        .visibility = static_cast<dx::BufferVisibility>(dx::CPU | dx::GPU),
    };
    void* cpu_ptr;
    THROW_IF_FALSE(
        m_context.create_buffer(vertex_buffer_desc, &m_vertex_buffer, "vertex buffer"));

    THROW_IF_FALSE(m_context.map_buffer(&m_vertex_buffer, &cpu_ptr));
    memcpy(cpu_ptr, vertices.data(), vertex_buffer_desc.size);
    m_context.unmap_buffer(&m_vertex_buffer);

    dx::BufferDesc index_buffer_desc{
        .size = sizeof(UINT) * indices.size(),
        .stride = sizeof(UINT),
        .visibility = static_cast<dx::BufferVisibility>(dx::CPU | dx::GPU),
    };
    THROW_IF_FALSE(
        m_context.create_buffer(index_buffer_desc, &m_index_buffer, "index buffer"));

    THROW_IF_FALSE(m_context.map_buffer(&m_index_buffer, &cpu_ptr));
    memcpy(cpu_ptr, indices.data(), index_buffer_desc.size);
    m_context.unmap_buffer(&m_index_buffer);

	constexpr UINT64 scratch_size = 16 * 1024 * 1024;

	// build blasses per mesh
	const UINT instance_count = (UINT)m_meshes.size();
	for (int i = 0; i < instance_count; i++)
	{
		MeshRecord mesh = m_meshes[i];
		m_meshes[i].blasID = build_mesh_blas(mesh.vertexCount, mesh.vertexOffset, mesh.indexCount, mesh.indexOffset, cmd_list);
	}

	build_tlas(cmd_list); // builds top level acceleration structure with blas buffer (can be called each frame likely)

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

	m_context.create_shader_resource_view_acceleration_structure(m_tlas.buffer.allocation->GetResource(), &m_rt_descriptors, 0);
	THROW_IF_FALSE(m_context.wait_fences(wait_info)); // Block CPU until done

	return;
}

// adds to vertex and index buffers depending on topology type
void glRemix::glRemixRenderer::read_geometry(std::vector<uint8_t>& ipcBuf,
                                            size_t& offset,
                                            std::vector<Vertex>& vertices,
                                            std::vector<uint32_t>& indices,
                                            glRemix::GLTopology topology,
                                            uint32_t bytesRead,
											ComPtr<ID3D12GraphicsCommandList7> cmd_list)
{
    bool endPrimitive = false;
    int verticesSize = vertices.size();

	MeshRecord mesh; // glBegin indicates start of new mesh
	mesh.vertexOffset = verticesSize;
	mesh.indexOffset = indices.size();

    while (offset + sizeof(glRemix::GLCommandUnifs) <= bytesRead)
    {
        const auto* header = reinterpret_cast<const glRemix::GLCommandUnifs*>(
            ipcBuf.data()  // get most recent header
            + offset);

        std::cout << "  Command: " << static_cast<int>(header->type)
                  << " (size: " << header->dataSize << ")" << std::endl;

        offset += sizeof(glRemix::GLCommandUnifs);  // move into data payload

        switch (header->type)
        {
            case glRemix::GLCommandType::GL_VERTEX3F: {
                const auto* v = reinterpret_cast<const glRemix::GLVertex3fCommand*>(ipcBuf.data()
                                                                                    + offset);
                Vertex vertex{{v->x, v->y, v->z}, {1.0f, 1.0f, 1.0f}};
                vertices.push_back(vertex);
                break;
            }
            case glRemix::GLCommandType::GL_END: // read vertices until GL_END is encountered at which point we will have reached end of geomtry
			{
                endPrimitive = true;
                break;
            }
            default: std::cout << "    (Unhandled primitive command)" << std::endl; break;
        }

        offset += header->dataSize;  // move past data to next command

        if (endPrimitive)
        {
            break;
        }
    }

	// determine indices based on specified topology
	// push absolute indices into index buffer
	uint32_t base = verticesSize;
	uint32_t numIndices = 0;
    if (topology == glRemix::GLTopology::GL_QUAD_STRIP)
    {
        for (uint32_t k = verticesSize; k + 3 < vertices.size(); k += 2)
        {
            uint32_t a = k + 0;
            uint32_t b = k + 1;
            uint32_t c = k + 2;
            uint32_t d = k + 3;

            indices.push_back(a - base);
            indices.push_back(b - base);
            indices.push_back(d - base);
            indices.push_back(a - base);
            indices.push_back(d - base);
            indices.push_back(c - base);

			numIndices += 6;
        }
    } else if (topology == glRemix::GLTopology::GL_QUADS)
    {
        for (uint32_t k = verticesSize; k + 3 < vertices.size(); k += 4)
        {
            uint32_t a = k + 0;
            uint32_t b = k + 1;
            uint32_t c = k + 2;
            uint32_t d = k + 3;

            indices.push_back(a - base);
            indices.push_back(b - base);
            indices.push_back(c - base);
            indices.push_back(a - base);
            indices.push_back(c - base);
            indices.push_back(d - base);

			numIndices += 6;
        }
    }

	// record mesh
	mesh.vertexCount = vertices.size() - verticesSize;
	mesh.indexCount = indices.size() - mesh.indexOffset;

	// hashing - logic from boost::hash_combine
	size_t seed = 0;
	auto hash_combine = [&seed](auto const& v) {
		seed ^= std::hash<std::decay_t<decltype(v)>>{}(v) + 0x9e3779b97f4a7c15ULL + (seed << 6) + (seed >> 2);
	};

	// reduces floating point instability
	auto quantize = [](float v, float precision = 1e-5f) -> float {
		return std::round(v / precision) * precision;
	};

	// get vertex data to hash
	for (int i = 0; i < mesh.vertexCount; ++i) {
		const Vertex& vertex = vertices[mesh.vertexOffset + i];
		hash_combine(quantize(vertex.position[0]));
		hash_combine(quantize(vertex.position[1]));
		hash_combine(quantize(vertex.position[2]));
		hash_combine(quantize(vertex.color[0]));
		hash_combine(quantize(vertex.color[1]));
		hash_combine(quantize(vertex.color[2]));
	}

	// get index data to hash
	for (int i = 0; i < mesh.indexCount; ++i) {
		const uint32_t& index = indices[mesh.indexOffset + i];
		hash_combine(index);
	}

	mesh.meshId = static_cast<uint64_t>(seed); // set hash to meshID

	m_meshes.push_back(mesh);
}


int glRemix::glRemixRenderer::build_mesh_blas(uint32_t vertex_count, uint32_t vertex_offset, uint32_t index_count, uint32_t index_offset, ComPtr<ID3D12GraphicsCommandList7> cmd_list)
{
	dx::D3D12Buffer t_blas_buffer;

	// Build BLAS here for now, but renderer will construct them dynamically for new geometry in render loop
	D3D12_RAYTRACING_GEOMETRY_DESC tri_desc
	{
		.Type = D3D12_RAYTRACING_GEOMETRY_TYPE_TRIANGLES,
		.Flags = D3D12_RAYTRACING_GEOMETRY_FLAG_OPAQUE, // Try to use this liberally as its faster
		.Triangles = m_context.get_buffer_rt_description_subrange(&m_vertex_buffer, vertex_count, vertex_offset, &m_index_buffer, index_count, index_offset),
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

	const auto blas_prebuild_info = m_context.get_acceleration_structure_prebuild_info(blas_desc);
	// TODO: Reuse scratch space for all BLAS
	// The same could be done for TLAS(s) as well

	constexpr UINT64 scratch_size = 16 * 1024 * 1024;
	assert(blas_prebuild_info.ScratchDataSizeInBytes < scratch_size);

	dx::BufferDesc blas_buffer_desc
	{
		.size = blas_prebuild_info.ResultDataMaxSizeInBytes,
		.stride = 0,
		.visibility = dx::GPU,
		.acceleration_structure = true,
	};
	THROW_IF_FALSE(m_context.create_buffer(blas_buffer_desc, &t_blas_buffer, "BLAS buffer"));
	
	const auto blas_build_desc = m_context.get_raytracing_acceleration_structure(blas_desc, &t_blas_buffer, nullptr, &m_scratch_space);

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
		.pResource = t_blas_buffer.allocation->GetResource(),
		.Offset = 0,
		.Size = UINT64_MAX,
	};

	std::array buffer_barriers{ scratch_space_uav, blas_buffer_barrier };

	D3D12_BARRIER_GROUP barrier_group0
	{
		.Type = D3D12_BARRIER_TYPE_BUFFER,
		.NumBarriers = static_cast<UINT>(buffer_barriers.size()),
		.pBufferBarriers = buffer_barriers.data(),
	};

	cmd_list->Barrier(1, &barrier_group0);

	cmd_list->BuildRaytracingAccelerationStructure(&blas_build_desc, 0, nullptr);

	// TODO: It would be convenient if resources tracked their current state for barriers
	// Transition BLAS to read state
	blas_buffer_barrier =
	{
		.SyncBefore = D3D12_BARRIER_SYNC_BUILD_RAYTRACING_ACCELERATION_STRUCTURE,
		.SyncAfter = D3D12_BARRIER_SYNC_RAYTRACING,
		.AccessBefore = D3D12_BARRIER_ACCESS_RAYTRACING_ACCELERATION_STRUCTURE_WRITE,
		.AccessAfter = D3D12_BARRIER_ACCESS_RAYTRACING_ACCELERATION_STRUCTURE_READ,
		.pResource = t_blas_buffer.allocation->GetResource(),
		.Offset = 0,
		.Size = UINT64_MAX,
	};
	D3D12_BARRIER_GROUP barrier_group_read
	{
		.Type = D3D12_BARRIER_TYPE_BUFFER,
		.NumBarriers = 1,
		.pBufferBarriers = &blas_buffer_barrier,
	};
	cmd_list->Barrier(1, &barrier_group_read);

	m_blas_buffers.push_back(t_blas_buffer);
	return m_blas_buffers.size() - 1;
}

 // builds top level acceleration structure with blas buffer (can be called each frame likely)
void glRemix::glRemixRenderer::build_tlas(ComPtr<ID3D12GraphicsCommandList7> cmd_list)
{
	// create an instance descriptor for all geometry
	const UINT instance_count = (UINT)m_meshes.size();

	if (instance_count == 0) return;

	std::vector<D3D12_RAYTRACING_INSTANCE_DESC> instance_descs(instance_count);
	for (int i = 0; i < instance_count; i++)
	{
		MeshRecord mesh = m_meshes[i];
		instance_descs[i] = 
		{
			.Transform
			{
				// Identity matrix (eventually change to mesh's associated matrix)
				1.0f, 0.0f, 0.0f, 0.0f,
				0.0f, 1.0f, 0.0f, 0.0f,
				0.0f, 0.0f, 1.0f, 0.0f,
			},
			.InstanceID = (UINT)i,
			.InstanceMask = 0xFF,
			.InstanceContributionToHitGroupIndex = 0,
			.Flags = D3D12_RAYTRACING_INSTANCE_FLAG_NONE,
			.AccelerationStructure = m_blas_buffers[i].allocation->GetResource()->GetGPUVirtualAddress(),
		};
	}

	
	dx::BufferDesc instance_buffer_desc
	{
		.size = sizeof(D3D12_RAYTRACING_INSTANCE_DESC) * instance_count,
		.stride = sizeof(D3D12_RAYTRACING_INSTANCE_DESC),
		.visibility = static_cast<dx::BufferVisibility>(dx::CPU | dx::GPU),
	};
	THROW_IF_FALSE(m_context.create_buffer(instance_buffer_desc, &m_tlas.instance, "TLAS instance buffer"));
	
	void* cpu_ptr;
	THROW_IF_FALSE(m_context.map_buffer(&m_tlas.instance, &cpu_ptr));
	memcpy(cpu_ptr, instance_descs.data(), instance_buffer_desc.size);
	m_context.unmap_buffer(&m_tlas.instance);

	D3D12_BUILD_RAYTRACING_ACCELERATION_STRUCTURE_INPUTS tlas_desc
	{
		.Type = D3D12_RAYTRACING_ACCELERATION_STRUCTURE_TYPE_TOP_LEVEL,
		// Lots of options here, probably want to use different ones, especially the update one
		.Flags = D3D12_RAYTRACING_ACCELERATION_STRUCTURE_BUILD_FLAG_NONE,
		.NumDescs = instance_count,
		.InstanceDescs = m_tlas.instance.allocation.Get()->GetResource()->GetGPUVirtualAddress(),
	};

	constexpr UINT64 scratch_size = 16 * 1024 * 1024;
	const auto tlas_prebuild_info = m_context.get_acceleration_structure_prebuild_info(tlas_desc);
	assert(tlas_prebuild_info.ScratchDataSizeInBytes < scratch_size);
	dx::BufferDesc tlas_buffer_desc
	{
		.size = tlas_prebuild_info.ResultDataMaxSizeInBytes,
		.stride = 0,
		.visibility = dx::GPU,
		.acceleration_structure = true,
	};
	THROW_IF_FALSE(m_context.create_buffer(tlas_buffer_desc, &m_tlas.buffer, "TLAS buffer"));

	D3D12_BUFFER_BARRIER tlas_buffer_barrier
	{
		.SyncBefore = D3D12_BARRIER_SYNC_NONE,
		.SyncAfter = D3D12_BARRIER_SYNC_BUILD_RAYTRACING_ACCELERATION_STRUCTURE,
		.AccessBefore = D3D12_BARRIER_ACCESS_NO_ACCESS,
		.AccessAfter = D3D12_BARRIER_ACCESS_RAYTRACING_ACCELERATION_STRUCTURE_WRITE,
		.pResource = m_tlas.buffer.allocation->GetResource(),
		.Offset = 0,
		.Size = UINT64_MAX,
	};
	D3D12_BARRIER_GROUP barrier_group_tlas
	{
		.Type = D3D12_BARRIER_TYPE_BUFFER,
		.NumBarriers = 1,
		.pBufferBarriers = &tlas_buffer_barrier,
	};
	cmd_list->Barrier(1, &barrier_group_tlas);

	const auto tlas_build_desc = m_context.get_raytracing_acceleration_structure(tlas_desc, &m_tlas.buffer, nullptr, &m_scratch_space);

	cmd_list->BuildRaytracingAccelerationStructure(&tlas_build_desc, 0, nullptr);

	// Transition TLAS to read state after build
	D3D12_BUFFER_BARRIER tlas_buffer_barrier_read
	{
		.SyncBefore = D3D12_BARRIER_SYNC_BUILD_RAYTRACING_ACCELERATION_STRUCTURE,
		.SyncAfter = D3D12_BARRIER_SYNC_RAYTRACING,
		.AccessBefore = D3D12_BARRIER_ACCESS_RAYTRACING_ACCELERATION_STRUCTURE_WRITE,
		.AccessAfter = D3D12_BARRIER_ACCESS_RAYTRACING_ACCELERATION_STRUCTURE_READ,
		.pResource = m_tlas.buffer.allocation->GetResource(),
		.Offset = 0,
		.Size = UINT64_MAX,
	};

	D3D12_BARRIER_GROUP tlas_barrier_group_read
	{
		.Type = D3D12_BARRIER_TYPE_BUFFER,
		.NumBarriers = 1,
		.pBufferBarriers = &tlas_buffer_barrier_read,
	};
	cmd_list->Barrier(1, &tlas_barrier_group_read);
}

void glRemix::glRemixRenderer::updateMVP(float rot)
{
    using namespace DirectX;

    XMUINT2 win_dims{};
    m_context.get_window_dimensions(&win_dims);

    MVP* mvpCPU = nullptr;
    THROW_IF_FALSE(m_context.map_buffer(&m_mvp, reinterpret_cast<void**>(&mvpCPU)));

    const float h = float(win_dims.y) / float(win_dims.x);
    XMMATRIX M = XMMatrixRotationRollPitchYaw(rot, rot, rot);
    XMMATRIX V = XMMatrixTranslation(0.0f, 0.0f, -40.0f);
    XMMATRIX P = XMMatrixPerspectiveOffCenterRH(-1.0f, 1.0f, -h, h, 5.0f, 60.0f);

    XMStoreFloat4x4(&mvpCPU->model, M);
    XMStoreFloat4x4(&mvpCPU->view, V);
    XMStoreFloat4x4(&mvpCPU->proj, P);

    m_context.unmap_buffer(&m_mvp);
}

void glRemix::glRemixRenderer::render()
{
	// read GL stream and set resoureces accordingly
	read_gl_command_stream();

	// Start ImGui frame
	m_context.start_imgui_frame();
	ImGui::ShowDemoWindow();

	//if (!m_vertex_buffer.allocation) return;

	// Be careful not to call the ID3D12Interface reset instead
	THROW_IF_FALSE(SUCCEEDED(m_cmd_pools[get_frame_index()].cmd_allocator->Reset()));

	// Create a command list in the open state
	ComPtr<ID3D12GraphicsCommandList7> cmd_list;
	THROW_IF_FALSE(m_context.create_command_list(cmd_list.ReleaseAndGetAddressOf(), m_cmd_pools[get_frame_index()]));

	const auto swapchain_idx = m_context.get_swapchain_index();

	auto make_tex_barrier = [](ID3D12Resource* res,
		D3D12_BARRIER_SYNC sync_before, D3D12_BARRIER_SYNC sync_after,
		D3D12_BARRIER_ACCESS access_before, D3D12_BARRIER_ACCESS access_after,
		D3D12_BARRIER_LAYOUT layout_before, D3D12_BARRIER_LAYOUT layout_after) -> D3D12_TEXTURE_BARRIER
	{
		return D3D12_TEXTURE_BARRIER
		{
			.SyncBefore = sync_before,
			.SyncAfter = sync_after,
			.AccessBefore = access_before,
			.AccessAfter = access_after,
			.LayoutBefore = layout_before,
			.LayoutAfter = layout_after,
			.pResource = res,
			.Subresources
			{
				.IndexOrFirstMipLevel = 0,
				.NumMipLevels = 1,
				.FirstArraySlice = 0,
				.NumArraySlices = 1,
				.FirstPlane = 0,
				.NumPlanes = 1,
			},
			.Flags = D3D12_TEXTURE_BARRIER_FLAG_NONE,
		};
	};

	auto submit_tex_barriers = [&](std::initializer_list<D3D12_TEXTURE_BARRIER> barriers)
	{
		D3D12_BARRIER_GROUP group
		{
			.Type = D3D12_BARRIER_TYPE_TEXTURE,
			.NumBarriers = static_cast<UINT>(barriers.size()),
			.pTextureBarriers = barriers.begin(),
		};
		cmd_list->Barrier(1, &group);
	};

	auto get_current_swapchain_resource = [&]() -> ID3D12Resource*
	{
		D3D12_TEXTURE_BARRIER tmp{};
		m_context.set_barrier_swapchain(&tmp); // Fills pResource and subresource range
		return tmp.pResource;
	};

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
	build_tlas(cmd_list);

	// Dispatch rays to UAV render target
	{
		static float increment = 0.f;
		increment += 0.01f;

		XMVECTOR eye_position = XMVectorSet(cosf(increment), sinf(increment), -12.0f, 1.0f);
		XMVECTOR focus_position = XMVectorSet(0.0f, 0.0f, 0.0f, 1.0f);
		XMVECTOR up_direction = XMVectorSet(0.0f, 1.0f, 0.0f, 0.0f);
		
		XMMATRIX view_matrix = XMMatrixLookAtLH(eye_position, focus_position, up_direction);
		XMMATRIX projection_matrix = XMMatrixPerspectiveFovLH(
			XM_PIDIV4,
			static_cast<float>(win_dims.x) / static_cast<float>(win_dims.y),
			0.1f,
			100.0f
		);
		
		XMMATRIX view_projection = XMMatrixMultiply(view_matrix, projection_matrix);
		XMMATRIX inverse_view_projection = XMMatrixInverse(nullptr, view_projection);
		
		RayGenConstantBuffer raygen_cb
		{
			.width = static_cast<float>(win_dims.x),
			.height = static_cast<float>(win_dims.y),
		};
		XMStoreFloat4x4(&raygen_cb.projection_matrix, XMMatrixTranspose(view_projection));
		XMStoreFloat4x4(&raygen_cb.inv_projection_matrix, XMMatrixTranspose(inverse_view_projection));
		
		// Copy constant buffer to GPU
		auto raygen_cb_ptr = &m_raygen_constant_buffers[get_frame_index()];
		void* cb_ptr;
		THROW_IF_FALSE(m_context.map_buffer(raygen_cb_ptr, &cb_ptr));
		memcpy(cb_ptr, &raygen_cb, sizeof(RayGenConstantBuffer));
		m_context.unmap_buffer(raygen_cb_ptr);

		// Transition UAV texture to UAV layout for raytracing dispatch
		auto uav_rt_dispatch = make_tex_barrier(
			m_uav_render_target.Get()->GetResource(),
			D3D12_BARRIER_SYNC_NONE,
			D3D12_BARRIER_SYNC_RAYTRACING,
			D3D12_BARRIER_ACCESS_NO_ACCESS,
			D3D12_BARRIER_ACCESS_UNORDERED_ACCESS,
			D3D12_BARRIER_LAYOUT_UNORDERED_ACCESS,
			D3D12_BARRIER_LAYOUT_UNORDERED_ACCESS
		);
		D3D12_BARRIER_GROUP barrier_group
		{
			.Type = D3D12_BARRIER_TYPE_TEXTURE,
			.NumBarriers = 1,
			.pTextureBarriers = &uav_rt_dispatch,
		};
		cmd_list->Barrier(1, &barrier_group);

		cmd_list->SetPipelineState1(m_rt_pipeline.Get());
		cmd_list->SetComputeRootSignature(m_rt_global_root_signature.Get());

		// TODO: Copy from a CPU descriptor heap instead of recreating each frame
		m_context.create_constant_buffer_view(&m_raygen_constant_buffers[get_frame_index()], &m_rt_descriptors, 2);

		// Set descriptor heap and table our descriptors
		m_context.set_descriptor_heap(cmd_list.Get(), m_rt_descriptor_heap);
		D3D12_GPU_DESCRIPTOR_HANDLE descriptor_table_handle{};
		m_rt_descriptor_heap.get_gpu_descriptor(&descriptor_table_handle, m_rt_descriptors.offset);
		cmd_list->SetComputeRootDescriptorTable(0, descriptor_table_handle);

		D3D12_GPU_VIRTUAL_ADDRESS shader_table_base_address = m_shader_table.allocation->GetResource()->GetGPUVirtualAddress();
		
		D3D12_DISPATCH_RAYS_DESC dispatch_desc
		{
			.RayGenerationShaderRecord
			{
				.StartAddress = shader_table_base_address + m_raygen_shader_table_offset,
				.SizeInBytes = D3D12_SHADER_IDENTIFIER_SIZE_IN_BYTES,
			},
			.MissShaderTable
			{
				.StartAddress = shader_table_base_address + m_miss_shader_table_offset,
				.SizeInBytes = D3D12_SHADER_IDENTIFIER_SIZE_IN_BYTES,
				.StrideInBytes = D3D12_SHADER_IDENTIFIER_SIZE_IN_BYTES,
			},
			.HitGroupTable
			{
				.StartAddress = shader_table_base_address + m_hit_group_shader_table_offset,
				.SizeInBytes = D3D12_SHADER_IDENTIFIER_SIZE_IN_BYTES,
				.StrideInBytes = D3D12_SHADER_IDENTIFIER_SIZE_IN_BYTES,
			},
			.Width = win_dims.x,
			.Height = win_dims.y,
			.Depth = 1,
		};

		cmd_list->DispatchRays(&dispatch_desc);
	}

	// Copy the ray traced UAV texture into the current swapchain backbuffer, then render ImGui on top
	{
		ID3D12Resource* backbuffer = get_current_swapchain_resource();

		// Transition UAV -> COPY_SOURCE and Swapchain(PRESENT) -> COPY_DEST
		auto uav_to_copy_src = make_tex_barrier(
			m_uav_render_target.Get()->GetResource(),
			D3D12_BARRIER_SYNC_RAYTRACING, D3D12_BARRIER_SYNC_COPY,
			D3D12_BARRIER_ACCESS_UNORDERED_ACCESS, D3D12_BARRIER_ACCESS_COPY_SOURCE,
			D3D12_BARRIER_LAYOUT_UNORDERED_ACCESS, D3D12_BARRIER_LAYOUT_COPY_SOURCE);

		auto swap_to_copy_dst = make_tex_barrier(
			nullptr,
			D3D12_BARRIER_SYNC_NONE,
			D3D12_BARRIER_SYNC_COPY,
			D3D12_BARRIER_ACCESS_NO_ACCESS,
			D3D12_BARRIER_ACCESS_COPY_DEST,
			D3D12_BARRIER_LAYOUT_PRESENT,
			D3D12_BARRIER_LAYOUT_COPY_DEST
		);
		m_context.set_barrier_swapchain(&swap_to_copy_dst); // Fills pResource and subresources


		submit_tex_barriers({ uav_to_copy_src, swap_to_copy_dst });

		cmd_list->CopyResource(backbuffer, m_uav_render_target.Get()->GetResource());

		// Transition Swapchain COPY_DEST -> RENDER_TARGET and UAV COPY_SOURCE -> UNORDERED_ACCESS
		auto swap_to_rtv = make_tex_barrier(
			nullptr,
			D3D12_BARRIER_SYNC_COPY,
			D3D12_BARRIER_SYNC_RENDER_TARGET,
			D3D12_BARRIER_ACCESS_COPY_DEST,
			D3D12_BARRIER_ACCESS_RENDER_TARGET,
			D3D12_BARRIER_LAYOUT_COPY_DEST,
			D3D12_BARRIER_LAYOUT_RENDER_TARGET
		);
		m_context.set_barrier_swapchain(&swap_to_rtv);

		auto uav_back_to_uav = make_tex_barrier(
			m_uav_render_target.Get()->GetResource(),
			D3D12_BARRIER_SYNC_COPY, D3D12_BARRIER_SYNC_RAYTRACING,
			D3D12_BARRIER_ACCESS_COPY_SOURCE, D3D12_BARRIER_ACCESS_UNORDERED_ACCESS,
			D3D12_BARRIER_LAYOUT_COPY_SOURCE, D3D12_BARRIER_LAYOUT_UNORDERED_ACCESS);

		submit_tex_barriers({ swap_to_rtv, uav_back_to_uav });
	}

	// Draw everything (ImGui over the copied ray traced image)
	D3D12_CPU_DESCRIPTOR_HANDLE swapchain_rtv{};
	m_swapchain_descriptors.heap->get_cpu_descriptor(&swapchain_rtv, m_swapchain_descriptors.offset + swapchain_idx);
	cmd_list->OMSetRenderTargets(1, &swapchain_rtv, FALSE, nullptr);

	// rasterization for testing
	{
        static float rot = 0.f;
        rot += 0.01f;
        updateMVP(rot);
        auto* cbRes = m_mvp.allocation->GetResource();

        cmd_list->SetGraphicsRootSignature(m_root_signature.Get());
        cmd_list->SetPipelineState(m_raster_pipeline.Get());

        cmd_list->SetGraphicsRootConstantBufferView(0, cbRes->GetGPUVirtualAddress());

        cmd_list->IASetPrimitiveTopology(D3D_PRIMITIVE_TOPOLOGY_TRIANGLELIST);

        if (m_vertex_buffer.allocation)
        {
            D3D12_VERTEX_BUFFER_VIEW vbv{};
            vbv.BufferLocation = m_vertex_buffer.allocation->GetResource()->GetGPUVirtualAddress();
            vbv.SizeInBytes = static_cast<UINT>(m_vertex_buffer.desc.size);
            vbv.StrideInBytes = static_cast<UINT>(m_vertex_buffer.desc.stride);
            cmd_list->IASetVertexBuffers(0, 1, &vbv);
        }

        if (m_index_buffer.allocation)
        {
            D3D12_INDEX_BUFFER_VIEW ibv{};
            ibv.BufferLocation = m_index_buffer.allocation->GetResource()->GetGPUVirtualAddress();
            ibv.SizeInBytes = static_cast<UINT>(m_index_buffer.desc.size);
            ibv.Format = DXGI_FORMAT_R32_UINT;
            cmd_list->IASetIndexBuffer(&ibv);

            const UINT indexCount = ibv.SizeInBytes / 4u;
			
			for (const MeshRecord& mesh : m_meshes)
			{
				/*
				cmd_list->DrawIndexedInstanced
				(
					mesh.indexCount,
					1,
					mesh.indexOffset,
					mesh.vertexOffset,
					0
				);*/
			}
        }
    }

    // Render ImGui
	m_context.render_imgui_draw_data(cmd_list.Get());

	// Transition swapchain image to present
	{
		auto swapchain_present = make_tex_barrier(
			nullptr,
			D3D12_BARRIER_SYNC_DRAW,
			D3D12_BARRIER_SYNC_NONE,
			D3D12_BARRIER_ACCESS_RENDER_TARGET,
			D3D12_BARRIER_ACCESS_NO_ACCESS,
			D3D12_BARRIER_LAYOUT_RENDER_TARGET,
			D3D12_BARRIER_LAYOUT_PRESENT
		);
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
	m_rt_descriptor_heap.deallocate(&m_rt_descriptors);
}

glRemix::glRemixRenderer::~glRemixRenderer()
{

}