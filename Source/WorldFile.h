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

class WorldFile
{
public:
    WorldFile();
    ~WorldFile();

    enum ECell
    {
        INVALID_CELL = -1, 
        EMPTY_CELL = 0, 
        OCCUPIED_CELL = 1, 
        CELL_MAX
    };

    bool        Load(const LPCWSTR szFilename);

    ECell       operator () ( int row, int col ) const;
	int         GetWidth() const  { return m_cx; }
	int         GetHeight() const { return m_cy; }

private:
    int m_cx, m_cy;
    ECell* m_pGrid;
};
