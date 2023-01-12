#include "pch.h"
#include "ModelManager.h"

#include "ObjParser.h"

ModelManager* ModelManager::m_Instance = nullptr;

ModelManager* ModelManager::GetInstance()
{
	if(!m_Instance)
	{
		m_Instance = new ModelManager{};
	}
	return m_Instance;
}

void ModelManager::Release()
{
	delete m_Instance;
}

ModelManager::ModelManager()
{
	Init();
}


void ModelManager::Init()
{
	OBJ::ParseOBJ("files/stanford_dragon.obj", m_Verts, m_Indices);
}
