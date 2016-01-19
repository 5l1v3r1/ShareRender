#include "CommandServerSet.h"
#include "WrapDirect3dvertexbuffer9.h"
#include "../LibCore/Opcode.h"
//#define FLOAT_COMPRESS
#define INT_COMPRESS
#define COMPRESS_TO_DWORD


//#define ENABLE_VERTEX_BUFFER_LOG


#ifdef MULTI_CLIENTS
// check the creation flag for each client
int WrapperDirect3DVertexBuffer9::sendCreation(void * ctx){
#ifdef ENABLE_VERTEX_BUFFER_LOG
	infoRecorder->logTrace("[WrapperDirect3DVertexBuffer9]: send creation.\n");
#endif

	ContextAndCache * c = (ContextAndCache *)ctx;

	c->beginCommand(CreateVertexBuffer_Opcode, getDeviceId());
	c->write_uint(getId());
	c->write_uint(Length);
	c->write_uint(Usage);
	c->write_uint(FVF);
	c->write_uint(Pool);
	c->endCommand();

	return 0;
}
int WrapperDirect3DVertexBuffer9::checkCreation(void *ctx){
	#ifdef ENABLE_VERTEX_BUFFER_LOG
	infoRecorder->logTrace("[WrapperDirect3DVertexBuffer9]: check creation.\n");
#endif
	int ret= 0;
	ContextAndCache * c = (ContextAndCache *)ctx;
	if(!c->isCreated(creationFlag)){
		// not created, send the creation command.
		ret = sendCreation(ctx);
		// change the creation flag
		c->setCreation(creationFlag);
		ret = 1;
	}
	return ret;
}
int WrapperDirect3DVertexBuffer9::checkUpdate(void * ctx){
	#ifdef ENABLE_VERTEX_BUFFER_LOG
	infoRecorder->logTrace("[WrapperDirect3DVertexBuffer9]: check update.\n");
#endif
	int ret = 0;
	ContextAndCache * c = (ContextAndCache *)ctx;
	if(c->isChanged(updateFlag)){
		ret = sendUpdate(ctx);

		c->resetChanged(updateFlag);
		ret = 1;
	}else{
		#ifdef ENABLE_VERTEX_BUFFER_LOG
		// no changed
		infoRecorder->logTrace("[WrapperDirect3DVertexBuffer9]: unchanged.\n");
#endif
	}
	return ret;
}
int WrapperDirect3DVertexBuffer9::sendUpdate(void * ctx){
	#ifdef ENABLE_VERTEX_BUFFER_LOG
	infoRecorder->logTrace("[WrapperDirect3DVertexBuffer9]: send update.\n");
#endif
	ContextAndCache * c = (ContextAndCache *)ctx;
	PrepareVertexBuffer(c);
	return 0;
}

#endif

WrapperDirect3DVertexBuffer9::WrapperDirect3DVertexBuffer9(IDirect3DVertexBuffer9* ptr, int _id, int _length): m_vb(ptr), isLock(false), IdentifierBase(_id), Length(_length) {
#ifdef ENABLE_VERTEX_BUFFER_LOG
	infoRecorder->logTrace("WrapperDirect3DVertexBuffer9 constructor called, id=%d, length=%d\n", _id, _length);
#endif

	cache_buffer = new char[_length];
	memset(cache_buffer, 0, _length);
	ram_buffer = new char[_length];
	m_LockData.pRAMBuffer = ram_buffer;
	isFirst = true;
	changed = false;
	decl = NULL;
	this->maxFlag = false;
	stride = -1;

	readed_ = false;

	y_mesh_ = NULL;

	m_list.AddMember(ptr, this);

	max_vb = 0;

	creationFlag = 0;
	updateFlag = 0x8fffffff;
	stable = false;
}

WrapperDirect3DVertexBuffer9::WrapperDirect3DVertexBuffer9(IDirect3DVertexBuffer9* ptr, int _id): m_vb(ptr), isLock(false), IdentifierBase(_id), Length(0) {
	//cache_buffer = new char[_length];
	isFirst = true;
	this->maxFlag = false;
	changed = false;
	decl = NULL;
	stride = -1;

	y_mesh_ = NULL;

	m_list.AddMember(ptr, this);
	max_vb = 0;
	creationFlag= 0;
	updateFlag = 0x8fffffff;
	stable = false;
}


LPDIRECT3DVERTEXBUFFER9 WrapperDirect3DVertexBuffer9::GetVB9() {
	return m_vb;
}

