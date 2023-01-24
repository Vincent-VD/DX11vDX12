#include "pch.h"
#include "GameDX12.h"

#include <Windows.UI.Core.h>

#include "DXSampleHelper.h"
#include "Logger.h"
#include "ModelManager.h"
#include "ReadData.h"

//
// GameDX12.cpp
//

extern void ExitGame() noexcept;

using namespace DirectX12;
using namespace DirectX12::SimpleMath;

using Microsoft::WRL::ComPtr;

GameDX12::GameDX12() noexcept :
	BaseGame(),
	m_MappedInstanceData(nullptr),
	m_InstanceDataGpuAddr(0),
	m_UsedInstanceCount(c_startInstanceCount),
	m_Lights{},
	m_Pitch(0.0f),
	m_Yaw(0.0f)
{
	XMStoreFloat4x4(&m_Proj, XMMatrixIdentity());

	// Use gamma-correct rendering.
	m_DeviceResources = std::make_unique<DX::DeviceResourcesDX12>(DXGI_FORMAT_B8G8R8A8_UNORM_SRGB);
	m_DeviceResources->RegisterDeviceNotify(this);
}

// Initialize the Direct3D resources required to run.
void GameDX12::Initialize(HWND window, int width, int height)
{
	m_DeviceResources->SetWindow(window, width, height);

	m_DeviceResources->CreateDeviceResources();
	CreateDeviceDependentResources();

	m_DeviceResources->CreateWindowSizeDependentResources();
	CreateWindowSizeDependentResources();

	m_FenceEvent.Attach(CreateEvent(nullptr, FALSE, FALSE, nullptr));
	if (!m_FenceEvent.IsValid())
	{
		throw std::exception("CreateEvent");
	}
}

// Executes the basic GameDX12 loop.
void GameDX12::Tick()
{
	m_Timer.Tick([&]()
	{
		Update(m_Timer);
		if (Logger::GetInstance()->Update(m_Timer))
		{
			Logger::GetInstance()->Log(m_Timer, nullptr, this);
		}
	});

	Render();
}

