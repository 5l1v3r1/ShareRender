#ifndef __WRAP_DIRECT3DINDEXBUFFER9__
#define __WRAP_DIRECT3DINDEXBUFFER9__

#include "GameServer.h"
#include "NewHash.hpp"
#include <map>
#include <iostream>
#include "CommandServerSet.h"
using namespace std;

class YMesh;

class WrapperDirect3DIndexBuffer9: public IDirect3DIndexBuffer9
#ifdef MULTI_CLIENTS
	, public IdentifierBase
#endif
{
private:
	bool isLock;

public:

#ifdef MULTI_CLIENTS
	virtual int checkCreation(void * ctx); // to check the index buffer is created at remote side or not
	virtual int sendCreation(void * ctx);   // send the creation command to remote.
	virtual int checkUpdate(void * ctx);
	virtual int sendUpdate(void * ctx);

#endif
	YashTable mesh_table_;
	// mesh concern
	YMesh * GetYMesh(int vb_id, INT baseVertexIndex, UINT startIndex);
	void PutYMesh(int vb_id, INT baseVertexIndex, UINT startIndex, YMesh * yMesh);

	IDirect3DIndexBuffer9* m_ib;
	char* cache_buffer; //������ʱ��ǵ�Ҫɾ��
	char * ram_buffer; // the temp buffer for lock, each context has a buffer
	char * ram_buffers;  // arrary for all buffers

	bool isFirst;

	DWORD Usage;
	D3DFORMAT Format;
	D3DPOOL Pool;
	char Stride;

	int length;
	BufferLockData m_LockData;

	static HashSet m_list;
	static int ins_count;

	WrapperDirect3DIndexBuffer9(IDirect3DIndexBuffer9* ptr, int _id, int _length);
	WrapperDirect3DIndexBuffer9(IDirect3DIndexBuffer9* ptr, int _id);
	LPDIRECT3DINDEXBUFFER9 GetIB9();

	int GetLength();

	int PrepareIndexBuffer();
	int PrepareIndexBuffer(ContextAndCache * ctx);
	void read_data_from_buffer(char** ptr, int offest, int size);
	void write_data_to_buffer(char* ptr, int offest, int size);

	static WrapperDirect3DIndexBuffer9* GetWrapperIndexedBuffer9(IDirect3DIndexBuffer9* base_indexed_buffer);

public:
	/*** IUnknown methods ***/
	COM_METHOD(HRESULT, QueryInterface)(THIS_ REFIID riid, void** ppvObj);
	COM_METHOD(ULONG,AddRef)(THIS);
	COM_METHOD(ULONG,Release)(THIS);

	/*** IDirect3DResource9 methods ***/
	COM_METHOD(HRESULT, GetDevice)(THIS_ IDirect3DDevice9** ppDevice);
	COM_METHOD(HRESULT, SetPrivateData)(THIS_ REFGUID refguid,CONST void* pData,DWORD SizeOfData,DWORD Flags);
	COM_METHOD(HRESULT, GetPrivateData)(THIS_ REFGUID refguid,void* pData,DWORD* pSizeOfData);
	COM_METHOD(HRESULT, FreePrivateData)(THIS_ REFGUID refguid);
	COM_METHOD(DWORD, SetPriority)(THIS_ DWORD PriorityNew);
	COM_METHOD(DWORD, GetPriority)(THIS);
	COM_METHOD(void, PreLoad)(THIS);
	COM_METHOD(D3DRESOURCETYPE, GetType)(THIS);
	COM_METHOD(HRESULT, Lock)(THIS_ UINT OffsetToLock,UINT SizeToLock,void** ppbData,DWORD Flags);
	COM_METHOD(HRESULT, Unlock)(THIS);
	COM_METHOD(HRESULT, GetDesc)(THIS_ D3DINDEXBUFFER_DESC *pDesc);
};

#endif
