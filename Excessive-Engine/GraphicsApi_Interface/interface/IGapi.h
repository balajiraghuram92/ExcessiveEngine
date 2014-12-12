﻿#pragma once

#include "mymath/mymath.h"

#include "Common.h"
#include "ITexture.h"
#include "IBuffer.h"
#include "IShaderProgram.h"
#include "IInputLayout.h"


class IGapi
{
  public:
	virtual IShaderProgram* createShaderProgram(const rShaderPaths& data) = 0;
	virtual IShaderProgram* createShaderProgram(const rShaderSources& data) = 0;

	virtual IUniformBuffer* createUniformBuffer(const rBuffer& data) = 0;
	virtual IVertexBuffer*	createVertexBuffer(const rBuffer& data) = 0;
	virtual IIndexBuffer*	createIndexBuffer(const rBuffer& data) = 0;
	virtual ITexture*		createTexture(const rTexture& data) = 0;

	// TODO Uram atyám teremtőm ezt minél hamarabb cseréljük le wchar_t* - ra
	virtual ITexture*		createTexture(const char* path) = 0;
	
	virtual void WriteTexture(ITexture* t, const rTextureUpdate& d) = 0;
  
    virtual void setDepthState(const rDepthState& state) = 0;
    virtual void setStencilState(const rStencilState& state) = 0;
    virtual void setBlendState(const rBlendState& state) = 0;
	virtual void setRasterizationState(const rRasterizerState& state) = 0;
	virtual void setSamplerState(const char* slotName, const rSamplerState& smpdata, ITexture* t) = 0;

    virtual void setSRGBWrites(bool val) = 0;
    virtual void setSeamlessCubeMaps(bool val) = 0;
    
    virtual void setViewport(int x, int y, u32 w, u32 h) = 0;    

    virtual bool getError() = 0;
    virtual void setDebugOutput(bool val) = 0;
    virtual void setSyncDebugOutput(bool val) = 0;

    //pass input/output to shader
    virtual void setShaderProgram(IShaderProgram* sp) = 0;
    virtual void setTexture(ITexture* t, u32 idx) = 0;
	virtual void setRenderTargets(const rRenderTargetInfo* render_targets, u32 size) = 0;
    virtual void setUniformBuffer(IUniformBuffer* buf, u32 idx) = 0;
	virtual void setVertexBuffers(IVertexBuffer** buffers, const rVertexAttrib* attrib_data, u32 num_buffers) = 0;
    virtual void setIndexBuffer(IIndexBuffer* ibo) = 0;
    
    //draw stuff
    virtual void draw(u32 num_indices) = 0;

	// input layout & vertex streams
#pragma message("Marci: ezt is implementálnod kéne [setVertexBuffer]")

	/*
	@see IInputLayout.h	
	*/
	virtual IInputLayout* createInputLayout(InputElement* elements, size_t num_elements) = 0;
	virtual void setInputLayout(IInputLayout* layout) = 0;

	/*
	Egyszerûen bindeli a vertex buffereket a megfelelõ 'slot'-ba.
	Ha több buffert adunk meg, akkor azokat a start_slot, start_slot+1, start_slot+2 helyekre bindeli.
	A nullptr buffer unbindeli az adott slotból a buffert.
	*/
	virtual void setVertexStreams(
		IVertexBuffer** buffers, // buffers to bind
		u32* strides, // size of one vertex in bytes; for each buffer
		u32* offsets, // how many bytes the 0th vertex is offseted from start of buffer
		u32 start_slot, // bind 1st buffer here
		u32 num_buffers) = 0; 
};