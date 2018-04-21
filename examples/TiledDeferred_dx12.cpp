#include "Application\MainLoop.inc"

void Init() {}
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
		command_list->DrawInstanced(3, 1, 0, 0);

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