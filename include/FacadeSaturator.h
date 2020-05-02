#pragma once

#include "IPlug_include_in_plug_hdr.h"

#include "GL3PluginWindow.h"
#include "TestUI.h"

const int kNumPrograms = 1;

const int kNumPoints = 16;

enum EParams
{
  kGain = 0,
  kNumParams
};

using namespace iplug;

class FacadeSaturator : public Plugin
{
public:
  FacadeSaturator(const InstanceInfo& info);
  
#if IPLUG_EDITOR
  void* OpenWindow(void* pParent);
  void CloseWindow();
  void OnParamChangeUI(int paramIdx, EParamSource source = kUnknown) override;
#endif

#if IPLUG_DSP // http://bit.ly/2S64BDd
  void ProcessBlock(sample** inputs, sample** outputs, int nFrames) override;
#endif

private:

	//Graphics Window Handling
	std::unique_ptr<GL3PluginWindow> mWindow;
	std::unique_ptr<GL3PluginUI> mUI;
	int mLastWidth = 0;
	int mLastHeight = 0;
	float mLastScale = 0.f;
	bool mClosing = false;
	
	
};