int WrapperDirect3DVertexBuffer9::GetLength() {
	return this->Length;
}

WrapperDirect3DVertexBuffer9* WrapperDirect3DVertexBuffer9::GetWrapperVertexBuffer9(IDirect3DVertexBuffer9* ptr) {
	WrapperDirect3DVertexBuffer9* ret = (WrapperDirect3DVertexBuffer9*)( m_list.GetDataPtr( ptr ) );
	#ifdef ENABLE_VERTEX_BUFFER_LOG
	if(ret == NULL) {
		infoRecorder->logTrace("WrapperDirect3DVertexBuffer9::GetWrapperVertexBuffer9(), ret is NULL\n");
	}
#endif
	return ret;
}

/*** IUnknown methods ***/
STDMETHODIMP WrapperDirect3DVertexBuffer9::QueryInterface(THIS_ REFIID riid, void** ppvObj) {
	#ifdef ENABLE_VERTEX_BUFFER_LOG
	infoRecorder->logTrace("WrapperDirect3DVertexBuffer9::QueryInterface() called, base_vb=%d, this=%d, ppvObj=%d\n", m_vb, this, *ppvObj);
#endif
	HRESULT hr = m_vb->QueryInterface(riid, ppvObj);
	*ppvObj = this;
	#ifdef ENABLE_VERTEX_BUFFER_LOG
	infoRecorder->logTrace("WrapperDirect3DVertexBuffer9::QueryInterface() called, base_vb=%d, this=%d, ppvObj=%d\n", m_vb, this, *ppvObj);
#endif
	return hr;
};
STDMETHODIMP_(ULONG) WrapperDirect3DVertexBuffer9::AddRef(THIS) {
	refCount++;
	return m_vb->AddRef();
}
STDMETHODIMP_(ULONG) WrapperDirect3DVertexBuffer9::Release(THIS) {
	ULONG hr = m_vb->Release();
#ifdef LOG_REF_COUNT
	#ifdef ENABLE_VERTEX_BUFFER_LOG
	infoRecorder->logTrace("WrapperDirect3DVertexBuffer9::Release(), ref:%d.\n", hr);
#endif
#endif
	refCount--;
	if(refCount <= 0){
		infoRecorder->logError("[WrapperDirect3DVertexBuffer9]: m_vb ref:%d, ref count:%d.\n", refCount, hr);
		m_list.DeleteMember(m_vb);
	}
	return hr;
}

/*** IDirect3DResource9 methods ***/
STDMETHODIMP WrapperDirect3DVertexBuffer9::GetDevice(THIS_ IDirect3DDevice9** ppDevice) {
	#ifdef ENABLE_VERTEX_BUFFER_LOG
	infoRecorder->logTrace("WrapperDirect3DVertexBuffer9::GetDevice() TODO\n");
#endif
	return m_vb->GetDevice(ppDevice);
}
STDMETHODIMP WrapperDirect3DVertexBuffer9::SetPrivateData(THIS_ REFGUID refguid,CONST void* pData,DWORD SizeOfData,DWORD Flags) {
	#ifdef ENABLE_VERTEX_BUFFER_LOG
	infoRecorder->logTrace("WrapperDirect3DVertexBuffer9::SetPrivateData() TODO\n");
#endif
	return m_vb->SetPrivateData(refguid, pData, SizeOfData, Flags);
}
STDMETHODIMP WrapperDirect3DVertexBuffer9::GetPrivateData(THIS_ REFGUID refguid,void* pData,DWORD* pSizeOfData) {
	return m_vb->GetPrivateData(refguid, pData, pSizeOfData);
}
STDMETHODIMP WrapperDirect3DVertexBuffer9::FreePrivateData(THIS_ REFGUID refguid) {
	return m_vb->FreePrivateData(refguid);
}
STDMETHODIMP_(DWORD) WrapperDirect3DVertexBuffer9::SetPriority(THIS_ DWORD PriorityNew) {
	#ifdef ENABLE_VERTEX_BUFFER_LOG
	infoRecorder->logTrace("WrapperDirect3DVertexBuffer9::SetPriority() TODO\n");
#endif
	return m_vb->SetPriority(PriorityNew);
}
STDMETHODIMP_(DWORD) WrapperDirect3DVertexBuffer9::GetPriority(THIS) {
	return m_vb->GetPriority();
}
STDMETHODIMP_(void) WrapperDirect3DVertexBuffer9::PreLoad(THIS) {
	#ifdef ENABLE_VERTEX_BUFFER_LOG
	infoRecorder->logTrace("WrapperDirect3DVertexBuffer9::PreLoad() TODO\n");
#endif
	return m_vb->PreLoad();
}
STDMETHODIMP_(D3DRESOURCETYPE) WrapperDirect3DVertexBuffer9::GetType(THIS) {
	return m_vb->GetType();
}

