/* 
 * Sailboat simulation game.
 *
 * Copyright (c) Nikolai Smolyanskiy, 2009-2010. All rights reserved. 
 *
 * This software is provided "as is" without express or implied
 * warranties. You may freely copy and compile this source into
 * applications you distribute.
 */

#pragma once
#include <vector>

/**
	Base class for all scene graph nodes. It provides update and rendering capabilities
*/
class Node
{
public:
	Node(IDirect3DDevice9* pd3dDevice);
	virtual	~Node();

	virtual HRESULT	Load(IDirect3DDevice9* pd3dDevice, const LPCWSTR szDataPath);
	virtual HRESULT	GetInstance(const LPCWSTR szFilename);
	virtual void Unload();

	// Update traversal for physics, AI, etc.
	virtual void Update(double fTime);

	// Render traversal for drawing objects
	virtual void Render(IDirect3DDevice9* pd3dDevice, const D3DXMATRIX& viewProj);

	// Hierarchy management
	int GetNumChildren() { return m_vecpChildren.size(); }
	std::tr1::shared_ptr<Node> GetChild(int iChild)	{return m_vecpChildren[iChild];}
	void AddChild(std::tr1::shared_ptr<Node> pNode);

protected:
	std::vector<std::tr1::shared_ptr<Node> >	m_vecpChildren;
};
