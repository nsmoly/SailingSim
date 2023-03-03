/* Copyright Steve Rabin, 2008. 
 * All rights reserved worldwide.
 *
 * This software is provided "as is" without express or implied
 * warranties. You may freely copy and compile this source into
 * applications you distribute provided that the copyright text
 * below is included in the resulting source code, for example:
 * "Portions Copyright Steve Rabin, 2008"
 */

#pragma once

#include <assert.h>
#include <iostream>

//Singleton class as authored by Scott Bilas in the book Game Programming Gems

template <typename T>
class Singleton
{
public:
	Singleton( void )
	{
		assert( ms_Singleton == 0 && "Singleton constructor" );
		intptr_t offset = (intptr_t)(T*)1 - (intptr_t)(Singleton <T> *)(T*)1;
		ms_Singleton = (T*)((intptr_t)this + offset);
	}
	~Singleton( void )  {  assert( ms_Singleton != 0 && "Singleton destructor" ); 
ms_Singleton = 0;  }

	static T&   GetSingleton      ( void )  {  assert( ms_Singleton != 0 && "Singleton - GetSingleton" );  return ( *ms_Singleton );  }
	static T*   GetSingletonPtr   ( void )  {  return ( ms_Singleton );  }
	static bool DoesSingletonExist( void )  {  return ( ms_Singleton != 0 );  }

private:
	static T* ms_Singleton;

};

template <typename T> T* Singleton <T>::ms_Singleton = 0;