STDMETHODIMP WrapperDirect3DVertexBuffer9::Lock(THIS_ UINT OffsetToLock,UINT SizeToLock,void** ppbData,DWORD Flags) {
#ifdef ENABLE_VERTEX_BUFFER_LOG
	infoRecorder->logTrace("WrapperDirect3DVertexBuffer9::Lock(), id=%d, length=%d, offest=%d, size_to_lock=%d, flag=%d\n",this->id, Length, OffsetToLock, SizeToLock, Flags);
#endif
#ifndef BUFFER_UNLOCK_UPDATE
	void * tmp = NULL;
	HRESULT hr = m_vb->Lock(OffsetToLock, SizeToLock, &tmp, Flags);
	infoRecorder->logTrace("WrapperDirect3DVertexBuffer9::Lock() end\n");
	m_LockData.OffsetToLock = OffsetToLock;
	m_LockData.SizeToLock = SizeToLock;
	m_LockData.Flags = Flags;

	if(SizeToLock == 0) m_LockData.SizeToLock = Length - OffsetToLock;

	*ppbData = tmp;

#ifdef MULTI_CLIENTS
	// changed to all context, like 0xffff?
	csSet->setChangedToAll(updateFlag);
#endif
	readed_ = false;
#else   // BUFFER_UNLOCK_UPDATE

#ifndef USE_MEM_VERTEX_BUFFER
	
	void * tmp = NULL;
	// lock the whole buffer
	HRESULT hr = m_vb->Lock(0, 0, &tmp, Flags);
	m_LockData.OffsetToLock = OffsetToLock;
	m_LockData.SizeToLock = SizeToLock;
	m_LockData.Flags = Flags;
	if(SizeToLock == 0) m_LockData.SizeToLock = Length - OffsetToLock;

	*ppbData = (void *)(((char *)tmp) + OffsetToLock);
	m_LockData.pVideoBuffer = tmp;   // to the start of the entire buffer
#ifdef MULTI_CLIENTS
	csSet->setChangedToAll(updateFlag);
#endif // MULTI_CLIENTS
	readed_ = false;
	return hr;
#else  // USE_MEM_VERTEX_BUFFER
	if(!ram_buffer){
		ram_buffer = (char *)malloc(sizeof(char) *Length);
		m_LockData.pRAMBuffer = ram_buffer;
	}
	// store the lock information
	m_LockData.OffsetToLock = OffsetToLock;
	m_LockData.SizeToLock = SizeToLock;
	m_LockData.Flags = Flags;

	if(SizeToLock == 0) 
		m_LockData.SizeToLock = Length - OffsetToLock;

	*ppbData = (void*)(((char *)ram_buffer)+OffsetToLock);

	// lock the video mem as well
	HRESULT hr = m_vb->Lock(OffsetToLock, SizeToLock, &(m_LockData.pVideoBuffer), Flags);
#ifdef MULTI_CLIENTS
	csSet->setChangedToAll(updateFlag);
#endif // MULTI_CLIENTS
	return hr;

#endif  // USE_MEM_VERTEX_BUFFER
#endif // BUFFER_UNLOCK_UPDATE
	
}

