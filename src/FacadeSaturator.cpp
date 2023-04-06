#include "FacadeSaturator.h"
#include "IPlug_include_in_plug_src.h"
#include "IControls.h"

#include <iostream>
#include <fstream>

//const char* PARAM_NAMES[] = {  };


FacadeSaturator::FacadeSaturator(const InstanceInfo& info)
: Plugin(info, MakeConfig(kNumParams, kNumPrograms))
{
}

#if IPLUG_DSP
void FacadeSaturator::OnActivate(bool enable){ // When the DSP turns on?
	
}

void FacadeSaturator::ProcessBlock(sample** inputs, sample** outputs, int nFrames)
{
	const int nChans = NOutChansConnected();
	
	//Copy values to oscilloscope
	if(mWindow){
		for(int c=0; c<2 && c<nChans; c++){
			oscilloscopeBuffer[c].Add((float*) outputs[c],nFrames);
		}
	}
}
#endif



// UI INTERFACE

#if IPLUG_EDITOR // http://bit.ly/2S64BDd

void* FacadeSaturator::OpenWindow(void* pParent){
	
	void* ret_ptr = nullptr;
	
	if(!mWindow){
		mUI = std::unique_ptr<TestUI>(new TestUI((IEditorDelegate*) this,oscilloscopeBuffer,NOutChansConnected()>2 ? 2 : NOutChansConnected() ));
		mWindow = std::unique_ptr<GL3PluginWindow>(new GL3PluginWindow(PLUG_WIDTH,PLUG_HEIGHT,PLUG_FPS,1.f, //Fix the HWND when porting
		{4,1,-1,false,false,false,false,false,false},
		(GL3PluginUI*) mUI.get())); 
		if(mLastWidth && mLastHeight && mLastScale){
			mWindow->Resize(mLastWidth, mLastHeight, mLastScale);
		}
	}
	
	if(mWindow) ret_ptr = mWindow->OpenWindow(pParent);
	
	return ret_ptr;
}

void FacadeSaturator::CloseWindow(){
	IEditorDelegate::CloseWindow(); //necessary??
	
	mWindow->CloseWindow();
	mWindow = nullptr;
	mUI = nullptr;
}

void FacadeSaturator::OnParamChangeUI(int paramIdx, EParamSource source){

}

#endif