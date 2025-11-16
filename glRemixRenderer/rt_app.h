#pragma once

#include <tsl/robin_map.h>
#include <DirectXMath.h>

#include <shared/ipc_protocol.h>
#include <shared/gl_commands.h>

#include "application.h"
#include "debug_window.h"
#include "dx/d3d12_as.h"

#include "descriptor_pager.h"
#include "gl/gl_matrix_stack.h"

#include "structs.h"

namespace glRemix
{
class glRemixRenderer : public Application
{
    std::array<dx::D3D12CommandAllocator, m_frames_in_flight> m_cmd_pools{};

    ComPtr<ID3D12RootSignature> m_root_signature{};
    ComPtr<ID3D12PipelineState> m_raster_pipeline{};

    ComPtr<ID3D12RootSignature> m_rt_global_root_signature{};
    ComPtr<ID3D12StateObject> m_rt_pipeline{};

    dx::D3D12DescriptorHeap m_GPU_descriptor_heap{};
    dx::D3D12DescriptorHeap m_CPU_descriptor_heap{};

    dx::DescriptorPager m_descriptor_pager{};

    std::array<dx::D3D12Buffer, m_frames_in_flight> m_raygen_constant_buffers{};
    std::array<dx::D3D12Descriptor, m_frames_in_flight> m_raygen_cbv_descriptors{};

    dx::D3D12Buffer m_scratch_space{};

    dx::D3D12TLAS m_tlas{};
    dx::D3D12Descriptor m_tlas_descriptor{};

    dx::D3D12Buffer m_lights_buffer{};

    // Shader table buffer for ray tracing pipeline (contains raygen, miss, and hit group)
    dx::D3D12Buffer m_shader_table{};
    UINT64 m_raygen_shader_table_offset{};
    UINT64 m_miss_shader_table_offset{};
    UINT64 m_hit_group_shader_table_offset{};

    dx::D3D12Texture m_uav_rt{};
    dx::D3D12Descriptor m_uav_rt_descriptor{};

    IPCProtocol m_ipc;

    // mesh resources
    tsl::robin_map<UINT64, MeshRecord> m_mesh_map;
    std::vector<MeshRecord> m_meshes;

    struct BufferAndDescriptor
    {
        dx::D3D12Buffer buffer;
        dx::D3D12Descriptor descriptor;
    };
    struct MeshResources
    {
        dx::D3D12Buffer blas;
        BufferAndDescriptor vertex_buffer;
        BufferAndDescriptor index_buffer;
    };

    struct PendingGeometry
    {
        std::vector<Vertex> vertices;
        std::vector<UINT32> indices;
        UINT64 hash;
        UINT32 mat_idx;
        UINT32 mv_idx;
    };
    std::vector<PendingGeometry> m_pending_geometries;

    // BLAS, VB, IB per mesh
    // VB and IB have descriptors for SRV to be allocated from pager
    std::vector<MeshResources> m_mesh_resources;
    // Materials per buffer
    static constexpr UINT MATERIALS_PER_BUFFER = 256;
    std::vector<BufferAndDescriptor> m_material_buffers;

    //BufferPool m_blas_pool;
    //BufferPool m_vertex_pool;
    //BufferPool m_index_pool;

    // matrix stack
    gl::glMatrixStack m_matrix_stack;

    std::vector<XMFLOAT4X4> m_matrix_pool;  // reset each frame
    XMMATRIX inverse_view;

    // display lists
    tsl::robin_map<int, std::vector<UINT8>> m_display_lists;

    // state trackers
    gl::GLMatrixMode matrix_mode
        = gl::GLMatrixMode::MODELVIEW;  // "The initial matrix mode is MODELVIEW" - glspec pg. 29
    gl::GLListMode list_mode_ = gl::GLListMode::COMPILE_AND_EXECUTE;

    // Global states
    // Current color (may need to be tracked globally)
    std::array<float, 4> m_color = { 1.0f, 1.0f, 1.0f, 1.0f };  
    // Default according to spec
    std::array<float, 3> m_normal = { 0.0f, 0.0f, 1.0f }; 
    Material m_material;

    BufferAndDescriptor m_light_buffer;
    std::array<Light, 8> m_lights{};
    std::vector<Material> m_materials;

    DebugWindow m_debug_window;

    void create_material_buffer();

    uint32_t frame_leniency = 10;
    uint32_t current_frame;

protected:
    void create() override;
    void render() override;
    void destroy() override;

    void create_uav_rt();

    void read_gl_command_stream();
    void read_ipc_buffer(std::vector<UINT8>& ipc_buf, size_t start_offset, UINT32 bytes_read,
                         bool call_list = false);
    void read_geometry(std::vector<UINT8>& ipc_buf, size_t* offset, GLTopology topology,
                       UINT32 bytes_read);

    struct BLASBuildInfo
    {
        const dx::D3D12Buffer* vertex_buffer;
        const dx::D3D12Buffer* index_buffer;
        dx::D3D12Buffer* blas;
    };

    // acceleration structure builders
    void build_mesh_blas_batch(const std::vector<BLASBuildInfo>& build_infos,
                              ID3D12GraphicsCommandList7* cmd_list);
    void build_pending_blas_buffers(ID3D12GraphicsCommandList7* cmd_list);
    void build_tlas(ID3D12GraphicsCommandList7* cmd_list);

public:
    glRemixRenderer() = default;
    ~glRemixRenderer() override;
};
}  // namespace glRemix