// Updates the world.
void GameDX12::Update(DX::StepTimer const& timer)
{
	PIXBeginEvent(PIX_COLOR_DEFAULT, L"Update");

	const auto elapsedTime = static_cast<float>(timer.GetElapsedSeconds());

	auto left{ static_cast<float>(std::abs(GetAsyncKeyState('A'))) };
	auto right{ static_cast<float>(std::abs(GetAsyncKeyState('D'))) };
	auto up{ static_cast<float>(std::abs(GetAsyncKeyState('W'))) };
	auto down{ static_cast<float>(std::abs(GetAsyncKeyState('S'))) };
	left = std::max(0.f, std::min(left, 1.f));
	right = std::max(0.f, std::min(right, 1.f));
	up = std::max(0.f, std::min(up, 1.f));
	down = std::max(0.f, std::min(down, 1.f));

	if ((left > 0.f) || (right > 0.f))
	{
		m_Yaw += ((right > 0.f) ? -0.1f : 0.1f);
	}

	if ((up > 0.f) || (down > 0.f))
	{
		m_Pitch += ((down > 0.f) ? 0.1f : -0.1f);
	}

	if (GetAsyncKeyState(VK_HOME))
	{
		m_Yaw = m_Pitch = 0.f;
	}
	

	if (GetAsyncKeyState(VK_ESCAPE))
	{
		ExitGame();
	}

	if (GetAsyncKeyState('R'))
	{
		m_UsedInstanceCount = std::max(c_minInstanceCount, m_UsedInstanceCount - c_increments);
		if(m_UsedInstanceCount <= c_maxInstances)
		{
			std::cout << m_UsedInstanceCount << std::endl;
		}
	}
	if (GetAsyncKeyState('E'))
	{
		m_UsedInstanceCount = std::min(c_maxInstances, m_UsedInstanceCount + c_increments);
		if (m_UsedInstanceCount <= c_maxInstances)
		{
			std::cout << m_UsedInstanceCount << std::endl;
		}
	}

	if (GetAsyncKeyState(VK_SPACE))
	{
		ResetSimulation();
		if (m_UsedInstanceCount <= c_maxInstances)
		{
			std::cout << m_UsedInstanceCount << std::endl;
		}
	}

	// Limit to avoid looking directly up or down
	const float limit = XM_PI / 2.0f - 0.01f;
	m_Pitch = std::max(-limit, std::min(+limit, m_Pitch));

	if (m_Yaw > XM_PI)
	{
		m_Yaw -= XM_PI * 2.f;
	}
	else if (m_Yaw < -XM_PI)
	{
		m_Yaw += XM_PI * 2.f;
	}

	XMVECTOR lookAt = XMVectorSet(
		sinf(m_Yaw),
		m_Pitch,
		cosf(m_Yaw),
		0);

	// Update transforms.
	XMMATRIX camera = XMMatrixLookAtLH(g_XMZero, lookAt, g_XMIdentityR1);
	XMMATRIX proj = XMLoadFloat4x4(&m_Proj);
	XMMATRIX clip = XMMatrixTranspose(XMMatrixMultiply(camera, proj));
	XMStoreFloat4x4(&m_Clip, clip);

	// Update instance data for the next frame.
	for (size_t i = 1; i < m_UsedInstanceCount; ++i)
	{
		// Update positions...
		float velocityMultiplier = i <= c_pointLightCount ? 5.0f * c_velocityMultiplier : c_velocityMultiplier;
		XMVECTOR position = XMLoadFloat4(&m_CPUInstanceData[i].positionAndScale);
		position += m_Velocities[i] * elapsedTime * velocityMultiplier;
		XMStoreFloat4(&m_CPUInstanceData[i].positionAndScale, position);

		float X = m_CPUInstanceData[i].positionAndScale.x;
		float Y = m_CPUInstanceData[i].positionAndScale.y;
		float Z = m_CPUInstanceData[i].positionAndScale.z;

		bool bounce = false;

		// If an instance pops out of bounds in any dimension, reverse velocity in that dimension...
		if (X < -c_boxBounds || X > c_boxBounds)
		{
			m_Velocities[i] *= XMVectorSet(-1.0f, 1.0f, 1.0f, 1.0f);
			bounce = true;
		}
		if (Y < -c_boxBounds || Y > c_boxBounds)
		{
			m_Velocities[i] *= XMVectorSet(1.0f, -1.0f, 1.0f, 1.0f);
			bounce = true;
		}
		if (Z < -c_boxBounds || Z > c_boxBounds)
		{
			m_Velocities[i] *= XMVectorSet(1.0f, 1.0f, -1.0f, 1.0f);
			bounce = true;
		}

		// Apply bounce here.
		if (bounce)
		{
			position = XMLoadFloat4(&m_CPUInstanceData[i].positionAndScale);
			position += m_Velocities[i] * elapsedTime * c_velocityMultiplier;
			XMStoreFloat4(&m_CPUInstanceData[i].positionAndScale, position);
		}

		// Set up point light info.
		if (i <= c_pointLightCount)
		{
			m_Lights.pointPositions[i - 1] = m_CPUInstanceData[i].positionAndScale;
		}

		XMVECTOR q = XMLoadFloat4(&m_CPUInstanceData[i].quaternion);
		q = XMQuaternionNormalizeEst(XMQuaternionMultiply(m_RotationQuaternions[i], q));
		XMStoreFloat4(&m_CPUInstanceData[i].quaternion, q);
	}

	PIXEndEvent();
}

