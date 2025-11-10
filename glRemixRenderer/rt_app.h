#pragma once
#include "application.h"
#include "dx/d3d12_as.h"
#include "gl/gl_matrix_stack.h"
#include <DirectXMath.h>
#include <ipc_protocol.h>
#include <unordered_map>

namespace glRemix
{
	class glRemixRenderer : public Application
	{
        struct Vertex
        {
            std::array<float, 3> position;
            std::array<float, 3> color;
        };

		// TODO add more parameters (such as enabled) when encountered
		struct Light
		{
			float ambient[4]  = { 0.f, 0.f, 0.f, 1.f };
    		float diffuse[4]  = { 1.f, 1.f, 1.f, 1.f };
    		float specular[4] = { 1.f, 1.f, 1.f, 1.f };

			float position[4] = { 0.f, 0.f, 1.f, 0.f }; // default head down

    		float spotDirection[3] = { 0.f, 0.f, -1.f };
    		float spotExponent = 0.f;
    		float spotCutoff = 180.f; 

    		float constantAttenuation  = 1.f;
    		float linearAttenuation    = 0.f;
    		float quadraticAttenuation = 0.f;
		};

		// TODO add more parameters if encountered
		struct Material
		{
			float ambient[4]   = { 1.f, 1.f, 1.f, 1.f };
    		float diffuse[4]   = { 1.f, 1.f, 1.f, 1.f };
    		float specular[4]  = { 0.f, 0.f, 0.f, 1.f };
    		float emission[4]  = { 0.f, 0.f, 0.f, 1.f };

			float shininess = 0.f;
		};

		struct MeshRecord
		{
			uint32_t meshId; // hashed
			uint32_t vertexOffset; // offset into vertex atlas
			uint32_t vertexCount;
			uint32_t indexOffset; // offset into index atlas
			uint32_t indexCount; // number of indices belonging to this mesh

			// pointers
			uint32_t blasID;
			uint32_t MVID; // index into model view array
			uint32_t matID;
			uint32_t texID;
		};

		struct alignas(16) MVP
		{
            DirectX::XMFLOAT4X4 model;
            DirectX::XMFLOAT4X4 view;
            DirectX::XMFLOAT4X4 proj;
        };

        std::array<dx::D3D12CommandAllocator, m_frames_in_flight> m_cmd_pools{};

		ComPtr<ID3D12RootSignature> m_root_signature{};
		ComPtr<ID3D12PipelineState> m_raster_pipeline{};

		ComPtr<ID3D12RootSignature> m_rt_global_root_signature{};
		ComPtr<ID3D12StateObject> m_rt_pipeline{};

		dx::D3D12DescriptorHeap m_rt_descriptor_heap{};
		dx::D3D12DescriptorTable m_rt_descriptors{};

		// TODO: Add other per frame constants as needed, rename accordingly
		// Copy parameters to this buffer each frame
		std::array<dx::D3D12Buffer, m_frames_in_flight> m_raygen_constant_buffers{};

		dx::D3D12Buffer m_vertex_buffer{};
		dx::D3D12Buffer m_index_buffer{};

		dx::D3D12Buffer m_scratch_space{};
		dx::D3D12Buffer m_blas_buffer{};
		dx::D3D12TLAS m_tlas{};

		dx::D3D12Buffer m_meshes_buffer{};
        dx::D3D12Buffer m_materials_buffer{};
        dx::D3D12Buffer m_lights_buffer{};

		// Shader table buffer for ray tracing pipeline (contains raygen, miss, and hit group)
		dx::D3D12Buffer m_shader_table{};
		UINT64 m_raygen_shader_table_offset{};
		UINT64 m_miss_shader_table_offset{};
		UINT64 m_hit_group_shader_table_offset{};

        dx::D3D12Texture m_uav_render_target{};

		IPCProtocol m_ipc;

		// mesh resources
		std::vector<MeshRecord> m_meshes;
		std::vector<dx::D3D12Buffer> m_blas_buffers;

		// matrix stack
		gl::glMatrixStack m_matrix_stack;
		std::vector<XMFLOAT4X4> m_matrix_pool;
		XMMATRIX inverse_view;

		// display lists
		std::unordered_map<int, std::vector<uint8_t>> m_display_lists;

		// state trackers
		gl::GLMatrixMode matrixMode = gl::GLMatrixMode::MODELVIEW; // "The initial matrix mode is MODELVIEW" - glspec pg. 29 (might need to be a global variable not sure how state is tracked)
		std::array<float, 4> color = {1.0f, 1.0f, 1.0f, 1.0f}; // current color (may need to be tracked globally)
		Material m_material; // global states that can be modified


		// shader resources
		std::vector<Light> m_lights = std::vector<Light>(8);
		std::vector<Material> m_materials;

	protected:
		void create() override;
		void render() override;
		void destroy() override;

		HWND wait_for_hwnd_command();
		void read_gl_command_stream();
		void read_geometry(std::vector<uint8_t>& ipcBuf,
                                            size_t& offset,
                                            std::vector<Vertex>& vertices,
                                            std::vector<uint32_t>& indices,
                                            glRemix::GLTopology topology,
                                            uint32_t bytesRead);


		// acceleration structure builders
		int build_mesh_blas(uint32_t vertex_count, uint32_t vertex_offset, uint32_t index_count, uint32_t index_offset, ComPtr<ID3D12GraphicsCommandList7> cmd_list);
		void build_tlas(ComPtr<ID3D12GraphicsCommandList7> cmd_list);

	public:
		glRemixRenderer() = default;
		~glRemixRenderer() override;
	};
}
