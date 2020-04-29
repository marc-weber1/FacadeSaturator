#include "FacadeSaturator.h"
#include "IPlug_include_in_plug_src.h"
#include "IControls.h"


FacadeSaturator::FacadeSaturator(const InstanceInfo& info)
: Plugin(info, MakeConfig(kNumParams, kNumPrograms))
{
  GetParam(kGain)->InitDouble("Gain", 0., 0., 100.0, 0.01, "%");
}

#if IPLUG_DSP
void FacadeSaturator::ProcessBlock(sample** inputs, sample** outputs, int nFrames)
{
  const double gain = GetParam(kGain)->Value() / 100.;
  const int nChans = NOutChansConnected();
  
  for (int s = 0; s < nFrames; s++) {
    for (int c = 0; c < nChans; c++) {
      outputs[c][s] = inputs[c][s] * gain;
    }
  }
}
#endif


// UI INTERFACE

#if IPLUG_EDITOR // http://bit.ly/2S64BDd
void* FacadeSaturator::OpenWindow(void* pParent){
	
	if(!mWindow){
		mUI = std::unique_ptr<GL3PluginUI>(new TestUI(kGain));
		mWindow = std::unique_ptr<GL3PluginWindow>(new GL3PluginWindow((HWND) pParent,PLUG_WIDTH,PLUG_HEIGHT,PLUG_FPS,1.f, //Fix the HWND when porting
		{4,1,-1,false,false,false,false,false,false},
		mUI)); 
		if(mLastWidth && mLastHeight && mLastScale){
			mWindow->Resize(mLastWidth, mLastHeight, mLastScale);
		}
	}
	
	if(mWindow) return mWindow->OpenWindow(pParent);
	else return nullptr;
	return nullptr;
}

void FacadeSaturator::CloseWindow(){
	IEditorDelegate::CloseWindow(); //necessary??
	
	mWindow->CloseWindow();
	mWindow = nullptr;
	mUI = nullptr;
}
#endif