STDMETHODIMP WrapperDirect3DVertexBuffer9::Unlock(THIS) {
#ifdef ENABLE_VERTEX_BUFFER_LOG
	infoRecorder->logTrace("WrapperDirect3DVertexBuffer9::Unlock(), id:%d, UnlockSize=%d Bytes, total len:%d, start:%d.\n", this->id,m_LockData.SizeToLock, Length, m_LockData.OffsetToLock);
#endif  //ENABLE_VERTEX_BUFFER_LOG

	// update the vertex buffer
#ifdef BUFFER_UNLOCK_UPDATE

	if(pTimer){
		pTimer->Start();
	}
	// the buffer is updated, read data to ram_buffer
	int last = 0, cnt = 0, c_len = 0, size = 0, base = 0;
#ifndef USE_MEM_VERTEX_BUFFER

	// copy from video buffer
	memcpy(ram_buffer, (char *)m_LockData.pVideoBuffer + m_LockData.OffsetToLock, m_LockData.SizeToLock);
#else   // USE_MEM_VERTEX_BUFFER
	// copy to video buffer
	memcpy(m_LockData.pVideoBuffer, (char *)m_LockData.pRAMBuffer + m_LockData.OffsetToLock, m_LockData.SizeToLock);


#endif  // USE_MEM_VERTEX_BUFFER
	base = m_LockData.OffsetToLock;

	csSet->checkCreation(dynamic_cast<IdentifierBase *>(this));

	csSet->beginCommand(VertexBufferUnlock_Opcode, id);
	csSet->writeUInt(m_LockData.OffsetToLock);
	csSet->writeUInt(m_LockData.SizeToLock);
	csSet->writeUInt(m_LockData.Flags);
	csSet->writeInt(CACHE_MODE_DIFF);

	for(unsigned int i = 0; i< m_LockData.SizeToLock; ++i){
		if(cache_buffer[base + i] ^ *((char *)(m_LockData.pRAMBuffer) + i) ){
			int d = i - last;
			csSet->writeInt(d);
			last = i;
			csSet->writeChar(*((char *)(m_LockData.pRAMBuffer) + i));
			cnt++;
			cache_buffer[base + i] = *((char *)(m_LockData.pRAMBuffer) + i);
		}
	}
	int neg = ( 1 << 28 ) - 1;
	csSet->writeInt(neg);

	c_len = csSet->getCommandLength();

	if(c_len > m_LockData.SizeToLock){
#ifdef ENABLE_VERTEX_BUFFER_LOG
		infoRecorder->logTrace("[WrapperDirect3DVertexBuffer9]: Unlock(), change too much, cancel command, id=%d.\n", id);
#endif
		c_len = m_LockData.SizeToLock;

		csSet->cancelCommand();
		csSet->beginCommand(VertexBufferUnlock_Opcode, id);
		csSet->writeUInt(m_LockData.OffsetToLock);
		csSet->writeUInt(m_LockData.SizeToLock);
		csSet->writeUInt(m_LockData.Flags);

		csSet->writeInt(CACHE_MODE_COPY);

		csSet->writeByteArr((char *)m_LockData.pRAMBuffer, c_len);
		csSet->endCommand();

		if(c_len > max_vb){
			max_vb = c_len;
#ifdef ENABLE_VERTEX_BUFFER_LOG
			infoRecorder->logTrace("[WrapperDirect3DVertexBuffer9]: max vb:%d.\n, id:%d.\n", c_len, id);
#endif
		}
	}
	else{
		if(cnt > 0){
#ifdef ENABLE_VERTEX_BUFFER_LOG
			infoRecorder->logTrace("[WrapperDirect3DVertexBuffer9]: unlock changed vertex count:%d, c_len:%d.\n", cnt, c_len);
#endif
			csSet->endCommand();
		}
		else{
#ifdef ENABLE_VERTEX_BUFFER_LOG
			infoRecorder->logTrace("[WrapperDirect3DVertexBuffer9]: unlock not changed, c_len:%d, cnt:%d.\n", c_len, cnt);
#endif
			csSet->cancelCommand();
		}
	}

	csSet->resetChanged(updateFlag);
	if(pTimer){
		unsigned int interval = pTimer->Stop();
		infoRecorder->logError("[WrapperDirect3DVertexBuffer]: unlock use time: %f.\n", interval * 1000.0 / pTimer->getFreq());
	}
#endif   // BUFFER_UNLOCK_UPDATE
	return m_vb->Unlock();
}
void WrapperDirect3DVertexBuffer9::read_data_from_buffer(char** ptr, int offest, int size) {
#ifdef ENABLE_VERTEX_BUFFER_LOG
	infoRecorder->logTrace("WrapperDirect3DVertexBuffer9::read_data_from_buffer() called\n");
#endif
	if(size == 0) size = Length - offest;
	if(!ram_buffer) {
		ram_buffer = (char*)(malloc(this->Length));
		m_LockData.pRAMBuffer = ram_buffer;
	}

	if(readed_) {
		*ptr = ram_buffer;
		return;
	}

	void* p = NULL;
	Lock(offest, size, &p, 2048);
	memcpy(ram_buffer, p, size);
	Unlock();

	readed_ = true;

	*ptr = ram_buffer;
}
void WrapperDirect3DVertexBuffer9::write_data_to_buffer(char* ptr, int offest, int size) {
	#ifdef ENABLE_VERTEX_BUFFER_LOG
	infoRecorder->logTrace("WrapperDirect3DVertexBuffer9::write_data_to_buffer() called\n");
#endif
	if(size == 0) size = Length - offest;
	void* p = NULL;
	Lock(offest, size, &p, 2048);
	memcpy((char*)p, ptr, size);
	Unlock();
}
STDMETHODIMP WrapperDirect3DVertexBuffer9::GetDesc(THIS_ D3DVERTEXBUFFER_DESC *pDesc) {
	//infoRecorder->logTrace("WrapperDirect3DVertexBuffer9::GetDesc() TODO\n");
	return m_vb->GetDesc(pDesc);
}
// prepare the vertex buffer with byte comparation
bool WrapperDirect3DVertexBuffer9::PrepareVertexBuffer(){
	#ifdef ENABLE_VERTEX_BUFFER_LOG
	infoRecorder->logTrace("[WrapperDirect3DVertexBuffer9]: prepare the vertex buffer.\n");
#endif
	
	if(isFirst){
		memset(cache_buffer, 0, Length);
		isFirst = false;
	}
	if(!csSet->isChanged(updateFlag)){
		#ifdef ENABLE_VERTEX_BUFFER_LOG
		infoRecorder->logTrace("[WrapperDirect3DVertexBuffer9]: not changed.\n");
#endif
		return 1;
	}
#ifdef ENABLE_VERTEX_BUFFER_LOG
	double tick_s = 0.0f, tick_e = 0.0f, tick_a = 0.0f;
	tick_s = GetTickCount();
#endif

	char * p = NULL;
	read_data_from_buffer(&p, 0, 0);

	int last = 0, cnt = 0, c_len = 0, size = 0;
	int base = m_LockData.OffsetToLock;
#ifdef ENABLE_VERTEX_BUFFER_LOG
	tick_e = GetTickCount();
#endif

	csSet->beginCommand(VertexBufferUnlock_Opcode, getId());
	csSet->writeUInt(m_LockData.OffsetToLock);
	csSet->writeUInt(m_LockData.SizeToLock);
	csSet->writeUInt(m_LockData.Flags);
	csSet->writeInt(CACHE_MODE_DIFF);

	for(UINT i = 0; i< m_LockData.SizeToLock; ++i){
		if(cache_buffer[base + i] ^ *((char *)(m_LockData.pRAMBuffer) + i) ){
			int d = i - last;
			csSet->writeInt(d);
			last = i;
			csSet->writeChar(*((char *)(m_LockData.pRAMBuffer) + i));
			cnt++;
			cache_buffer[base + i] = *((char *)(m_LockData.pRAMBuffer) + i);
		}
	}
	int neg = ( 1 << 28 ) - 1;
	csSet->writeInt(neg);
#ifdef ENABLE_VERTEX_BUFFER_LOG
	tick_a = GetTickCount();
#endif
	// get current command length
	c_len = csSet->getCommandLength();
	#ifdef ENABLE_VERTEX_BUFFER_LOG
	infoRecorder->logTrace("[WrapperDirect3DVertexBuffer9]: \tLock vb:%f, cache time:%f\n", tick_e - tick_s, tick_a - tick_s );
#endif

	if(c_len > m_LockData.SizeToLock){
		#ifdef ENABLE_VERTEX_BUFFER_LOG
		infoRecorder->logTrace("[WrapperDirect3DVertexBuffer9]: PrepareVertexBuffer(), change too much, cancel command, id=%d.\n", id);
#endif
		c_len = m_LockData.SizeToLock;
		
		csSet->cancelCommand();
		csSet->beginCommand(VertexBufferUnlock_Opcode, getId());
		csSet->writeUInt(m_LockData.OffsetToLock);
		csSet->writeUInt(m_LockData.SizeToLock);
		csSet->writeUInt(m_LockData.Flags);

		csSet->writeInt(CACHE_MODE_COPY);
		
		csSet->writeByteArr((char *)m_LockData.pRAMBuffer, c_len);
		csSet->endCommand();

		if(c_len > max_vb){
			max_vb = c_len;
			#ifdef ENABLE_VERTEX_BUFFER_LOG
			infoRecorder->logTrace("[WrapperDirect3DVertexBuffer9]: max vb:%d.\n, id:%d.\n", c_len, id);
#endif
		}
	}
	else{
		if(cnt > 0){
			#ifdef ENABLE_VERTEX_BUFFER_LOG
			infoRecorder->logTrace("[WrapperDirect3DVertexBuffer9]: PrepareVertex buffer, changed vertex count:%d.\n", cnt);
#endif
			csSet->endCommand();
		}
		else{
			csSet->cancelCommand();
		}
	}
	csSet->setChanged(updateFlag);
	return (cnt > 0);
}


