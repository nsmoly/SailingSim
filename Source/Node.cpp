/* 
 * Sailboat simulation game.
 *
 * Copyright (c) Nikolai Smolyanskiy, 2009-2010. All rights reserved. 
 *
 * This software is provided "as is" without express or implied
 * warranties. You may freely copy and compile this source into
 * applications you distribute.
 */

#include "DXUT.h"
#include "Node.h"

Node::Node(IDirect3DDevice9* pd3dDevice)
{
}

Node::~Node()
{
	Unload();
}

HRESULT Node::Load(IDirect3DDevice9* /* pd3dDevice */, const LPCWSTR /* szDataPath */)
{
	return MAKE_HRESULT(SEVERITY_SUCCESS, 0, 0);
}

HRESULT Node::GetInstance(const LPCWSTR /* szDataPath */)
{
	return MAKE_HRESULT(SEVERITY_SUCCESS, 0, 0);
}

void Node::Unload()
{
	// Recurse on children
	std::vector<std::tr1::shared_ptr<Node> >::const_iterator it;
	for (it = m_vecpChildren.begin(); it != m_vecpChildren.end(); ++it)
	{
		(*it)->Unload();
		// no need to delete each Node since they are pointed to by the smart pointers - shared_ptr<Node>
	}
}

void Node::AddChild(std::tr1::shared_ptr<Node> pNode)
{
	m_vecpChildren.push_back(pNode);
}

void Node::Update(double fTime)
{
	// Recurse on children
	std::vector<std::tr1::shared_ptr<Node> >::const_iterator it;
	for (it = m_vecpChildren.begin(); it != m_vecpChildren.end(); ++it)
	{
		(*it)->Update(fTime);
	}
}

void Node::Render(IDirect3DDevice9* pd3dDevice, const D3DXMATRIX& viewProj)
{
	// Recurse on children
	std::vector<std::tr1::shared_ptr<Node> >::const_iterator it;
	for (it = m_vecpChildren.begin(); it != m_vecpChildren.end(); ++it)
	{
		(*it)->Render(pd3dDevice, viewProj);
	}
}