// Draws the scene.
void GameDX12::Render()
{
	// Don't try to render anything before the first Update.
	if (m_Timer.GetFrameCount() == 0)
	{
		return;
	}

	// Check to see if the GPU is keeping up
	int frameIdx = m_DeviceResources->GetCurrentFrameIndex();
	int numBackBuffers = m_DeviceResources->GetBackBufferCount();
	uint64_t completedValue = m_Fence->GetCompletedValue();
	if ((frameIdx > completedValue) // if frame index is reset to zero it may temporarily be smaller than the last GPU signal
		&& (frameIdx - completedValue > numBackBuffers))
	{
		// GPU not caught up, wait for at least one available frame
		DX::ThrowIfFailed(m_Fence->SetEventOnCompletion(frameIdx - numBackBuffers, m_FenceEvent.Get()));
		WaitForSingleObjectEx(m_FenceEvent.Get(), INFINITE, FALSE);
	}

	// Prepare the command list to render a new frame.
	m_DeviceResources->Prepare();
	Clear();

	auto commandList = m_DeviceResources->GetCommandList();
	PIXBeginEvent(commandList, PIX_COLOR_DEFAULT, L"Render");

	commandList->SetGraphicsRootSignature(m_RootSignature.Get());
	commandList->SetPipelineState(m_PipelineState.Get());

	// We use the DirectX Tool Kit helper for managing constants memory
	// (see SimpleLightingUWP12 for how to provide constants without this helper)
	auto vertexConstants = m_GraphicsMemory->AllocateConstant<XMFLOAT4X4>(m_Clip);
	auto pixelConstants = m_GraphicsMemory->AllocateConstant<Lights>(m_Lights);

	commandList->SetGraphicsRootConstantBufferView(0, vertexConstants.GpuAddress());
	commandList->SetGraphicsRootConstantBufferView(1, pixelConstants.GpuAddress());

	// Set necessary state.
	commandList->IASetPrimitiveTopology(D3D_PRIMITIVE_TOPOLOGY_TRIANGLELIST);

	// Provide per-frame instance data
	int instanceIdx = (frameIdx % numBackBuffers);
	int frameOffset = (c_maxInstances * sizeof(Instance)) * instanceIdx;

	memcpy(m_MappedInstanceData + frameOffset, m_CPUInstanceData.get(), sizeof(Instance) * m_UsedInstanceCount);

	m_VertexBufferView[1].BufferLocation = m_InstanceDataGpuAddr + frameOffset;
	m_VertexBufferView[1].StrideInBytes = sizeof(Instance);
	m_VertexBufferView[1].SizeInBytes = sizeof(Instance) * m_UsedInstanceCount;

	// Set up the vertex buffers. We have 3 streams:
	// Stream 1 contains per-primitive vertices defining the cubes.
	// Stream 2 contains the per-instance data for scale, position and orientation
	// Stream 3 contains the per-instance data for color.
	commandList->IASetVertexBuffers(0, _countof(m_VertexBufferView), m_VertexBufferView);

	// The per-instance data is referenced by index...
	commandList->IASetIndexBuffer(&m_IndexBufferView);

	// Draw the entire scene...
	commandList->DrawIndexedInstanced(c_cubeIndexCount, m_UsedInstanceCount, 0, 0, 0);

	// Draw UI.
	ID3D12DescriptorHeap* heaps[] = { m_ResourceDescriptors->Heap() };
	commandList->SetDescriptorHeaps(_countof(heaps), heaps);

	auto size = m_DeviceResources->GetOutputSize();
	auto safe = SimpleMath::Viewport::ComputeTitleSafeArea(size.right, size.bottom);

	m_SpriteBatch->Begin(commandList);

	wchar_t str[32] = {};
	swprintf_s(str, L"Instancing count: %u", m_UsedInstanceCount);
	m_SmallFont->DrawString(m_SpriteBatch.get(), str, XMFLOAT2(float(safe.left), float(safe.top)), Colors::LightGray);

	m_SpriteBatch->End();

	PIXEndEvent(commandList);

	// Show the new frame.
	PIXBeginEvent(m_DeviceResources->GetCommandQueue(), PIX_COLOR_DEFAULT, L"Present");
	m_DeviceResources->Present();
	m_GraphicsMemory->Commit(m_DeviceResources->GetCommandQueue());

	// GPU will signal an increasing value each frame
	m_DeviceResources->GetCommandQueue()->Signal(m_Fence.Get(), frameIdx);

	PIXEndEvent(m_DeviceResources->GetCommandQueue());
}

// Helper method to prepare the command list for rendering and clear the back buffers.
void GameDX12::Clear()
{
	auto commandList = m_DeviceResources->GetCommandList();
	PIXBeginEvent(commandList, PIX_COLOR_DEFAULT, L"Clear");

	// Clear the views.
	auto rtvDescriptor = m_DeviceResources->GetRenderTargetView();
	auto dsvDescriptor = m_DeviceResources->GetDepthStencilView();

	commandList->OMSetRenderTargets(1, &rtvDescriptor, FALSE, &dsvDescriptor);

	// Use linear clear color for gamma-correct rendering.
	commandList->ClearRenderTargetView(rtvDescriptor, Colors::CornflowerBlue, 0, nullptr);
	commandList->ClearDepthStencilView(dsvDescriptor, D3D12_CLEAR_FLAG_DEPTH, 1.0f, 0, 0, nullptr);

	// Set the viewport and scissor rect.
	auto viewport = m_DeviceResources->GetScreenViewport();
	auto scissorRect = m_DeviceResources->GetScissorRect();
	commandList->RSSetViewports(1, &viewport);
	commandList->RSSetScissorRects(1, &scissorRect);

	PIXEndEvent(commandList);
}