bool WrapperDirect3DVertexBuffer9::PrepareVertexBuffer(ContextAndCache * ctx){
	#ifdef ENABLE_VERTEX_BUFFER_LOG
	infoRecorder->logTrace("[WrapperDirect3DVertexBuffer9]: prepare the vertex buffer.\n");
#endif
	if(isFirst){
		memset(cache_buffer, 0, Length);
		isFirst = false;
	}
	if(!ctx->isChanged(updateFlag)){
		#ifdef ENABLE_VERTEX_BUFFER_LOG
		infoRecorder->logTrace("[WrapperDirect3DVertexBuffer9]: not changed.\n");
#endif
		return 1;
	}
#ifdef ENABLE_VERTEX_BUFFER_LOG
	double tick_s = 0.0f, tick_e = 0.0f, tick_a = 0.0f;
	tick_s = GetTickCount();
#endif

	char * p = NULL;
	read_data_from_buffer(&p, 0, 0);

	int last = 0, cnt = 0, c_len = 0, size = 0;
	int base = m_LockData.OffsetToLock;
	#ifdef ENABLE_VERTEX_BUFFER_LOG
	tick_e = GetTickCount();
#endif

	ctx->beginCommand(VertexBufferUnlock_Opcode, getId());
	ctx->write_uint(m_LockData.OffsetToLock);
	ctx->write_uint(m_LockData.SizeToLock);
	ctx->write_uint(m_LockData.Flags);
	ctx->write_int(CACHE_MODE_DIFF);

	for(int i = 0; i< m_LockData.SizeToLock; ++i){
		if(cache_buffer[base + i] ^ *((char *)(m_LockData.pRAMBuffer) + i) ){
			int d = i - last;
			last = i;
			ctx->write_int(d);
			ctx->write_char(*((char *)(m_LockData.pRAMBuffer) + i));
			cnt++;
			cache_buffer[base + i] = *((char *)(m_LockData.pRAMBuffer) + i);
		}
	}
	int neg = ( 1 << 28 ) - 1;
	
	ctx->write_int(neg);
#ifdef ENABLE_VERTEX_BUFFER_LOG
	tick_a = GetTickCount();
#endif
	// get current command length
	c_len = ctx->getCommandLength();
	//c_len = csSet->getCommandLength();
	#ifdef ENABLE_VERTEX_BUFFER_LOG
	infoRecorder->logTrace("[WrapperDirect3DVertexBuffer9]: \tLock vb:%f, cache time:%f\n", tick_e - tick_s, tick_a - tick_s );
#endif

	if(c_len > m_LockData.SizeToLock){
		#ifdef ENABLE_VERTEX_BUFFER_LOG
		infoRecorder->logTrace("[WrapperDirect3DVertexBuffer9]: PrepareVertexBuffer(), change too much, cancel command, id=%d.\n", id);
#endif
		c_len = m_LockData.SizeToLock;

		ctx->cancelCommand();
		ctx->beginCommand(VertexBufferUnlock_Opcode, getId());
		ctx->write_uint(m_LockData.OffsetToLock);
		ctx->write_uint(m_LockData.SizeToLock);
		ctx->write_uint(m_LockData.Flags);

		ctx->write_int(CACHE_MODE_COPY);
		ctx->write_byte_arr((char *)m_LockData.pRAMBuffer, c_len);
		ctx->endCommand();

		if(c_len > max_vb){
			max_vb = c_len;
			#ifdef ENABLE_VERTEX_BUFFER_LOG
			infoRecorder->logTrace("[WrapperDirect3DVertexBuffer9]: max vb:%d.\n, id:%d.\n", c_len, id);
#endif
		}
	}
	else{
		if(cnt > 0){
			#ifdef ENABLE_VERTEX_BUFFER_LOG
			infoRecorder->logTrace("[WrapperDirect3DVertexBuffer9]: PrepareVertexBuffer(ctx), changed vertex count:%d.\n", cnt);
#endif
			ctx->endCommand();
			//csSet->endCommand();
		}
		else{
			//csSet->cancelCommand();
			ctx->cancelCommand();
		}
	}
	
	return (cnt > 0);
}