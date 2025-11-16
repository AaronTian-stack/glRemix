#include "descriptor_pager.h"

#include <cassert>
#include <shared/math_utils.h>

UINT glRemix::dx::DescriptorPager::calculate_global_offset(const PageType type, const UINT page_index) const
{
    UINT ret = 0;
    for (UINT8 i = 0; i < type; i++)
    {
        assert(!u64_overflows_u32(m_pages[i].size()));
        ret += static_cast<UINT>(m_pages[i].size()) * m_descriptors_per_page[i];
    }
    return ret + page_index * m_descriptors_per_page[type];
}

UINT glRemix::dx::DescriptorPager::allocate_descriptor(
    const D3D12Context& context, const PageType type, D3D12Descriptor* descriptor)
{
    assert(descriptor);

	auto& pages = m_pages[type];
	const UINT descriptors_per_page = m_descriptors_per_page[type];
	
	// Try to allocate from current pages
    for (UINT i = 0; i < pages.size(); i++)
    {
        auto& page = pages[i];
        if (page.allocate(descriptor))
        {
            return i;
        }
    }

    // Need a new page
    pages.emplace_back();
    const D3D12_DESCRIPTOR_HEAP_DESC desc
    {
        .Type = D3D12_DESCRIPTOR_HEAP_TYPE_CBV_SRV_UAV,
        .NumDescriptors = descriptors_per_page,
        .Flags = D3D12_DESCRIPTOR_HEAP_FLAG_NONE,
    };
    context.create_descriptor_heap(desc, &pages.back());

    // TODO: Finer grained update based off specific page index instead of whole type
    m_dirty_index = std::min(type, m_dirty_index);

    pages.back().allocate(descriptor);

    // Should really not have that many pages
    assert(!u64_overflows_u32(pages.size()));

    return static_cast<UINT>(pages.size() - 1);
}

UINT glRemix::dx::DescriptorPager::calculate_global_index(const PageType type, const UINT page_index,
                                                               const D3D12Descriptor& descriptor) const
{
    return calculate_global_offset(type, page_index) + descriptor.offset;
}

void glRemix::dx::DescriptorPager::copy_pages_to_gpu(const D3D12Context& context, D3D12DescriptorHeap* gpu_heap, const UINT offset)
{
    assert(gpu_heap);
    assert(gpu_heap->desc.Type == D3D12_DESCRIPTOR_HEAP_TYPE_CBV_SRV_UAV);

    {
        // Assert that gpu heap can hold all the descriptors to be copied
        UINT total_descriptors = 0;
        for (UINT8 type = 0; type < END; type++)
        {
            total_descriptors += m_pages[type].size() * m_descriptors_per_page[type];
        }
        assert(total_descriptors <= gpu_heap->desc.NumDescriptors - offset);
    }

    // Copy pages in order starting from offset
    for (UINT8 type = m_dirty_index; type < END; type++)
    {
        auto& pages = m_pages[type];
        const UINT descriptors_per_page = m_descriptors_per_page[type];
        for (UINT page_index = 0; page_index < pages.size(); page_index++)
        {
            const UINT dst_offset = offset + calculate_global_offset(static_cast<PageType>(type), page_index);

            //auto dest = gpu_heap->m_heap->GetCPUDescriptorHandleForHeapStart();
            //dest.ptr += static_cast<unsigned long long>(dst_offset) * gpu_heap->m_descriptor_size;

            //context->CopyDescriptorsSimple(
            //    descriptors_per_page,
            //    dest,
            //    pages[page_index].m_heap->GetCPUDescriptorHandleForHeapStart(),
            //    D3D12_DESCRIPTOR_HEAP_TYPE_CBV_SRV_UAV);

            const D3D12Descriptor gpu{
                .heap = gpu_heap,
                .offset = dst_offset
            };

            const D3D12Descriptor cpu{
                .heap = &pages[page_index],
                .offset = 0
            };

            context.copy_descriptors(gpu, cpu, descriptors_per_page);
        }
    }

    m_dirty_index = END;
}