void GameDX12::CreateDeviceDependentResources()
{
	auto device = m_DeviceResources->GetD3DDevice();

	m_GraphicsMemory = std::make_unique<GraphicsMemory>(device);

	m_ResourceDescriptors = std::make_unique<DescriptorHeap>(device, Descriptors::Count);

	ResourceUploadBatch resourceUpload(device);

	resourceUpload.Begin();

	{
		RenderTargetState rtState(m_DeviceResources->GetBackBufferFormat(), m_DeviceResources->GetDepthBufferFormat());

		SpriteBatchPipelineStateDescription pd(rtState);

		m_SpriteBatch = std::make_unique<SpriteBatch>(device, resourceUpload, pd);
	}

	m_SmallFont = std::make_unique<SpriteFont>(device, resourceUpload,
		L"files/SegoeUI_18.spritefont",
		m_ResourceDescriptors->GetCpuHandle(Descriptors::TextFont),
		m_ResourceDescriptors->GetGpuHandle(Descriptors::TextFont));

	// Create a root signature
	{
		CD3DX12_ROOT_PARAMETER rootParameters[2] = {};
		rootParameters[0].InitAsConstantBufferView(0, 0, D3D12_SHADER_VISIBILITY_VERTEX);
		rootParameters[1].InitAsConstantBufferView(0, 0, D3D12_SHADER_VISIBILITY_PIXEL);

		// Allow input layout and deny uneccessary access to certain pipeline stages.
		D3D12_ROOT_SIGNATURE_FLAGS rootSignatureFlags =
			D3D12_ROOT_SIGNATURE_FLAG_ALLOW_INPUT_ASSEMBLER_INPUT_LAYOUT |
			D3D12_ROOT_SIGNATURE_FLAG_DENY_HULL_SHADER_ROOT_ACCESS |
			D3D12_ROOT_SIGNATURE_FLAG_DENY_DOMAIN_SHADER_ROOT_ACCESS |
			D3D12_ROOT_SIGNATURE_FLAG_DENY_GEOMETRY_SHADER_ROOT_ACCESS;

		CD3DX12_ROOT_SIGNATURE_DESC rootSignatureDesc = {};
		rootSignatureDesc.Init(_countof(rootParameters), rootParameters, 0, nullptr, rootSignatureFlags);

		ComPtr<ID3DBlob> signature;
		ComPtr<ID3DBlob> error;
		DX::ThrowIfFailed(
			D3D12SerializeRootSignature(&rootSignatureDesc, D3D_ROOT_SIGNATURE_VERSION_1, &signature, &error));
		DX::ThrowIfFailed(
			device->CreateRootSignature(0, signature->GetBufferPointer(), signature->GetBufferSize(),
				IID_PPV_ARGS(m_RootSignature.ReleaseAndGetAddressOf())));
	}

	// Create the pipeline state, which includes loading shaders.
	auto vertexShaderBlob = DX::ReadData(L"VertexShader.cso");

	auto pixelShaderBlob = DX::ReadData(L"PixelShader.cso");

	static const D3D12_INPUT_ELEMENT_DESC s_inputElementDesc[] =
	{
		// SemanticName SemanticIndex   Format                          InputSlot AlignedByteOffset             InputSlotClass                                  InstanceDataStepRate
		{ "POSITION",   0,              DXGI_FORMAT_R32G32B32_FLOAT,    0,        0,                            D3D12_INPUT_CLASSIFICATION_PER_VERTEX_DATA,     0 },  // Vertex local position
		{ "NORMAL",     0,              DXGI_FORMAT_R32G32B32_FLOAT,    0,        D3D12_APPEND_ALIGNED_ELEMENT, D3D12_INPUT_CLASSIFICATION_PER_VERTEX_DATA,     0 },  // Vertex normal
		{ "I_ROTATION", 0,              DXGI_FORMAT_R32G32B32A32_FLOAT, 1,        0,                            D3D12_INPUT_CLASSIFICATION_PER_INSTANCE_DATA,   1 },  // Instance rotation quaternion
		{ "I_POSSCALE", 0,              DXGI_FORMAT_R32G32B32A32_FLOAT, 1,        D3D12_APPEND_ALIGNED_ELEMENT, D3D12_INPUT_CLASSIFICATION_PER_INSTANCE_DATA,   1 },  // Instance position and scale (scale in "w")
		{ "I_COLOR",    0,              DXGI_FORMAT_R8G8B8A8_UNORM,     2,        D3D12_APPEND_ALIGNED_ELEMENT, D3D12_INPUT_CLASSIFICATION_PER_INSTANCE_DATA,   1 },  // Instance color
	};

	// Describe and create the graphics pipeline state object (PSO).
	D3D12_GRAPHICS_PIPELINE_STATE_DESC psoDesc = {};
	psoDesc.InputLayout = { s_inputElementDesc, _countof(s_inputElementDesc) };
	psoDesc.pRootSignature = m_RootSignature.Get();
	psoDesc.VS = { vertexShaderBlob.data(), vertexShaderBlob.size() };
	psoDesc.PS = { pixelShaderBlob.data(), pixelShaderBlob.size() };
	psoDesc.RasterizerState = CD3DX12_RASTERIZER_DESC(D3D12_DEFAULT);
	psoDesc.BlendState = CD3DX12_BLEND_DESC(D3D12_DEFAULT);
	psoDesc.DepthStencilState.DepthEnable = FALSE;
	psoDesc.DepthStencilState.StencilEnable = FALSE;
	psoDesc.DSVFormat = m_DeviceResources->GetDepthBufferFormat();
	psoDesc.SampleMask = UINT_MAX;
	psoDesc.PrimitiveTopologyType = D3D12_PRIMITIVE_TOPOLOGY_TYPE_TRIANGLE;
	psoDesc.NumRenderTargets = 1;
	psoDesc.RTVFormats[0] = m_DeviceResources->GetBackBufferFormat();
	psoDesc.SampleDesc.Count = 1;
	DX::ThrowIfFailed(
		device->CreateGraphicsPipelineState(&psoDesc,
			IID_PPV_ARGS(m_PipelineState.ReleaseAndGetAddressOf())));

	m_DeviceResources->GetCommandList()->SetPipelineState(m_PipelineState.Get());

	// Create and initialize the vertex buffer
	{
		CD3DX12_HEAP_PROPERTIES heapUpload(D3D12_HEAP_TYPE_UPLOAD);
		CD3DX12_HEAP_PROPERTIES heapDefault(D3D12_HEAP_TYPE_DEFAULT);

		std::vector<Vertex> verts{ ModelManager::GetInstance()->GetVerts() };

		// Note: using upload heaps to transfer static data like vert buffers is not 
		// recommended. Every time the GPU needs it, the upload heap will be marshalled 
		// over. Please read up on Default Heap usage. An upload heap is used here for 
		// code simplicity and because there are very few verts to actually transfer.
		auto resDesc = CD3DX12_RESOURCE_DESC::Buffer(sizeof(Vertex) * static_cast<uint32_t>(verts.size()));

		DX::ThrowIfFailed(
			device->CreateCommittedResource(
				&heapDefault,
				D3D12_HEAP_FLAG_NONE,
				&resDesc,
				D3D12_RESOURCE_STATE_COPY_DEST,
				nullptr,
				IID_PPV_ARGS(m_VertexBuffer.ReleaseAndGetAddressOf())));

		m_VertexBuffer->SetName(L"Vertex Buffer");

		DX::ThrowIfFailed(
			device->CreateCommittedResource(
				&heapUpload,
				D3D12_HEAP_FLAG_NONE,
				&resDesc,
				D3D12_RESOURCE_STATE_GENERIC_READ,
				nullptr,
				IID_PPV_ARGS(m_VertexBufferUpload.ReleaseAndGetAddressOf())));

		m_VertexBufferUpload->SetName(L"Vertex Upload Buffer");

		// Copy data to the upload heap and then schedule a copy 
		// from the upload heap to the vertex buffer.
		D3D12_SUBRESOURCE_DATA vertexData{};
		vertexData.pData = reinterpret_cast<BYTE*>(verts.data());
		vertexData.RowPitch = sizeof(Vertex) * static_cast<uint32_t>(verts.size());
		vertexData.SlicePitch = vertexData.RowPitch;

		PIXBeginEvent(m_DeviceResources->GetCommandList(), 0, L"Copy vertex buffer data to default resource...");
		
		auto transition{ CD3DX12_RESOURCE_BARRIER::Transition(m_VertexBuffer.Get(), D3D12_RESOURCE_STATE_COPY_DEST,
													 D3D12_RESOURCE_STATE_VERTEX_AND_CONSTANT_BUFFER) };

		UpdateSubresources<1>(m_DeviceResources->GetCommandList(), m_VertexBuffer.Get(), m_VertexBufferUpload.Get(), 0, 0, 1, &vertexData);
		m_DeviceResources->GetCommandList()->ResourceBarrier(1, &transition);

		PIXEndEvent(m_DeviceResources->GetCommandList());

		// Initialize the vertex buffer view.
		m_VertexBufferView[0].BufferLocation = m_VertexBuffer->GetGPUVirtualAddress();
		m_VertexBufferView[0].StrideInBytes = sizeof(Vertex);
		m_VertexBufferView[0].SizeInBytes = sizeof(Vertex) * static_cast<uint32_t>(verts.size());
	}

	// Create vertex buffer memory for per-instance data.
	{
		D3D12_HEAP_PROPERTIES defaultHeap{ D3D12_HEAP_TYPE_DEFAULT };
		const D3D12_HEAP_PROPERTIES uploadHeapProperties = CD3DX12_HEAP_PROPERTIES(D3D12_HEAP_TYPE_UPLOAD);
		size_t cbSize = c_maxInstances * m_DeviceResources->GetBackBufferCount() * sizeof(Instance);
		const D3D12_RESOURCE_DESC instanceBufferDesc = CD3DX12_RESOURCE_DESC::Buffer(cbSize);
		{
			DX::ThrowIfFailed(device->CreateCommittedResource(
				&uploadHeapProperties,
				D3D12_HEAP_FLAG_NONE,
				&instanceBufferDesc,
				D3D12_RESOURCE_STATE_GENERIC_READ,
				nullptr,
				IID_PPV_ARGS(m_InstanceData.ReleaseAndGetAddressOf())));

			m_InstanceData->SetName(L"Instance Buffer");
		}
		

		DX::ThrowIfFailed(m_InstanceData->Map(0, nullptr, reinterpret_cast<void**>(&m_MappedInstanceData)));

		m_InstanceDataGpuAddr = m_InstanceData->GetGPUVirtualAddress();
	}

	// Create a static vertex buffer with color data.
	{
		static const XMVECTORF32 s_bigCubeColor = { 1.f, 1.f, 1.f, 0.f };
		uint32_t colors[c_maxInstances];
		colors[0] = PackedVector::XMCOLOR(s_bigCubeColor);
		for (uint32_t i = 1; i < c_maxInstances; ++i)
		{
			if (i <= c_pointLightCount)
			{
				m_Lights.pointColors[i - 1] = XMFLOAT4(FloatRand(0.25f, 1.0f), FloatRand(0.25f, 1.0f), FloatRand(0.25f, 1.0f), 1.0f);
				colors[i] = PackedVector::XMCOLOR(m_Lights.pointColors[i - 1].x, m_Lights.pointColors[i - 1].y, m_Lights.pointColors[i - 1].z, 1.f);
			}
			else
			{
				colors[i] = PackedVector::XMCOLOR(FloatRand(0.25f, 1.0f), FloatRand(0.25f, 1.0f), FloatRand(0.25f, 1.0f), 0.f);
			}
		}

		CD3DX12_HEAP_PROPERTIES heapUpload(D3D12_HEAP_TYPE_UPLOAD);
		CD3DX12_HEAP_PROPERTIES heapDefault(D3D12_HEAP_TYPE_DEFAULT);
		auto resDesc = CD3DX12_RESOURCE_DESC::Buffer(sizeof(uint32_t) * c_maxInstances);

		DX::ThrowIfFailed(
			device->CreateCommittedResource(
				&heapDefault,
				D3D12_HEAP_FLAG_NONE,
				&resDesc,
				D3D12_RESOURCE_STATE_COPY_DEST,
				nullptr,
				IID_PPV_ARGS(m_BoxColors.ReleaseAndGetAddressOf())));
		m_BoxColors->SetName(L"Color Buffer");
		{
			DX::ThrowIfFailed(
				device->CreateCommittedResource(
					&heapUpload,
					D3D12_HEAP_FLAG_NONE,
					&resDesc,
					D3D12_RESOURCE_STATE_GENERIC_READ,
					nullptr,
					IID_PPV_ARGS(m_BoxColorsUpload.ReleaseAndGetAddressOf())));
			m_BoxColorsUpload->SetName(L"Color Upload Buffer");

			D3D12_SUBRESOURCE_DATA colorData{};
			colorData.pData = reinterpret_cast<BYTE*>(colors);
			colorData.RowPitch = sizeof(uint32_t) * c_maxInstances;
			colorData.SlicePitch = colorData.RowPitch;

			PIXBeginEvent(m_DeviceResources->GetCommandList(), 0, L"Copy vertex buffer data to default resource...");

			auto transition{ CD3DX12_RESOURCE_BARRIER::Transition(m_BoxColors.Get(), D3D12_RESOURCE_STATE_COPY_DEST,
														 D3D12_RESOURCE_STATE_VERTEX_AND_CONSTANT_BUFFER) };


			UpdateSubresources<1>(m_DeviceResources->GetCommandList(), m_BoxColors.Get(), m_BoxColorsUpload.Get(), 0, 0, 1, &colorData);
			m_DeviceResources->GetCommandList()->ResourceBarrier(1, &transition);

			PIXEndEvent(m_DeviceResources->GetCommandList());

		}

		// Initialize the vertex buffer view.
		m_VertexBufferView[2].BufferLocation = m_BoxColors->GetGPUVirtualAddress();
		m_VertexBufferView[2].StrideInBytes = sizeof(uint32_t);
		m_VertexBufferView[2].SizeInBytes = sizeof(uint32_t) * c_maxInstances;
	}

	// Create and initialize the index buffer
	{
		std::vector<uint32_t> indcs{ ModelManager::GetInstance()->GetIndices() };
		c_cubeIndexCount = static_cast<uint32_t>(indcs.size());

		// See note above
		CD3DX12_HEAP_PROPERTIES heapUpload(D3D12_HEAP_TYPE_UPLOAD);
		CD3DX12_HEAP_PROPERTIES heapDefault(D3D12_HEAP_TYPE_DEFAULT);
		auto resDesc = CD3DX12_RESOURCE_DESC::Buffer(sizeof(uint32_t) * static_cast<uint32_t>(indcs.size()));

		DX::ThrowIfFailed(
			device->CreateCommittedResource(
				&heapDefault,
				D3D12_HEAP_FLAG_NONE,
				&resDesc,
				D3D12_RESOURCE_STATE_COPY_DEST,
				nullptr,
				IID_PPV_ARGS(m_IndexBuffer.ReleaseAndGetAddressOf())));

		m_IndexBuffer->SetName(L"Index Buffer");

		{
			DX::ThrowIfFailed(
				device->CreateCommittedResource(
					&heapUpload,
					D3D12_HEAP_FLAG_NONE,
					&resDesc,
					D3D12_RESOURCE_STATE_GENERIC_READ,
					nullptr,
					IID_PPV_ARGS(m_IndexBufferUpload.ReleaseAndGetAddressOf())));

			D3D12_SUBRESOURCE_DATA indexData{};
			indexData.pData = reinterpret_cast<BYTE*>(indcs.data());

			auto transition{ CD3DX12_RESOURCE_BARRIER::Transition(m_IndexBuffer.Get(), D3D12_RESOURCE_STATE_COPY_DEST, D3D12_RESOURCE_STATE_INDEX_BUFFER) };

			UpdateSubresources<1>(m_DeviceResources->GetCommandList(), m_IndexBuffer.Get(), m_IndexBufferUpload.Get(), 0, 0, 1, &indexData);
			m_DeviceResources->GetCommandList()->ResourceBarrier(1, &transition);

		}

		// Initialize the index buffer view.
		m_IndexBufferView.BufferLocation = m_IndexBuffer->GetGPUVirtualAddress();
		m_IndexBufferView.Format = DXGI_FORMAT_R32_UINT;
		m_IndexBufferView.SizeInBytes = sizeof(uint32_t) * static_cast<uint32_t>(indcs.size());
	}

	ThrowIfFailed(m_DeviceResources->GetCommandList()->Close());
	ID3D12CommandList* ppCommandLists[] = { m_DeviceResources->GetCommandList() };
	m_DeviceResources->GetCommandQueue()->ExecuteCommandLists(_countof(ppCommandLists), ppCommandLists);

	m_CPUInstanceData.reset(new Instance[c_maxInstances]);
	m_RotationQuaternions.reset(reinterpret_cast<XMVECTOR*>(_aligned_malloc(sizeof(XMVECTOR) * c_maxInstances, 16)));
	m_Velocities.reset(reinterpret_cast<XMVECTOR*>(_aligned_malloc(sizeof(XMVECTOR) * c_maxInstances, 16)));

	// Initialize the directional light.
	XMStoreFloat4(&m_Lights.directional, XMVector3Normalize(XMVectorSet(1.0f, 4.0f, -2.0f, 0)));

	// Initialize the positions/state of all the cubes in the scene.
	ResetSimulation();

	// Wait until assets have been uploaded to the GPU.
	auto uploadResourcesFinished = resourceUpload.End(m_DeviceResources->GetCommandQueue());
	uploadResourcesFinished.wait();

	// Create a fence for synchronizing between the CPU and the GPU
	DX::ThrowIfFailed(device->CreateFence(m_DeviceResources->GetCurrentFrameIndex(), D3D12_FENCE_FLAG_NONE,
		IID_PPV_ARGS(m_Fence.ReleaseAndGetAddressOf())));

}

