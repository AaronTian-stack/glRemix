#pragma once

#include <tsl/robin_map.h>
#include <DirectXMath.h>
#include <span>

#include <shared/ipc_protocol.h>
#include <shared/gl_commands.h>

#include "application.h"
#include "debug_window.h"
#include "dx/d3d12_as.h"

#include "descriptor_pager.h"
#include "gl/gl_matrix_stack.h"

#include "structs.h"
#include <shared/containers/free_list_vector.h>

#include <filesystem>

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

    // Shader table buffer for ray tracing pipeline (contains raygen, miss, and hit group)
    dx::D3D12Buffer m_shader_table{};
    UINT64 m_raygen_shader_table_offset{};
    UINT64 m_miss_shader_table_offset{};
    UINT64 m_hit_group_shader_table_offset{};

    dx::D3D12Texture m_uav_rt{};
    dx::D3D12Descriptor m_uav_rt_descriptor{};

    IPCProtocol m_ipc;
    std::vector<UINT8> m_ipc_buffer;

    // mesh resources
    tsl::robin_map<UINT64, MeshRecord> m_mesh_map;

    std::vector<MeshRecord> m_meshes;

    BufferAndDescriptor m_gpu_mesh_record;

    // Geometry to be built in current frame (mesh resource)
    std::vector<PendingGeometry> m_pending_geometries;

    // BLAS, VB, IB per mesh
    // VB and IB have descriptors for SRV to be allocated from pager
    // TODO: Stop using GPU Upload memory for these because they are committed resources!!!
    // Since VB/IB/BLAS are destroyed and created so often we would like them to be placed resources
    FreeListVector<MeshResources> m_mesh_resources;

    // Materials per buffer
    // TODO: Make this a macro instead?
    static constexpr UINT MATERIALS_PER_BUFFER = 256;

    // This is written to by CPU potentially in two consecutive frames so we need to double buffer it
    FreeListVector<std::array<BufferAndDescriptor, m_frames_in_flight>> m_material_buffers;

    std::unordered_map<UINT64, MeshRecord> m_replacement_map;

    // matrix stack
    gl::glMatrixStack m_matrix_stack;

    std::vector<XMFLOAT4X4> m_matrix_pool;  // reset each frame

    // display lists
    tsl::robin_map<int, std::vector<UINT8>> m_display_lists;

    // state trackers
    gl::GLMatrixMode matrix_mode
        = gl::GLMatrixMode::MODELVIEW;  // "The initial matrix mode is MODELVIEW" - glspec pg. 29
    gl::GLListMode list_mode_ = gl::GLListMode::COMPILE_AND_EXECUTE;

    // Global states
    // Current color (may need to be tracked globally)
    XMFLOAT4 m_color = { 1.0f, 1.0f, 1.0f, 1.0f };
    // Default according to spec
    XMFLOAT3 m_normal = { 0.0f, 0.0f, 1.0f };
    Material m_material;

    std::array<BufferAndDescriptor, m_frames_in_flight> m_light_buffer;
    std::array<Light, 8> m_lights{};
    std::vector<Material> m_materials;

    DebugWindow m_debug_window;

    void create_material_buffer();

    // TODO: Expose this parameter in debug window?
    static constexpr UINT FRAME_LENIENCY = 10;
    UINT m_current_frame = 0;

    void create_uav_rt();

    void read_gl_command_stream();
    void read_ipc_buffer(std::vector<UINT8>& ipc_buf, size_t start_offset, UINT32 bytes_read,
                         bool call_list = false);
    void read_geometry(void* ipc_buf, size_t* offset, GLTopology topology, UINT32 bytes_read,
                       bool call_list);

    uint64_t create_hash(std::vector<Vertex> vertices, std::vector<UINT32> indices);

    // This should only be called from create_pending_buffers
    void build_mesh_blas_batch(size_t start_idx, size_t count, ID3D12GraphicsCommandList7* cmd_list);
    void create_pending_buffers(ID3D12GraphicsCommandList7* cmd_list);
    void build_tlas(ID3D12GraphicsCommandList7* cmd_list);

protected:
    void create() override;
    void render() override;
    void destroy() override;

    // asset replacement
    void replace_mesh(uint64_t meshID, const std::string& new_asset_path);
    bool load_mesh_from_path(std::filesystem::path asset_path, std::vector<Vertex>& out_vertices,
                             std::vector<uint32_t>& out_indices);
    void transform_replacement_vertices(std::vector<Vertex>& gltf_vertices, XMFLOAT4X4 mv);

public:
    glRemixRenderer() = default;
    ~glRemixRenderer() override;
};
}  // namespace glRemix
