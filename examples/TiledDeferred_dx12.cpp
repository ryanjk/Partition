#include "Application\MainLoop.inc"

pn::mesh_t mesh;

ComPtr<ID3D12Resource>		vertex_buffer;
D3D12_VERTEX_BUFFER_VIEW	vertex_buffer_view;

ComPtr<ID3D12Resource>		index_buffer;
D3D12_INDEX_BUFFER_VIEW		index_buffer_view;

void Init() {
	bool success = LoadMesh(pn::GetResourcePath("dragon.fbx"), mesh);

	// === CREATE VERTEX BUFFER ===
	struct Vertex {
		vec3f position;
		vec3f normal;
	};
	
	const auto vertex_buffer_size = sizeof(Vertex) * mesh.vertices.size();
	ThrowIfFailed(device->CreateCommittedResource(
		&CD3DX12_HEAP_PROPERTIES(D3D12_HEAP_TYPE_UPLOAD),
		D3D12_HEAP_FLAG_NONE,
		&CD3DX12_RESOURCE_DESC::Buffer(vertex_buffer_size),
		D3D12_RESOURCE_STATE_GENERIC_READ,
		nullptr,
		IID_PPV_ARGS(&vertex_buffer)
	));

	UINT8* vertex_data_begin;
	CD3DX12_RANGE read_range(0, 0);
	ThrowIfFailed(vertex_buffer->Map(0, &read_range, reinterpret_cast<void**>(&vertex_data_begin)));

	for (unsigned long long i = 0; i < mesh.vertices.size(); ++i) {
		memcpy(vertex_data_begin + i * sizeof(Vertex), mesh.vertices.data() + i, sizeof(pn::vec3f));
		memcpy(vertex_data_begin + i * sizeof(Vertex) + sizeof(pn::vec3f), mesh.normals.data() + i, sizeof(pn::vec3f));
	}

	vertex_buffer_view.BufferLocation = vertex_buffer->GetGPUVirtualAddress();
	vertex_buffer_view.StrideInBytes = sizeof(Vertex);
	vertex_buffer_view.SizeInBytes = vertex_buffer_size;

	// === CREATE INDEX BUFFER ===
	const auto index_buffer_size = mesh.indices.size() * sizeof(unsigned int);
	ThrowIfFailed(device->CreateCommittedResource(
		&CD3DX12_HEAP_PROPERTIES(D3D12_HEAP_TYPE_UPLOAD),
		D3D12_HEAP_FLAG_NONE,
		&CD3DX12_RESOURCE_DESC::Buffer(index_buffer_size),
		D3D12_RESOURCE_STATE_GENERIC_READ,
		nullptr,
		IID_PPV_ARGS(&index_buffer)
	));

	UINT8* index_buffer_begin;
	CD3DX12_RANGE index_read_range(0, 0);
	ThrowIfFailed(index_buffer->Map(0, &index_read_range, (void**) &index_buffer_begin));
	memcpy(index_buffer_begin, mesh.indices.data(), index_buffer_size);

	index_buffer_view.BufferLocation = index_buffer->GetGPUVirtualAddress();
	index_buffer_view.Format = DXGI_FORMAT::DXGI_FORMAT_R32_UINT;
	index_buffer_view.SizeInBytes = index_buffer_size;

}
void Update() {}      // Called once per frame
void FixedUpdate() {} // Called at fixed rate independent of frame-rate

void Render() {
	
	{
		command_allocator->Reset();
		command_list->Reset(command_allocator.Get(), pipeline_state.Get());

		command_list->SetGraphicsRootSignature(root_signature.Get());
		command_list->RSSetViewports(1, &viewport);
		command_list->RSSetScissorRects(1, &scissor_rect);

		command_list->ResourceBarrier(1, &CD3DX12_RESOURCE_BARRIER::Transition(render_targets[frame_index].Get(), D3D12_RESOURCE_STATE_PRESENT, D3D12_RESOURCE_STATE_RENDER_TARGET));

		CD3DX12_CPU_DESCRIPTOR_HANDLE current_rtv_handle(rtv_heap->GetCPUDescriptorHandleForHeapStart(), frame_index, rtv_descriptor_size);
		command_list->OMSetRenderTargets(1, &current_rtv_handle, FALSE, nullptr);

		const float color[4] = { 0.0f, 0.0f, 0.0f, 1.0f };
		command_list->ClearRenderTargetView(current_rtv_handle, color, 0, nullptr);
		command_list->IASetPrimitiveTopology(D3D_PRIMITIVE_TOPOLOGY_TRIANGLELIST);
		command_list->IASetVertexBuffers(0, 1, &vertex_buffer_view);
		command_list->IASetIndexBuffer(&index_buffer_view);
		command_list->DrawIndexedInstanced(mesh.indices.size(), 1, 0, 0, 0);

		command_list->ResourceBarrier(1, &CD3DX12_RESOURCE_BARRIER::Transition(render_targets[frame_index].Get(), D3D12_RESOURCE_STATE_RENDER_TARGET, D3D12_RESOURCE_STATE_PRESENT));

		command_list->Close();
	}

	ID3D12CommandList* command_lists[] = { command_list.Get() };
	command_queue->ExecuteCommandLists(_countof(command_lists), command_lists);

}

void Close() {}
void Resize() {}

void MainLoopBegin() {}
void MainLoopEnd() {
	const UINT64 cur_fence_value = fence_value;
	command_queue->Signal(fence.Get(), cur_fence_value);
	fence_value++;

	if (fence->GetCompletedValue() < cur_fence_value) {
		fence->SetEventOnCompletion(cur_fence_value, fence_event);
		WaitForSingleObject(fence_event, INFINITE);
	}

	frame_index = swap_chain->GetCurrentBackBufferIndex();
}