void GameDX12::CreateWindowSizeDependentResources()
{
	// Initialize the projection matrix.
	auto size = m_DeviceResources->GetOutputSize();

	XMMATRIX proj = XMMatrixPerspectiveFovLH(XM_PIDIV4, float(size.right) / float(size.bottom), 0.1f, 500.0f);

	//XMFLOAT4X4 orient = m_DeviceResources->GetOrientationTransform3D();
	XMStoreFloat4x4(&m_Proj, proj /** XMLoadFloat4x4(&orient)*/);

	// Set the viewport for our SpriteBatch.
	m_SpriteBatch->SetViewport(m_DeviceResources->GetScreenViewport());

	// The frame index will be reset to zero when the window size changes
	// So we need to tell the GPU to signal our fence starting with zero
	uint64_t currentIdx = m_DeviceResources->GetCurrentFrameIndex();
	m_DeviceResources->GetCommandQueue()->Signal(m_Fence.Get(), currentIdx);
}

void GameDX12::ResetSimulation()
{
	// Reset positions to starting point, and orientations to identity.
	// Note that instance 0 is the scene bounding box, and the position, orientation and scale are static (i.e. never update).
	for (size_t i = 1; i < c_maxInstances; ++i)
	{
		m_CPUInstanceData[i].positionAndScale = XMFLOAT4(0.0f, 0.0f, c_boxBounds / 2.0f, FloatRand(40.f, 45.f));
		m_CPUInstanceData[i].quaternion = XMFLOAT4(0.0f, 0.0f, 0.0f, 1.0f);

		// For the first c_pointLightCount in the updated array, we scale up by a small factor so they stand out.
		if (i <= c_pointLightCount)
		{
			m_CPUInstanceData[i].positionAndScale.w = 1.53f;
			m_Lights.pointPositions[i - 1] = m_CPUInstanceData[i].positionAndScale;
		}

		// Apply a random spin to each instance.
		m_RotationQuaternions[i] = XMQuaternionRotationAxis(XMVector3Normalize(XMVectorSet(FloatRand(), FloatRand(), FloatRand(), 0)), FloatRand(0.001f, 0.1f));

		// ...and a random velocity.
		m_Velocities[i] = XMVectorSet(FloatRand(-0.01f, 0.01f), FloatRand(-0.01f, 0.01f), FloatRand(-0.01f, 0.01f), 0);
	}
}

