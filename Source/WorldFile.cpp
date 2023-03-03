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
#include "WorldFile.h"

#pragma warning(disable : 4996)

////////////////////////////////////////////////////////////////////////////


WorldFile::WorldFile() : 
    m_cx(0)
,   m_cy(0)
,	m_pGrid(0)
{
}


WorldFile::~WorldFile()
{
    delete [] m_pGrid;
}


bool WorldFile::Load(const LPCWSTR szFilename)
{
    SAFE_DELETE_ARRAY(m_pGrid);

	FILE*   fp = _wfopen(szFilename, L"rt");
    if (fp)
    {
        char sz[32768];
        fgets (sz, sizeof(sz), fp);
        sscanf(sz, "%d,%d", &m_cx, &m_cy);
        m_pGrid = new ECell[m_cy * m_cx];

        for (int y = m_cy - 1; y >= 0; --y)
        {
            fgets (sz, sizeof(sz), fp);
            char *pc = sz;
            for (int x = 0; x < m_cx; x++)
            {
                if (*pc == '#')
                {
                    m_pGrid[y * m_cx + x] = OCCUPIED_CELL;
                }
                else if (*pc == '.')
                {
                    m_pGrid[y * m_cx + x] = EMPTY_CELL;
                }
                else
                {
                    m_pGrid[y * m_cx + x] = INVALID_CELL;
                }
                ++pc;
            }
        }
        fclose (fp);
        return true;
    }
    return false;
}

WorldFile::ECell WorldFile::operator () ( int row, int col ) const
{
    if (m_pGrid)
    {
        if (0 <= row && row < m_cy && 0 <= col && col < m_cx)
        {
            return m_pGrid[row * m_cx + col];
        }
    }
    return INVALID_CELL;
}
