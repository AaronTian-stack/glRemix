#pragma once

#include <array>

#include <dx/d3d12_descriptor_heap.h>
#include <dx/d3d12_context.h>

namespace glRemix::dx
{
class DescriptorPager
{
public:
    // Try to arrange in order of frequency of new page allocations (least to most)
    // This does not include double buffered resources or TLAS
    enum PageType : UINT8
    {
        // Could technically be allocated in slab fashion but there needs to be a second index
        MATERIALS,

        // Grow as app runs, maybe shrink in the future too
        VB_IB,
        TEXTURES,
        MESH_RECORDS,

        END,
    };

private:
    // CPU descriptor heaps
    std::array<std::vector<D3D12DescriptorHeap>, END> m_pages{};

#define PAGE_CATEGORIES 4

    const std::array<UINT, PAGE_CATEGORIES> m_descriptors_per_page{
        64,  // MATERIALS
        64,  // VB_IB
        64,  // TEXTURES
        64,  // MESH_RECORDS
    };

    static_assert(END == PAGE_CATEGORIES);

#ifdef PAGE_CATEGORIES
#undef PAGE_CATEGORIES
#endif

    // All pages including and following that index are dirty and need to be re-uploaded to GPU heap
    PageType m_dirty_index = END;

    inline UINT calculate_global_offset(PageType type, UINT page_index) const;

public:
    // Returns page index
    UINT allocate_descriptor(const D3D12Context& context, PageType type,
                             D3D12Descriptor* descriptor);
    // Need global index when updating descriptor indices in structures like MeshRecord
    UINT calculate_global_index(PageType type, UINT page_index,
                                const D3D12Descriptor& descriptor) const;
    // This should only called once per frame
    void copy_pages_to_gpu(const D3D12Context& context, D3D12DescriptorHeap* gpu_heap, UINT offset);
};
}  // namespace glRemix::dx