float GameDX12::FloatRand(float lowerBound, float upperBound)
{
	if (lowerBound == upperBound)
		return lowerBound;

	std::uniform_real_distribution<float> dist(lowerBound, upperBound);

	return dist(m_RandomEngine);
}


void GameDX12::OnActivated()
{
	// TODO: Game is becoming active window.
}

void GameDX12::OnDeactivated()
{
	// TODO: Game is becoming background window.
}

void GameDX12::OnSuspending()
{
	// TODO: Game is being power-suspended (or minimized).
}

void GameDX12::OnResuming()
{
	m_Timer.ResetElapsedTime();

	// TODO: Game is being power-resumed (or returning from minimize).
}

void GameDX12::OnWindowMoved()
{
	auto const r = m_DeviceResources->GetOutputSize();
	m_DeviceResources->WindowSizeChanged(r.right, r.bottom);
}

void GameDX12::OnDisplayChange()
{
	//m_DeviceResources->UpdateColorSpace();
}

void GameDX12::OnWindowSizeChanged(int width, int height)
{
	BaseGame::OnWindowSizeChanged(width, height);
	if (!m_DeviceResources->WindowSizeChanged(width, height))
		return;

	CreateWindowSizeDependentResources();

	// TODO: Game window is being resized.
}

void GameDX12::OnDeviceLost()
{
	// If using the DirectX Tool Kit for DX12, uncomment this line:
	m_SpriteBatch.reset();
	//m_SmallFont.reset();
	//m_ctrlFont.reset();

	m_RootSignature.Reset();
	m_PipelineState.Reset();
	m_VertexBuffer.Reset();
	m_IndexBuffer.Reset();
	m_BoxColors.Reset();

	m_InstanceData.Reset();
	m_MappedInstanceData = nullptr;
	m_InstanceDataGpuAddr = 0;
	m_Fence.Reset();

	m_ResourceDescriptors.reset();
	m_GraphicsMemory.reset();
}

void GameDX12::OnDeviceRestored()
{
	CreateDeviceDependentResources();

	CreateWindowSizeDependentResources();
}